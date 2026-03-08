#include "koopa.hpp"
#include "parser.hpp"
#include <vector>

void Visitor_ast::ir_init(CompUnitAST& comp_unit) {
    this -> program = new ProgramIR();
    comp_unit.func_def.get() -> accept(*this);
    (program -> functions).push_back(this -> function);
    return;
}

void Visitor_ast::ir_init(DeclAST_1& decl) {
    // 没写就是什么都不用做
}

void Visitor_ast::ir_init(DeclAST_2& decl) { // 变量还是要往下传的
    decl.vardecl.get() -> accept(*this);
}

void Visitor_ast::ir_init(ConstDeclAST& const_decl) {

}

void Visitor_ast::ir_init(BTypeAST& btype) {

}

void Visitor_ast::ir_init(ConstDefListAST& const_def_list) {

}

void Visitor_ast::ir_init(ConstDefAST& const_def) {

}

void Visitor_ast::ir_init(ConstInitValAST& const_init_val) {

}

void Visitor_ast::ir_init(VarDeclAST& var_decl) {
    var_decl.vardeflist.get() -> accept(*this);
}

void Visitor_ast::ir_init(VarDefListAST& var_def_list) {
    for (int i = 0; i < var_def_list.vardefs.size(); i++) {
        var_def_list.vardefs[i].get() -> accept(*this);
    }
}

void Visitor_ast::ir_init(VarDefAST_1& var_def) {
    // 是时候 alloc 了
    ValueIR_3* value = new ValueIR_3();
    value -> opcode = "alloc";
    value -> target = "@" + var_def.ident;
    value -> operand = "i32";
    (this -> basic_block -> values).push_back(value);
}
void Visitor_ast::ir_init(VarDefAST_2& var_def) {
    // 左边 alloc
    ValueIR_3* value1 = new ValueIR_3();
    value1 -> opcode = "alloc";
    value1 -> target = "@" + var_def.ident;
    value1 -> operand = "i32";
    (this -> basic_block -> values).push_back(value1);

    // 右边 load
    this->lval_mode = LOAD;
    var_def.initval.get() -> accept(*this);
    this->lval_mode = START;
    ValueIR_4* value2 = new ValueIR_4();
    value2 -> opcode = "store";
    value2 -> operand2 = "@" + var_def.ident;
    value2 -> operand1 = (this->stk).top();
    (this->stk).pop();
    (this -> basic_block -> values).push_back(value2);
}

void Visitor_ast::ir_init(InitValAST& init_val) {
    init_val.exp.get() -> accept(*this);
}

void Visitor_ast::ir_init(FuncDefAST& func_def) {
    this -> function = new FunctionIR();
    func_def.func_type.get() -> accept(*this);
    this -> function -> name = func_def.ident;
    // 先把基本块的定义放在函数里，因为 BlockAST 与 IR 中的基本块并不等同
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%entry";
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
    block.blockitemlist.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(BlockItemListAST& block_item_list) {
    for (int i = 0; i < block_item_list.blockitems.size(); i++) {
        block_item_list.blockitems[i].get() -> accept(*this);
    }
}

void Visitor_ast::ir_init(BlockItemAST_1& block_item) {
    block_item.decl.get() -> accept(*this);
}

void Visitor_ast::ir_init(BlockItemAST_2& block_item) {
    block_item.stmt.get() -> accept(*this);
}

// Stmt      ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block;
void Visitor_ast::ir_init(StmtAST_1& stmt) {
    this->lval_mode = LOAD;
    stmt.exp.get() -> accept(*this);
    
    this->lval_mode = STORE;
    stmt.lval.get() -> accept(*this);
    this->lval_mode = START;

}
void Visitor_ast::ir_init(StmtAST_2& stmt) {
    // 只有 return 一种指令，所以无需指令判断
    
    if (!stmt.exp) { // 只有 return; 的时候，默认 return 0
        ValueIR_1* value = new ValueIR_1();
        value -> opcode = "ret";
        value -> operand = "0";
        (this -> basic_block -> values).push_back(value);
        return;
    }

    this->lval_mode = LOAD;
    stmt.exp.get() -> accept(*this);
    this->lval_mode = START;

    ValueIR_1* value = new ValueIR_1();
    value -> opcode = "ret";
    // 但需要判断是直接返回数字还是临时变量，有了栈以后不用了
    value -> operand = (this->stk).top();
    (this->stk).pop();

    (this -> basic_block -> values).push_back(value);
    return;
}
void Visitor_ast::ir_init(StmtAST_3& stmt) {
    // 没用的计算
    if (stmt.exp) {
        this->lval_mode = LOAD;
        stmt.exp.get() -> accept(*this);
        this->lval_mode = START;
        (this->stk).pop();
    }
}
void Visitor_ast::ir_init(StmtAST_4& stmt) {
    stmt.block.get() -> accept(*this);
}

void Visitor_ast::ir_init(ExpAST& exp) {
    exp.lorexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(LValAST& lval) {
    // 只要进来了，就是变量，因为常量的 AST 路径是切断的
    if (this->lval_mode == LOAD) {
        ValueIR_3* value = new ValueIR_3();
        value -> opcode = "load";
        value -> operand = "@" + lval.ident;
        value -> target = "%" + std::to_string(this->tmp_symbol);
        this->tmp_symbol++;
        (this->stk).push(value -> target);
        (this -> basic_block -> values).push_back(value);
    }
    else if (this->lval_mode == STORE) {
        ValueIR_4* value = new ValueIR_4();
        value -> opcode = "store";
        value -> operand1 = (this->stk).top();
        value -> operand2 = "@" + lval.ident;
        (this->stk).pop();
        (this -> basic_block -> values).push_back(value);
    }
}

void Visitor_ast::ir_init(PrimaryExpAST_1& primary_exp) {
    primary_exp.exp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(PrimaryExpAST_2& primary_exp) {
    primary_exp.lval.get() -> accept(*this);
}

void Visitor_ast::ir_init(PrimaryExpAST_3& primary_exp) {
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

void Visitor_ast::ir_init(RelExpAST_1& rel_exp) {
    rel_exp.addexp.get() -> accept(*this);
    return;
}
// 均采用右序遍历，实际上左右好像都可以
void Visitor_ast::ir_init(RelExpAST_2& rel_exp) {
    rel_exp.addexp.get() -> accept(*this);
    rel_exp.relexp.get() -> accept(*this);
    
    ValueIR_2* value = new ValueIR_2();
    if (rel_exp.cmp_str == ">")
        value -> opcode = "gt";
    else if (rel_exp.cmp_str == "<")
        value -> opcode = "lt";
    else if (rel_exp.cmp_str == ">=")
        value -> opcode = "ge";
    else if (rel_exp.cmp_str == "<=")
        value -> opcode = "le";

    value -> operand1 = (this->stk).top();
    (this->stk).pop();
    value -> operand2 = (this->stk).top();
    (this->stk).pop();
    value -> target = "%" + std::to_string(this->tmp_symbol);

    (this->stk).push(value -> target);
    this->tmp_symbol++;

    (this->basic_block -> values).push_back(value);
    return;
}

void Visitor_ast::ir_init(EqExpAST_1& eq_exp) {
    eq_exp.relexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(EqExpAST_2& eq_exp) {
    eq_exp.relexp.get() -> accept(*this);
    eq_exp.eqexp.get() -> accept(*this);

    ValueIR_2* value = new ValueIR_2();
    if (eq_exp.cmp_str == "==")
        value -> opcode = "eq";
    else if (eq_exp.cmp_str == "!=")
        value -> opcode = "ne";

    value -> operand1 = (this->stk).top();
    (this->stk).pop();
    value -> operand2 = (this->stk).top();
    (this->stk).pop();
    value -> target = "%" + std::to_string(this->tmp_symbol);

    (this->stk).push(value -> target);
    this->tmp_symbol++;

    (this->basic_block -> values).push_back(value);
    return;
}

void Visitor_ast::ir_init(LAndExpAST_1& l_and_exp) {
    l_and_exp.eqexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(LAndExpAST_2& l_and_exp) {
    l_and_exp.eqexp.get() -> accept(*this);
    l_and_exp.landexp.get() -> accept(*this);

    // 第一个 ne 指令
    ValueIR_2* value1 = new ValueIR_2();
    value1 -> opcode = "ne";
    value1 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value1 -> operand2 = "0";
    value1 -> target = "%" + std::to_string(this->tmp_symbol);
    this->tmp_symbol++;
    (this->basic_block -> values).push_back(value1);

    // 第二个 ne 指令
    ValueIR_2* value2 = new ValueIR_2();
    value2 -> opcode = "ne";
    value2 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value2 -> operand2 = "0";
    value2 -> target = "%" + std::to_string(this->tmp_symbol);
    this->tmp_symbol++;
    (this->basic_block -> values).push_back(value2);

    // 最后才是按位 and 指令
    ValueIR_2* value3 = new ValueIR_2();
    value3 -> opcode = "and";
    value3 -> operand1 = value1 -> target;
    value3 -> operand2 = value2 -> target;
    value3 -> target = "%" + std::to_string(this->tmp_symbol);

    (this->stk).push(value3 -> target);
    this->tmp_symbol++;

    (this->basic_block -> values).push_back(value3);
    return;
}

void Visitor_ast::ir_init(LOrExpAST_1& l_or_exp) {
    l_or_exp.landexp.get() -> accept(*this);
    return;
}

void Visitor_ast::ir_init(LOrExpAST_2& l_or_exp) {
    l_or_exp.landexp.get() -> accept(*this);
    l_or_exp.lorexp.get() -> accept(*this);

    // 第一个 ne 指令
    ValueIR_2* value1 = new ValueIR_2();
    value1 -> opcode = "ne";
    value1 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value1 -> operand2 = "0";
    value1 -> target = "%" + std::to_string(this->tmp_symbol);
    this->tmp_symbol++;
    (this->basic_block -> values).push_back(value1);

    // 第二个 ne 指令
    ValueIR_2* value2 = new ValueIR_2();
    value2 -> opcode = "ne";
    value2 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value2 -> operand2 = "0";
    value2 -> target = "%" + std::to_string(this->tmp_symbol);
    this->tmp_symbol++;
    (this->basic_block -> values).push_back(value2);

    // 最后才是按位 or 指令
    ValueIR_2* value3 = new ValueIR_2();
    value3 -> opcode = "or";
    value3 -> operand1 = value1 -> target;
    value3 -> operand2 = value2 -> target;
    value3 -> target = "%" + std::to_string(this->tmp_symbol);

    (this->stk).push(value3 -> target);
    this->tmp_symbol++;

    (this->basic_block -> values).push_back(value3);
    return;
}

void Visitor_ast::ir_init(ConstExpAST& const_exp) {

}