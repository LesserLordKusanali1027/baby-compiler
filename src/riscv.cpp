# include <iostream>
# include <vector>
# include <fstream>
# include "koopa.hpp"
# include "riscv.hpp"


// 工具函数
int Visitor_ir::ParamReg(std::string param) {
    if (param[0] == '%') { // 临时符号
        // 把内存加载到寄存器
        file << "  lw    " << register_name[current_register]
             << ", " << stack_usage[param] << "(sp)\n";
        current_register++;
        return current_register-1;
    }
    else { // 数值
        if (param == "0") {
            return 0;
        }
        else {
            file << "  li    " << register_name[current_register]
                << ", " << param << '\n';
            current_register++;
            return current_register-1;
        }
    }
}
// 寄存器覆盖选择
int Visitor_ir::TargetReg(int param1, int param2) {
    if (param1 < param2)
        return param2;
    else if (param1 > param2)
        return param1;
    else if (param1 != 0)
        return param1;
    else
        return current_register++;
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

    return dest;
}
// 函数栈空间分配计算
int Visitor_ir::stack_setup(FunctionIR& function) {
    int space = 0; // 栈指针需要下移的空间

    for (int i = 0; i < function.basic_blocks.size(); i++) {
        BasicBlockIR* basic_block = dynamic_cast<BasicBlockIR*>(function.basic_blocks[i]);

        for (int j = 0; j < basic_block->values.size(); j++) {
            if (dynamic_cast<ValueIR_2*>(basic_block->values[j])) {
                ValueIR_2* value = dynamic_cast<ValueIR_2*>(basic_block->values[j]);
                stack_usage[value->target] = space;
                space += 4;
            }
            else if (dynamic_cast<ValueIR_3*>(basic_block->values[j])) {
                ValueIR_3* value = dynamic_cast<ValueIR_3*>(basic_block->values[j]);
                stack_usage[value->target] = space;
                space += 4;
            }
        }
    }

    return space;
}


void Visitor_ir::riscv_get(ProgramIR& program) {
    file << "  .text\n";
    for (int i = 0; i < program.functions.size(); i++) {
        program.functions[i] -> accept(*this);
    }
}

void Visitor_ir::riscv_get(FunctionIR& function) {
    file << "  .globl " << function.name << "\n";
    file << function.name << ":\n";

    // 计算要分配的栈空间
    this->stack_frame = stack_setup(function);
    file << "  addi sp, sp, -" << std::to_string(this->stack_frame) << '\n';

    for (int i = 0; i < function.basic_blocks.size(); i++) {
        function.basic_blocks[i] -> accept(*this);
    }

    // 返回，注意要返回的内容应该已经被放到了 a0 中
    // 下面两条指令放到 ret 中了
    // file << "  addi sp, sp, " << std::to_string(this->stack_frame) << '\n';
    stack_usage.clear();
    // file << "  ret\n";
}

void Visitor_ir::riscv_get(BasicBlockIR& basic_block) {
    if (basic_block.name != "%entry") {
        file << '\n' <<basic_block.name.substr(1) << ":\n";
    }
    for (int i = 0; i < basic_block.values.size(); i++) {
        basic_block.values[i] -> accept(*this);
    }
}

// 处理 ret 和 jump
void Visitor_ir::riscv_get(ValueIR_1& value) {
    if (value.opcode == "ret") {
        if (value.operand[0] == '%') { // 临时符号，lw 内存
            file << "  lw    a0, " << stack_usage[value.operand] << "(sp)\n";
        }
        else {
            file << "  li    a0, " << value.operand << '\n';
        }
        
        // 恢复栈帧 + 返回
        file << "  addi sp, sp, " << std::to_string(this->stack_frame) << '\n';
        file << "  ret\n";
    }
    else if (value.opcode == "jump") {
        file << "  j     " << value.operand.substr(1) << '\n';
    }

    current_register = 1;
}

// 最复杂的一个
void Visitor_ir::riscv_get(ValueIR_2& value) {
    int dest;
    if (value.opcode == "sub") {
        dest = ThreeOp(value, "  sub   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "eq") {
        // 目前这里第二个参数一定是 0，但仍然放到一起
        dest = ThreeOp(value, "  xor   ");
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "add") {
        // 目前看来，不存在一个临时符号被多次使用的情况，所以前面的寄存器也不用保留，默认覆盖排在后面的寄存器
        dest = ThreeOp(value, "  add   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "mul") {
        dest = ThreeOp(value, "  mul   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    // 注意除法和取模都是不可交换顺序的运算
    else if (value.opcode == "div") {
        dest = ThreeOp(value, "  div   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "mod") {
        dest = ThreeOp(value, "  rem   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "gt") {
        dest = ThreeOp(value, "  sgt   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "lt") {
        dest = ThreeOp(value, "  slt   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "ge") {
        // 先弄个 slt
        dest = ThreeOp(value, "  slt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "le") {
        // 先弄个 sgt
        dest = ThreeOp(value, "  sgt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "ne") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 snez
        file << "  snez  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "eq") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "and") {
        dest = ThreeOp(value, "  and   ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }
    else if (value.opcode == "or") {
        dest = ThreeOp(value, "  or    ");
        file << "  sw    " << register_name[dest] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }

    current_register = 1;
}

// 用于 alloc 和 load，但 alloc 啥也不用做了，只需要处理 load
void Visitor_ir::riscv_get(ValueIR_3& value) {
    if (value.opcode == "load") {
        file << "  lw    " << register_name[current_register] << ", "
             << stack_usage[value.operand] << "(sp)\n";
        file << "  sw    " << register_name[current_register] << ", "
             << stack_usage[value.target] << "(sp)\n";
    }

    current_register = 1;
}

// 用于处理 store，逻辑和 load 一样
void Visitor_ir::riscv_get(ValueIR_4& value) {
    if (value.opcode == "store") {
        if (value.operand1[0]!='%' && value.operand1[0]!='@') {
            file << "  li    " << register_name[current_register] << ", "
                 << value.operand1 << '\n';
            file << "  sw    " << register_name[current_register] << ", "
                 << stack_usage[value.operand2] << "(sp)\n";
        }
        else {
            file << "  lw    " << register_name[current_register] << ", "
                 << stack_usage[value.operand1] << "(sp)\n";
            file << "  sw    " << register_name[current_register] << ", "
                 << stack_usage[value.operand2] << "(sp)\n";
        }
    }

    current_register = 1;
}

// 用于处理 br 指令
void Visitor_ir::riscv_get(ValueIR_5& value) {
    if (value.opcode == "br") {
        if (value.operand1[0] == '%') {
            file << "  lw    " << register_name[current_register] << ", "
                 << stack_usage[value.operand1] << "(sp)\n";
        }
        else {
            file << "  li    " << register_name[current_register] << ", "
                 << value.operand1 << '\n';
        }
        file << "  bnez  " << register_name[current_register] << ", "
             << value.operand2.substr(1) << '\n';
        file << "  j     " << value.operand3.substr(1) << '\n';
    }

    current_register = 1;
}