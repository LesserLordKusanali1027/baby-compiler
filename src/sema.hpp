# ifndef SEMA_HPP
# define SEMA_HPP

# include <unordered_map>
# include <string>
# include <stack>
# include <variant>
# include <vector>

class BaseAST;

class CompUnitAST;
class CompUnitListAST;
class CompUnitItemAST_1;
class CompUnitItemAST_2;

class DeclAST_1;
class DeclAST_2;
class ConstDeclAST;
class BTypeAST;
class ConstDefListAST;
class ConstDefAST_1;
class ConstDefAST_2;
class ConstSizeListAST;
class ConstInitValAST_1;
class ConstInitValAST_2;
class ConstInitValListAST;
class VarDeclAST;
class VarDefListAST;
class VarDefAST_1;
class VarDefAST_2;
class VarDefAST_3;
class VarDefAST_4;
class VarSizeListAST;
class InitValAST_1;
class InitValAST_2;
class InitValListAST;

class FuncDefAST;
class FuncFParamListAST;
class FuncFParamAST_1;
class FuncFParamAST_2;
class ConstExpListAST;
class BlockAST;
class BlockItemListAST;
class BlockItemAST_1;
class BlockItemAST_2;
class StmtAST_1;
class StmtAST_2;
class MatchedStmtAST_1;
class MatchedStmtAST_2;
class MatchedStmtAST_3;
class MatchedStmtAST_4;
class MatchedStmtAST_5;
class MatchedStmtAST_6;
class MatchedStmtAST_7;
class MatchedStmtAST_8;
class UnmatchedStmtAST_1;
class UnmatchedStmtAST_2;
class ExpAST;
class LValAST_1;
class LValAST_2;
class ExpListAST;
class PrimaryExpAST_1;
class PrimaryExpAST_2;
class PrimaryExpAST_3;
class NumberAST;
class UnaryExpAST_1;
class UnaryExpAST_2;
class UnaryExpAST_3;
class FuncRParamListAST;
class UnaryOpAST;
class MulExpAST_1;
class MulExpAST_2;
class AddExpAST_1;
class AddExpAST_2;
class RelExpAST_1;
class RelExpAST_2;
class EqExpAST_1;
class EqExpAST_2;
class LAndExpAST_1;
class LAndExpAST_2;
class LOrExpAST_1;
class LOrExpAST_2;
class ConstExpAST;


// 常量、变量信息
class SymbolInfo {
  public:
    // 常量、变量类型
    enum SymbolType { CONST = 0, VAR, CONST_ARRAY, VAR_ARRAY};
    // 值的类型
    using Value = std::variant<int, std::string>;
  private:
    SymbolType symbol_type;
    Value symbol_value;
    int array_dim; // 专门为数组添加
  public:
    // 添加符号信息
    void add_const (int value) {
        symbol_type = CONST;
        symbol_value = value;
    }
    void add_var (std::string& value) {
        symbol_type = VAR;
        symbol_value = value;
    }
    void add_const_array (std::string& value) {
        symbol_type = CONST_ARRAY;
        symbol_value = value;
    }
    void add_var_array (std::string& value) {
        symbol_type = VAR_ARRAY;
        symbol_value = value;
    }

    // 判断类型
    bool if_const () {
        return symbol_type == CONST;
    }
    bool if_var () {
        return symbol_type == VAR;
    }
    bool if_const_array () {
        return symbol_type == CONST_ARRAY;
    }
    bool if_var_array () {
        return symbol_type == VAR_ARRAY;
    }

    // 取得内容
    int get_const () {
        return std::get<int>(symbol_value);
    }
    std::string get_var () {
        return std::get<std::string>(symbol_value);
    }
    std::string get_const_array () {
        return std::get<std::string>(symbol_value);
    }
    std::string get_var_array () {
        return std::get<std::string>(symbol_value);
    }

    // 添加数组维度
    void add_array_dim(int dim) {
        array_dim = dim;
    }
    // 获取数组维度
    int get_array_dim() {
        return array_dim;
    }
};

// 常量/变量符号表类
class SymbolTable {
  public:
    using Value = std::variant<int, std::string>;
  private:
    std::unordered_map<std::string, SymbolInfo*> table;
  public:
    ~SymbolTable () {
        table.clear();
    }

    // 添加常量和变量、常量数组和变量数组
    void add_const (std::string& name, int value) {
        SymbolInfo* symbol_info = new SymbolInfo();
        table[name] = symbol_info;
        table[name] -> add_const(value);
        symbol_info = NULL;
    }
    void add_var (std::string& name, std::string value) {
        SymbolInfo* symbol_info = new SymbolInfo();
        table[name] = symbol_info;
        table[name] -> add_var(value);
        symbol_info = NULL;
    }
    void add_const_array (std::string& name, std::string value) {
        SymbolInfo* symbol_info = new SymbolInfo();
        table[name] = symbol_info;
        table[name] -> add_const_array(value);
        symbol_info = NULL;
    }
    void add_var_array (std::string& name, std::string value) {
        SymbolInfo* symbol_info = new SymbolInfo();
        table[name] = symbol_info;
        table[name] -> add_var_array(value);
        symbol_info = NULL;
    }

    // 检查键是否存在
    bool if_exist (std::string& name) {
        return (bool)table.count(name);
    }

    // 判断是常量还是变量，是常量数组还是变量数组，但后面估计用不到
    bool if_const (std::string& name) {
        return table[name] -> if_const();
    }
    bool if_var (std::string& name) {
        return table[name] -> if_var();
    }
    bool if_const_array (std::string& name) {
        return table[name] -> if_const_array();
    }
    bool if_var_array (std::string& name) {
        return table[name] -> if_var_array();
    }

    // 返回值，类型检查保险措施放在这里，后面就不用再写了
    int get_const (std::string& name) {
        if (!(table[name] -> if_const())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't regarded as a const, which is the developer's fault.\n";
            exit(-2);
        }
        return table[name] -> get_const();
    }
    std::string get_var (std::string& name) {
        if (!(table[name] -> if_var())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't regarded as a var, which is the developer's fault.\n";
            exit(-2);
        }
        return table[name] -> get_var();
    }
    std::string get_const_array (std::string& name) {
        if (!(table[name] -> if_const_array())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't regarded as a const array, which is the developer's fault.\n";
            exit(-2);
        }
        return table[name] -> get_const_array();
    }
    std::string get_var_array (std::string& name) {
        if (!(table[name] -> if_var_array())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't regarded as a var array, which is the developer's fault.\n";
            exit(-2);
        }
        return table[name] -> get_var_array();
    }

    // 数组相关操作
    void add_array_dim(std::string& name, int dim) {
        if (!(table[name] -> if_const_array()) && !(table[name] -> if_var_array())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't an array, which is the developer's fault.\n";
            exit(-2);
        }
        table[name] -> add_array_dim(dim);
    }
    int get_array_dim(std::string& name) {
        if (!(table[name] -> if_const_array()) && !(table[name] -> if_var_array())) {
            std::cout << "Symbol table has a mistake: '" << name << "' isn't an array, which is the developer's fault.\n";
            exit(-2);
        }
        return table[name] -> get_array_dim();
    }
};

// 多层常量/变量符号表
class SymbolTableStack {
  private:
    std::vector<SymbolTable*> table_stack;
    // 记录出现过的同名的变量、数组的数目，以便添加编号
    std::unordered_map<std::string, int> symbol_count;

  public:
    ~SymbolTableStack () { // 释放内存
        for (int i = 0; i < table_stack.size(); i++) {
            delete table_stack[i];
        }
    }
    
    // 添加、删除单个符号表
    void push_table () {
        SymbolTable* table = new SymbolTable();
        table_stack.push_back(table);
    }
    void pop_table () {
        SymbolTable* table = table_stack.back();
        table_stack.pop_back();
        delete table;
    }

    // 向位于最后的符号表添加常量、变量和常量数组、变量数组
    void add_const (std::string& name, int value) {
        SymbolTable* table = table_stack.back();
        table -> add_const(name, value);
    }
    void add_var (std::string& name) {
        SymbolTable* table = table_stack.back();
        symbol_count[name]++;
        std::string value; // 打个补丁
        if (name == "end")
            value = name + "_var_" + std::to_string(symbol_count[name]);
        else
            value = name + "_" + std::to_string(symbol_count[name]);
        table -> add_var(name, value);
    }
    void add_const_array (std::string& name) {
        SymbolTable* table = table_stack.back();
        symbol_count[name]++;
        std::string value = name + "_" + std::to_string(symbol_count[name]);
        table -> add_const_array(name, value);
    }
    void add_var_array (std::string& name) {
        SymbolTable* table = table_stack.back();
        symbol_count[name]++;
        std::string value = name + "_" + std::to_string(symbol_count[name]);
        table -> add_var_array(name, value);
    }

    // 工具函数，返回存在某个符号的最近符号表的索引
    int get_index (std::string& name) {
        int index;
        for (index = table_stack.size()-1; index >= 0; index--) {
            if (table_stack[index]->if_exist(name))
                break;
        }
        return index;
    }

    // 检查键是否存在
    bool if_exist_last (std::string& name) { // 在最后的符号表中
        return table_stack.back() -> if_exist(name);
    }
    bool if_exist_all (std::string& name) { // 在所有符号表中
        int index = get_index(name);
        return index == -1 ? false : true;
    }

    // 判断符号代表的是常量/变量还是常量数组/变量数组
    bool if_const (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> if_const(name);
    }
    bool if_var (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> if_var(name);
    }
    bool if_const_array (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> if_const_array(name);
    }
    bool if_var_array (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> if_var_array(name);
    }

    // 返回结果
    int get_const (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> get_const(name);
    }
    std::string get_var (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> get_var(name);
    }
    std::string get_const_array (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> get_const_array(name);
    }
    std::string get_var_array (std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> get_var_array(name);
    }

    // 数组相关操作
    void add_array_dim(std::string& name, int dim) {
        int index = get_index(name);

        table_stack[index] -> add_array_dim(name, dim);
    }
    int get_array_dim(std::string& name) {
        int index = get_index(name);

        return table_stack[index] -> get_array_dim(name);
    }
};


// 记录函数信息的类
class FunctionInfo {
  private:
    std::string func_type;
    std::vector<int> params_dim; // 参数的维度，如 int a 是 0 维，int a[] 是一维
    std::unordered_map<int, std::vector<int>> params_size; // 对于数组参数加入大小信息，如 int a[][3][4] 就加入 3、4

  public:
    ~FunctionInfo() {
        params_size.clear();
    }

    // 存入返回值类型
    void add_type(std::string& type) {
        func_type = type;
    }
    // 加入参数，传入维度信息
    void add_param(int dim) {
        params_dim.push_back(dim);
    }
    // 加入数组参数的大小信息，第 index 个参数，新增大小为 size
    bool add_size(int index, int size) {
        if (index < params_dim.size() && params_size[index].size() < params_dim[index]-1) {
            params_size[index].push_back(size);
            return true;
        }
        else
            return false;
    }
    
    // 判断返回值类型
    bool if_int() {
        return func_type == "int";
    }
    bool if_void() {
        return func_type == "void";
    }

    // 判断参数个数对不对，只需要比较个数，不比较具体类型
    int get_param_num() {
        return params_dim.size();
    }
    // 判断某个参数的维度是否匹配，传入 index，返回参数的维度
    int get_param_dim(int index) {
        if (index < params_dim.size())
            return params_dim[index];
        else
            return -1;
    }
    // 判断某个数组参数的某维度的大小是否匹配，传入 index，dim_index，返回该维度的大小
    int get_param_size(int index, int dim_index) {
        // 防止内存泄露
        if (index >= params_dim.size())
            return -1;
        if (dim_index >= params_dim[index]-1)
            return -1;
        
        return params_size[index][dim_index];
    }
};

// 函数符号表类 (全局单层)
class FunctionTable {
  private:
    std::unordered_map<std::string, FunctionInfo*> table;

  public:
    ~FunctionTable() {
        table.clear();
    }

    // 判断函数名有没有重复定义
    bool if_exist(std::string& name) {
        return (bool)table.count(name);
    }
    // 新增函数
    void add_func(std::string& name) {
        // 这里加一个保险，判断是否重复定义
        if (table.count(name)) {
            std::cout << "Function table has a mistake: function '" << name << "' is redefined, which is the developer's fault.\n";
            exit(-1);
        }

        auto f_info = new FunctionInfo();
        table[name] = f_info;
    }
    // 添加返回类型
    void add_type(std::string& name, std::string& func_type) {
        table[name] -> add_type(func_type);
    }
    // 参数++
    void add_param(std::string& name, int dim) {
        table[name] -> add_param(dim);
    }
    // 为数组参数添加维度
    void add_size(std::string& name, int index, int size) {
        bool result = table[name]->add_size(index, size);
        if (!result) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "' or doesn't need more size, which is the developer's fault.\n";
            exit(-1); 
        }
    }

    // 报错检测相关
    // 返回参数个数，最基本的检测
    int get_param_num(std::string& name) {
        return table[name] -> get_param_num();
    }
    // 返回某个参数的维度信息，数字就是 0 维，数组就是对应维度
    int get_param_dim(std::string& name, int index) {
        int result = table[name]->get_param_dim(index);
        if (result == -1) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "', which is the developer's fault.\n";
            exit(-1);
        }
        return result;
    }
    // 返回某个数组参数的某维的大小，这里维度从 [][3][4] 第二个 [3] 开始编号 0,1
    int get_param_size(std::string& name, int index, int dim_index) {
        int result = table[name]->get_param_size(index, dim_index);
        if (result == -1) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "' or the parameter doesn't have the dim index '" << dim_index << "', which is the developer's fault.\n";
            exit(-1);
        }
        return result;
    }
    // 判断是否要有返回值
    bool if_ret_int(std::string& name) {
        return table[name] -> if_int();
    }
};

// 函数声明符号表类
class FunctionDeclTable {
  private:
    std::unordered_map<std::string, FunctionInfo*> table;

  public:
    ~FunctionDeclTable() {
        table.clear();
    }
    // 判断函数有没有重复声明
    bool if_exist(std::string& name) {
        return (bool)table.count(name);
    }
    // 新增函数
    void add_func(std::string& name) {
        // 这里加一个保险，判断是否重复定义
        if (table.count(name)) {
            std::cout << "Function declaration table has a mistake: function '" << name << "' is redefined, which is the developer's fault.\n";
            exit(-1);
        }

        auto f_info = new FunctionInfo();
        table[name] = f_info;
    }
    // 添加返回类型
    void add_type(std::string& name, std::string& func_type) {
        table[name] -> add_type(func_type);
    }
    // 参数++
    void add_param(std::string& name, int dim) {
        table[name] -> add_param(dim);
    }
    // 为数组参数添加维度
    void add_size(std::string& name, int index, int size) {
        bool result = table[name]->add_size(index, size);
        if (!result) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "' or doesn't need more size, which is the developer's fault.\n";
            exit(-1); 
        }
    }

    // 报错检测相关
    // 判断参数是否匹配
    int get_param_num(std::string& name) {
        return table[name] -> get_param_num();
    }
    // 返回某个参数的维度信息，数字就是 0 维，数组就是对应维度
    int get_param_dim(std::string& name, int index) {
        int result = table[name]->get_param_dim(index);
        if (result == -1) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "', which is the developer's fault.\n";
            exit(-1);
        }
        return result;
    }
    // 返回某个数组参数的某维的大小，这里维度从 [][3][4] 第二个 [3] 开始编号 0,1
    int get_param_size(std::string& name, int index, int dim_index) {
        int result = table[name]->get_param_size(index, dim_index);
        if (result == -1) {
            std::cout << "Function table has a mistake: function '" << name << "' doesn't have parameter with index '" << index
                      << "' or the parameter doesn't have the dim index '" << dim_index << "', which is the developer's fault.\n";
            exit(-1);
        }
        return result;
    }
    // 判断是否要有返回值
    bool if_ret_int(std::string& name) {
        return table[name] -> if_int();
    }

    // 移除某个声明
    void erase(std::string& name) {
        table.erase(name);
    }
};

// 常量数组初始化列表标准化要使用的方法类，在 sema.cpp 中实现
class ConstInitVal_Std;
// 变量数组初始化列表标准化要使用的方法类，在 sema.cpp 中实现
class InitVal_Std;

// 定义枚举类型，用于设定错误检查模式
enum ErrorMode { NONE = 0, VAR_CARRAY_ARRAY_UNDF, CONST_CARRAY_ARRAY_UNDF, CARRAY_ARRAY_UNDF, PARAM_UNDF };

// CompUnit      ::= CompUnitList;
// CompUnitList  ::= CompUnitItem | CompUnitList CompUnitItem;
// CompUnitItem  ::= FuncDef | Decl;

// Decl          ::= ConstDecl | VarDecl;
// ConstDecl     ::= "const" BType ConstDefList ";";
// BType         ::= "int" | "void";
// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
// ConstDef      ::= IDENT "=" ConstInitVal | IDENT ConstSizeList "=" ConstInitVal;
// ConstSizeList ::= "[" ConstExp "]" | ConstSizeList "[" ConstExp "]";
// ConstInitVal  ::= ConstExp | "{" [ConstInitValList] "}";
// ConstInitValList ::= ConstInitVal | ConstInitValList "," ConstInitVal;
// VarDecl       ::= BType VarDefList ";";
// VarDefList    ::= VarDef | VarDefList "," VarDef;
// VarDef        ::= IDENT 
//                 | IDENT "=" InitVal
//                 | IDENT VarSizeList
//                 | IDENT VarSizeList "=" InitVal;
// VarSizeList   ::= "[" ConstExp "]" | VarSizeList "[" ConstExp "]";
// InitVal       ::= Exp | "{" [InitValList] "}";
// InitValList   ::= InitVal | InitValList "," InitVal;

// FuncDef       ::= BType IDENT "(" [FuncFParamList] ")" Block;
// FuncFParamList::= FuncFParam | FuncFParamList "," FuncFParam;
// FuncFParam    ::= BType IDENT | BType IDENT "[" "]" [ConstExpList];
// ConstExpList ::= "[" ConstExp "]" | ConstExpList "[" ConstExp "]";
// Block         ::= "{" BlockItemList "}";
// BlockItemList ::= %empty | BlockItemList BlockItem;
// BlockItem     ::= Decl | Stmt;
// Stmt          ::= MatchedStmt | UnmatchedStmt
// MatchedStmt   ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt | "while" "(" Exp ")" Stmt;
// UnmatchedStmt ::= "if" "(" Exp ")" Stmt | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt;
// Exp           ::= LOrExp;
// LVal          ::= IDENT | IDENT ExpList;
// ExpList       ::= "[" Exp "]" | ExpList "[" Exp "]";
// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
// Number        ::= INT_CONST;
// UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp | IDENT "(" [FuncRParamList] ")";
// FuncRParamList::= Exp | FuncRParamList "," Exp;
// UnaryOp       ::= "+" | "-" | "!";
// MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
// AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
// RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
// EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
// LAndExp       ::= EqExp | LAndExp "&&" EqExp;
// LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
// ConstExp      ::= Exp;

class Visitor_sema {
  private:
    // 表达式计算相关
    std::stack<int> stk; // 计算用的栈
    int num; // 用来把 LVal 换成 Number 的变量
    char ch; // 传递计算符号
    bool cal_mode = false; // 计算模式是否开启
    std::stack<ErrorMode> error_mode_stk; // 报错检测模式，下面是三个辅助函数
    void add_error_mode(ErrorMode mode) {
        error_mode_stk.push(mode);
    }
    ErrorMode get_error_mode() {
        if (error_mode_stk.empty()) {
            std::cout << "ErrorMode stack has a mistake, which is the developer's fault.\n";
            exit(-2);
        }
        return error_mode_stk.top();
    }
    void recover_error_mode() {
        error_mode_stk.pop();
    }
    bool if_fold = false; // 是否常量折叠

    // 循环相关
    // 表明 while 循环的嵌套层数，0 表示不在 while 中
    int while_levels = 0;

    // 常量、变量相关
    SymbolTableStack symbol_table_stack; // 多层符号表

    // 函数相关
    FunctionTable func_def_table; // 全局函数符号表
    std::string func_name; // 当前所在函数的名字
    std::string func_call; // 仅用来判断函数调用时传参个数对不对
    bool if_func_def = false; // BType 节点是否作为 FuncType
    FunctionDeclTable func_decl_table; // 已声明的函数表
    // 添加库函数声明以供报错检测
    void def_lib_func () {
        std::string name_str;
        std::string int_str = "int";
        std::string void_str = "void";
        // int getint()
        name_str = "getint";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, int_str);
        // int getch()
        name_str = "getch";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, int_str);
        // int getarray(int[])
        name_str = "getarray";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, int_str);
        func_def_table.add_param(name_str, 1);
        // void putint(int)
        name_str = "putint";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, void_str);
        func_def_table.add_param(name_str, 0);
        // void putch(int)
        name_str = "putch";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, void_str);
        func_def_table.add_param(name_str, 0);
        // void putarray(int, int[])
        name_str = "putarray";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, void_str);
        func_def_table.add_param(name_str, 0);
        func_def_table.add_param(name_str, 1);
        // void starttime()
        name_str = "starttime";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, void_str);
        // void stoptime()
        name_str = "stoptime";
        func_def_table.add_func(name_str);
        func_def_table.add_type(name_str, void_str);
    }

    // 全局常量、变量相关
    bool global_decl; // 告诉 DeclAST 定义的是全局常量、变量

    // 数组相关
    // 将常量数组的初始化列表标准化
    ConstInitVal_Std* const_init_val_std;
    // 将变量数组的初始化列表标准化
    InitVal_Std* init_val_std;
    // 传递数组名
    std::string array_name;

    // 数组函数相关
    std::stack<int> func_param_index; // 记录参数大小时要用的参数索引
    std::stack<std::string> func_call_stk; // 存放函数名，用来判断调用函数时参数个数、维度、大小是否匹配
    
  public:
    // 以下函数用来遍历语法树生成符号表
    void sema_analysis(CompUnitAST& comp_unit);
    void sema_analysis(CompUnitListAST& comp_unit_list);
    void sema_analysis(CompUnitItemAST_1& comp_unit_item);
    void sema_analysis(CompUnitItemAST_2& comp_unit_item);

    void sema_analysis(DeclAST_1& decl);
    void sema_analysis(DeclAST_2& decl);
    void sema_analysis(ConstDeclAST& const_decl);
    void sema_analysis(BTypeAST& btype);
    void sema_analysis(ConstDefListAST& const_def_list);
    void sema_analysis(ConstDefAST_1& const_def);
    void sema_analysis(ConstDefAST_2& const_def);
    void sema_analysis(ConstSizeListAST& const_size_list);
    void sema_analysis(ConstInitValAST_1& const_init_val);
    void sema_analysis(ConstInitValAST_2& const_init_val);
    void sema_analysis(ConstInitValListAST& const_init_val_list);
    void sema_analysis(VarDeclAST& var_decl);
    void sema_analysis(VarDefListAST& var_del_list);
    void sema_analysis(VarDefAST_1& var_def);
    void sema_analysis(VarDefAST_2& var_def);
    void sema_analysis(VarDefAST_3& var_def);
    void sema_analysis(VarDefAST_4& var_def);
    void sema_analysis(VarSizeListAST& var_size_list);
    void sema_analysis(InitValAST_1& init_val);
    void sema_analysis(InitValAST_2& init_val);
    void sema_analysis(InitValListAST& init_val_list);

    void sema_analysis(FuncDefAST& func_def);
    void sema_analysis(FuncFParamListAST& func_f_param_list);
    void sema_analysis(FuncFParamAST_1& func_f_param);
    void sema_analysis(FuncFParamAST_2& func_f_param);
    void sema_analysis(ConstExpListAST& const_exp_list);
    void sema_analysis(BlockAST& block);
    void sema_analysis(BlockItemListAST& block_item_list);
    void sema_analysis(BlockItemAST_1& block_item);
    void sema_analysis(BlockItemAST_2& block_item);
    void sema_analysis(StmtAST_1& stmt);
    void sema_analysis(StmtAST_2& stmt);
    void sema_analysis(MatchedStmtAST_1& matched_stmt);
    void sema_analysis(MatchedStmtAST_2& matched_stmt);
    void sema_analysis(MatchedStmtAST_3& matched_stmt);
    void sema_analysis(MatchedStmtAST_4& matched_stmt);
    void sema_analysis(MatchedStmtAST_5& matched_stmt);
    void sema_analysis(MatchedStmtAST_6& matched_stmt);
    void sema_analysis(MatchedStmtAST_7& matched_stmt);
    void sema_analysis(MatchedStmtAST_8& matched_stmt);
    void sema_analysis(UnmatchedStmtAST_1& unmatched_stmt);
    void sema_analysis(UnmatchedStmtAST_2& unmatched_stmt);
    void sema_analysis(ExpAST& exp);
    void sema_analysis(LValAST_1& lval);
    void sema_analysis(LValAST_2& lval);
    void sema_analysis(ExpListAST& exp_list);
    void sema_analysis(PrimaryExpAST_1& primary_exp);
    void sema_analysis(PrimaryExpAST_2& primary_exp);
    void sema_analysis(PrimaryExpAST_3& primary_exp);
    void sema_analysis(NumberAST& number);
    void sema_analysis(UnaryExpAST_1& unary_exp);
    void sema_analysis(UnaryExpAST_2& unary_exp);
    void sema_analysis(UnaryExpAST_3& unary_exp);
    void sema_analysis(FuncRParamListAST& func_r_param_list);
    void sema_analysis(UnaryOpAST& unary_op);
    void sema_analysis(MulExpAST_1& mul_exp);
    void sema_analysis(MulExpAST_2& mul_exp);
    void sema_analysis(AddExpAST_1& add_exp);
    void sema_analysis(AddExpAST_2& add_exp);
    void sema_analysis(RelExpAST_1& rel_exp);
    void sema_analysis(RelExpAST_2& rel_exp);
    void sema_analysis(EqExpAST_1& eq_exp);
    void sema_analysis(EqExpAST_2& eq_exp);
    void sema_analysis(LAndExpAST_1& l_and_exp);
    void sema_analysis(LAndExpAST_2& l_and_exp);
    void sema_analysis(LOrExpAST_1& l_or_exp);
    void sema_analysis(LOrExpAST_2& l_or_exp);
    void sema_analysis(ConstExpAST& const_exp);

};

# endif