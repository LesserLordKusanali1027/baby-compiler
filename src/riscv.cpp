# include "koopa.hpp"
# include <iostream>
# include <vector>
# include <fstream>
# include "riscv.hpp"

void Riscv(const ProgramIR* program_ir) {
    std::cout << "  .text" << std::endl;
    int func_num = program_ir->function.size();
    for (int i = 0; i < func_num; i++)
        Riscv((FunctionIR*)program_ir->function[i]);
}

void Riscv(const FunctionIR* function_ir) {
    std::cout << "  .globl " <<  function_ir->name << std::endl;
    std::cout << function_ir->name << ":" << std::endl;
    int block_num = function_ir->basic_block.size();
    for (int i = 0; i < block_num; i++)
        Riscv((BasicBlockIR*)function_ir->basic_block[i]);
}

void Riscv(const BasicBlockIR* basic_block_ir) {
    int value_num = basic_block_ir->value.size();
    for (int i = 0; i < value_num; i++)
        Riscv((ValueIR*)basic_block_ir->value[i]);
}

void Riscv(const ValueIR* value_ir) {
    if (value_ir->opcode == "ret") {
        std::cout << "  li a0, " << value_ir->operand << std::endl;
        std::cout << "  ret" << std::endl;
    }
}

void Riscv_file(const ProgramIR* program_ir, std::ofstream& file) {
    file << "  .text" << std::endl;
    int func_num = program_ir->function.size();
    for (int i = 0; i < func_num; i++)
        Riscv_file((FunctionIR*)program_ir->function[i], file);
}

void Riscv_file(const FunctionIR* function_ir, std::ofstream& file) {
    file << "  .globl " <<  function_ir->name << std::endl;
    file << function_ir->name << ":" << std::endl;
    int block_num = function_ir->basic_block.size();
    for (int i = 0; i < block_num; i++)
        Riscv_file((BasicBlockIR*)function_ir->basic_block[i], file);
}

void Riscv_file(const BasicBlockIR* basic_block_ir, std::ofstream& file) {
    int value_num = basic_block_ir->value.size();
    for (int i = 0; i < value_num; i++)
        Riscv_file((ValueIR*)basic_block_ir->value[i], file);
}

void Riscv_file(const ValueIR* value_ir, std::ofstream& file) {
    if (value_ir->opcode == "ret") {
        file << "  li a0, " << value_ir->operand << std::endl;
        file << "  ret" << std::endl;
    }
}