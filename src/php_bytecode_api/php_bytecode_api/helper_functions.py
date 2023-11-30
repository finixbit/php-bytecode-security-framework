import os

FUNCTION_CALL_OPCODE_NAMES = ["ZEND_DO_FCALL_BY_NAME", "ZEND_DO_ICALL", "ZEND_DO_FCALL"]


def recursively_get_directory_php_files(rootdir):
    for subdir, dirs, files in os.walk(rootdir):
        for file in files:
            file_path = os.path.join(subdir, file)
            if not file_path.endswith(".php"):
                continue
            yield file_path


def args(instruction, filter_operand_type=None):
    results = []
    for operand in [instruction.op1, instruction.op2, instruction.result]:
        if filter_operand_type and operand.type not in filter_operand_type:
            continue
        results.append(operand)
    return results


def args_without_result(instruction, filter_operand_type=None):
    results = []
    for operand in [instruction.op1, instruction.op2]:
        if filter_operand_type and operand.type not in filter_operand_type:
            continue
        results.append(operand)
    return results


def variables_used_at_function_call_args(function, instruction_index):
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


def variables_used_at_instruction(
    function, instruction_index, is_function_call_instruction=False
):
    variables = []
    if is_function_call_instruction:
        variables = variables_used_at_function_call_args(function, instruction_index)
    else:
        for var in function.ssa.ssa_variables:
            # use_chain is an instruction' index
            if var.use_chain >= 0:
                if var.use_chain == instruction_index:
                    variables.append(var.var_num)
    return variables


def get_instruction_index_where_variable_number_is_defined(function, variable_number):
    instruction_index = -1
    for var in function.ssa.ssa_variables:
        # definition is an instruction' index
        if var.definition >= 0:
            if var.var_num == variable_number:
                instruction_index = var.definition
    return instruction_index


def get_recent_function_call_from_instruction_index(function, instruction_index):
    # https://php-legacy-docs.zend.com/manual/php5/en/internals2.opcodes.init-fcall-by-name

    fcall_name = "UNDEFINED - UNABLE TO FETCH FCALL NAME"
    current_instruction_index = instruction_index
    while current_instruction_index > -1:
        insn = function.instructions[current_instruction_index]
        if insn.opcode_name in [
            "ZEND_INIT_FCALL_BY_NAME",
            "ZEND_INIT_FCALL",
            "ZEND_INIT_STATIC_METHOD_CALL",
        ]:
            fcall_name = insn.op2.value
            break

        current_instruction_index = current_instruction_index - 1
    return fcall_name


def get_instructions_index_where_variable_number_is_defined_phi(
    function, variable_number
):
    definition_phi_sources = []
    for var in function.ssa.ssa_variables:
        if var.var_num == variable_number:
            definition_phi_sources = var.definition_phi.sources

    instructions_index = []
    if definition_phi_sources:
        for source in definition_phi_sources:
            for var in function.ssa.ssa_variables:
                if var.ssa_var_num == source:
                    instructions_index.append(var.definition)
    return instructions_index
