import logging
import sys
import datetime
import re
from pydantic import BaseModel
from collections import defaultdict
from typing import List
from php_bytecode_api import convert_and_lift, helper_functions, models

rootdir = "/app/targets"

TOTAL_LOW_VULNS_FOUND = 0
TOTAL_HIGH_VULNS_FOUND = 0

NOW_TS = int(datetime.datetime.now().timestamp())
LOG_FILENAME = f"{'__'.join(rootdir.split('/'))}_{NOW_TS}.log"

file_handler = logging.FileHandler(filename=f"reports/{LOG_FILENAME}")
stdout_handler = logging.StreamHandler(stream=sys.stdout)
handlers = [file_handler, stdout_handler]

logging.basicConfig(
    level=logging.INFO,
    # format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s - %(message)s',
    format="%(message)s",
    handlers=handlers,
)

VULNS_IDS_TRACKER = defaultdict(lambda: [])
VULNS = []


class StateModel(BaseModel):
    visited_instructions_index: List[int] = []
    has_sql_string: bool = False
    visited_function_calls: List[str] = []


def _is_sql_string(data):
    SIMPLE_SQL_RE = re.compile(
        r"(select\s.*from\s|"
        r"delete\s+from\s|"
        r"insert\s+into\s.*values\s|"
        r"update\s.*set\s)",
        re.IGNORECASE | re.DOTALL,
    )
    return SIMPLE_SQL_RE.search(data) is not None


def save_vuln(
    impact: str,
    function: models.FunctionModel,
    has_sql_string: bool,
    visited_instructions_index: List[int],
    visited_function_calls: List[str],
):
    vuln_file_fn = f"{function.filename}:{function.class_name}:{function.function_name}"
    vuln_key = "::".join([str(i) for i in visited_instructions_index])

    if vuln_key in VULNS_IDS_TRACKER[vuln_file_fn]:
        return

    VULNS_IDS_TRACKER[vuln_file_fn].append(vuln_key)

    VULN_INFO = {
        "impact": impact,
        "vuln_name": "sql_injection" if has_sql_string else "code_injection",
        "visited_function_calls": ",".join(visited_function_calls),
        "vuln_file": vuln_file_fn,
        "vuln_key": vuln_key,
        "lines": [],
    }

    visited_lines = []
    for instruction_index in visited_instructions_index[::-1]:
        lineno = function.instructions[instruction_index].lineno
        if lineno not in visited_lines:
            line = (
                f"[*] lineno={lineno}"
                f" ------ {function.instructions[instruction_index].source_line.strip()}"  # noqa: E501
            )
            visited_lines.append(lineno)
            VULN_INFO["lines"].append(line)

    vuln_key = ":_:".join([str(i) for i in visited_lines])
    if vuln_key in VULNS_IDS_TRACKER[vuln_file_fn]:
        return

    global TOTAL_HIGH_VULNS_FOUND
    global TOTAL_LOW_VULNS_FOUND

    if impact == "HIGH":
        TOTAL_HIGH_VULNS_FOUND = TOTAL_HIGH_VULNS_FOUND + 1

    if impact == "LOW":
        TOTAL_LOW_VULNS_FOUND = TOTAL_LOW_VULNS_FOUND + 1

    VULNS_IDS_TRACKER[vuln_file_fn].append(vuln_key)
    VULNS.append(VULN_INFO)


def check_vuln(
    function: models.FunctionModel, state: StateModel, instruction_index: int
):
    instruction = function.instructions[instruction_index]

    if instruction.is_global_fetch_instruction:
        prev_instruction_index = state.visited_instructions_index[
            len(state.visited_instructions_index) - 1
        ]
        prev_instruction = function.instructions[prev_instruction_index]

        if prev_instruction.opcode_name.startswith("ZEND_FETCH"):
            if (
                prev_instruction.op1.type == "IS_CONST"
                and prev_instruction.op1.value
                in ["_POST", "_GET", "_COOKIE", "_REQUEST", "_FILES"]
            ):  # _SESSION
                save_vuln(
                    "HIGH",
                    function,
                    state.has_sql_string,
                    state.visited_instructions_index,
                    state.visited_function_calls,
                )
                return True

    if (
        instruction.opcode_name in ["ZEND_RECV", "ZEND_RECV_INIT", "ZEND_RECV_VARIADIC"]
        and state.has_sql_string
    ):
        save_vuln(
            "LOW",
            function,
            state.has_sql_string,
            state.visited_instructions_index,
            state.visited_function_calls,
        )
        return True

    return False


def trace_instruction_variable_to_global_variable(
    function: models.FunctionModel, state: StateModel, instruction_index: int
):
    new_state = state.copy()

    if instruction_index in new_state.visited_instructions_index:
        return

    if instruction_index not in new_state.visited_instructions_index:
        new_state.visited_instructions_index.append(instruction_index)

    insn = function.instructions[instruction_index]

    if check_vuln(function, new_state, instruction_index):
        return

    # for function calls, add results
    if insn.is_function_call_instruction:
        visited_fn = helper_functions.get_recent_function_call_from_instruction_index(
            function, instruction_index
        )
        if visited_fn not in new_state.visited_function_calls:
            new_state.visited_function_calls.append(visited_fn)

    for arg in helper_functions.args_without_result(insn):
        new_state = state.copy()
        if arg.type == "IS_CONST" and _is_sql_string(arg.value.lower()):
            new_state.has_sql_string = True
            continue

        if arg.variable_number is None:
            continue

        # :
        # if its not a function call, variable must be used in only current instruction
        if (
            not insn.is_function_call_instruction
            and arg.variable_number not in insn.variables_used_here
        ):
            continue

        var_definition_at_instruction_index = (
            helper_functions.get_instruction_index_where_variable_number_is_defined(
                function=function, variable_number=arg.variable_number
            )
        )
        if var_definition_at_instruction_index > -1:
            trace_instruction_variable_to_global_variable(
                function=function,
                state=new_state,
                instruction_index=var_definition_at_instruction_index,
            )

        var_definition_phi_at_instructions_index = helper_functions.get_instructions_index_where_variable_number_is_defined_phi(  # noqa: E501
            function=function, variable_number=arg.variable_number
        )
        if var_definition_phi_at_instructions_index:
            for (
                var_definition_phi_at_instruction_index
            ) in var_definition_phi_at_instructions_index:
                trace_instruction_variable_to_global_variable(
                    function=function,
                    state=new_state,
                    instruction_index=var_definition_phi_at_instruction_index,
                )


def analyze_function(function: models.FunctionModel):
    potential_vuln_opcode_names = [
        "ZEND_ECHO",
        "ZEND_ROPE_INIT",
        "ZEND_ROPE_ADD",
        "ZEND_ROPE_END",
        "ZEND_CONCAT",
        "ZEND_FAST_CONCAT",
        "ZEND_INCLUDE_OR_EVAL",
        "ZEND_EVAL",
        "ZEND_INCLUDE",
        "ZEND_INCLUDE_ONCE",
        "ZEND_REQUIRE",
        "ZEND_REQUIRE_ONCE",
    ]

    for instruction_index, instruction in enumerate(function.instructions):
        has_sql_string = False

        if instruction.opcode_name not in potential_vuln_opcode_names:
            continue

        if instruction.opcode_name in ["ZEND_ROPE_ADD", "ZEND_ROPE_END"]:
            """
            check if it's predecessor instructions (ZEND_ROPE_INIT, ZEND_ROPE_ADD)
            is an SQL query string
            """
            current_instruction_index = instruction_index - 1
            while True:
                if current_instruction_index < 1:
                    break

                prev_insn = function.instructions[current_instruction_index]
                if prev_insn not in ["ZEND_ROPE_ADD", "ZEND_ROPE_INIT"]:
                    break

                for arg in helper_functions.args_without_result(prev_insn):
                    if arg.type == "IS_CONST" and _is_sql_string(arg.value.lower()):
                        has_sql_string = True

                current_instruction_index -= 1

        for arg in helper_functions.args_without_result(instruction):
            state = StateModel(has_sql_string=has_sql_string)

            if arg.type == "IS_CONST" and _is_sql_string(arg.value.lower()):
                state.has_sql_string = True
                continue

            if arg.variable_number is None:
                continue

            if (
                arg.variable_number
                not in helper_functions.variables_used_at_instruction(
                    function, instruction_index
                )
            ):
                continue

            var_definition_at_instruction_index = (
                helper_functions.get_instruction_index_where_variable_number_is_defined(
                    function=function, variable_number=arg.variable_number
                )
            )

            if var_definition_at_instruction_index > -1:
                if instruction_index not in state.visited_instructions_index:
                    state.visited_instructions_index.append(instruction_index)

                trace_instruction_variable_to_global_variable(
                    function=function,
                    state=state,
                    instruction_index=var_definition_at_instruction_index,
                )


def main():
    for file_path in helper_functions.recursively_get_directory_php_files(rootdir):
        functions = convert_and_lift(file_path)
        if not functions:
            logging.error(f"Loading ... {file_path} -- No Functions Found")
        else:
            for function in functions:
                print(
                    f"Analyzing function ... {file_path}:{function.class_name}:{function.function_name}"  # noqa: E501
                )
                analyze_function(function)

    def log_total_vulns():
        logging.info(f"[*] {'-'*100}")
        logging.info(f"[*] {'-'*100}")
        logging.info(f"[*] TOTAL_VULNS = {TOTAL_HIGH_VULNS_FOUND}")
        logging.info("[*]")

    log_total_vulns()

    for index, vuln in enumerate(VULNS):
        logging.info(f"[*] {'-'*100}")
        logging.info(f"[*] {'-'*100}")
        logging.info(f"[*] INDEX = {index+1}")
        logging.info(f"[*] IMPACT = {vuln['impact']}")
        logging.info(f"[*] VULNERABILTY TYPE = {vuln['vuln_name']}")
        logging.info(f"[*] VISITED FUNCTION CALLS = {vuln['visited_function_calls']}")
        logging.info(f"[*] FILE = {vuln['vuln_file']}")
        logging.info("[*] ")
        logging.info("[*] --- SOURCE PATH TO VULNERABILTY ---")

        for line in vuln["lines"]:
            logging.info(line)

        logging.info("[*]")
        logging.info("[*]")

    log_total_vulns()


if __name__ == "__main__":
    main()
