# ifndef SEMA_HPP
# define SEMA_HPP

# include <unordered_map>
# include <string>
# include <stack>
# include <variant>

class BaseAST;
class CompUnitAST;

class DeclAST_1; // 改
class DeclAST_2; // 增
class ConstDeclAST;
class BTypeAST;
class ConstDefListAST;
class ConstDefAST;
class ConstInitValAST;
class VarDeclAST; // 增
class VarDefListAST; // 增
class VarDefAST_1; // 增
class VarDefAST_2; // 增
class InitValAST;  // 增

class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class BlockItemListAST;
class BlockItemAST_1;
class BlockItemAST_2;
class StmtAST_1; // 改
class StmtAST_2; // 增
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
        if (!table.count(name)) // 保险起见
            return false;
        return std::holds_alternative<int>(table[name]);
    }
    bool if_var(std::string& name) {
        if (!table.count(name))
            return false;
        return std::holds_alternative<std::string>(table[name]);
    }

    // 返回结果
    int get_const(std::string& name) {
        if (if_var(name)) {
            std::cout << "Symbol table has a mistake: '" << name << "' is recorded as a var, not a const.\n";
            exit(-1); 
        }
        return std::get<int>(table[name]);
    }
    std::string get_var(std::string& name) {
        if (if_const(name)) {
            std::cout << "Symbol table has a mistake: '" << name << "' is recorded as a const, not a var.\n";
            exit(-1);
        }
        return std::get<std::string>(table[name]);
    }
};

// 定义枚举类型，用于设定检查模式
enum Mode { NONE = 0, VAR_UNDF, CONST_UNDF, UNDF };

// CompUnit      ::= FuncDef;

// Decl          ::= ConstDecl | VarDecl; 改
// ConstDecl     ::= "const" BType ConstDefList ";";
// BType         ::= "int";
// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
// ConstDef      ::= IDENT "=" ConstInitVal;
// ConstInitVal  ::= ConstExp;
// VarDecl       ::= BType VarDefList ";"; 增
// VarDefList    ::= VarDef | VarDefList "," VarDef; 增
// VarDef        ::= IDENT | IDENT "=" InitVal; 增
// InitVal       ::= Exp; 增

// FuncDef       ::= FuncType IDENT "(" ")" Block;
// FuncType      ::= "int";
// Block         ::= "{" BlockItemList "}";
// BlockItemList ::= %empty | BlockItemList BlockItem
// BlockItem     ::= Decl | Stmt;
// Stmt          ::= LVal "=" Exp ";" | "return" Exp ";"; 改
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
    SymbolTable symbol_table; // 符号表
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

    void sema_analysis(DeclAST_1& decl); // 改
    void sema_analysis(DeclAST_2& decl); // 增
    void sema_analysis(ConstDeclAST& const_decl);
    void sema_analysis(BTypeAST& byte) {return;}
    void sema_analysis(ConstDefListAST& const_def_list);
    void sema_analysis(ConstDefAST& const_def);
    void sema_analysis(ConstInitValAST& const_init_val);
    void sema_analysis(VarDeclAST& var_decl); // 增
    void sema_analysis(VarDefListAST& var_del_list); // 增
    void sema_analysis(VarDefAST_1& var_def); // 增
    void sema_analysis(VarDefAST_2& var_def); // 增
    void sema_analysis(InitValAST& init_val); // 增

    void sema_analysis(FuncDefAST& func_def);
    void sema_analysis(FuncTypeAST& fun_type) {return;}
    void sema_analysis(BlockAST& block);
    void sema_analysis(BlockItemListAST& block_item_list);
    void sema_analysis(BlockItemAST_1& block_item);
    void sema_analysis(BlockItemAST_2& block_item);
    void sema_analysis(StmtAST_1& stmt); // 增
    void sema_analysis(StmtAST_2& stmt); // 改
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

    // 供 IR 生成时查询
    int get_const(std::string& name) {
        return symbol_table.get_const(name);
    }
    std::string get_var(std::string& name) {
        return symbol_table.get_var(name);
    }
};

# endif