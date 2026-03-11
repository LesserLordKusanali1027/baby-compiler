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
    // 定义新的函数 IR
    this -> function = new FunctionIR();
    func_def.func_type.get() -> accept(*this);
    this -> function -> name = func_def.ident;
    
    // 先把基本块的定义放在函数里，因为 BlockAST 与 IR 中的基本块并不等同
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%entry";

    // 初始化 then-else-end 的组数，和 return 的组数
    this -> branch_num = 1;
    this -> return_num = 1;
    // 初始化短路求值的状态
    this -> sub_exp = 1;
    this -> and_sce = 1;
    this -> and_exit = 1;
    this -> or_sce = 1;
    this -> or_exit = 1;

    // 定义存放短路求值结果的局部变量
    ValueIR_3* value = new ValueIR_3();
    value -> opcode = "alloc";
    value -> operand = "i32";
    value -> target = this -> sce_var;
    (this -> basic_block -> values).push_back(value);

    func_def.block.get() -> accept(*this);
    // 现在 push 基本块的地方有很多了， FuncDefAST 这里暂时不需要了
    // (function -> basic_blocks).push_back(this -> basic_block);

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

// Stmt          ::= MatchedStmt | UnmatchedStmt
void Visitor_ast::ir_init(StmtAST_1& stmt) {
    stmt.matchedstmt.get() -> accept(*this);
}
void Visitor_ast::ir_init(StmtAST_2& stmt) {
    stmt.unmatchedstmt.get() -> accept(*this);
}

// MatchedStmt   ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt;
void Visitor_ast::ir_init(MatchedStmtAST_1& matched_stmt) {
    this->lval_mode = LOAD;
    matched_stmt.exp.get() -> accept(*this);
    
    this->lval_mode = STORE;
    matched_stmt.lval.get() -> accept(*this);
    this->lval_mode = START;
}
void Visitor_ast::ir_init(MatchedStmtAST_2& matched_stmt) {
    if (!matched_stmt.exp) { // 只有 return; 的时候，默认 return 0
        ValueIR_1* value = new ValueIR_1();
        value -> opcode = "ret";
        value -> operand = "0";
        (this -> basic_block -> values).push_back(value);
        return;
    }

    this->lval_mode = LOAD;
    matched_stmt.exp.get() -> accept(*this);
    this->lval_mode = START;

    ValueIR_1* value = new ValueIR_1();
    value -> opcode = "ret";
    // 但需要判断是直接返回数字还是临时变量，有了栈以后不用了
    value -> operand = (this->stk).top();
    (this->stk).pop();

    (this -> basic_block -> values).push_back(value);
    (this -> function -> basic_blocks).push_back(this -> basic_block);

    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%return_" + std::to_string(this->return_num);
    this->return_num++;

    return;
}
void Visitor_ast::ir_init(MatchedStmtAST_3& matched_stmt) {
    if (matched_stmt.exp) {
        this->lval_mode = LOAD;
        matched_stmt.exp.get() -> accept(*this);
        this->lval_mode = START;
        (this->stk).pop();
    }
}
void Visitor_ast::ir_init(MatchedStmtAST_4& matched_stmt) {
    matched_stmt.block.get() -> accept(*this);
}
void Visitor_ast::ir_init(MatchedStmtAST_5& matched_stmt) {
    this->lval_mode = LOAD;
    matched_stmt.exp.get() -> accept(*this);
    this->lval_mode = START;

    // 准备 br 跳转指令
    std::string then_block = "%then_" + std::to_string(this->branch_num);
    std::string else_block = "%else_" + std::to_string(this->branch_num);
    std::string end_block = "%end_" + std::to_string(this->branch_num);
    this->branch_num++;

    ValueIR_5* value1 = new ValueIR_5();
    value1 -> opcode = "br";
    value1 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value1 -> operand2 = then_block;
    value1 -> operand3 = else_block;

    // 加入指令并新建基本块
    (this -> basic_block -> values).push_back(value1);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = then_block;

    // then 分支
    matched_stmt.matchedstmt1.get() -> accept(*this);
    ValueIR_1* value2 = new ValueIR_1();
    value2 -> opcode = "jump";
    value2 -> operand = end_block;
    (this -> basic_block -> values).push_back(value2);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = else_block;

    // else 分支
    matched_stmt.matchedstmt2.get() -> accept(*this);
    ValueIR_1* value3 = new ValueIR_1();
    value3 -> opcode = "jump";
    value3 -> operand = end_block;
    (this -> basic_block -> values).push_back(value3);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = end_block;
}

// UnmatchedStmt ::= "if" "(" Exp ")" Stmt | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt;
void Visitor_ast::ir_init(UnmatchedStmtAST_1& unmatched_stmt) {
    this->lval_mode = LOAD;
    unmatched_stmt.exp.get() -> accept(*this);
    this->lval_mode = START;

    // 准备基本块名字
    std::string then_block = "%then_" + std::to_string(this->branch_num);
    std::string end_block = "%end_" + std::to_string(this->branch_num);
    this->branch_num++;

    // 准备 br 指令
    ValueIR_5* value1 = new ValueIR_5();
    value1 -> opcode = "br";
    value1 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value1 -> operand2 = then_block;
    value1 -> operand3 = end_block;

    // 加入指令并新建基本块
    (this -> basic_block -> values).push_back(value1);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = then_block;

    // then 分支
    unmatched_stmt.stmt.get() -> accept(*this);
    ValueIR_1* value2 = new ValueIR_1();
    value2 -> opcode = "jump";
    value2 -> operand = end_block;
    (this -> basic_block -> values).push_back(value2);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = end_block;
}
void Visitor_ast::ir_init(UnmatchedStmtAST_2& unmatched_stmt) {
    this->lval_mode = LOAD;
    unmatched_stmt.exp.get() -> accept(*this);
    this->lval_mode = START;

    // 准备基本块名字
    std::string then_block = "%then_" + std::to_string(this->branch_num);
    std::string else_block = "%else_" + std::to_string(this->branch_num);
    std::string end_block = "%end_" + std::to_string(this->branch_num);
    this->branch_num++;

    // 准备 br 指令
    ValueIR_5* value1 = new ValueIR_5();
    value1 -> opcode = "br";
    value1 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value1 -> operand2 = then_block;
    value1 -> operand3 = else_block;

    // 加入指令并新建基本块
    (this -> basic_block -> values).push_back(value1);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = then_block;

    // then 分支
    unmatched_stmt.matchedstmt.get() -> accept(*this);
    ValueIR_1* value2 = new ValueIR_1();
    value2 -> opcode = "jump";
    value2 -> operand = end_block;
    (this -> basic_block -> values).push_back(value2);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = else_block;

    // else 分支
    unmatched_stmt.unmatchedstmt.get() -> accept(*this);
    ValueIR_1* value3 = new ValueIR_1();
    value3 -> opcode = "jump";
    value3 -> operand = end_block;
    (this -> basic_block -> values).push_back(value3);
    (function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = end_block;
}

void Visitor_ast::ir_init(ExpAST& exp) {
    exp.lorexp.get() -> accept(*this);

    if (dynamic_cast<LOrExpAST_2*>(exp.lorexp.get())) { // 给 LOrExpAST_2 善后
        // 先把 or 最后一块 sub_exp 补完
        // 最终计算结果放到 sce_result 中
        ValueIR_4* value1 = new ValueIR_4();
        value1 -> opcode = "store";
        value1 -> operand1 = (this->stk).top();
        (this->stk).pop();
        value1 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value1);
        // jump 到最终块
        ValueIR_1* value2 = new ValueIR_1();
        value2 -> opcode = "jump";
        value2 -> operand = "%or_exit_" + std::to_string(this->or_exit);
        (this -> basic_block -> values).push_back(value2);
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 短路求值的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%or_sce_" + std::to_string(this->or_sce++);
        // 把结果 0 放到 sce_result 中
        ValueIR_4* value3 = new ValueIR_4();
        value3 -> opcode = "store";
        value3 -> operand1 = "1";
        value3 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value3);
        // jump 到最终块
        ValueIR_1* value4 = new ValueIR_1();
        value4 -> opcode = "jump";
        value4 -> operand = "%or_exit_" + std::to_string(this->or_exit);
        (this -> basic_block -> values).push_back(value4);
        // 把块放入函数
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 最终的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%or_exit_" + std::to_string(this->or_exit++);
        // 把 sce_result 加载到新的临时符号中
        ValueIR_3* value5 = new ValueIR_3();
        value5 -> opcode = "load";
        value5 -> operand = this -> sce_var;
        value5 -> target = "%" + std::to_string(this->tmp_symbol++);
        (this -> basic_block -> values).push_back(value5);
        (this->stk).push(value5 -> target);
    }

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
    // 先计算左边
    l_and_exp.landexp.get() -> accept(*this);
    // 此时结果放在 stk 顶

    // br 指令
    ValueIR_5* value1 = new ValueIR_5();
    value1 -> opcode = "br";
    value1 -> operand1 = (this->stk).top();
    value1 -> operand2 = "%sub_exp_" + std::to_string(this->sub_exp);
    value1 -> operand3 = "%and_sce_" + std::to_string(this->and_sce);
    (this -> basic_block -> values).push_back(value1);
    // 建立下一个基本块
    (this -> function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%sub_exp_" + std::to_string(this->sub_exp++);

    // 计算右边
    l_and_exp.eqexp.get() -> accept(*this);

    // 第一个 ne 指令
    ValueIR_2* value2 = new ValueIR_2();
    value2 -> opcode = "ne";
    value2 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value2 -> operand2 = "0";
    value2 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this -> basic_block -> values).push_back(value2);

    // 第二个 ne 指令
    ValueIR_2* value3 = new ValueIR_2();
    value3 -> opcode = "ne";
    value3 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value3 -> operand2 = "0";
    value3 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this -> basic_block -> values).push_back(value3);

    // 最后按位 and 
    ValueIR_2* value4 = new ValueIR_2();
    value4 -> opcode = "and";
    value4 -> operand1 = value2 -> target;
    value4 -> operand2 = value3 -> target;
    value4 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this -> basic_block -> values).push_back(value4);
    // 结果放入 stk
    (this->stk).push(value4 -> target);

    return;
}

void Visitor_ast::ir_init(LOrExpAST_1& l_or_exp) {
    l_or_exp.landexp.get() -> accept(*this);
    if (dynamic_cast<LAndExpAST_2*>(l_or_exp.landexp.get())) {
        // 原本是没有这些的，但是 LAndExpAST_2 短路求值的后半部分加在了这里
        // 先把 and 最后一块 sub_exp 补完
        // 最终计算结果放到 sce_result 中
        ValueIR_4* value1 = new ValueIR_4();
        value1 -> opcode = "store";
        value1 -> operand1 = (this->stk).top();
        (this->stk).pop();
        value1 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value1);
        // jump 到最终块
        ValueIR_1* value2 = new ValueIR_1();
        value2 -> opcode = "jump";
        value2 -> operand = "%and_exit_" + std::to_string(this->and_exit);
        (this -> basic_block -> values).push_back(value2);
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 短路求值的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%and_sce_" + std::to_string(this->and_sce++);
        // 把结果 0 放到 sce_result 中
        ValueIR_4* value3 = new ValueIR_4();
        value3 -> opcode = "store";
        value3 -> operand1 = "0";
        value3 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value3);
        // jump 到最终块
        ValueIR_1* value4 = new ValueIR_1();
        value4 -> opcode = "jump";
        value4 -> operand = "%and_exit_" + std::to_string(this->and_exit);
        (this -> basic_block -> values).push_back(value4);
        // 把块放入函数
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 最终的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%and_exit_" + std::to_string(this->and_exit++);
        // 把 sce_result 加载到新的临时符号中
        ValueIR_3* value5 = new ValueIR_3();
        value5 -> opcode = "load";
        value5 -> operand = this -> sce_var;
        value5 -> target = "%" + std::to_string(this->tmp_symbol++);
        (this -> basic_block -> values).push_back(value5);
        (this->stk).push(value5 -> target);
    }
 
    return;
}
void Visitor_ast::ir_init(LOrExpAST_2& l_or_exp) {
    // 先执行左边
    l_or_exp.lorexp.get() -> accept(*this);
    // 此时结果放在 stk 顶
    
    // 开始处理 || 的短路求值
    // br 指令
    ValueIR_5* value1 = new ValueIR_5();
    value1 -> opcode = "br";
    value1 -> operand1 = (this->stk).top();
    value1 -> operand2 = "%or_sce_" + std::to_string(this->or_sce);
    value1 -> operand3 = "%sub_exp_" + std::to_string(this->sub_exp);
    (this -> basic_block -> values).push_back(value1);
    // 建立下一个基本块
    (this -> function -> basic_blocks).push_back(this -> basic_block);
    this -> basic_block = new BasicBlockIR();
    this -> basic_block -> name = "%sub_exp_" + std::to_string(this->sub_exp++);

    // 执行右边
    l_or_exp.landexp.get() -> accept(*this);
    // 此时结果在 stk 顶
    if (dynamic_cast<LAndExpAST_2*>(l_or_exp.landexp.get())) {
        // 为有短路求值的 LAndExpAST_2 善后
        // 补上 and 最后一个 sub_exp 块
        // 把最终计算结果放到 sce_result 中
        ValueIR_4* value2 = new ValueIR_4();
        value2 -> opcode = "store";
        value2 -> operand1 = (this->stk).top();
        (this->stk).pop();
        value2 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value2);
        // jump 到最终块
        ValueIR_1* value3 = new ValueIR_1();
        value3 -> opcode = "jump";
        value3 -> operand = "%and_exit_" + std::to_string(this->and_exit);
        (this -> basic_block -> values).push_back(value3);
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 短路求值的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%and_sce_" + std::to_string(this->and_sce++);
        // 把结果 0 放到 sce_result 中
        ValueIR_4* value4 = new ValueIR_4();
        value4 -> opcode = "store";
        value4 -> operand1 = "0";
        value4 -> operand2 = this -> sce_var;
        (this -> basic_block -> values).push_back(value4);
        // jump 到最终块
        ValueIR_1* value5 = new ValueIR_1();
        value5 -> opcode = "jump";
        value5 -> operand = "%and_exit_" + std::to_string(this->and_exit);
        (this -> basic_block -> values).push_back(value5);
        // 把块放入函数
        (this -> function -> basic_blocks).push_back(this -> basic_block);

        // 最终的块
        this -> basic_block = new BasicBlockIR();
        this -> basic_block -> name = "%and_exit_" + std::to_string(this->and_exit++);
        // 把 sce_result 加载到新的临时符号中
        ValueIR_3* value6 = new ValueIR_3();
        value6 -> opcode = "load";
        value6 -> operand = this -> sce_var;
        value6 -> target = "%" + std::to_string(this->tmp_symbol++);
        (this -> basic_block -> values).push_back(value6);
        (this->stk).push(value6 -> target);
    }

    // 然后是常规的 2 个 ne 一个 or
    // 第一个 ne 指令
    ValueIR_2* value7 = new ValueIR_2();
    value7 -> opcode = "ne";
    value7 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value7 -> operand2 = "0";
    value7 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this->basic_block -> values).push_back(value7);

    // 第二个 ne 指令
    ValueIR_2* value8 = new ValueIR_2();
    value8 -> opcode = "ne";
    value8 -> operand1 = (this->stk).top();
    (this->stk).pop();
    value8 -> operand2 = "0";
    value8 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this->basic_block -> values).push_back(value8);

    // 最后是按位 or 指令
    ValueIR_2* value9 = new ValueIR_2();
    value9 -> opcode = "or";
    value9 -> operand1 = value7 -> target;
    value9 -> operand2 = value8 -> target;
    value9 -> target = "%" + std::to_string(this->tmp_symbol++);
    (this -> basic_block -> values).push_back(value9);
    // 结果入栈
    (this->stk).push(value9 -> target);

    return;
}

void Visitor_ast::ir_init(ConstExpAST& const_exp) {

}