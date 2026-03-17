# ifndef KOOPA_HPP
# define KOOPA_HPP

# include <vector>
# include <iostream>
# include <fstream>
# include <stack>
# include <unordered_map>
# include "riscv.hpp"

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
class ConstDefAST;
class ConstInitValAST;
class VarDeclAST;
class VarDefListAST;
class VarDefAST_1;
class VarDefAST_2;
class InitValAST;

class FuncDefAST;
class FuncFParamListAST;
class FuncFParamAST;
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
class LValAST;
class PrimaryExpAST_1;
class PrimaryExpAST_2;
class PrimaryExpAST_3;
class NumberAST;
class UnaryExpAST_1;
class UnaryExpAST_2;
class UnaryExpAST_3;
class FuncRParamListAST;
class UnaryOpAST;
// 算术表达式
class MulExpAST_1;
class MulExpAST_2;
class AddExpAST_1;
class AddExpAST_2;
// 比较和逻辑表达式
class RelExpAST_1;
class RelExpAST_2;
class EqExpAST_1;
class EqExpAST_2;
class LAndExpAST_1;
class LAndExpAST_2;
class LOrExpAST_1;
class LOrExpAST_2;
class ConstExpAST;

// 定义数据结构
// 所有 IR 的基类
class BaseIR {
  public:
    virtual ~BaseIR() = default;

    virtual void Dump() const = 0;

    virtual void Dump_file(std::ofstream& file) = 0;

    virtual void accept(Visitor_ir& visitor){};
};

// ProgramIR 类
class ProgramIR : public BaseIR {
  public:
    std::vector<BaseIR*> globals;
    std::vector<BaseIR*> functions;

    void Dump() const override {
        for (int i = 0; i < globals.size(); i++)
            globals[i] -> Dump();
        for (int i = 0; i < functions.size(); i++) {
            functions[i] -> Dump();
            if (i != functions.size()-1)
                std::cout << "\n\n";
        }
    }

    void Dump_file(std::ofstream& file) override {
        for (int i = 0; i < globals.size(); i++)
            globals[i] -> Dump_file(file);
        for (int i = 0; i < functions.size(); i++) {
            functions[i] -> Dump_file(file);
            if (i != functions.size()-1)
                file << "\n\n";
        }
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class GlobalIR : public BaseIR {
  public:
    std::string name;
    std::string type;
    std::string init_val;

    void Dump() const override {
        std::cout << "global " << name << " = alloc ";
        if (type == "int")
            std::cout << "i32, ";
        std::cout << init_val << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "global " << name << " = alloc ";
        if (type == "int")
            file << "i32, ";
        file << init_val << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class FunctionIR : public BaseIR {
  public:
    std::string name;
    std::string function_type;
    std::vector<std::string> parameters;
    std::vector<BaseIR*> basic_blocks;

    void Dump() const override {
        std::cout << "fun " << name << "(";
        for (int i = 0; i < parameters.size(); i++) {
            std::cout << parameters[i] << ": i32";
            if (i != parameters.size()-1)
                std::cout << ", ";
        }
        std::cout << ")";
        
        if (function_type == "int")
            std::cout << ": i32 {\n";
        else if (function_type == "void")
            std::cout << " {\n";
        
        for (int i = 0; i < basic_blocks.size(); i++) {
            basic_blocks[i] -> Dump();
            if (i != basic_blocks.size()-1)
                std::cout << '\n';
        }
        std::cout << "}";
    }

    void Dump_file(std::ofstream& file) override {
        file << "fun " << name << "(";
        for (int i = 0; i < parameters.size(); i++) {
            file << parameters[i] << ": i32";
            if (i != parameters.size()-1)
                file << ", ";
        }
        file << ") ";
        
        if (function_type == "int")
            file << ":i32 {\n";
        else if (function_type == "void")
            file << " {\n";

        for (int i = 0; i < basic_blocks.size(); i++) {
            basic_blocks[i] -> Dump_file(file);
            if (i != basic_blocks.size()-1)
                file << '\n';
        }
        file << "}";
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class FunctionDeclIR : public BaseIR {
  public:
    std::string name; // 这里的 name 是带 @ 的
    std::string function_type; // 先认为有 "void"、"int"
    std::vector<std::string> param_type; // 先认为有 "int"、"int[]"

    void Dump() const override {
        std::cout << "decl " << name << "(";
        for (int i = 0; i < param_type.size(); i++) {
            if (param_type[i] == "int")
                std::cout << "i32";
            else if (param_type[i] == "int[]")
                std::cout << "*i32";
            
            if (i != param_type.size()-1)
                std::cout << ", ";
        }
        std::cout << ")";
        if (function_type == "int")
            std::cout << ": i32";
        // std::cout << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "decl " << name << "(";
        for (int i = 0; i < param_type.size(); i++) {
            if (param_type[i] == "int")
                file << "i32";
            else if (param_type[i] == "int[]")
                file << "*i32";
            
            if (i != param_type.size()-1)
                file << ", ";
        }
        file << ")";
        if (function_type == "int")
            file << ": i32";
        // file << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class BasicBlockIR : public BaseIR {
  public:
    std::string name;
    std::vector<BaseIR*> values;

    void Dump() const override {
        std::cout << name << ":\n";
        for (int i = 0; i < values.size(); i++)
            values[i] -> Dump();
    }

    void Dump_file(std::ofstream& file) override {
        file << name << ":\n";
        for (int i = 0; i < values.size(); i++)
            values[i] -> Dump_file(file);
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class ValueIR_1 : public BaseIR {
  public:
    std::string opcode;
    std::string operand;

    void Dump() const override {
        std::cout << "  " << opcode << " " << operand << "\n";
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << opcode << " " << operand << "\n";
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_2 : public BaseIR {
  public:
    std::string target;
    std::string opcode;
    std::string operand1;
    std::string operand2;

    void Dump() const override {
        std::cout << "  " << target << " = " << opcode << " ";
        std::cout << operand1 << ", " << operand2 << "\n";
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << target << " = " << opcode << " ";
        file << operand1 << ", " << operand2 << "\n";
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_3 : public BaseIR {
    // 用于 alloc 和 load 这种单操作数单目标指令
  public:
    std::string target;
    std::string opcode;
    std::string operand;

    void Dump() const override {
        std::cout << "  " << target << " = " << opcode << " ";
        std::cout << operand << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << target << " = " << opcode << " ";
        file << operand << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_4 : public BaseIR {
    // 用于 store 这种双操作数无目标指令
  public:
    std::string opcode;
    std::string operand1;
    std::string operand2;

    void Dump() const override {
        std::cout << "  " << opcode << " " << operand1 << ", ";
        std::cout << operand2 << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << opcode << " " << operand1 << ", ";
        file << operand2 << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_5 : public BaseIR {
    // 三操作数无目标指令，用于 br
  public:
    std::string opcode;
    std::string operand1;
    std::string operand2;
    std::string operand3;

    void Dump() const override {
        std::cout << "  " << opcode << " ";
        std::cout << operand1 << ", " << operand2 << ", " << operand3 << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << opcode << " ";
        file << operand1 << ", " << operand2 << ", " << operand3 << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_6 : public BaseIR {
    // 用来放 call
  public:
    std::string opcode;
    std::string operand;
    std::vector<std::string> parameters;
    std::string target;

    void Dump() const override {
        if (!target.empty()) {
            std::cout << "  " << target << " = ";
            std::cout << opcode << " ";
        }
        else {
            std::cout << "  " << opcode << " ";
        }
        std::cout << operand << "(";
        for (int i = 0; i < parameters.size(); i++) {
            std::cout << parameters[i];
            if (i != parameters.size()-1)
                std::cout << ", ";
        }
        std::cout << ")\n";
    }

    void Dump_file(std::ofstream& file) override {
        if (!target.empty()) {
            file << "  " << target << " = ";
            file << opcode << " ";
        }
        else {
            file << "  " << opcode << " ";
        }
        file << operand << "(";
        for (int i = 0; i < parameters.size(); i++) {
            file << parameters[i];
            if (i != parameters.size()-1)
                file << ", ";
        }
        file << ")\n";
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};
class ValueIR_7 : public BaseIR {
    // 放 ret
  public:
    std::string opcode;

    void Dump() const override {
        std::cout << "  " << opcode << '\n';
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << opcode << '\n';
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

enum LVal_Mode { START = 0, LOAD, STORE };

// CompUnit      ::= CompUnitList;
// CompUnitList  ::= CompUnitItem | CompUnitList CompUnitItem;
// CompUnitItem  ::= FuncDef | Decl;

// Decl          ::= ConstDecl | VarDecl;
// ConstDecl     ::= "const" BType ConstDefList ";";
// BType         ::= "int" | "void";
// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
// ConstDef      ::= IDENT "=" ConstInitVal;
// ConstInitVal  ::= ConstExp;
// VarDecl       ::= BType VarDefList ";";
// VarDefList    ::= VarDef | VarDefList "," VarDef;
// VarDef        ::= IDENT | IDENT "=" InitVal;
// InitVal       ::= Exp;

// FuncDef       ::= BType IDENT "(" [FuncFParamList] ")" Block;
// FuncFParamList::= FuncFParam | FuncFParamList "," FuncFParam;
// FuncFParam    ::= BType IDENT;
// Block         ::= "{" BlockItemList "}";
// BlockItemList ::= %empty | BlockItemList BlockItem
// BlockItem     ::= Decl | Stmt;
// Stmt          ::= MatchedStmt | UnmatchedStmt
// MatchedStmt   ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt | "while" "(" Exp ")" Stmt;
// UnmatchedStmt ::= "if" "(" Exp ")" Stmt | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt;
// Exp           ::= LOrExp;
// LVal          ::= IDENT;
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

// 定义 visitor 模式
class Visitor_ast {
    // 因为 ir_init 参数要统一，所以想要在各个节点之间传参协作，需要一些状态变量
  public:
    // IR 数据结构相关
    // 1. 最后会将 program 指向构建的 Koopa IR
    ProgramIR* program;
  private:
    // 2. 遍历 AST 某个函数时构建的 FunctionIR
    FunctionIR* function;
    // 3. 遍历 AST 时构建的 BasicBlockIR，注意和 AST 中的 Block 并不等同
    BasicBlockIR* basic_block;

    // 表达式相关
    // 1. 临时符号，这种写法可能埋了个雷：函数不能太长，否则 int 溢出
    int tmp_symbol;
    // 2. 传递计算符号 + - * / ...
    char ch;
    // 3. 算术表达式添加：需要一个栈来存放数值和临时符号
    std::stack<std::string> exp_stk;
    // 4. 进入 LVal 后有两种模式，load 或 store
    LVal_Mode lval_mode = START;
    
    // if-else 分支语句相关
    // 1. 记录 then-else-end 该用第几组了
    int branch_num;
    // 2. 记录 return 是第几组了
    int return_num;

    // 短路求值相关
    // 1. 基本块命名
    int sub_exp, and_sce, and_exit, or_sce, or_exit;
    // 2. 短路求值结果变量，可能埋了个雷，由于绕过了语义分析，之后可能出现命名冲突
    // 后续或许可以定义成全局变量？
    std::string sce_var = "@sce_result";

    // while 循环语句相关
    // 1. 记录 while 的 entry-body-end 该用第几组了，现在 break、continue 跳转后新建块也用这个
    int while_num;
    // 2. 记录各层 while 循环使用的标签的序号，用于为 break、continue 提供跳转目标
    std::stack<int> while_stk;

    // 函数相关
    // 1. 记录函数返回值类型，用来在生成 call 指令时判断是否要有 target
    std::unordered_map<std::string, std::string> func_table;
    // 2. call 指令需要由 UnaryExpAST_3 和 FuncRParamListAST 共同完成，固需要一个状态变量
    // 因为有函数嵌套，所以需要用栈记录
    std::stack<BaseIR*> value_stk;
    // 3. 有了函数嵌套后，需要用栈来记录 lval_mode了
    std::stack<LVal_Mode> lval_stk;
    // 4. 记录 BType 是否作为函数定义中的 FuncType
    bool if_func_def = false;

    // 全局变量相关
    bool global_decl; // 全局的变量声明
  
    // 工具函数
    void set_lval(LVal_Mode mode) {
        lval_stk.push(lval_mode);
        lval_mode = mode;
    }
    void recover_lval() {
        lval_mode = lval_stk.top();
        lval_stk.pop();
    }
    void init_states() { // 初始化一些状态变量，每个函数开头初始化一次
        // 临时符号
        tmp_symbol = 0;
        // then-else-end 的组数
        branch_num = 1;
        // return 的组数
        return_num = 1;
        // 短路求值的状态
        sub_exp = 1;
        and_sce = 1;
        and_exit = 1;
        or_sce = 1;
        or_exit = 1;
        // while 状态
        while_num = 1;
    }
    void decl_lib_func () { // 将库函数的声明加入 program->functions 和 func_table 中
        FunctionDeclIR* func_decl;
        // int getint()
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@getint";
        func_decl -> function_type = "int";
        (program -> functions).push_back(func_decl);
        func_table["@getint"] = "int";
        // int getch()
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@getch";
        func_decl -> function_type = "int";
        (program -> functions).push_back(func_decl);
        func_table["@getch"] = "int";
        // int getarray(int[])
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@getarray";
        func_decl -> function_type = "int";
        func_decl -> param_type = {"int[]"};
        (program -> functions).push_back(func_decl);
        func_table["@getarray"] = "int";
        // void putint(int)
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@putint";
        func_decl -> function_type = "void";
        func_decl -> param_type = {"int"};
        (program -> functions).push_back(func_decl);
        func_table["@putint"] = "void";
        // void putch(int)
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@putch";
        func_decl -> function_type = "void";
        func_decl -> param_type = {"int"};
        (program -> functions).push_back(func_decl);
        func_table["@putch"] = "void";
        // void putarray(int, int[])
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@putarray";
        func_decl -> function_type = "void";
        func_decl -> param_type = {"int", "int[]"};
        (program -> functions).push_back(func_decl);
        func_table["@putarray"] = "void";
        // void starttime()
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@starttime";
        func_decl -> function_type = "void";
        (program -> functions).push_back(func_decl);
        func_table["@starttime"] = "void";
        // void stoptime()
        func_decl = new FunctionDeclIR();
        func_decl -> name = "@stoptime";
        func_decl -> function_type = "void";
        (program -> functions).push_back(func_decl);
        func_table["stoptime"] = "void";
    }

  public:
    // 节点函数
    void ir_init(CompUnitAST& comp_unit);
    void ir_init(CompUnitListAST& comp_unit_list);
    void ir_init(CompUnitItemAST_1& comp_unit_item);
    void ir_init(CompUnitItemAST_2& comp_unit_item);

    void ir_init(DeclAST_1& decl);
    void ir_init(DeclAST_2& decl);
    void ir_init(ConstDeclAST& const_decl);
    void ir_init(BTypeAST& btype);
    void ir_init(ConstDefListAST& const_def_list);
    void ir_init(ConstDefAST& const_def);
    void ir_init(ConstInitValAST& const_init_val);
    void ir_init(VarDeclAST& var_decl);
    void ir_init(VarDefListAST& var_def_list);
    void ir_init(VarDefAST_1& var_def);
    void ir_init(VarDefAST_2& var_def);
    void ir_init(InitValAST& init_val);

    void ir_init(FuncDefAST& func_def);
    void ir_init(FuncFParamListAST& func_f_param_list);
    void ir_init(FuncFParamAST& func_f_param);
    void ir_init(BlockAST& block);
    void ir_init(BlockItemListAST& block_item_list);
    void ir_init(BlockItemAST_1& block_item);
    void ir_init(BlockItemAST_2& block_item);
    void ir_init(StmtAST_1& stmt);
    void ir_init(StmtAST_2& stmt);
    void ir_init(MatchedStmtAST_1& matched_stmt);
    void ir_init(MatchedStmtAST_2& matched_stmt);
    void ir_init(MatchedStmtAST_3& matched_stmt);
    void ir_init(MatchedStmtAST_4& matched_stmt);
    void ir_init(MatchedStmtAST_5& matched_stmt);
    void ir_init(MatchedStmtAST_6& matched_stmt);
    void ir_init(MatchedStmtAST_7& matched_stmt);
    void ir_init(MatchedStmtAST_8& matched_stmt);
    void ir_init(UnmatchedStmtAST_1& unmatched_stmt);
    void ir_init(UnmatchedStmtAST_2& unmatched_stmt);
    void ir_init(ExpAST& exp);
    void ir_init(LValAST& lval);
    void ir_init(PrimaryExpAST_1& primary_exp);
    void ir_init(PrimaryExpAST_2& primary_exp);
    void ir_init(PrimaryExpAST_3& primary_exp);
    void ir_init(NumberAST& number);
    void ir_init(UnaryExpAST_1& unary_exp);
    void ir_init(UnaryExpAST_2& unary_exp);
    void ir_init(UnaryExpAST_3& unary_exp);
    void ir_init(FuncRParamListAST& func_r_param_list);
    void ir_init(UnaryOpAST& unary_op);
    void ir_init(MulExpAST_1& mul_exp);
    void ir_init(MulExpAST_2& mul_exp);
    void ir_init(AddExpAST_1& add_exp);
    void ir_init(AddExpAST_2& add_exp);
    void ir_init(RelExpAST_1& rel_exp);
    void ir_init(RelExpAST_2& rel_exp);
    void ir_init(EqExpAST_1& eq_exp);
    void ir_init(EqExpAST_2& eq_exp);
    void ir_init(LAndExpAST_1& l_and_exp);
    void ir_init(LAndExpAST_2& l_and_exp);
    void ir_init(LOrExpAST_1& l_or_exp);
    void ir_init(LOrExpAST_2& l_or_exp);
    void ir_init(ConstExpAST& const_exp);

    void Dump() {
        program -> Dump();
    }
    void Dump_file(std::ofstream& file) {
        program -> Dump_file(file);
    }
};

# endif