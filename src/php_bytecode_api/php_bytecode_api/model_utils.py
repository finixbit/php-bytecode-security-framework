from php_bytecode_api.helper_functions import args, FUNCTION_CALL_OPCODE_NAMES


BRANCH_OPCODE_NAMES = [
    "ZEND_RECV",
    "ZEND_RECV_INIT" "ZEND_RETURN",
    "ZEND_RETURN_BY_REF",
    "ZEND_GENERATOR_RETURN",
    "ZEND_EXIT",
    "ZEND_THROW" "ZEND_INCLUDE_OR_EVAL",
    "ZEND_GENERATOR_CREATE",
    "ZEND_YIELD",
    "ZEND_YIELD_FROM",
    "ZEND_DO_FCALL",
    "ZEND_DO_UCALL",
    "ZEND_DO_FCALL_BY_NAME",
    "ZEND_FAST_CALL",
    "ZEND_FAST_RET",
    "ZEND_JMP",
    "ZEND_JMPZNZ",
    "ZEND_JMPZ",
    "ZEND_JMPNZ",
    "ZEND_JMPZ_EX",
    "ZEND_JMPNZ_EX",
    "ZEND_JMP_SET",
    "ZEND_COALESCE",
    "ZEND_ASSERT_CHECK",
    "ZEND_CATCH",
    "ZEND_DECLARE_ANON_CLASS",
    "ZEND_DECLARE_ANON_INHERITED_CLASS",
    "ZEND_FE_FETCH_R",
    "ZEND_FE_FETCH_RW",
    "ZEND_FE_RESET_R",
    "ZEND_FE_RESET_RW",
    "ZEND_SWITCH_LONG",
    "ZEND_SWITCH_STRING",
]


def _is_branch_instruction(opcode_name: str):
    if opcode_name in BRANCH_OPCODE_NAMES:
        return True
    return False


def _instruction_is_global_fetch(opcode_flags, extended_value):
    ZEND_VM_EXT_VAR_FETCH = 0x00010000

    # global/local fetches
    ZEND_FETCH_GLOBAL = 1 << 1
    ZEND_FETCH_LOCAL = 1 << 2
    ZEND_FETCH_GLOBAL_LOCK = 1 << 3

    if ZEND_VM_EXT_VAR_FETCH & opcode_flags:
        # (global)
        if extended_value & ZEND_FETCH_GLOBAL:
            return True

        # (local)
        if extended_value & ZEND_FETCH_LOCAL:
            return True

        # (global+lock)
        if extended_value & ZEND_FETCH_GLOBAL_LOCK:
            return True
    return False


def _instruction_is_function_call(opcode_name):
    if opcode_name in FUNCTION_CALL_OPCODE_NAMES:
        return True
    return False


def _variables_used_at_function_call_args(function, instruction_index):
    instruction = function.instructions[instruction_index]
    variables = []
    instructions_to_extract = []

    if instruction.opcode_name in FUNCTION_CALL_OPCODE_NAMES:
        current_instruction_index = instruction_index
        while current_instruction_index >= 0:
            insn = function.instructions[current_instruction_index]
            # print("[***]", insn.opcode_name)

            # break if function start opcode is reached
            if insn.opcode_name in ["ZEND_INIT_FCALL_BY_NAME", "ZEND_INIT_FCALL"]:
                break

            # retrieve instruction if function argument is sent to call
            if insn.opcode_name in [
                "ZEND_SEND_VAL",
                "ZEND_SEND_FUNC_ARG",
                "ZEND_SEND_VAR_EX",
                "ZEND_SEND_VAR",
            ]:
                # print("[yes]", insn.opcode_name)
                instructions_to_extract.append(insn)

            current_instruction_index = current_instruction_index - 1

    for insn in instructions_to_extract:
        for arg in args(insn):
            if arg.variable_number is not None and arg.variable_number >= 0:
                if arg.type != "IS_UNUSED" and arg.variable_number not in variables:
                    variables.append(arg.variable_number)
    return variables


def _function_call_args(function, instruction_index):
    instruction = function.instructions[instruction_index]
    instructions_to_extract = []

    if instruction.opcode_name in FUNCTION_CALL_OPCODE_NAMES:
        current_instruction_index = instruction_index
        while current_instruction_index >= 0:
            insn = function.instructions[current_instruction_index]
            # print("[***]", insn.opcode_name)

            # break if function start opcode is reached
            if insn.opcode_name in ["ZEND_INIT_FCALL_BY_NAME", "ZEND_INIT_FCALL"]:
                break

            # retrieve instruction if function argument is sent to call
            if insn.opcode_name in [
                "ZEND_SEND_VAL",
                "ZEND_SEND_FUNC_ARG",
                "ZEND_SEND_VAR_EX",
                "ZEND_SEND_VAR",
            ]:
                # print("[yes]", insn.opcode_name)
                instructions_to_extract.append(insn)

            current_instruction_index = current_instruction_index - 1

    function_args = []
    for insn in instructions_to_extract:
        for arg in args(insn):
            if (
                arg.variable_number is not None
                and arg.variable_number >= 0
                and arg.type != "IS_UNUSED"
            ):
                function_args.append(arg)

    return function_args


def _variables_used_at_instruction(
    function, instruction_index, is_function_call_instruction=False
):
    variables = []
    if is_function_call_instruction:
        variables = _variables_used_at_function_call_args(function, instruction_index)
    else:
        for var in function.ssa.ssa_variables:
            # use_chain is an instruction' index
            if var.use_chain >= 0:
                if var.use_chain == instruction_index:
                    variables.append(var.var_num)
    return variables


def _ssa_variables_used_at_instruction(function, instruction_index):
    ssa_variables = []
    for var in function.ssa.ssa_variables:
        # use_chain is an instruction' index
        if var.use_chain >= 0:
            if var.use_chain == instruction_index:
                ssa_variables.append(var.ssa_var_num)
    return ssa_variables


def _get_block_instructions(block):
    instructions_start_index = block.start
    instructions_end_index = block.start + block.len
    block_instructions = list(range(instructions_start_index, instructions_end_index))
    return block_instructions
