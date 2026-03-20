# include <iostream>
# include "parser.hpp"
# include "sema.hpp"

// CompUnit      ::= CompUnitList;
void Visitor_sema::sema_analysis(CompUnitAST& comp_unit) {
    comp_unit.comp_unit_list.get() -> accept(*this);
}

// CompUnitList  ::= CompUnitItem | CompUnitList CompUnitItem;
void Visitor_sema::sema_analysis(CompUnitListAST& comp_unit_list) {
    // 加入库函数声明
    def_lib_func();
    // 加入全局符号表
    symbol_table_stack.push_table();

    for (int i = 0; i < comp_unit_list.comp_unit_items.size(); i++) {
        comp_unit_list.comp_unit_items[i].get() -> accept(*this);
    }

    // 拿走全局符号表
    symbol_table_stack.pop_table();
}

// CompUnitItem  ::= FuncDef | Decl;
void Visitor_sema::sema_analysis(CompUnitItemAST_1& comp_unit_item) {
    comp_unit_item.func_def.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(CompUnitItemAST_2& comp_unit_item) {
    this -> global_decl = true;
    comp_unit_item.decl.get() -> accept(*this);
    this -> global_decl = false;
}

// Decl          ::= ConstDecl | VarDecl;
void Visitor_sema::sema_analysis(DeclAST_1& decl) {
    decl.constdecl.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(DeclAST_2& decl) {
    decl.vardecl.get() -> accept(*this);
}

// ConstDecl     ::= "const" BType ConstDefList ";";
void Visitor_sema::sema_analysis(ConstDeclAST& const_decl) {
    const_decl.constdeflist.get() -> accept(*this);
}

// BType         ::= "int" | "void";
void Visitor_sema::sema_analysis(BTypeAST& btype) {
    if (this -> if_func_def) {
        func_def_table.add_type(this->func_name, btype.btype);
    }
}

// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
void Visitor_sema::sema_analysis(ConstDefListAST& const_def_list) {
    for (int i = 0; i < const_def_list.constdefs.size(); i++) {
        const_def_list.constdefs[i].get() -> accept(*this);
    }
}

// ConstDef      ::= IDENT "=" ConstInitVal;
void Visitor_sema::sema_analysis(ConstDefAST_1& const_def) {
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

// ConstInitVal  ::= ConstExp;
void Visitor_sema::sema_analysis(ConstInitValAST_1& const_init_val) {
    const_init_val.constexp.get() -> accept(*this);
}

// VarDecl       ::= BType VarDefList ";";
void Visitor_sema::sema_analysis(VarDeclAST& var_decl) {
    var_decl.vardeflist.get() -> accept(*this);
}

// VarDefList    ::= VarDef | VarDefList "," VarDef;
void Visitor_sema::sema_analysis(VarDefListAST& var_def_list) {
    for (int i = 0; i < var_def_list.vardefs.size(); i++) {
        var_def_list.vardefs[i].get() -> accept(*this);
    }
}

// VarDef        ::= IDENT | IDENT "=" InitVal;
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

    if (this -> global_decl) {
        // 设定成计算 const 右侧的模式
        this->cal_mode = true;
        this->error_mode = VAR_UNDF;
        // 计算
        var_def.initval.get() -> accept(*this);
        // 恢复
        this->cal_mode = false;
        this->error_mode = NONE;

        // 修改 AST 树
        // 先新建节点
        auto init_val = std::make_unique<InitValAST_1>();
        auto exp = std::make_unique<ExpAST>();
        auto l_or_exp = std::make_unique<LOrExpAST_1>();
        auto l_and_exp = std::make_unique<LAndExpAST_1>();
        auto eq_exp = std::make_unique<EqExpAST_1>();
        auto rel_exp = std::make_unique<RelExpAST_1>();
        auto add_exp = std::make_unique<AddExpAST_1>();
        auto mul_exp = std::make_unique<MulExpAST_1>();
        auto unary_exp = std::make_unique<UnaryExpAST_1>();
        auto primary_exp = std::make_unique<PrimaryExpAST_3>();
        auto number = std::make_unique<NumberAST>();
        // 获取计算出的结果
        number -> num = (this ->stk).top();
        (this ->stk).pop();
        // 把节点从下向上拼接，进行修改
        primary_exp -> number = std::move(number);
        unary_exp -> primaryexp = std::move(primary_exp);
        mul_exp -> unaryexp = std::move(unary_exp);
        add_exp -> mulexp = std::move(mul_exp);
        rel_exp -> addexp = std::move(add_exp);
        eq_exp -> relexp = std::move(rel_exp);
        l_and_exp -> eqexp = std::move(eq_exp);
        l_or_exp -> landexp = std::move(l_and_exp);
        exp -> lorexp = std::move(l_or_exp);
        init_val -> exp = std::move(exp);
        var_def.initval = std::move(init_val);
    }
    else {
        this->error_mode = UNDF;
        var_def.initval.get() -> accept(*this);
        this->error_mode = UNDF;
    }
}

// InitVal       ::= Exp;
void Visitor_sema::sema_analysis(InitValAST_1& init_val) {
    init_val.exp.get() -> accept(*this);
}

// FuncDef     ::= BType IDENT "(" [FuncFParamList] ")" Block;
void Visitor_sema::sema_analysis(FuncDefAST& func_def) {
    // 函数名相关操作
    // 判断是否重复定义
    if (func_def_table.if_exist(func_def.ident)) {
        std::cout << "Semantic analysis failed: function '" << func_def.ident << "' is redefined or conflict with the name of library function.\n";
        exit(-1);
    }
    // 向函数定义表中加入信息
    // 记下函数名，并将函数名加入表
    this -> func_name = func_def.ident;
    func_def_table.add_func(this -> func_name);
    // 加入返回值
    this -> if_func_def = true;
    func_def.btype.get() -> accept(*this);
    this -> if_func_def = false;

    // 为变量/常量符号表加一层
    symbol_table_stack.push_table();

    // 加入参数
    if (func_def.func_f_param_list)
        func_def.func_f_param_list.get() -> accept(*this);

    // 检查函数定义表是否与函数声明表的内容匹配
    if (func_decl_table.if_exist(this -> func_name)) {
        if (func_decl_table.param_num(this -> func_name) != func_def_table.param_num(this -> func_name)) {
            std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different parameters with its declaration.\n";
            exit(-1);
        }
        if (func_decl_table.if_ret_int(this -> func_name) != func_def_table.if_ret_int(this -> func_name)) {
            std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different return with its declaration.\n";
            exit(-1);
        }

        // 最后把函数声明移除，只留函数定义
        func_decl_table.erase(this -> func_name);
    }

    // 正式进入语句块
    func_def.block.get() -> accept(*this);

    // 离开前去掉
    symbol_table_stack.pop_table();
}

// FuncFParamList ::= FuncFParam | FuncFParamList "," FuncFParam;
void Visitor_sema::sema_analysis(FuncFParamListAST& func_f_param_list) {
    for (int i = 0; i < func_f_param_list.func_f_params.size(); i++) {
        func_def_table.add_param(this->func_name);
        func_f_param_list.func_f_params[i].get() -> accept(*this);
    }
}

// FuncFParam  ::= BType IDENT;
void Visitor_sema::sema_analysis(FuncFParamAST& func_f_param) {
    if (symbol_table_stack.if_exist_last(func_f_param.ident)) {
        std::cout << "Semantic analysis failed: '" << func_f_param.ident << "' redefined.\n";
        exit(-1);
    }

    symbol_table_stack.add_var(func_f_param.ident);

    // 为变量名做注释
    func_f_param.ident = symbol_table_stack.get_var(func_f_param.ident);
}

// Block         ::= "{" BlockItemList "}";
void Visitor_sema::sema_analysis(BlockAST& block) {
    symbol_table_stack.push_table();
    block.blockitemlist.get() -> accept(*this);
    symbol_table_stack.pop_table();
}

// BlockItemList ::= %empty | BlockItemList BlockItem
void Visitor_sema::sema_analysis(BlockItemListAST& block_item_list) {
    for (int i = 0; i < block_item_list.blockitems.size(); i++) {
        block_item_list.blockitems[i].get() -> accept(*this);
    }
}

// BlockItem     ::= Decl | Stmt;
void Visitor_sema::sema_analysis(BlockItemAST_1& block_item) {
    block_item.decl.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(BlockItemAST_2& block_item) {
    block_item.stmt.get() -> accept(*this);
}

// Stmt          ::= MatchedStmt | UnmatchedStmt
// 进行了较大的改动
void Visitor_sema::sema_analysis(StmtAST_1& stmt) {
    stmt.matchedstmt.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(StmtAST_2& stmt) {
    stmt.unmatchedstmt.get() -> accept(*this);
}

// MatchedStmt   ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt | "while" "(" Exp ")" Stmt;
// 复制了之前 Stmt 的四个函数
void Visitor_sema::sema_analysis(MatchedStmtAST_1& matched_stmt) {
    this->error_mode = CONST_UNDF;
    matched_stmt.lval.get() -> accept(*this);

    this->error_mode = UNDF;
    matched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(MatchedStmtAST_2& matched_stmt) {
    if (!matched_stmt.exp && !func_def_table.if_ret_int(this->func_name))
        return;
    else if (!matched_stmt.exp && func_def_table.if_ret_int(this->func_name)) {
        std::cout << "Semantic analysis failed: function '" << this->func_name << "' should return int.\n";
        exit(-1);
    }
    else if (matched_stmt.exp && !func_def_table.if_ret_int(this->func_name)) {
        std::cout << "Semantic analysis failed: function '" << this->func_name << "' should return nothing.\n";
        exit(-1);
    }
    this->error_mode = UNDF;
    matched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(MatchedStmtAST_3& matched_stmt) {
    if (!matched_stmt.exp)
        return;
    this->error_mode = UNDF;
    matched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;
}
void Visitor_sema::sema_analysis(MatchedStmtAST_4& matched_stmt) {
    matched_stmt.block.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(MatchedStmtAST_5& matched_stmt) {
    this->error_mode = UNDF;
    matched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;

    matched_stmt.matchedstmt1.get() -> accept(*this);
    matched_stmt.matchedstmt2.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(MatchedStmtAST_6& matched_stmt) {
    this->error_mode = UNDF;
    matched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;

    // 为了确保 break 和 continue 位于 while 中，否则语义错误
    this->while_levels++;
    matched_stmt.stmt.get() -> accept(*this);
    this->while_levels--;
}
void Visitor_sema::sema_analysis(MatchedStmtAST_7& matched_stmt) { // break
    if (this->while_levels <= 0) {
        std::cout << "Semantic analysis failed: 'break' must be used in the while loop here.\n";
        exit(-1); 
    }
}
void Visitor_sema::sema_analysis(MatchedStmtAST_8& matched_stmt) { // continue
    if (this->while_levels <= 0) {
        std::cout << "Semantic analysis continue: 'break' must be used in the while loop here.\n";
        exit(-1); 
    }
}

// UnmatchedStmt ::= "if" "(" Exp ")" Stmt | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt;
void Visitor_sema::sema_analysis(UnmatchedStmtAST_1& unmatched_stmt) {
    this->error_mode = UNDF;
    unmatched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;

    unmatched_stmt.stmt.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(UnmatchedStmtAST_2& unmatched_stmt) {
    this->error_mode = UNDF;
    unmatched_stmt.exp.get() -> accept(*this);
    this->error_mode = NONE;

    unmatched_stmt.matchedstmt.get() -> accept(*this);
    unmatched_stmt.unmatchedstmt.get() -> accept(*this);
}

// Exp           ::= LOrExp;
void Visitor_sema::sema_analysis(ExpAST& exp) {
    exp.lorexp.get() -> accept(*this);
}

// LVal          ::= IDENT;
void Visitor_sema::sema_analysis(LValAST_1& lval) {
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
            std::cout << "Semantic analysis failed: var '" << lval.ident << "' cannot be used to assign for const or global var.\n";
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

// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
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

// Number        ::= INT_CONST;
void Visitor_sema::sema_analysis(NumberAST& number) {
    if (this->cal_mode) {
        stk.push(number.num);
    }
}

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp | IDENT "(" [FuncRParamList] ")";
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
void Visitor_sema::sema_analysis(UnaryExpAST_3& unary_exp) {
    // 检查函数声明表和定义表
    if (!func_decl_table.if_exist(unary_exp.ident) && !func_def_table.if_exist(unary_exp.ident)) {
        std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' not declared or defined.\n";
        exit(-1);
    }

    if (unary_exp.func_r_param_list) {
        this -> func_call = unary_exp.ident;
        unary_exp.func_r_param_list.get() -> accept(*this);
    }
    else {
        if (func_decl_table.if_exist(unary_exp.ident)) {
            if (func_decl_table.param_num(unary_exp.ident) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' parameter not match when called.\n";
                exit(-1);
            }
        }
        else if (func_def_table.if_exist(unary_exp.ident)) {
            if (func_def_table.param_num(unary_exp.ident) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' parameter not match when called.\n";
                exit(-1);
            }
        }
    }
}

// FuncRParamList ::= Exp | FuncRParamList "," Exp;
void Visitor_sema::sema_analysis(FuncRParamListAST& func_r_param_list) {
    if (func_decl_table.if_exist(this->func_call)) {
        if (func_decl_table.param_num(this->func_call) != func_r_param_list.func_r_params.size()) {
            std::cout << "Semantic analysis failed: function '" << this->func_call << "' parameter not match when called.\n";
            exit(-1);
        }
    }
    else if (func_def_table.if_exist(this->func_call)) {
        if (func_def_table.param_num(this->func_call) != func_r_param_list.func_r_params.size()) {
            std::cout << "Semantic analysis failed: function '" << this->func_call << "' parameter not match when called.\n";
            exit(-1);
        }
    }

    this->error_mode = UNDF;
    for (int i = 0; i < func_r_param_list.func_r_params.size(); i++) {
        func_r_param_list.func_r_params[i].get() -> accept(*this);
    }
    this->error_mode = NONE;
}

// UnaryOp     ::= "+" | "-" | "!";
void Visitor_sema::sema_analysis(UnaryOpAST& unary_op) {
    if (this->cal_mode)
        this->ch = unary_op.ch;
}

// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
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

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
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

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
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

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
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

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
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

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
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

// ConstExp      ::= Exp;
void Visitor_sema::sema_analysis(ConstExpAST& const_exp) {
    const_exp.exp.get() -> accept(*this);
}