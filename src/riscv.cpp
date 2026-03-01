# include <iostream>
# include <vector>
# include <fstream>
# include "koopa.hpp"
# include "riscv.hpp"

void Visitor_ir::riscv_get(ProgramIR& program) {
    file << "  .text\n";
    for (int i = 0; i < program.functions.size(); i++) {
        program.functions[i] -> accept(*this);
    }
}

void Visitor_ir::riscv_get(FunctionIR& function) {
    file << "  .globl " << function.name << "\n";
    file << function.name << ":\n";
    for (int i = 0; i < function.basic_blocks.size(); i++) {
        function.basic_blocks[i] -> accept(*this);
    }
}

void Visitor_ir::riscv_get(BasicBlockIR& basic_block) {
    for (int i = 0; i < basic_block.values.size(); i++) {
        basic_block.values[i] -> accept(*this);
    }
}

void Visitor_ir::riscv_get(ValueIR_1& value) {
    if (value.opcode == "ret") {
        if (value.operand[0] == '%') {
            // std::cout << "  mv    a0, " << register_name[current_register-1] << '\n';
            // std::cout << "  ret\n";
            file << "  mv    a0, " << register_name[current_register-1] << '\n';
            file << "  ret\n";
        }
        else {
            // std::cout << "  li    a0, " << value.operand << '\n';
            // std::cout << "  ret\n";
            file << "  li    a0, " << value.operand << '\n';
            file << "  ret\n";
        }
    }
}

// 最复杂的一个
void Visitor_ir::riscv_get(ValueIR_2& value) {
    if (value.opcode == "sub") {
        if (value.operand1 == "0") {
            if (value.operand2[0] == '%') {
                // std::cout << "  sub   " << register_name[current_register] << ", x0, " << register_name[current_register-1] << '\n';
                file << "  sub   " << register_name[current_register] << ", x0, " << register_name[current_register-1] << '\n';
                current_register++;
            }
            else {
                // std::cout << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
                file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
                current_register++;
                // std::cout << "  sub   " << register_name[current_register] << ", x0, " << register_name[current_register-1] << '\n';
                file << "  sub   " << register_name[current_register] << ", x0, " << register_name[current_register-1] << '\n';
                current_register++;
            }
        }
    }
    else if (value.opcode == "eq") {
        if (value.operand2 == "0") {
            if (value.operand1[0] == '%') {
                // std::cout << "  xor   " << register_name[current_register-1] << ", " << register_name[current_register-1] << ", x0\n";
                // std::cout << "  seqz  " << register_name[current_register-1] << ", " << register_name[current_register-1] << '\n';
                file << "  xor   " << register_name[current_register-1] << ", " << register_name[current_register-1] << ", x0\n";
                file << "  seqz  " << register_name[current_register-1] << ", " << register_name[current_register-1] << '\n';
            }
            else {
                // std::cout << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
                file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
                current_register++;
                // std::cout << "  xor   " << register_name[current_register-1] << ", " << register_name[current_register-1] << ", x0\n";
                // std::cout << "  seqz  " << register_name[current_register-1] << ", " << register_name[current_register-1] << '\n';
                file << "  xor   " << register_name[current_register-1] << ", " << register_name[current_register-1] << ", x0\n";
                file << "  seqz  " << register_name[current_register-1] << ", " << register_name[current_register-1] << '\n';
            }
        }
    }
}