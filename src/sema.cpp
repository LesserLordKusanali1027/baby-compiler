# include <iostream>
# include "parser.hpp"
# include "sema.hpp"

// 常量数组初始化列表标准化要使用的方法类
class ConstInitVal_Std {
  private:
    std::vector<int> array_size; // 数组大小信息，如 a[2][3][4] 就是 2,3,4
    ConstInitValAST_2* ret_ast; // 最终构造的 AST 的根节点
    std::vector<std::stack<ConstInitValListAST*>*> list_stk; // 存放若干层次的 ConstInitValList
    int assigned_level; // 当前所在 { } 要对齐到的层次，一旦对齐了，之后再加入的数字或 { } 都会被丢弃，直到该 { } 结束
    int last_level; // 记录上一个 { } 对齐到的层次
    std::stack<int> level_stk; // { } 嵌套时存放之前 { } 要对齐的层次
    bool if_drop; // 是否丢弃
    std::stack<bool> if_drop_stk;
    // 对最外层的特殊处理
    bool if_outer;

  public:
    ConstInitVal_Std() {
        ret_ast = NULL;
        assigned_level = -1;
        last_level = 0;
        if_drop = false;
        if_outer = true;
    }

    // 操作 if_drop_stk
    void record_if_drop() {
        if_drop_stk.push(if_drop);
    }
    void recover_if_drop() {
        if_drop = if_drop_stk.top();
        if_drop_stk.pop();
    }

    // 加入数组大小信息
    void Add_Size(int size) {
        array_size.push_back(size);
    }

    // 获取完大小信息后，做构造 AST 前的准备
    void Init_Build() {
        ret_ast = new ConstInitValAST_2();

        // 布置 ConstInitValListAST* 栈
        for (int i = 0; i < array_size.size(); i++) {
            auto ast = new std::stack<ConstInitValListAST*>();
            list_stk.push_back(ast);
        }
        int nums = 1;
        for (int i = 0; i < array_size.size(); i++) {
            for (int j = 0; j < nums; j++) {
                auto ast = new ConstInitValListAST();
                list_stk[i] -> push(ast);
            }
            nums *= array_size[i];
        }
    }

    // 加入数字，返回接收/丢弃
    bool Add_Val(int const_val) {
        if (if_drop)  return false;

        int current_level = array_size.size()-1;
        ConstInitValListAST* small_ast = list_stk[current_level] -> top();
        // 建立新的 ConstInitVal -> Number 的小 AST
        auto const_init_val = new ConstInitValAST_1();
        auto const_exp = std::make_unique<ConstExpAST>();
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
        number -> num = const_val;
        primary_exp -> number = std::move(number);
        unary_exp -> primaryexp = std::move(primary_exp);
        mul_exp -> unaryexp = std::move(unary_exp);
        add_exp -> mulexp = std::move(mul_exp);
        rel_exp -> addexp = std::move(add_exp);
        eq_exp -> relexp = std::move(rel_exp);
        l_and_exp -> eqexp = std::move(eq_exp);
        l_or_exp -> landexp = std::move(l_and_exp);
        exp -> lorexp = std::move(l_or_exp);
        const_exp -> exp = std::move(exp);
        const_init_val -> constexp = std::move(const_exp);
        // 将小 AST 接到 ConstInitValList 上
        (small_ast -> constinitvals).push_back(std::unique_ptr<BaseAST>(const_init_val));

        // 如果下面有已经满了的 ConstInitValList，就拼接到上面
        while (current_level > assigned_level && current_level > 0) {
            if ((list_stk[current_level]->top()->constinitvals).size() == array_size[current_level]) {
                // 为已经完成任务的 ConstInitValList 接上 ConstInitVal
                auto const_init_val_list = list_stk[current_level] -> top();
                list_stk[current_level] -> pop();
                auto const_init_val = new ConstInitValAST_2();
                const_init_val -> constinitvallist = std::unique_ptr<BaseAST>(const_init_val_list);
                // 将 ConstInitVal 放入上一级的 ConstInitValList
                const_init_val_list = list_stk[current_level-1] -> top();
                (const_init_val_list -> constinitvals).push_back(std::unique_ptr<BaseAST>(const_init_val));
                // 向上一级移动
                current_level--;
            }
            else {
                break;
            }
        }

        // 如果对齐了，就可以先停止了
        if (current_level == assigned_level) {
            if_drop = true;
        }
        // 如果是最高级的 ConstInitValList
        else if (current_level == 0 && assigned_level == -1) {
            // 如果最高级的 ConstInitValList 已经满了，将它拼到 ret_ast 下，已经可以准备返回了
            if ((list_stk[0]->top()->constinitvals).size() == array_size[0]) {
                auto const_init_val_list = list_stk[0] -> top();
                list_stk[0] -> pop();

                ret_ast -> constinitvallist = std::unique_ptr<BaseAST>(const_init_val_list);

                if_drop = true;
            }
        }

        return true;
    }

    // 遇到 { ，要判断是否对齐，如果对齐了还要更新 assigned_level
    bool Start_Brace() {
        if (if_outer) {
            // 最外层 Start_Brace 效果
            level_stk.push(assigned_level);
            record_if_drop();
            if_outer = false;
            return true;
        }

        record_if_drop();
        // 已满情况下 { } 是合法的，目前认为不需要 push assigned_level
        if (if_drop)  return true;

        // 若最低级不是 0，就是没对齐，不合法
        int current_level = array_size.size()-1;
        if ((list_stk[current_level]->top()->constinitvals).size() != 0)
            return false;
        // 如果 assigned_level 已经到了最低级，再嵌套也不合法
        if (assigned_level == array_size.size()-2)
            return false;
        
        // 对齐了，就开始找对齐到哪里
        while (current_level > assigned_level+1 && (list_stk[current_level]->top()->constinitvals).size() == 0)
            current_level--;
        // 更新对齐情况
        level_stk.push(assigned_level);
        assigned_level = current_level;
        
        return true;
    }

    // 遇到 } 结束，没全就补 0 直到对齐，
    void End_Brace() {
        // 补 0 对齐
        while (!if_drop)  Add_Val(0);

        // 恢复之前的 if_drop
        recover_if_drop();
        if (if_drop)  return;

        // 恢复之前的对齐情况
        last_level = assigned_level;
        assigned_level = level_stk.top();
        level_stk.pop();

        // 如果 ConstInitValList 直接就满了，还要继续往上拼接
        int current_level = last_level;
        if (current_level > assigned_level && current_level > 0) {
            if ((list_stk[current_level]->top()->constinitvals).size() == array_size[current_level]) {
                // 为已经完成任务的 ConstInitValList 接上 ConstInitVal
                auto const_init_val_list = list_stk[current_level] -> top();
                list_stk[current_level] -> pop();
                auto const_init_val = new ConstInitValAST_2();
                const_init_val -> constinitvallist = std::unique_ptr<BaseAST>(const_init_val_list);
                // 将 ConstInitVal 放入上一级的 ConstInitValList
                const_init_val_list = list_stk[current_level-1] -> top();
                (const_init_val_list -> constinitvals).push_back(std::unique_ptr<BaseAST>(const_init_val));
                // 向上一级移动
                current_level--;
            }
        }

        // 这时对齐了，可以先停止了
        if (current_level == assigned_level) {
            if_drop = true;
        }
        else if (current_level == 0 && assigned_level == -1) {
            // 如果最高级的 ConstInitValList 已经满了，将它拼到 ret_ast 下
            if ((list_stk[0]->top()->constinitvals).size() == array_size[0]) {
                auto const_init_val_list = list_stk[0] -> top();
                list_stk[0] -> pop();

                ret_ast -> constinitvallist = std::unique_ptr<BaseAST>(const_init_val_list);

                if_drop = true;
            }
        }
    }

    ConstInitValAST_2* Get_AST() {
        return ret_ast;
    }
};
// 变量数组初始化列表标准化要使用的方法类
class InitVal_Std {
  private:
    std::vector<int> array_size; // 数组大小信息，如 a[2][3][4] 就是 2,3,4
    InitValAST_2* ret_ast; // 最终构造的 AST 的根节点
    std::vector<std::stack<InitValListAST*>*> list_stk; // 存放若干层次的 ConstInitValList
    int assigned_level; // 当前所在 { } 要对齐到的层次，一旦对齐了，之后再加入的数字或 { } 都会被丢弃，直到该 { } 结束
    int last_level; // 记录上一个 { } 对齐到的层次
    std::stack<int> level_stk; // { } 嵌套时存放之前 { } 要对齐的层次
    bool if_drop; // 是否丢弃
    std::stack<bool> if_drop_stk;
    // 对最外层的特殊处理
    bool if_outer;

  public:
    InitVal_Std() {
        ret_ast = NULL;
        assigned_level = -1;
        last_level = -1;
        if_drop = false;
        if_outer = true;
    }
    
    // 操作 if_drop_stk
    void record_if_drop() {
        if_drop_stk.push(if_drop);
    }
    void recover_if_drop() {
        if_drop = if_drop_stk.top();
        if_drop_stk.pop();
    }

    // 加入数组大小信息
    void Add_Size(int size) {
        array_size.push_back(size);
    }

    // 获取完大小信息后，做构造 AST 前的准备
    void Init_Build() {
        ret_ast = new InitValAST_2();

        // 布置 ConstInitValListAST* 栈
        for (int i = 0; i < array_size.size(); i++) {
            auto ast = new std::stack<InitValListAST*>();
            list_stk.push_back(ast);
        }
        int nums = 1;
        for (int i = 0; i < array_size.size(); i++) {
            for (int j = 0; j < nums; j++) {
                auto ast = new InitValListAST();
                list_stk[i] -> push(ast);
            }
            nums *= array_size[i];
        }
    }

    bool Add_Val(int val) {
        if (if_drop)  return false;

        int current_level = array_size.size()-1;
        InitValListAST* small_ast = list_stk[current_level] -> top();
        // 建立新的 AST 树
        // 先新建节点
        auto init_val = new InitValAST_1();
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
        number -> num = val;
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
        (small_ast -> initvals).push_back(std::unique_ptr<BaseAST>(init_val));

        // 如果有已经满了的 InitValList，就拼接到上面
        while (current_level > assigned_level && current_level > 0) {
            if ((list_stk[current_level]->top()->initvals).size() == array_size[current_level]) {
                // 为已经完成任务的 InitValList 接上 InitVal
                auto init_val_list = list_stk[current_level] -> top();
                list_stk[current_level] -> pop();
                auto init_val = new InitValAST_2();
                init_val -> initvallist = std::unique_ptr<BaseAST>(init_val_list);
                // 将 InitVal 放入上一级的 InitValList
                init_val_list = list_stk[current_level-1] -> top();
                (init_val_list -> initvals).push_back(std::unique_ptr<BaseAST>(init_val));
                // 向上一级移动
                current_level--;
            }
            else {
                break;
            }
        }
        // 如果对齐了，就停止
        if (current_level == assigned_level) {
            if_drop = true;
        }
        // 如果是最高级的 InitValList
        else if (current_level == 0 && assigned_level == -1) {
            // 如果最高级的 ConstInitValList 已经满了，将它拼到 ret_ast 下，已经可以准备返回了
            if ((list_stk[0]->top()->initvals).size() == array_size[0]) {
                auto init_val_list = list_stk[0] -> top();
                list_stk[0] -> pop();

                ret_ast -> initvallist = std::unique_ptr<BaseAST>(init_val_list);

                if_drop = true;
            }
        }
        return true;
    }

    bool Add_Val_AST(std::unique_ptr<BaseAST> init_val_ast) { // 和 const 比增加了传入已有 ast 的情况，用于局部变量数组初始化
        if (if_drop)  return false;

        int current_level = array_size.size()-1;
        InitValListAST* small_ast = list_stk[current_level] -> top();
        // 将 init_val 放入
        (small_ast -> initvals).push_back(std::move(init_val_ast));

        // 然后就是一系列善后了
        // 如果下面有已经满了的 InitValList，就拼接到上面
        while (current_level > assigned_level && current_level > 0) {
            if ((list_stk[current_level]->top()->initvals).size() == array_size[current_level]) {
                // 为已经完成任务的 InitValList 接上 InitVal
                auto init_val_list = list_stk[current_level] -> top();
                list_stk[current_level] -> pop();
                auto init_val = new InitValAST_2();
                init_val -> initvallist = std::unique_ptr<BaseAST>(init_val_list);
                // 将 InitVal 放入上一级的 InitValList
                init_val_list = list_stk[current_level-1] -> top();
                (init_val_list -> initvals).push_back(std::unique_ptr<BaseAST>(init_val));
                // 向上一级移动
                current_level--;
            }
            else {
                break;
            }
        }
        // 如果对齐了，就可以先停止了
        if (current_level == assigned_level) {
            if_drop = true;
        }
        // 如果是最高级的 InitValList
        else if (current_level == 0 && assigned_level == -1) {
            // 如果最高级的 ConstInitValList 已经满了，将它拼到 ret_ast 下，已经可以准备返回了
            if ((list_stk[0]->top()->initvals).size() == array_size[0]) {
                auto init_val_list = list_stk[0] -> top();
                list_stk[0] -> pop();

                ret_ast -> initvallist = std::unique_ptr<BaseAST>(init_val_list);

                if_drop = true;
            }
        }

        return true;
    }

    // 遇到 { ，要判断是否对齐，如果对齐了还要更新 assigned_level
    bool Start_Brace() {
        if (if_outer) {
            // 最外层 Start_Brace 效果
            level_stk.push(assigned_level);
            record_if_drop();
            if_outer = false;
            return true;
        }

        record_if_drop();
        // 已满情况下 { } 是合法的，目前认为不需要 push assigned_level
        if (if_drop)  return true;

        // 若最低级不是 0，就是没对齐，不合法
        int current_level = array_size.size()-1;
        if ((list_stk[current_level]->top()->initvals).size() != 0)
            return false;
        // 如果 assigned_level 已经到了最低级，再嵌套也不合法
        if (assigned_level == array_size.size()-2)
            return false;
        
        // 对齐了，就开始找对齐到哪里
        while (current_level > assigned_level+1 && (list_stk[current_level]->top()->initvals).size() == 0)
            current_level--;
        // 更新对齐情况
        level_stk.push(assigned_level);
        assigned_level = current_level;
        
        return true;
    }

    // 遇到 } 结束，没全就补 0 直到对齐，
    void End_Brace() {
        // 补 0 对齐
        while (!if_drop)  Add_Val(0);

        // 恢复外层的 if_drop
        recover_if_drop();
        if (if_drop)  return;

        // 恢复之前的对齐情况
        last_level = assigned_level;
        assigned_level = level_stk.top();
        level_stk.pop();

        // 如果 ConstInitValList 直接就满了，还要继续往上拼接
        int current_level = last_level;
        if (current_level > assigned_level && current_level > 0) {
            if ((list_stk[current_level]->top()->initvals).size() == array_size[current_level]) {
                // 为已经完成任务的 ConstInitValList 接上 ConstInitVal
                auto init_val_list = list_stk[current_level] -> top();
                list_stk[current_level] -> pop();
                auto init_val = new InitValAST_2();
                init_val -> initvallist = std::unique_ptr<BaseAST>(init_val_list);
                // 将 ConstInitVal 放入上一级的 ConstInitValList
                init_val_list = list_stk[current_level-1] -> top();
                (init_val_list -> initvals).push_back(std::unique_ptr<BaseAST>(init_val));
                // 向上一级移动
                current_level--;
            }
        }

        // 这时对齐了，可以先停止了
        if (current_level == assigned_level) {
            if_drop = true;
        }
        else if (current_level == 0 && assigned_level == -1) {
            // 如果最高级的 ConstInitValList 已经满了，将它拼到 ret_ast 下
            if ((list_stk[0]->top()->initvals).size() == array_size[0]) {
                auto init_val_list = list_stk[0] -> top();
                list_stk[0] -> pop();

                ret_ast -> initvallist = std::unique_ptr<BaseAST>(init_val_list);

                if_drop = true;
            }
        }
    }

    InitValAST_2* Get_AST() {
        return ret_ast;
    }
};

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

// CompUnitItem  ::= FuncDef | Decl | FuncDecl;
void Visitor_sema::sema_analysis(CompUnitItemAST_1& comp_unit_item) {
    comp_unit_item.func_def.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(CompUnitItemAST_2& comp_unit_item) {
    this -> global_decl = true;
    comp_unit_item.decl.get() -> accept(*this);
    this -> global_decl = false;
}
void Visitor_sema::sema_analysis(CompUnitItemAST_3& comp_unit_item) {
    comp_unit_item.funcdecl.get() -> accept(*this);
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
    else if (this -> if_func_decl) {
        func_decl_table.add_type(this->func_name, btype.btype);
    }
}

// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
void Visitor_sema::sema_analysis(ConstDefListAST& const_def_list) {
    for (int i = 0; i < const_def_list.constdefs.size(); i++) {
        const_def_list.constdefs[i].get() -> accept(*this);
    }
}

// ConstDef      ::= IDENT "=" ConstInitVal | IDENT ConstSizeList "=" ConstInitVal;
void Visitor_sema::sema_analysis(ConstDefAST_1& const_def) {
    if (symbol_table_stack.if_exist_last(const_def.ident)) {
        std::cout << "Semantic analysis failed: '" << const_def.ident << "' redefined.\n";
        exit(-1);
    }

    // 类型检查，赋值语句不能是 ConstInitValAST_2
    if (dynamic_cast<ConstInitValAST_2*>(const_def.constinitval.get())) {
        std::cout << "Semantic analysis failed: const '" << const_def.ident << "' cannot be initialized as a const array.\n";
        exit(-1);
    }

    this->cal_mode = true;
    add_error_mode(VAR_CARRAY_ARRAY_UNDF);

    const_def.constinitval.get() -> accept(*this);
    symbol_table_stack.add_const(const_def.ident, stk.top());
    stk.pop();

    this->cal_mode = false;
    recover_error_mode();
}
void Visitor_sema::sema_analysis(ConstDefAST_2& const_def) {
    // 符号名有重复就报错，数组也一样
    if (symbol_table_stack.if_exist_last(const_def.ident)) {
        std::cout << "Semantic analysis failed: '" << const_def.ident << "' redefined.\n";
        exit(-1);
    }

    // 类型检查，赋值语句不能是 ConstInitValAST_1
    if (dynamic_cast<ConstInitValAST_1*>(const_def.constinitval.get())) {
        std::cout << "Semantic analysis failed: const array '" << const_def.ident << "' cannot be initialized as a const.\n";
        exit(-1);
    }

    // 记录数组名用于之后的报错、维度记录
    this -> array_name = const_def.ident;

    // 将常量数组名加入符号表，并改掉原来的名字
    symbol_table_stack.add_const_array(const_def.ident);
    const_def.ident = symbol_table_stack.get_const_array(const_def.ident);
    
    // 准备将初始化列表标准化
    this -> const_init_val_std = new ConstInitVal_Std();
    // 数组大小
    const_def.constsizelist.get() -> accept(*this);
    // 准备
    const_init_val_std -> Init_Build();
    // 前往初始化
    const_def.constinitval.get() -> accept(*this);
    // 导出并替换 constinitval
    const_def.constinitval = std::unique_ptr<BaseAST>(const_init_val_std -> Get_AST());
    // delete const_init_val_std; 先别这么干了，我怕把 AST 给删了
}

// ConstSizeList ::= "[" ConstExp "]" | ConstSizeList "[" ConstExp "]";
void Visitor_sema::sema_analysis(ConstSizeListAST& const_size_list) {
    // 记录常量数组维度
    symbol_table_stack.add_array_dim(this->array_name, const_size_list.constexps.size());

    for (int i = 0; i < const_size_list.constexps.size(); i++) {
        this -> cal_mode = true;
        add_error_mode(VAR_CARRAY_ARRAY_UNDF);
        const_size_list.constexps[i].get() -> accept(*this);
        this -> cal_mode = false;
        recover_error_mode();

        // 将大小存入 ConstInitVal_Std
        const_init_val_std -> Add_Size((this -> stk).top());

        // 修改 AST 树
        // 先新建节点
        auto const_exp = std::make_unique<ConstExpAST>();
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
        const_exp -> exp = std::move(exp);
        // 接到上面，就结束咯
        const_size_list.constexps[i] = std::move(const_exp);
    }
}

// ConstInitVal  ::= ConstExp | "{" [ConstInitValList] "}";
void Visitor_sema::sema_analysis(ConstInitValAST_1& const_init_val) {
    const_init_val.constexp.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(ConstInitValAST_2& const_init_val) {
    // 进入 '{' 并判断是否对齐
    bool if_correct = this -> const_init_val_std -> Start_Brace();
    if (!if_correct) {
        std::cout << "Semantic analysis failed: const array '" << this->array_name << "' has the wrong initialization list.\n";
        exit(-1);
    }

    if (!const_init_val.constinitvallist) {
        // 没东西，直接补 0 结束 '}'
        this -> const_init_val_std -> End_Brace();
        return;
    }
    
    const_init_val.constinitvallist.get() -> accept(*this);
    // 结束 '}'
    this -> const_init_val_std -> End_Brace();
    return;
}

// ConstInitValList ::= ConstInitVal | ConstInitValList "," ConstInitVal;
void Visitor_sema::sema_analysis(ConstInitValListAST& const_init_val_list) {
    for (int i = 0; i < const_init_val_list.constinitvals.size(); i++) {
        // 如果下面是 ConstExp，求值
        if (dynamic_cast<ConstInitValAST_1*>(const_init_val_list.constinitvals[i].get())) {
            // 求值
            this -> cal_mode = true;
            add_error_mode(VAR_CARRAY_ARRAY_UNDF);
            const_init_val_list.constinitvals[i].get() -> accept(*this);
            this -> cal_mode = false;
            recover_error_mode();

            // 将结果存入 AST
            this -> const_init_val_std -> Add_Val((this->stk).top());
            (this -> stk).pop();
        }
        else { // 如果是 { }，进入
            const_init_val_list.constinitvals[i].get() -> accept(*this);
        }
    }
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

// VarDef        ::= IDENT 
//                 | IDENT "=" InitVal
//                 | IDENT VarSizeList
//                 | IDENT VarSizeList "=" InitVal;
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

    // 类型检测，普通常量的赋值语句不能是 InitValAST_2
    if (dynamic_cast<InitValAST_2*>(var_def.initval.get())) {
        std::cout << "Semantic analysis failed: var '" << var_def.ident << "' cannot be initialized as a var array.\n";
        exit(-1);
    }

    symbol_table_stack.add_var(var_def.ident);
    var_def.ident = symbol_table_stack.get_var(var_def.ident);

    if (this -> global_decl) {
        // 设定成计算 const 右侧的模式
        this->cal_mode = true;
        add_error_mode(VAR_CARRAY_ARRAY_UNDF);
        // 计算
        var_def.initval.get() -> accept(*this);
        // 恢复
        this->cal_mode = false;
        recover_error_mode();

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
        add_error_mode(CARRAY_ARRAY_UNDF);
        var_def.initval.get() -> accept(*this);
        recover_error_mode();
    }
}
void Visitor_sema::sema_analysis(VarDefAST_3& var_def) {
    if (symbol_table_stack.if_exist_last(var_def.ident)) {
        std::cout << "Semantic analysis failed: '" << var_def.ident << "' redefined.\n";
        exit(-1);
    }

    // 记录数组名用于之后的维度记录
    this -> array_name = var_def.ident;

    // 将变量数组名加入符号表，并改掉原来的名字
    symbol_table_stack.add_var_array(var_def.ident);
    var_def.ident = symbol_table_stack.get_var_array(var_def.ident);

    this -> init_val_std = new InitVal_Std();
    var_def.varsizelist.get() -> accept(*this);
    delete init_val_std;
}
void Visitor_sema::sema_analysis(VarDefAST_4& var_def) {
    if (symbol_table_stack.if_exist_last(var_def.ident)) {
        std::cout << "Semantic analysis failed: '" << var_def.ident << "' redefined.\n";
        exit(-1);
    }

    // 类型检测，常量数组的赋值语句不能是 InitValAST_1
    if (dynamic_cast<InitValAST_1*>(var_def.initval.get())) {
        std::cout << "Semantic analysis failed: var array '" << var_def.ident << "' cannot be initialized as a var.\n";
        exit(-1);
    }

    // 记录数组名用于之后的报错、维度记录
    this -> array_name = var_def.ident;

    // 将变量数组名加入符号表，并改掉原来的名字
    symbol_table_stack.add_var_array(var_def.ident);
    var_def.ident = symbol_table_stack.get_var_array(var_def.ident);

    // 准备将初始化列表标准化
    this -> init_val_std = new InitVal_Std();
    // 数组大小
    var_def.varsizelist.get() -> accept(*this);
    // 准备
    init_val_std -> Init_Build();
    // 前往初始化
    var_def.initval.get() -> accept(*this);
    // 导出并替换 constinitval
    var_def.initval = std::unique_ptr<BaseAST>(init_val_std -> Get_AST());
}

// VarSizeList   ::= "[" ConstExp "]" | VarSizeList "[" ConstExp "]";
void Visitor_sema::sema_analysis(VarSizeListAST& var_size_list) {
    // 记录维度
    symbol_table_stack.add_array_dim(this->array_name, var_size_list.constexps.size());

    for (int i = 0; i < var_size_list.constexps.size(); i++) {
        // 对数组大小做常量折叠
        this -> cal_mode = true;
        add_error_mode(VAR_CARRAY_ARRAY_UNDF);
        var_size_list.constexps[i].get() -> accept(*this);
        this -> cal_mode = false;
        recover_error_mode();

        // 将大小存入 InitVal_Std
        this -> init_val_std -> Add_Size((this -> stk).top());

        // 修改 AST 树
        // 先新建节点
        auto const_exp = std::make_unique<ConstExpAST>();
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
        number -> num = (this -> stk).top();
        (this -> stk).pop();
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
        const_exp -> exp = std::move(exp);
        var_size_list.constexps[i] = std::move(const_exp);
    }
}

// InitVal       ::= Exp | "{" [InitValList] "}";
void Visitor_sema::sema_analysis(InitValAST_1& init_val) {
    init_val.exp.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(InitValAST_2& init_val) {
    // 进入 '{'
    bool if_correct = init_val_std -> Start_Brace();
    if (!if_correct) {
        std::cout << "Semantic analysis failed: var array '" << this->array_name << "' has the wrong initialization list.\n";
        exit(-1);
    }

    // 如果是空的，直接结束
    if (!init_val.initvallist) {
        init_val_std -> End_Brace();
        return;
    }
    
    // 进入 InitValList
    init_val.initvallist.get() -> accept(*this);

    // 离开 '}'
    init_val_std -> End_Brace();
    return;
}

// InitValList   ::= InitVal | InitValList "," InitVal;
void Visitor_sema::sema_analysis(InitValListAST& init_val_list) {
    // 全局变量数组，求值替换
    if (this -> global_decl) {
        for (int i = 0; i < init_val_list.initvals.size(); i++) {
            if (dynamic_cast<InitValAST_1*>(init_val_list.initvals[i].get())) {
                // 求值
                this -> cal_mode = true;
                add_error_mode(VAR_CARRAY_ARRAY_UNDF);
                init_val_list.initvals[i].get() -> accept(*this);
                this -> cal_mode = false;
                recover_error_mode();

                // 将结果存入 InitVal_Std
                this -> init_val_std -> Add_Val((this -> stk).top());
                (this -> stk).pop();
            }
            else { // 为 { } 则进入
                init_val_list.initvals[i].get() -> accept(*this);
            }
        }
    }
    else { // 局部变量数组，要把 AST 的节点拼到新 AST 上
        for (int i = 0; i < init_val_list.initvals.size(); i++) {
            if (dynamic_cast<InitValAST_1*>(init_val_list.initvals[i].get())) {
                // 不计算，只折叠常量
                add_error_mode(CARRAY_ARRAY_UNDF);
                init_val_list.initvals[i].get() -> accept(*this);
                recover_error_mode();

                // 将节点拼入 InitVal_Std
                // 即使是传参也要用 std::move 转移所有权
                this -> init_val_std -> Add_Val_AST(std::move(init_val_list.initvals[i]));
            }
            else {
                init_val_list.initvals[i] -> accept(*this);
            }
        }
    }
}

// FuncDef ::= BType IDENT "(" [FuncFParamList] ")" Block;
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
    if (func_def.func_f_param_list) // 这里会用到记下的函数名
        func_def.func_f_param_list.get() -> accept(*this);

    // 检查函数定义表是否与函数声明表的内容匹配
    if (func_decl_table.if_exist(this -> func_name)) {
        // 判断参数个数是否匹配
        if (func_decl_table.get_param_num(this -> func_name) != func_def_table.get_param_num(this -> func_name)) {
            std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different parameters with its declaration.\n";
            exit(-1);
        }
        // 返回值是否匹配
        if (func_decl_table.if_ret_int(this -> func_name) != func_def_table.if_ret_int(this -> func_name)) {
            std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different return with its declaration.\n";
            exit(-1);
        }

        // 每个参数的维度是否匹配
        int param_num = func_decl_table.get_param_num(this -> func_name);
        for (int i = 0; i < param_num; i++) {
            if (func_decl_table.get_param_dim(this -> func_name, i) != func_def_table.get_param_dim(this -> func_name, i)) {
                std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different parameter dimension with its declaration.\n";
                exit(-1);
            }

            // 每个参数的每个维度的大小是否匹配
            int dim = func_decl_table.get_param_dim(this -> func_name, i);
            for (int j = 0; j < dim-1; j++) {
                if (func_decl_table.get_param_size(this -> func_name, i, j) != func_def_table.get_param_size(this -> func_name, i, j)) {
                    std::cout << "Semantic analysis failed: function '" << this -> func_name << "' definition has different parameter size with its declaration.\n";
                    exit(-1);
                }
            }
        }

        // 最后把函数声明移除，只留函数定义
        func_decl_table.erase(this -> func_name);
    }

    // 正式进入语句块
    func_def.block.get() -> accept(*this);

    // 离开前去掉
    symbol_table_stack.pop_table();
}

// FuncDecl      ::= BType IDENT "(" [FuncFParamList] ")" ";";
void Visitor_sema::sema_analysis(FuncDeclAST& func_decl) {
    // 判断是否已经定义了/声明了
    if (func_def_table.if_exist(func_decl.ident)) {
        std::cout << "Semantic analysis failed: function '" << func_decl.ident << "' is already defined or conflict with the name of library function.\n";
        exit(-1);
    }
    if (func_decl_table.if_exist(func_decl.ident)) {
        std::cout << "Semantic analysis failed: function '" << func_decl.ident << "' is already declared.\n";
        exit(-1);
    }

    this -> if_func_decl = true;

    // 向函数声明表中加入信息
    // 加入函数名
    this -> func_name = func_decl.ident;
    func_decl_table.add_func(this -> func_name);

    // 加入返回值
    func_decl.btype.get() -> accept(*this);

    // 为变量/常量符号表加一层
    symbol_table_stack.push_table();

    // 记录参数
    if (func_decl.func_f_param_list)
        func_decl.func_f_param_list.get() -> accept(*this);

    symbol_table_stack.pop_table();
    this -> if_func_decl = false;
}

// FuncFParamList ::= FuncFParam | FuncFParamList "," FuncFParam;
void Visitor_sema::sema_analysis(FuncFParamListAST& func_f_param_list) {
    for (int i = 0; i < func_f_param_list.func_f_params.size(); i++) {
        (this -> func_param_index).push(i); // 记录某个参数的大小时可能要用
        func_f_param_list.func_f_params[i].get() -> accept(*this);
        (this -> func_param_index).pop();
    }
}

// FuncFParam    ::= BType IDENT | BType IDENT "[" "]" [ConstExpList];
void Visitor_sema::sema_analysis(FuncFParamAST_1& func_f_param) {
    if (symbol_table_stack.if_exist_last(func_f_param.ident)) {
        std::cout << "Semantic analysis failed: '" << func_f_param.ident << "' redefined.\n";
        exit(-1);
    }

    // 函数声明/定义
    if (this -> if_func_decl) {
        func_decl_table.add_param(this->func_name, 0);
        symbol_table_stack.add_var(func_f_param.ident);
        // 不用为变量名做注释了
    }
    else {
        // 将参数加入函数信息表
        func_def_table.add_param(this->func_name, 0); // 0 维的参数

        symbol_table_stack.add_var(func_f_param.ident);
        // 为变量名做注释
        func_f_param.ident = symbol_table_stack.get_var(func_f_param.ident);
    }
}
void Visitor_sema::sema_analysis(FuncFParamAST_2& func_f_param) {
    if (symbol_table_stack.if_exist_last(func_f_param.ident)) {
        std::cout << "Semantic analysis failed: '" << func_f_param.ident << "' redefined.\n";
        exit(-1);
    }

    if (this -> if_func_decl) {
        // 加入符号表
        symbol_table_stack.add_var_array(func_f_param.ident);

        // 如果只有 [] ，维度为 1
        if (!func_f_param.constexplist) {
            func_decl_table.add_param(this->func_name, 1);
            symbol_table_stack.add_array_dim(func_f_param.ident, 1);
        }
        else {
            this -> array_name = func_f_param.ident;
            func_f_param.constexplist.get() -> accept(*this);
        }
        // 同样不用为变量名做注释了
    }
    else {
        // 加入符号表
        symbol_table_stack.add_var_array(func_f_param.ident);

        // 如果只有 [] ，维度为 1
        if (!func_f_param.constexplist) {
            func_def_table.add_param(this->func_name, 1);
            symbol_table_stack.add_array_dim(func_f_param.ident, 1);
        }
        else {
            this -> array_name = func_f_param.ident;
            func_f_param.constexplist.get() -> accept(*this);
        }

        // 为变量名做注释
        func_f_param.ident = symbol_table_stack.get_var_array(func_f_param.ident);
    }
}

// ConstExpList ::= "[" ConstExp "]" | ConstExpList "[" ConstExp "]";
void Visitor_sema::sema_analysis(ConstExpListAST& const_exp_list) {
    // 将参数维度加入函数声明/定义表
    if (this -> if_func_decl) {
        func_decl_table.add_param(this->func_name, const_exp_list.constexps.size()+1);
    }
    else {
        func_def_table.add_param(this->func_name, const_exp_list.constexps.size()+1);
    }
    // 将数组变量维度加入符号表
    symbol_table_stack.add_array_dim(this->array_name, const_exp_list.constexps.size()+1);

    for (int i = 0; i < const_exp_list.constexps.size(); i++) {
        // 先计算常量
        this -> cal_mode = true;
        add_error_mode(VAR_CARRAY_ARRAY_UNDF);
        const_exp_list.constexps[i].get() -> accept(*this);
        this -> cal_mode = false;
        recover_error_mode();

        // 获得结果
        int size = (this -> stk).top();
        (this -> stk).pop();

        // 修改 AST
        auto const_exp = std::make_unique<ConstExpAST>();
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
        number -> num = size;
        primary_exp -> number = std::move(number);
        unary_exp -> primaryexp = std::move(primary_exp);
        mul_exp -> unaryexp = std::move(unary_exp);
        add_exp -> mulexp = std::move(mul_exp);
        rel_exp -> addexp = std::move(add_exp);
        eq_exp -> relexp = std::move(rel_exp);
        l_and_exp -> eqexp = std::move(eq_exp);
        l_or_exp -> landexp = std::move(l_and_exp);
        exp -> lorexp = std::move(l_or_exp);
        const_exp -> exp = std::move(exp);
        const_exp_list.constexps[i] = std::move(const_exp);

        // 将参数大小加入
        if (this -> if_func_decl) {
            func_decl_table.add_size(this->func_name, (this->func_param_index).top(), size);
        }
        else {
            func_def_table.add_size(this->func_name, (this->func_param_index).top(), size);
        }
    }
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
    add_error_mode(CONST_CARRAY_ARRAY_UNDF);
    matched_stmt.lval.get() -> accept(*this);
    recover_error_mode();

    add_error_mode(CARRAY_ARRAY_UNDF);
    matched_stmt.exp.get() -> accept(*this);
    recover_error_mode();
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
    add_error_mode(CARRAY_ARRAY_UNDF);
    matched_stmt.exp.get() -> accept(*this);
    recover_error_mode();
}
void Visitor_sema::sema_analysis(MatchedStmtAST_3& matched_stmt) {
    if (!matched_stmt.exp)
        return;
    add_error_mode(CARRAY_ARRAY_UNDF);
    matched_stmt.exp.get() -> accept(*this);
    recover_error_mode();
}
void Visitor_sema::sema_analysis(MatchedStmtAST_4& matched_stmt) {
    matched_stmt.block.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(MatchedStmtAST_5& matched_stmt) {
    add_error_mode(CARRAY_ARRAY_UNDF);
    matched_stmt.exp.get() -> accept(*this);
    recover_error_mode();

    matched_stmt.matchedstmt1.get() -> accept(*this);
    matched_stmt.matchedstmt2.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(MatchedStmtAST_6& matched_stmt) {
    add_error_mode(CARRAY_ARRAY_UNDF);
    matched_stmt.exp.get() -> accept(*this);
    recover_error_mode();

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
    add_error_mode(CARRAY_ARRAY_UNDF);
    unmatched_stmt.exp.get() -> accept(*this);
    recover_error_mode();

    unmatched_stmt.stmt.get() -> accept(*this);
}
void Visitor_sema::sema_analysis(UnmatchedStmtAST_2& unmatched_stmt) {
    add_error_mode(CARRAY_ARRAY_UNDF);
    unmatched_stmt.exp.get() -> accept(*this);
    recover_error_mode();

    unmatched_stmt.matchedstmt.get() -> accept(*this);
    unmatched_stmt.unmatchedstmt.get() -> accept(*this);
}

// Exp           ::= LOrExp;
void Visitor_sema::sema_analysis(ExpAST& exp) {
    exp.lorexp.get() -> accept(*this);
}

// LVal          ::= IDENT | IDENT ExpList;
void Visitor_sema::sema_analysis(LValAST_1& lval) {
    // 先进行报错检测
    // 检查是否未定义
    if (!symbol_table_stack.if_exist_all(lval.ident)) {
        std::cout << "Semantic analysis failed: ident '" << lval.ident << "' not defined.\n";
        exit(-1);
    }

    if (get_error_mode() == CONST_CARRAY_ARRAY_UNDF) { // 赋值语句左侧
        if (symbol_table_stack.if_const(lval.ident)) {
            std::cout << "Semantic analysis failed: const '" << lval.ident << "' cannot be assigned again.\n";
            exit(-1);
        }
        else if (symbol_table_stack.if_const_array(lval.ident)) {
            std::cout << "Semantic analysis failed: const array '" << lval.ident << "' can neither be assigned again nor be assigned as a whole.\n";
            exit(-1);
        }
        else if (symbol_table_stack.if_var_array(lval.ident)) {
            std::cout << "Semantic analysis failed: var array '" << lval.ident << "' cannot be assigned as a whole.\n";
            exit(-1);
        }
    }
    else if (get_error_mode() == VAR_CARRAY_ARRAY_UNDF) { // 位于常量表达式中的时候
        if (symbol_table_stack.if_var(lval.ident)) {
            std::cout << "Semantic analysis failed: var '" << lval.ident << "' cannot be used to assign for const or global var.\n";
            exit(-1);
        }
        else if (symbol_table_stack.if_const_array(lval.ident)) {
            std::cout << "Semantic analysis failed: const array '" << lval.ident << "' can neither be used to assign for const or global var nor be used as a whole here.\n";
            exit(-1);
        }
        else if (symbol_table_stack.if_var_array(lval.ident)) {
            std::cout << "Semantic analysis failed: var array '" << lval.ident << "' can neither be used to assign for const or global var nor be used as a whole here.\n";
            exit(-1);
        }
    }
    else if (get_error_mode() == CARRAY_ARRAY_UNDF) { // 位于普通表达式中时
        if (symbol_table_stack.if_const_array(lval.ident)) {
            std::cout << "Semantic analysis failed: const array '" << lval.ident << "' cannot be used as a whole here.\n";
            exit(-1);
        }
        else if (symbol_table_stack.if_var_array(lval.ident)) {
            std::cout << "Semantic analysis failed: var array '" << lval.ident << "' cannot be used as a whole here.\n";
            exit(-1);
        }
    }
    else if (get_error_mode() == PARAM_UNDF) { // 作为函数参数时
        if (symbol_table_stack.if_const(lval.ident)) {
            int param_dim = -1;
            if (func_decl_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            else if (func_def_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }

            if (param_dim != 0) {
                std::cout << "Semantic analysis failed: const '" << lval.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
        else if (symbol_table_stack.if_var(lval.ident)) {
            int param_dim = -1;
            if (func_decl_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            else if (func_def_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }

            if (param_dim != 0) {
                std::cout << "Semantic analysis failed: var '" << lval.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
        else if (symbol_table_stack.if_const_array(lval.ident)) {
            int array_dim = (this -> symbol_table_stack).get_array_dim(lval.ident);
            int param_dim = -1;
            if (func_decl_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            else if (func_def_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            
            if (param_dim != array_dim) {
                std::cout << "Semantic analysis failed: const array '" << lval.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
        else if (symbol_table_stack.if_var_array(lval.ident)) {
            int array_dim = (this -> symbol_table_stack).get_array_dim(lval.ident);
            int param_dim = -1;
            if (func_decl_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            else if (func_def_table.if_exist((this->func_call_stk).top())) {
                param_dim = func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
            }
            
            if (param_dim != array_dim) {
                std::cout << "Semantic analysis failed: var array '" << lval.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
    }

    // 获取 IDENT 对应的值，向上返回，以便替换成 NumberAST
    if (symbol_table_stack.if_const(lval.ident)) {
        this->num = symbol_table_stack.get_const(lval.ident);
        this->if_fold = true;
    }
    else if (symbol_table_stack.if_var(lval.ident)) // 修改变量名
        lval.ident = symbol_table_stack.get_var(lval.ident);
    else if (symbol_table_stack.if_const_array(lval.ident))
        lval.ident = symbol_table_stack.get_const_array(lval.ident);
    else if (symbol_table_stack.if_var_array(lval.ident))
        lval.ident = symbol_table_stack.get_var_array(lval.ident);
}
void Visitor_sema::sema_analysis(LValAST_2& lval) {
    // 先进行报错检测
    // 未定义检测
    if (!symbol_table_stack.if_exist_all(lval.ident)) {
        std::cout << "Semantic analysis failed: ident '" << lval.ident << "' not defined.\n";
        exit(-1);
    }
    // 非数组变量检测
    if (symbol_table_stack.if_const(lval.ident)) {
        std::cout << "Semantic analysis failed: '" << lval.ident << "' is a const rather than an array.\n";
        exit(-1);
    }
    if (symbol_table_stack.if_var(lval.ident)) {
        std::cout << "Semantic analysis failed: '" << lval.ident << "' is a var rather than an array.\n";
        exit(-1);
    }

    if (get_error_mode() == CONST_CARRAY_ARRAY_UNDF) { // 数组常量不能再次赋值
        if (symbol_table_stack.if_const_array(lval.ident)) {
            std::cout << "Semantic analysis failed: const array '" << lval.ident << "' cannot be assigned again.\n";
            exit(-1);
        }
    }
    else if (get_error_mode() == VAR_CARRAY_ARRAY_UNDF) { // 只要是数组就不能给常量和全局变量赋值
        if (symbol_table_stack.if_const_array(lval.ident) || symbol_table_stack.if_var_array(lval.ident)) {
            std::cout << "Semantic analysis failed: array '" << lval.ident << "' cannot be used to assign for const or global var.\n";
            exit(-1);
        }
    }

    // 记录数组名用于检错
    this -> array_name = lval.ident;
    lval.explist.get() -> accept(*this);
    // 修改数组名
    if (symbol_table_stack.if_const_array(lval.ident))
        lval.ident = symbol_table_stack.get_const_array(lval.ident);
    else if (symbol_table_stack.if_var_array(lval.ident))
        lval.ident = symbol_table_stack.get_var_array(lval.ident);
}

// ExpList       ::= "[" Exp "]" | ExpList "[" Exp "]";
void Visitor_sema::sema_analysis(ExpListAST& exp_list) {
    // 数组不做参数的时候，下标要给全，必须是 int，要求必须每个维度都有 index
    if (get_error_mode() == CARRAY_ARRAY_UNDF) {
        if (symbol_table_stack.get_array_dim(this->array_name) != exp_list.exps.size()) {
            std::cout << "Semantic analysis failed: array '" << this->array_name << "' index doesn't match its dimension.\n";
            exit(-1);
        }
    }
    // 做参数的时候，就要计算 参数维度+传参维度 ?= 数组维度
    else if (get_error_mode() == PARAM_UNDF) {
        int array_dim = symbol_table_stack.get_array_dim(this->array_name);
        int param_dim = -1;
        if (func_decl_table.if_exist((this->func_call_stk).top())) {
            param_dim = func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
        }
        else if (func_def_table.if_exist((this->func_call_stk).top())) {
            param_dim = func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top());
        }

        if (array_dim != param_dim + exp_list.exps.size()) {
            std::cout << "Semantic analysis failed: array '" << this->array_name << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
            exit(-1);
        }
    }

    for (int i = 0; i < exp_list.exps.size(); i++) {
        add_error_mode(CARRAY_ARRAY_UNDF);
        exp_list.exps[i].get() -> accept(*this);
        recover_error_mode();
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
    // 这里也要加检测报错了
    if (get_error_mode() == PARAM_UNDF) {
        if (func_decl_table.if_exist((this->func_call_stk).top())) {
            if (func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top()) != 0) {
                std::cout << "Semantic analysis failed: number '" << number.num << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
        else if (func_def_table.if_exist((this->func_call_stk).top())) {
            if (func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top()) != 0) {
                std::cout << "Semantic analysis failed: number '" << number.num << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
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

    // 如果这个函数是作为参数被调用的
    // 即使是 1+Func() 这种形式，也需要函数返回类型是 int，此处参数维度是 0
    if (get_error_mode() == PARAM_UNDF) {
        if (func_decl_table.if_exist(unary_exp.ident)) {
            if (!func_decl_table.if_ret_int(unary_exp.ident)) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' should return int as parameter.\n";
                exit(-1);
            }
            if (func_decl_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top()) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
        else if (func_def_table.if_exist(unary_exp.ident)) {
            if (!func_def_table.if_ret_int(unary_exp.ident)) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' should return int as parameter.\n";
                exit(-1);
            }
            if (func_def_table.get_param_dim((this->func_call_stk).top(), (this->func_param_index).top()) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' doesn't match the dimension of parameter required by the function '" << (this->func_call_stk).top() << "'.\n";
                exit(-1);
            }
        }
    }

    if (unary_exp.func_r_param_list) {
        // 函数名入栈，供后面检测
        (this -> func_call_stk).push(unary_exp.ident);
        // 进入参数列表
        unary_exp.func_r_param_list.get() -> accept(*this);
        // 复原
        (this -> func_call_stk).pop();
    }
    else {
        if (func_decl_table.if_exist(unary_exp.ident)) {
            if (func_decl_table.get_param_num(unary_exp.ident) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' parameter not match when called.\n";
                exit(-1);
            }
        }
        else if (func_def_table.if_exist(unary_exp.ident)) {
            if (func_def_table.get_param_num(unary_exp.ident) != 0) {
                std::cout << "Semantic analysis failed: function '" << unary_exp.ident << "' parameter not match when called.\n";
                exit(-1);
            }
        }
    }
}

// FuncRParamList ::= Exp | FuncRParamList "," Exp;
void Visitor_sema::sema_analysis(FuncRParamListAST& func_r_param_list) {
    this -> func_call = (this->func_call_stk).top();

    if (func_decl_table.if_exist(this -> func_call)) {
        if (func_decl_table.get_param_num(this->func_call) != func_r_param_list.func_r_params.size()) {
            std::cout << "Semantic analysis failed: function '" << this->func_call << "' parameter not match when called.\n";
            exit(-1);
        }
    }
    else if (func_def_table.if_exist(this -> func_call)) {
        if (func_def_table.get_param_num(this->func_call) != func_r_param_list.func_r_params.size()) {
            std::cout << "Semantic analysis failed: function '" << this->func_call << "' parameter not match when called.\n";
            exit(-1);
        }
    }

    add_error_mode(PARAM_UNDF); // 参数报错模式
    for (int i = 0; i < func_r_param_list.func_r_params.size(); i++) {
        (this -> func_param_index).push(i); // 参数索引，又用上了
        func_r_param_list.func_r_params[i].get() -> accept(*this);
        (this -> func_param_index).pop();
    }
    recover_error_mode();
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