#ifndef RISCV_HPP
#define RISCV_HPP

# include "koopa.hpp"
# include <fstream>

void Riscv(const ProgramIR* program_ir);

void Riscv(const FunctionIR* function_ir);

void Riscv(const BasicBlockIR* basic_block_ir);

void Riscv(const ValueIR* value_ir);

void Riscv_file(const ProgramIR* program_ir, std::ofstream& file);

void Riscv_file(const FunctionIR* function_ir, std::ofstream& file);

void Riscv_file(const BasicBlockIR* basic_block_ir, std::ofstream& file);

void Riscv_file(const ValueIR* value_ir, std::ofstream& file);

#endif