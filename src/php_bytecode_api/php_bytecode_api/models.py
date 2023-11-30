from pydantic import BaseModel
from typing import List, Optional, Union
from php_bytecode_api import model_utils


class LiteralModel(BaseModel):
    type: str
    value: str


class ExtraModel(BaseModel):
    arg_flags: Optional[List[int]]
    cache_size: int
    last_literal: int
    last_live_range: int
    last_try_catch: int
    number_of_cv_variables: int
    number_of_tmp_variables: int
    type: Optional[int]
    literals: List[LiteralModel]


class OperandModel(BaseModel):
    type: Optional[str] = "IS_UNUSED"
    value: Union[str, None]
    variable_number: Optional[int] = 0


class VariableModel(BaseModel):
    var_name: Union[str, None]
    var_num: int


class DFGBasicBlockModel(BaseModel):
    block_index: int
    var_def: List[VariableModel]
    var_in: List[VariableModel]
    var_out: List[VariableModel]
    var_tmp: List[VariableModel]
    var_use: List[VariableModel]


class SSAPhiConstraintModel(BaseModel):
    range_range_min: Optional[int] = 0
    range_range_max: Optional[int] = 0
    range_range_underflow: Optional[int] = 0
    range_range_overflow: Optional[int] = 0
    range_min_var: Optional[int] = 0
    range_max_var: Optional[int] = 0
    range_min_ssa_var: Optional[int] = 0
    range_max_ssa_var: Optional[int] = 0


class SSAPhiModel(BaseModel):
    constraint: Optional[SSAPhiConstraintModel]
    current_block_index: Optional[int]
    has_range_constraint: Optional[int]
    pi: Optional[int]
    sources: Optional[List[int]] = []
    ssa_variable_index: Optional[int]
    variable: Optional[VariableModel]
    variable_index: Optional[int]
    visited: Optional[int]


class SSABlockModel(BaseModel):
    block_index: int
    phis: List[SSAPhiModel]


class SSAInstructionModel(BaseModel):
    op1_def: int
    op1_use: int
    op1_use_chain: int
    op2_def: int
    op2_use: int
    op2_use_chain: int
    result_def: int
    result_use: int
    res_use_chain: int


class SSAVariableModel(BaseModel):
    definition: int
    definition_phi: SSAPhiModel
    escape_state: int
    no_val: int
    ssa_var_num: int
    strongly_connected_component: int
    strongly_connected_component_entry: int
    use_chain: int
    var_name: str
    var_num: int


class SSAModel(BaseModel):
    number_of_sccs: int
    number_of_ssa_variables: int
    ssa_variables: List[SSAVariableModel]
    ssa_instructions: List[SSAInstructionModel]
    ssa_blocks: List[SSABlockModel]


class CFGBasicBlockModel(BaseModel):
    """
    default props
    """

    children: int
    idom: int
    len: int
    level: int
    loop_header: int
    next_child: int
    predecessor_offset: int
    predecessors_count: int
    start: int
    successors: List[int]
    successors_count: int
    successors_storage: List[int]

    """
    additional generated/updated props
    """
    block_instructions: List[int] = []
    block_edges: List[int] = []

    def source_lines(self, fn):
        source_lines = []
        for instruction_index in self.block_instructions:
            instruction = fn.updated_instructions[instruction_index]
            if instruction.source_line not in source_lines:
                source_lines.append(instruction.source_line)
        return source_lines

    def instruction_names(self, fn):
        _instructions_opcode_names = []
        for instruction_index in self.block_instructions:
            instruction = fn.updated_instructions[instruction_index]
            _instructions_opcode_names.append(instruction.opcode_name)
        return _instructions_opcode_names


class CFGModel(BaseModel):
    blocks: List[CFGBasicBlockModel]
    blocks_count: int
    edges_count: int
    predecessors: List[int]


class InstructionModel(BaseModel):
    """
    Default props
    """

    lineno: int
    num: int
    op1: OperandModel
    op1_type: int
    op2: OperandModel
    op2_type: int
    opcode: int
    opcode_flags: int
    opcode_name: str
    result: OperandModel
    result_type: int
    extended_value: int

    """
    Additional generated/updated props
    """
    source_line: Optional[str]
    basicblock_index: Optional[int]
    is_branch_instruction: bool = False
    is_global_fetch_instruction: bool = False
    is_function_call_instruction: bool = False
    variables_used_at_function_call_args: List[int] = []
    function_call_args: List[OperandModel] = []
    variables_used_here: List[int] = []
    ssa_variables_used_here: List[int] = []


class FunctionModel(BaseModel):
    class_name: str
    function_name: str
    num_args: int
    required_num_args: int
    number_of_instructions: int
    filename: str
    extra: ExtraModel
    instructions: List[InstructionModel]
    cfg: CFGModel
    dfg: List[DFGBasicBlockModel]
    ssa: SSAModel
    source_lines: List[str] = []

    def update_all(self) -> "FunctionModel":
        for index, instruction in enumerate(self.instructions):
            if model_utils._is_branch_instruction(instruction.opcode_name):
                self.instructions[index].is_branch_instruction = True

            if model_utils._instruction_is_global_fetch(
                instruction.opcode_flags, instruction.extended_value
            ):
                self.instructions[index].is_global_fetch_instruction = True

            if model_utils._instruction_is_function_call(instruction.opcode_name):
                self.instructions[index].is_function_call_instruction = True
                self.instructions[
                    index
                ].variables_used_at_function_call_args = model_utils._variables_used_at_function_call_args(  # noqa: E501
                    function=self, instruction_index=index
                )
                self.instructions[
                    index
                ].function_call_args = model_utils._function_call_args(
                    function=self, instruction_index=index
                )

            self.instructions[
                index
            ].variables_used_here = model_utils._variables_used_at_instruction(
                function=self,
                instruction_index=index,
                is_function_call_instruction=self.instructions[
                    index
                ].is_function_call_instruction,
            )

            self.instructions[
                index
            ].ssa_variables_used_here = model_utils._ssa_variables_used_at_instruction(
                function=self, instruction_index=index
            )

            try:
                self.instructions[index].source_line = self.source_lines[
                    self.instructions[index].lineno - 1
                ]
            except IndexError:
                # if file is empty, set to emtpy string
                if len(self.source_lines) == 0:
                    self.instructions[index].source_line = ""
                else:
                    self.instructions[index].source_line = self.source_lines[
                        len(self.source_lines) - 1
                    ]

        for block_index, block in enumerate(self.cfg.blocks):
            block_instructions = model_utils._get_block_instructions(block)
            for instruction_index in block_instructions:
                self.instructions[instruction_index].basicblock_index = block_index

            self.cfg.blocks[block_index].block_instructions = block_instructions

            if block.successors_count == 1:
                self.cfg.blocks[block_index].block_edges.append(
                    self.cfg.blocks[block.successors_storage[0]].start
                )

            elif block.successors_count == 2:
                self.cfg.blocks[block_index].block_edges.append(
                    self.cfg.blocks[block.successors_storage[0]].start
                )
                self.cfg.blocks[block_index].block_edges.append(
                    self.cfg.blocks[block.successors_storage[1]].start
                )

        return self
