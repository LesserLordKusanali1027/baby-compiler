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

// 最复杂的一个
void Visitor_ir::riscv_get(ValueIR_2& value) {
    // 对于 sub ，先不管三七二十一全都存到新寄存器里
    if (value.opcode == "sub") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        file << "  sub   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
        symbol_register[value.target] = current_register;
        current_register++;
    }
    else if (value.opcode == "eq") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        // 目前这里第二个参数一定是 0

        file << "  xor   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
        file << "  seqz  " << register_name[current_register] << ", " << register_name[current_register] << '\n';
        symbol_register[value.target] = current_register;
        current_register++;
    }
    else if (value.opcode == "add") {
        // 目前看来，不存在一个临时符号被多次使用的情况，所以前面的寄存器也不用保留，默认覆盖排在后面的寄存器
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  add   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else {
            file << "  add   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
    }
    else if (value.opcode == "mul") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  mul   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  mul   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  mul   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else {
            file << "  mul   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
    }
    // 注意除法和取模都是不可交换顺序的运算
    else if (value.opcode == "div") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  div   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  div   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  div   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        // 先认为不会出现 0/0
    }
    else if (value.opcode == "mod") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  rem   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  rem   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  rem   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        // 先认为不会出现 0%0
    }
    else if (value.opcode == "gt") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  sgt   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  sgt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  sgt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  sgt   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
    }
    else if (value.opcode == "lt") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  slt   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  slt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  slt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  slt   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
    }
    else if (value.opcode == "ge") {
        // 先弄个 slt
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  slt   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  slt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  slt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  slt   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
        // 再弄 seqz，虽然结果应该是对的，但我整的还是有些乱
        file << "  seqz  " << register_name[symbol_register[value.target]] << ", " << register_name[symbol_register[value.target]] << '\n';
    }
    else if (value.opcode == "le") {
        // 先弄个 sgt
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  sgt   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  sgt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  sgt   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  sgt   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
        // 再弄 seqz，虽然结果应该是对的，但我整的还是有些乱
        file << "  seqz  " << register_name[symbol_register[value.target]] << ", " << register_name[symbol_register[value.target]] << '\n';
    }
    else if (value.opcode == "ne") {
        // 先弄个 xor
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  xor   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  xor   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  xor   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  xor   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
        // 再弄 snez，虽然结果应该是对的，但我整的还是有些乱
        file << "  snez  " << register_name[symbol_register[value.target]] << ", " << register_name[symbol_register[value.target]] << '\n';
    }
    else if (value.opcode == "eq") {
        // 先弄个 xor
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  xor   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  xor   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  xor   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  xor   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
        // 再弄 seqz，虽然结果应该是对的，但我整的还是有些乱
        file << "  seqz  " << register_name[symbol_register[value.target]] << ", " << register_name[symbol_register[value.target]] << '\n';
    }
    else if (value.opcode == "and") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  and   " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  and   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  and   " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  and   " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
    }
    else if (value.opcode == "or") {
        if (!symbol_register.count(value.operand1)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand1 << '\n';
            symbol_register[value.operand1] = current_register;
            current_register++;
        }
        if (!symbol_register.count(value.operand2)) {
            file << "  li    " << register_name[current_register] << ", " << value.operand2 << '\n';
            symbol_register[value.operand2] = current_register;
            current_register++;
        }

        if (symbol_register[value.operand1] < symbol_register[value.operand2]) {
            file << "  or    " << register_name[symbol_register[value.operand2]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand2];
            symbol_register.erase(value.operand2);
        }
        else if (symbol_register[value.operand1] > symbol_register[value.operand2]) {
            file << "  or    " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else if (symbol_register[value.operand1] != 0) {
            file << "  or    " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = symbol_register[value.operand1];
            symbol_register.erase(value.operand1);
        }
        else {
            file << "  or    " << register_name[current_register] << ", " << register_name[symbol_register[value.operand1]] << ", " << register_name[symbol_register[value.operand2]] << '\n';
            symbol_register[value.target] = current_register;
            current_register++;
        }
    }
}