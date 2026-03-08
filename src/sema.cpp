# include <iostream>
# include "parser.hpp"
# include "sema.hpp"

void Visitor_sema::sema_analysis(CompUnitAST& comp_unit) {
    comp_unit.func_def.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(DeclAST_1& decl) {
    decl.constdecl.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(DeclAST_2& decl) {
    decl.vardecl.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(ConstDeclAST& const_decl) {
    const_decl.constdeflist.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(ConstDefListAST& const_def_list) {
    for (int i = 0; i < const_def_list.constdefs.size(); i++) {
        const_def_list.constdefs[i].get() -> accept(*this);
    }
}

void Visitor_sema::sema_analysis(ConstDefAST& const_def) {
    if (symbol_table_stack.if_exist_last(const_def.ident)) {
        std::cout << "Semantic analysis failed: '" << const_def.ident << "' redefined.\n";
        exit(-1);
    }

    this->cal_mode = true;
    this->error_mode = VAR_UNDF;

    const_def.constinitval.get() -> accept(*this);
    symbol_table_stack.add_const(const_def.ident, stk.top());
    stk.pop();

    this->cal_mode = false;
    this->error_mode = NONE;
}

void Visitor_sema::sema_analysis(ConstInitValAST& const_init_val) {
    const_init_val.constexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(VarDeclAST& var_decl) {
    var_decl.vardeflist.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(VarDefListAST& var_def_list) {
    for (int i = 0; i < var_def_list.vardefs.size(); i++) {
        var_def_list.vardefs[i].get() -> accept(*this);
    }
}

void Visitor_sema::sema_analysis(VarDefAST_1& var_def) {
    if (symbol_table_stack.if_exist_last(var_def.ident)) {
        std::cout << "Semantic analysis failed: '" << var_def.ident << "' redefined.\n";
        exit(-1);
    }
    symbol_table_stack.add_var(var_def.ident);

    var_def.ident = symbol_table_stack.get_var(var_def.ident);
}
void Visitor_sema::sema_analysis(VarDefAST_2& var_def) {
    if (symbol_table_stack.if_exist_last(var_def.ident)) {
        std::cout << "Semantic analysis failed: '" << var_def.ident << "' redefined.\n";
        exit(-1);
    }
    symbol_table_stack.add_var(var_def.ident);

    var_def.ident = symbol_table_stack.get_var(var_def.ident);

    this->error_mode = UNDF;
    var_def.initval.get() -> accept(*this);
    this->error_mode = UNDF;
}

void Visitor_sema::sema_analysis(InitValAST& init_val) {
    init_val.exp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(FuncDefAST& func_def) {
    func_def.block.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(BlockAST& block) {
    symbol_table_stack.push_table();
    block.blockitemlist.get() -> accept(*this);
    symbol_table_stack.pop_table();
}

void Visitor_sema::sema_analysis(BlockItemListAST& block_item_list) {
    for (int i = 0; i < block_item_list.blockitems.size(); i++) {
        block_item_list.blockitems[i].get() -> accept(*this);
    }
}

void Visitor_sema::sema_analysis(BlockItemAST_1& block_item) {
    block_item.decl.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(BlockItemAST_2& block_item) {
    block_item.stmt.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(StmtAST_1& stmt) {
    this->error_mode = CONST_UNDF;
    stmt.lval.get() -> accept(*this);

    this->error_mode = UNDF;
    stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(StmtAST_2& stmt) {
    if (!stmt.exp)
        return;
    this->error_mode = UNDF;
    stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(StmtAST_3& stmt) {
    if (!stmt.exp)
        return;
    this->error_mode = UNDF;
    stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(StmtAST_4& stmt) {
    stmt.block.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(ExpAST& exp) {
    exp.lorexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(LValAST& lval) {
    // 先进行报错检测
    if (!symbol_table_stack.if_exist_all(lval.ident)) {
        std::cout << "Semantic analysis failed: ident '" << lval.ident << "' not defined.\n";
        exit(-1);
    }

    if (this->error_mode == CONST_UNDF) {
        if (symbol_table_stack.if_const(lval.ident)) {
            std::cout << "Semantic analysis failed: const '" << lval.ident << "' cannot be assigned again.\n";
            exit(-1);
        }
    }
    else if (this->error_mode == VAR_UNDF) {
        if (symbol_table_stack.if_var(lval.ident)) {
            std::cout << "Semantic analysis failed: var '" << lval.ident << "' cannot be used to assign for const.\n";
            exit(-1);
        }
    }

    // 获取 IDENT 对应的值，向上返回，以便替换成 NumberAST
    if (symbol_table_stack.if_const(lval.ident)) {
        this->num = symbol_table_stack.get_const(lval.ident);
        this->if_fold = true;
    }
    else { // 修改变量名
        lval.ident = symbol_table_stack.get_var(lval.ident);
    }
}

void Visitor_sema::sema_analysis(PrimaryExpAST_1& primary_exp) {
    primary_exp.exp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(PrimaryExpAST_2& primary_exp) {
    this->num = 0;
    primary_exp.lval.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(PrimaryExpAST_3& primary_exp) {
    primary_exp.number.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(NumberAST& number) {
    if (this->cal_mode) {
        stk.push(number.num);
    }
}

void Visitor_sema::sema_analysis(UnaryExpAST_1& unary_exp) {
    // 将 LVal 换成 Number 就在这里进行
    this->if_fold = false;
    unary_exp.primaryexp.get() -> accept(*this);

    // 利用了运行时类型判断
    if (this->if_fold && dynamic_cast<PrimaryExpAST_2*>(unary_exp.primaryexp.get())) {
        auto ast1 = std::make_unique<PrimaryExpAST_3>();
        auto ast2 = std::make_unique<NumberAST>();
        ast2 -> num = this->num;
        // 用 std::move 转移所有权
        ast1 -> number = std::move(ast2);

        unary_exp.primaryexp = std::move(ast1);
        // 原来的内存会自动释放

        // 最后不要忘了把值加入计算栈
        if (this->cal_mode)
            stk.push(this->num);
    }
    this->if_fold = false;
}

void Visitor_sema::sema_analysis(UnaryExpAST_2& unary_exp) {
    // 从这里就要开始右序遍历了
    unary_exp.unaryexp.get() -> accept(*this);
    unary_exp.unaryop.get() -> accept(*this);

    if (this->cal_mode) {
        int tmp;
        switch (this->ch) {
            case '+':
                break;
            case '-':
                tmp = -stk.top();
                stk.pop();
                stk.push(tmp);
                break;
            case '!':
                tmp = stk.top()==0 ? 1 : 0;
                stk.pop();
                stk.push(tmp);
                break;
        }
    }
}

void Visitor_sema::sema_analysis(UnaryOpAST& unary_op) {
    if (this->cal_mode)
        this->ch = unary_op.ch;
}

void Visitor_sema::sema_analysis(MulExpAST_1& mul_exp) {
    mul_exp.unaryexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(MulExpAST_2& mul_exp) {
    mul_exp.unaryexp.get() -> accept(*this);
    mul_exp.mulexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        switch (mul_exp.ch) {
            case '*':
                target = num1 * num2;
                break;
            case '/':
                target = num1 / num2;
                break;
            case '%':
                target = num1 % num2;
                break;
        }
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(AddExpAST_1& add_exp) {
    add_exp.mulexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(AddExpAST_2& add_exp) {
    add_exp.mulexp.get() -> accept(*this);
    add_exp.addexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        switch (add_exp.ch) {
            case '+':
                target = num1 + num2;
                break;
            case '-':
                target = num1 - num2;
                break;
        }
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(RelExpAST_1& rel_exp) {
    rel_exp.addexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(RelExpAST_2& rel_exp) {
    rel_exp.addexp.get() -> accept(*this);
    rel_exp.relexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        if (rel_exp.cmp_str == ">")
            target = num1>num2 ? 1 : 0;
        else if (rel_exp.cmp_str == "<")
            target = num1<num2 ? 1 : 0;
        else if (rel_exp.cmp_str == ">=")
            target = num1>=num2 ? 1 : 0;
        else if (rel_exp.cmp_str == "<=")
            target = num1<=num2 ? 1 : 0;
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(EqExpAST_1& eq_exp) {
    eq_exp.relexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(EqExpAST_2& eq_exp) {
    eq_exp.relexp.get() -> accept(*this);
    eq_exp.eqexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        if (eq_exp.cmp_str == "==")
            target = num1==num2 ? 1 : 0;
        else if (eq_exp.cmp_str == "!=")
            target = num1!=num2 ? 1 : 0;
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(LAndExpAST_1& l_and_exp) {
    l_and_exp.eqexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(LAndExpAST_2& l_and_exp) {
    l_and_exp.eqexp.get() -> accept(*this);
    l_and_exp.landexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        target = num1 && num2;
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(LOrExpAST_1& l_or_exp) {
    l_or_exp.landexp.get() -> accept(*this);
}

void Visitor_sema::sema_analysis(LOrExpAST_2& l_or_exp) {
    l_or_exp.landexp.get() -> accept(*this);
    l_or_exp.lorexp.get() -> accept(*this);

    if (this->cal_mode) {
        int num1, num2, target;
        num1 = stk.top();
        stk.pop();
        num2 = stk.top();
        stk.pop();
        target = num1 || num2;
        stk.push(target);
    }
}

void Visitor_sema::sema_analysis(ConstExpAST& const_exp) {
    const_exp.exp.get() -> accept(*this);
}