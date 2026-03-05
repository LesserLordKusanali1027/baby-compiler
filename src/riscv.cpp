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
            file << "  mv    a0, " << register_name[symbol_register[value.operand]] << '\n';
            file << "  ret\n";
        }
        else {
            file << "  li    a0, " << value.operand << '\n';
            file << "  ret\n";
        }
    }
}

// 工具函数
int Visitor_ir::ParamReg(std::string param) {
    if (symbol_register.count(param))
        return symbol_register[param];
    else {
        file << "  li    " << register_name[current_register]
             << ", " << param << '\n';
        symbol_register[param] = current_register;
        current_register++;
        return symbol_register[param];
    }
}

int Visitor_ir::TargetReg(int param1, int param2) {
    if (param1 < param2)
        return param2;
    else if (param1 > param2)
        return param1;
    else if (param1 != 0)
        return param1;
    else
        return current_register;
}

// 将一条 RISC-V 的结果存入寄存器，但不更新 symbol_register
// 因为这只是一条 RISC-V 指令，而更新 symbol_register 要到一条 IR 指令结束
int Visitor_ir::ThreeOp(ValueIR_2& value, std::string opcode) {
    int r1 = ParamReg(value.operand1);
    int r2 = ParamReg(value.operand2);
    int dest = TargetReg(r1, r2);

    file << opcode << register_name[dest] << ", "
         << register_name[r1] << ", "
         << register_name[r2] << '\n';

    if (dest == r1)
        symbol_register.erase(value.operand1);
    else if (dest == r2)
        symbol_register.erase(value.operand2);
    else
        current_register++;

    return dest;
}

// 最复杂的一个
void Visitor_ir::riscv_get(ValueIR_2& value) {
    int dest;
    if (value.opcode == "sub") {
        dest = ThreeOp(value, "  sub   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "eq") {
        // 目前这里第二个参数一定是 0，但仍然放到一起
        dest = ThreeOp(value, "  xor   ");
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "add") {
        // 目前看来，不存在一个临时符号被多次使用的情况，所以前面的寄存器也不用保留，默认覆盖排在后面的寄存器
        dest = ThreeOp(value, "  add   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "mul") {
        dest = ThreeOp(value, "  mul   ");
        symbol_register[value.target] = dest;
    }
    // 注意除法和取模都是不可交换顺序的运算
    else if (value.opcode == "div") { // 先认为不会出现 0/0
        dest = ThreeOp(value, "  div   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "mod") { // 先认为不会出现 0%0
        dest = ThreeOp(value, "  rem   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "gt") {
        dest = ThreeOp(value, "  sgt   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "lt") {
        dest = ThreeOp(value, "  slt   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "ge") {
        // 先弄个 slt
        dest = ThreeOp(value, "  slt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "le") {
        // 先弄个 sgt
        dest = ThreeOp(value, "  sgt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "ne") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 snez
        file << "  snez  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "eq") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "and") {
        dest = ThreeOp(value, "  and   ");
        symbol_register[value.target] = dest;
    }
    else if (value.opcode == "or") {
        dest = ThreeOp(value, "  or    ");
        symbol_register[value.target] = dest;
    }
}