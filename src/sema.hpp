# ifndef SEMA_HPP
# define SEMA_HPP

# include <unordered_map>
# include <string>
# include <stack>
# include <variant>
# include <vector>

class BaseAST;
class CompUnitAST;

class DeclAST_1;
class DeclAST_2;
class ConstDeclAST;
class BTypeAST;
class ConstDefListAST;
class ConstDefAST;
class ConstInitValAST;
class VarDeclAST;
class VarDefListAST;
class VarDefAST_1;
class VarDefAST_2;
class InitValAST;

class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class BlockItemListAST;
class BlockItemAST_1;
class BlockItemAST_2;
class StmtAST_1;
class StmtAST_2;
class StmtAST_3;
class StmtAST_4;
class ExpAST;
class LValAST;
class PrimaryExpAST_1;
class PrimaryExpAST_2;
class PrimaryExpAST_3;
class NumberAST;
class UnaryExpAST_1;
class UnaryExpAST_2;
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

// 符号表类
class SymbolTable {
  public:
    using Value = std::variant<int, std::string>;
  private:
    std::unordered_map<std::string, Value> table;
  public:
    ~SymbolTable() {
        table.clear();
    }
    // 添加常量和变量
    void add_const(std::string& name, int value) {
        table[name] = value; // Value 会自动识别的
    }
    void add_var(std::string& name, std::string value) {
        table[name] = value;
    }

    // 检查键是否存在
    bool if_exist(std::string& name) {
        return (bool)table.count(name);
    }

    // 判断是常量还是变量
    bool if_const(std::string& name) {
        return std::holds_alternative<int>(table[name]);
    }
    bool if_var(std::string& name) {
        return std::holds_alternative<std::string>(table[name]);
    }

    // 返回结果
    int get_const(std::string& name) {
        return std::get<int>(table[name]);
    }
    std::string get_var(std::string& name) {
        return std::get<std::string>(table[name]);
    }
};

// 多层符号表
class SymbolTableStack {
  private:
    std::vector<SymbolTable*> table_stack;
    // 记录出现过的同名变量的数目，以便添加编号
    std::unordered_map<std::string, int> var_count;

  public:
    ~SymbolTableStack() { // 释放内存
        for (int i = 0; i < table_stack.size(); i++) {
            delete table_stack[i];
        }
    }
    
    // 添加、删除单个符号表
    void push_table() {
        SymbolTable* table = new SymbolTable();
        table_stack.push_back(table);
    }
    void pop_table() {
        SymbolTable* table = table_stack.back();
        table_stack.pop_back();
        delete table;
    }

    // 向位于最后的符号表添加常量和变量
    void add_const(std::string& name, int value) {
        SymbolTable* table = table_stack.back();
        table -> add_const(name, value);
    }
    void add_var(std::string& name) {
        SymbolTable* table = table_stack.back();
        var_count[name]++;
        std::string value = name + "_" + std::to_string(var_count[name]);
        table -> add_var(name, value);
    }

    // 工具函数，返回存在某个符号的最近符号表的索引
    int get_index(std::string& name) {
        int index;
        for (index = table_stack.size()-1; index >= 0; index--) {
            if (table_stack[index]->if_exist(name))
                break;
        }
        return index;
    }

    // 检查键是否存在
    bool if_exist_last(std::string& name) {
        return table_stack.back()->if_exist(name);
    }
    bool if_exist_all(std::string& name) {
        int index = get_index(name);
        return index == -1 ? false : true;
    }

    // 判断是常量还是变量
    bool if_const(std::string& name) {
        int index = get_index(name);
        
        return table_stack[index]->if_const(name);
    }
    bool if_var(std::string& name) {
        int index = get_index(name);
        
        return table_stack[index]->if_var(name);
    }

    // 返回结果
    int get_const(std::string& name) {
        if (if_var(name)) {
            std::cout << "Symbol table has a mistake: '" << name << "' is recorded as a var, not a const.\n";
            exit(-1); 
        }

        int index = get_index(name);

        return table_stack[index] -> get_const(name);
    }
    std::string get_var(std::string& name) {
        if (if_const(name)) {
            std::cout << "Symbol table has a mistake: '" << name << "' is recorded as a const, not a var.\n";
            exit(-1);
        }

        int index = get_index(name);

        return table_stack[index] -> get_var(name);
    }
};

// 定义枚举类型，用于设定检查模式
enum Mode { NONE = 0, VAR_UNDF, CONST_UNDF, UNDF };

// CompUnit      ::= FuncDef;

// Decl          ::= ConstDecl | VarDecl;
// ConstDecl     ::= "const" BType ConstDefList ";";
// BType         ::= "int";
// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
// ConstDef      ::= IDENT "=" ConstInitVal;
// ConstInitVal  ::= ConstExp;
// VarDecl       ::= BType VarDefList ";";
// VarDefList    ::= VarDef | VarDefList "," VarDef;
// VarDef        ::= IDENT | IDENT "=" InitVal;
// InitVal       ::= Exp;

// FuncDef       ::= FuncType IDENT "(" ")" Block;
// FuncType      ::= "int";
// Block         ::= "{" BlockItemList "}";
// BlockItemList ::= %empty | BlockItemList BlockItem
// BlockItem     ::= Decl | Stmt;
// Stmt          ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block;
// Exp           ::= LOrExp;
// LVal          ::= IDENT;
// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
// Number        ::= INT_CONST;
// UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp;
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
    SymbolTableStack symbol_table_stack; // 多层符号表
    std::stack<int> stk; // 计算用的栈
    int num; // 用来把 LVal 换成 Number 的变量
    char ch; //传递计算符号
    // 信号
    bool cal_mode = false; // 计算模式是否开启
    Mode error_mode = NONE; // 报错检测模式
    bool if_fold = false; // 是否常量折叠

  public:
    // 以下函数用来遍历语法树生成符号表，并将 LValAST 替换成 NumberAST
    void sema_analysis(CompUnitAST& comp_unit);

    void sema_analysis(DeclAST_1& decl);
    void sema_analysis(DeclAST_2& decl);
    void sema_analysis(ConstDeclAST& const_decl);
    void sema_analysis(BTypeAST& byte) {return;}
    void sema_analysis(ConstDefListAST& const_def_list);
    void sema_analysis(ConstDefAST& const_def);
    void sema_analysis(ConstInitValAST& const_init_val);
    void sema_analysis(VarDeclAST& var_decl);
    void sema_analysis(VarDefListAST& var_del_list);
    void sema_analysis(VarDefAST_1& var_def);
    void sema_analysis(VarDefAST_2& var_def);
    void sema_analysis(InitValAST& init_val);

    void sema_analysis(FuncDefAST& func_def);
    void sema_analysis(FuncTypeAST& fun_type) {return;}
    void sema_analysis(BlockAST& block);
    void sema_analysis(BlockItemListAST& block_item_list);
    void sema_analysis(BlockItemAST_1& block_item);
    void sema_analysis(BlockItemAST_2& block_item);
    void sema_analysis(StmtAST_1& stmt);
    void sema_analysis(StmtAST_2& stmt);
    void sema_analysis(StmtAST_3& stmt);
    void sema_analysis(StmtAST_4& stmt);
    void sema_analysis(ExpAST& exp);
    void sema_analysis(LValAST& lval);
    void sema_analysis(PrimaryExpAST_1& primary_exp);
    void sema_analysis(PrimaryExpAST_2& primary_exp);
    void sema_analysis(PrimaryExpAST_3& primary_exp);
    void sema_analysis(NumberAST& number);
    void sema_analysis(UnaryExpAST_1& unary_exp);
    void sema_analysis(UnaryExpAST_2& unary_exp);
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