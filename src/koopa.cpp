#include "koopa.hpp"
#include "parser.hpp"
#include <vector>

void Visitor_ast::ir_init(CompUnitAST& comp_unit) {
    this -> program = new ProgramIR();
    comp_unit.func_def.get() -> accept(*this);
    (program -> functions).push_back(this -> function);
    return;
}

void Visitor_ast::ir_init(FuncDefAST& func_def) {
    this -> function = new FunctionIR();
    func_def.func_type.get() -> accept(*this);
    this -> function -> name = func_def.ident;
    // 这里只有一个基本块，逻辑简单了许多
    func_def.block.get() -> accept(*this);
    (function -> basic_blocks).push_back(this -> basic_block);
    return;
}

void Visitor_ast::ir_init(FuncTypeAST& func_type) {
    if (func_type.func_type == "int")
        this -> function -> function_type = "i32";
    return;
}

void Visitor_ast::ir_init(BlockAST& block) {
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%entry";
    block.stmt.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(StmtAST& stmt) {
    // 只有 return 一种指令，所以无需指令判断
    stmt.exp.get() -> accept(*this);
    ValueIR_1* value = new ValueIR_1();
    value -> opcode = "ret";
    // 但需要判断是直接返回数字还是临时变量，有了栈以后不用了
    value -> operand = (this->stk).top();
    (this->stk).pop();

    (this -> basic_block -> values).push_back(value);
    return;
}

void Visitor_ast::ir_init(ExpAST& exp) {
    exp.addexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(PrimaryExpAST_1& primary_exp) {
    primary_exp.exp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(PrimaryExpAST_2& primary_exp) {
    primary_exp.number.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(NumberAST& number) {
    (this->stk).push(std::to_string(number.num));
    return;
}

void Visitor_ast::ir_init(UnaryExpAST_1& unary_exp) {
    unary_exp.primaryexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(UnaryExpAST_2& unary_exp) {
    unary_exp.unaryexp.get() -> accept(*this);
    unary_exp.unaryop.get() -> accept(*this);

    ValueIR_2* value = new ValueIR_2();
    switch(this->ch) {
        case '+':
            delete value;
            return;
        case '-':
            value -> target = "%" + std::to_string(this->tmp_symbol);
            value -> opcode = "sub";
            value -> operand1 = "0";
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
        case '!':
            value -> target = "%" + std::to_string(this->tmp_symbol);
            value -> opcode = "eq";
            value -> operand2 = "0";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
    }
    (this->basic_block -> values).push_back(value);
    return;
}

void Visitor_ast::ir_init(UnaryOpAST& unary_op) {
    this -> ch = unary_op.ch;
    return;
}

// 算术表达式
void Visitor_ast::ir_init(MulExpAST_1& mul_exp) {
    mul_exp.unaryexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(MulExpAST_2& mul_exp) {
    mul_exp.unaryexp.get() -> accept(*this);
    mul_exp.mulexp.get() -> accept(*this);

    ValueIR_2* value = new ValueIR_2();
    switch(mul_exp.ch) {
        case '*':
            value -> opcode = "mul";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            value -> target = "%" + std::to_string(this->tmp_symbol);

            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
        case '/':
            value -> opcode = "div";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            value -> target = "%" + std::to_string(this->tmp_symbol);

            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
        case '%':
            value -> opcode = "mod";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            value -> target = "%" + std::to_string(this->tmp_symbol);

            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
    }

    (this->basic_block -> values).push_back(value);
    return;
}

void Visitor_ast::ir_init(AddExpAST_1& add_exp) {
    add_exp.mulexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(AddExpAST_2& add_exp) {
    add_exp.mulexp.get() -> accept(*this);
    add_exp.addexp.get() -> accept(*this);

    ValueIR_2* value = new ValueIR_2();
    switch(add_exp.ch) {
        case '+':
            value -> opcode = "add";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            value -> target = "%" + std::to_string(this->tmp_symbol);

            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
        case '-':
            value -> opcode = "sub";
            value -> operand1 = (this->stk).top();
            (this->stk).pop();
            value -> operand2 = (this->stk).top();
            (this->stk).pop();
            value -> target = "%" + std::to_string(this->tmp_symbol);

            (this->stk).push(value -> target);
            this->tmp_symbol++;
            break;
    }

    (this->basic_block -> values).push_back(value);
    return;
}