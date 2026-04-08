# include <iostream>
# include <vector>
# include <fstream>
# include <unordered_map>
# include "koopa.hpp"
# include "riscv.hpp"


// lw 和 sw 指令的函数
void Visitor_ir::riscv_lw(std::string target_register, int offset) {
    if (offset >= -2048 && offset <= 2047) {
        file << "  lw    " << target_register << ", " << offset << "(sp)\n";
    }
    else {
        file << "  li    " << target_register << ", " << offset << '\n';
        file << "  add   " << target_register << ", " << target_register << ", sp\n";
        file << "  lw    " << target_register << ", 0(" << target_register << ")\n";
    }
}
void Visitor_ir::riscv_sw(std::string target_register, int offset, std::string tool_register) {
    if (offset >= -2048 && offset <= 2047) {
        file << "  sw    " << target_register << ", " << offset << "(sp)\n";
    }
    else {
        file << "  li    " << tool_register<< ", " << offset << '\n';
        file << "  add   " << tool_register << ", " << tool_register << ", sp\n";
        file << "  sw    " << target_register << ", 0(" << tool_register << ")\n";
    }
}


// 工具函数
int Visitor_ir::ParamReg(std::string param) {
    if (param[0] == '%') { // 临时符号
        // 把内存加载到寄存器
        riscv_lw(register_name[current_register], var_offset[param]);

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
void Visitor_ir::stack_setup(FunctionIR& function) {
    stack_frame = 0; // 栈指针需要下移的空间
    var_space = 0;
    ra_space = 0;
    param_space = 0;

    for (int i = 0; i < function.basic_blocks.size(); i++) {
        BasicBlockIR* basic_block = dynamic_cast<BasicBlockIR*>(function.basic_blocks[i]);

        for (int j = 0; j < basic_block->values.size(); j++) {
            // 计算 var_space
            if (dynamic_cast<ValueIR_2*>(basic_block->values[j])) {
                ValueIR_2* value = dynamic_cast<ValueIR_2*>(basic_block->values[j]);
                var_offset[value->target] = var_space;
                var_space += 4;
            }
            else if (dynamic_cast<ValueIR_3*>(basic_block->values[j])) {
                ValueIR_3* value = dynamic_cast<ValueIR_3*>(basic_block->values[j]);
                var_offset[value->target] = var_space;
                var_space += 4;
            }
            else if (dynamic_cast<ValueIR_6*>(basic_block->values[j])) {
                ValueIR_6* value = dynamic_cast<ValueIR_6*>(basic_block->values[j]);
                if (!(value->target).empty()) {
                    var_offset[value->target] = var_space;
                    var_space += 4;
                }
            }
            else if (dynamic_cast<ValueIR_8*>(basic_block->values[j])) {
                ValueIR_8* value = dynamic_cast<ValueIR_8*>(basic_block->values[j]);
                int size = 1;
                for (int i = 0; i < (value->operand2s).size(); i++) // 计算有多少个 int
                    size *= value->operand2s[i];
                var_offset[value->target] = var_space;
                var_space += 4*size;
            }
            else if (dynamic_cast<ValueIR_9*>(basic_block->values[j])) {
                ValueIR_9* value = dynamic_cast<ValueIR_9*>(basic_block->values[j]);
                // 数组参数 alloc ，只需 4 字节
                var_offset[value->target] = var_space;
                var_space += 4;
            }

            // 计算 a0_space 和 param_space
            if (dynamic_cast<ValueIR_6*>(basic_block->values[j])) {
                ValueIR_6* value = dynamic_cast<ValueIR_6*>(basic_block->values[j]);

                ra_space = 4;
                int param_num = (value->parameters).size()-8 > 0 ? (value->parameters).size()-8 : 0;
                if (param_num != 0 && param_num*4 > param_space)
                    param_space = param_num*4;
            }
        }
    }

    if (param_space != 0) {
        // 遍历 unordered_map var_offset 调整偏移量
        std::unordered_map<std::string,int>::iterator it;
        for (it = var_offset.begin(); it != var_offset.end(); it++)
            it -> second += param_space;
    }

    stack_frame = var_space + ra_space + param_space;
    if (stack_frame != 0 && stack_frame % 16 != 0)
        stack_frame = (stack_frame / 16 + 1) * 16;

    return;
}


void Visitor_ir::riscv_get(ProgramIR& program) {
    for (int i = 0; i < program.globals.size(); i++) {
        program.globals[i] -> accept(*this);
    }
    for (int i = 0; i < program.functions.size(); i++) {
        program.functions[i] -> accept(*this);
    }
}

void Visitor_ir::riscv_get(GlobalIR_1& global) {
    // 放到 data 里面
    file << "  .data\n";
    file << "  .globl " << global.name.substr(1) << '\n';
    file << global.name.substr(1) << ":\n";
    if (global.init_val == "zeroinit" && global.type == "int") {
        file << "  .zero 4\n\n";
    }
    else if (global.type == "int") {
        file << "  .word " << global.init_val << "\n\n";
    }

    // 记录到全局变量表中，供 store 和 load 使用
    this -> global_vars[global.name] = 1;
}

void Visitor_ir::riscv_get(GlobalIR_2& global) {
    file << "  .data\n";
    file << "  .globl " << global.name.substr(1) << '\n';
    file << global.name.substr(1) << ":\n";
    if (global.init_val.size() != 0) {
        for (int i = 0; i < global.init_val.size(); i++)
            file << "  .word " << global.init_val[i] << '\n';
    }
    else {
        int sum = 4;
        for (int i = 0; i < global.size.size(); i++)
            sum *= global.size[i];
        file << "  .zero " << sum << '\n';
    }

    file << '\n';
    // 记录到全局变量表中，供 store 和 load 使用
    this -> global_vars[global.name] = 1;

    // 记录到 array_info 中
    array_info.Add_Array(global.size, global.name);
    array_info.Add_Symbol(global.name);
    // 再记录到 global_arrays 中
    global_arrays[global.name] = 1;
}

void Visitor_ir::riscv_get(FunctionIR& function) {
    file << "  .text\n";
    file << "  .globl " << function.name.substr(1) << "\n";
    file << function.name.substr(1) << ":\n";

    // 计算要分配的栈空间
    stack_setup(function);
    if (stack_frame != 0) {
        int offset = - stack_frame;
        if (offset >= -2048 && offset <= 2047) {
            file << "  addi  sp, sp, " << offset << '\n';
        }
        else {
            file << "  li    t0, " << offset << '\n';
            file << "  add   sp, t0, sp\n";
        }
    }

    // 保存返回地址
    if (ra_space != 0) {
        riscv_sw("ra", stack_frame - 4, "t0");
    }

    // 记下各个参数的名字，用到的时候再加载
    for (int i = 0; i < function.parameters.size(); i++) {
        param_name[function.parameters[i]] = i;
    }

    for (int i = 0; i < function.basic_blocks.size(); i++) {
        function.basic_blocks[i] -> accept(*this);
    }

    // 返回，注意要返回的内容应该已经被放到了 a0 中
    // 下面两条指令放到 ret 中了
    // file << "  addi sp, sp, " << std::to_string(this->stack_frame) << '\n';
    // file << "  ret\n";
    var_offset.clear();
    param_name.clear();
    
    file << "\n";

    // 清空 array_info 中的临时符号
    array_info.CleanSymbol();
}

void Visitor_ir::riscv_get(BasicBlockIR& basic_block) {
    if (basic_block.name != "%entry") {
        file << '\n' << basic_block.name.substr(1) << ":\n";
    }
    for (int i = 0; i < basic_block.values.size(); i++) {
        basic_block.values[i] -> accept(*this);
    }
}

// 处理 ret 和 jump
void Visitor_ir::riscv_get(ValueIR_1& value) {
    if (value.opcode == "ret") {
        if (value.operand[0] == '%') { // 临时符号，lw 内存
            riscv_lw("a0", var_offset[value.operand]);
        }
        else {
            file << "  li    a0, " << value.operand << '\n';
        }
        
        // 加载 ra
        if (ra_space != 0) {
            riscv_lw("ra", stack_frame - 4);
        }
        // 恢复栈帧 + 返回
        if (stack_frame != 0) {
            if (stack_frame >= -2048 && stack_frame <= 2047) {
                file << "  addi  sp, sp, " << this->stack_frame << '\n';
            }
            else {
                file << "  li    t0, " << this->stack_frame << '\n';
                file << "  add   sp, t0, sp\n";
            }
        }
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
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "eq") {
        // 目前这里第二个参数一定是 0，但仍然放到一起
        dest = ThreeOp(value, "  xor   ");
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "add") {
        // 目前看来，不存在一个临时符号被多次使用的情况，所以前面的寄存器也不用保留，默认覆盖排在后面的寄存器
        dest = ThreeOp(value, "  add   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "mul") {
        dest = ThreeOp(value, "  mul   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    // 注意除法和取模都是不可交换顺序的运算
    else if (value.opcode == "div") {
        dest = ThreeOp(value, "  div   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "mod") {
        dest = ThreeOp(value, "  rem   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "gt") {
        dest = ThreeOp(value, "  sgt   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "lt") {
        dest = ThreeOp(value, "  slt   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "ge") {
        // 先弄个 slt
        dest = ThreeOp(value, "  slt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "le") {
        // 先弄个 sgt
        dest = ThreeOp(value, "  sgt   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "ne") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 snez
        file << "  snez  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "eq") {
        // 先弄个 xor
        dest = ThreeOp(value, "  xor   ");
        // 再弄 seqz
        file << "  seqz  " << register_name[dest] << ", "
             << register_name[dest] << '\n';
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "and") {
        dest = ThreeOp(value, "  and   ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "or") {
        dest = ThreeOp(value, "  or    ");
        riscv_sw(register_name[dest], var_offset[value.target], register_name[dest+1]);
    }
    else if (value.opcode == "getelemptr") {
        if (value.operand2[0] == '%') {
            riscv_lw(register_name[current_register++], var_offset[value.operand2]);
        }
        else {
            file << "  li    " << register_name[current_register++] << ", "
                 << value.operand2 << '\n';
        }

        // 获得偏移单位
        int size = array_info.Get_Size_Low(value.operand1);

        file << "  li    " << register_name[current_register] << ", " << size << "\n";

        file << "  mul   " << register_name[current_register] << ", "
             << register_name[current_register-1] << ", " << register_name[current_register] << '\n';

        // 如果是全局数组
        if (global_arrays.count(value.operand1)) {
            file << "  la    " << register_name[current_register+1] << ", "
                 << value.operand1.substr(1) << '\n';
        }
        else if (value.operand1[0] == '@') { // 局部数组
            int offset = var_offset[value.operand1];
            if (offset >= -2048 && offset <= 2047) {
                file << "  addi  " << register_name[current_register+1] << ", sp, "
                    << offset << '\n';
            }
            else {
                file << "  li    " << register_name[current_register+2] << ", " << offset << '\n';
                file << "  add   " << register_name[current_register+1] << ", " << register_name[current_register+2] << ", sp\n";
            }
        }
        else if (value.operand1[0] == '%') { // 临时符号
            riscv_lw(register_name[current_register+1], var_offset[value.operand1]);
        }
        
        file << "  add   " << register_name[current_register+1] << ", "
             << register_name[current_register] << ", "
             << register_name[current_register+1] << '\n';

        riscv_sw(register_name[current_register+1], var_offset[value.target], register_name[current_register+2]);

        // 记录新符号
        array_info.Add_Symbol(value.target, value.operand1, true);
    }
    else if (value.opcode == "getptr") {
        if (value.operand2[0] == '%') {
            riscv_lw(register_name[current_register++], var_offset[value.operand2]);
        }
        else {
            file << "  li    " << register_name[current_register++] << ", "
                 << value.operand2 << '\n';
        }

        // 获得偏移单位
        int size = array_info.Get_Size_Equal(value.operand1);

        file << "  li    " << register_name[current_register] << ", " << size << "\n";

        file << "  mul   " << register_name[current_register] << ", "
             << register_name[current_register-1] << ", " << register_name[current_register] << '\n';

        // 只可能是临时符号，这就比较好了
        riscv_lw(register_name[current_register+1], var_offset[value.operand1]);

        file << "  add   " << register_name[current_register+1] << ", "
             << register_name[current_register] << ", "
             << register_name[current_register+1] << '\n';

        riscv_sw(register_name[current_register+1], var_offset[value.target], register_name[current_register+2]);

        // 记录新符号
        array_info.Add_Symbol(value.target, value.operand1, false);
    }

    current_register = 1;
}

// 用于 alloc 和 load，但 alloc 啥也不用做了，只需要处理 load
void Visitor_ir::riscv_get(ValueIR_3& value) {
    if (value.opcode == "load") {
        if (global_vars.count(value.operand)) { // load 的是全局变量时
            file << "  la    " << register_name[current_register] << ", "
                 << value.operand.substr(1) << '\n';
            file << "  lw    " << register_name[current_register] << ", "
                 << "0(" << register_name[current_register] << ")\n";

            riscv_sw(register_name[current_register], var_offset[value.target], register_name[current_register+1]);
        }
        else if (value.operand[0] == '%') { // 先认为 %1 = load %0 只会出现在 %0 为指针的时候
            riscv_lw(register_name[current_register], var_offset[value.operand]);

            file << "  lw    " << register_name[current_register] << ", "
                 << "0(" << register_name[current_register] << ")\n";

            riscv_sw(register_name[current_register], var_offset[value.target], register_name[current_register+1]);
        }
        else { // load 的是局部变量时
            riscv_lw(register_name[current_register], var_offset[value.operand]);
            riscv_sw(register_name[current_register], var_offset[value.target], register_name[current_register+1]);

            // 如果 load 的操作数是数组，把临时符号记入 array_info
            if (array_info.If_Exist(value.operand)) {
                array_info.Add_Symbol(value.target, value.operand, false);
            }
        }
    }

    current_register = 1;
}

// 用于处理 store，逻辑和 load 一样
void Visitor_ir::riscv_get(ValueIR_4& value) {
    if (value.opcode == "store") {
        if (param_name.count(value.operand1)) { // 当里面是函数参数时
            int idx = param_name[value.operand1];
            if (idx < 8) {
                riscv_sw(param_register[idx], var_offset[value.operand2], "t0");
            }
            else {
                riscv_lw(register_name[current_register], stack_frame + (idx-8)*4);
                riscv_sw(register_name[current_register], var_offset[value.operand2], register_name[current_register+1]);
            }
        }
        else if (global_vars.count(value.operand2)) { // 当目标是全局变量时
            if (value.operand1[0]!='%' && value.operand1[0]!='@') { // 数字
                file << "  li    " << register_name[current_register] << ", "
                     << value.operand1 << '\n';
            }
            else {
                riscv_lw(register_name[current_register], var_offset[value.operand1]);
            }
            file << "  la    " << register_name[current_register+1] << ", "
                 << value.operand2.substr(1) << '\n';
            file << "  sw    " << register_name[current_register] << ", "
                 << "0(" << register_name[current_register+1] << ")\n";
        }
        else if (value.operand2[0] == '%') { // 为临时符号，目前 store 好像 operand2 不是 @ 就是 %
            if (value.operand1[0]!='%' && value.operand1[0]!='@') { // 数字
                file << "  li    " << register_name[current_register] << ", "
                     << value.operand1 << '\n';
            }
            else {
                riscv_lw(register_name[current_register], var_offset[value.operand1]);
            }
            riscv_lw(register_name[current_register+1], var_offset[value.operand2]);
            file << "  sw    " << register_name[current_register] << ", "
                 << "0(" << register_name[current_register+1] << ")\n";
        }
        else if (value.operand1[0]!='%' && value.operand1[0]!='@') { // 数字存入局部变量 @
            file << "  li    " << register_name[current_register] << ", "
                 << value.operand1 << '\n';

            riscv_sw(register_name[current_register], var_offset[value.operand2], register_name[current_register+1]);
        }
        else {
            riscv_lw(register_name[current_register], var_offset[value.operand1]);
            riscv_sw(register_name[current_register], var_offset[value.operand2], register_name[current_register+1]);
        }
    }

    current_register = 1;
}

// 用于处理 br 指令
void Visitor_ir::riscv_get(ValueIR_5& value) {
    if (value.opcode == "br") {
        if (value.operand1[0] == '%') {
            riscv_lw(register_name[current_register], var_offset[value.operand1]);
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

// 处理 call 指令
void Visitor_ir::riscv_get(ValueIR_6& value) {
    for (int i = 0; i < value.parameters.size(); i++) {
        if (i < 8) {
            if (value.parameters[i][0] == '%') {
                riscv_lw(param_register[i], var_offset[value.parameters[i]]);
            }
            else {
                file << "  li    " << param_register[i] << ", "
                     << value.parameters[i] << '\n';
            }
        }
        else {
            if (value.parameters[i][0] == '%') {
                riscv_lw(register_name[current_register], var_offset[value.parameters[i]]);
                riscv_sw(register_name[current_register], (i-8)*4, register_name[current_register+1]);
            }
            else {
                file << "  li    " << register_name[current_register] << ", "
                     << value.parameters[i] << '\n';

                riscv_sw(register_name[current_register], (i-8)*4, register_name[current_register+1]);
            }
        }
    }

    file << "  call  " << value.operand.substr(1) << '\n';

    if (!value.target.empty()) {
        riscv_sw("a0", var_offset[value.target], "t0");
    }
}

// 处理 ret 指令
void Visitor_ir::riscv_get(ValueIR_7& value) {
    // 加载 ra
    if (ra_space != 0) {
        riscv_lw("ra", stack_frame - 4);
    }
    // 恢复栈帧 + 返回
    if (stack_frame != 0) {
        if (stack_frame >= -2048 && stack_frame <= 2047) {
            file << "  addi  sp, sp, " << this->stack_frame << '\n';
        }
        else {
            file << "  li    t0, " << this->stack_frame << '\n';
            file << "  add   sp, t0, sp\n";
        }
    }
    file << "  ret\n";
}

// 数组 alloc 指令
void Visitor_ir::riscv_get(ValueIR_8& value) {
    // 记录到 array_info 中
    array_info.Add_Array(value.operand2s, value.target);
    array_info.Add_Symbol(value.target);
}

// 数组参数 alloc 指令
void Visitor_ir::riscv_get(ValueIR_9& value) {
    // 记录到 array_info 中
    array_info.Add_Array(value.operand2s, value.target);
    array_info.Add_Symbol(value.target);
}