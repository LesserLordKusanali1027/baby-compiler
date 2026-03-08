# ifndef KOOPA_HPP
# define KOOPA_HPP

# include <vector>
# include <iostream>
# include <fstream>
# include <stack>
# include "riscv.hpp"

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
        for (int i = 0; i < functions.size(); i++)
            functions[i] -> Dump();
    }

    void Dump_file(std::ofstream& file) override {
        for (int i = 0; i < globals.size(); i++)
            globals[i] -> Dump_file(file);
        for (int i = 0; i < functions.size(); i++)
            functions[i] -> Dump_file(file);
    }

    void accept(Visitor_ir& visitor) override {
        visitor.riscv_get(*this);
    }
};

class FunctionIR : public BaseIR {
  public:
    std::string name;
    std::string function_type;
    std::vector<BaseIR*> basic_blocks;

    void Dump() const override {
        std::cout << "fun @" << name << "(): " << function_type << " {\n";
        for (int i = 0; i < basic_blocks.size(); i++)
            basic_blocks[i] -> Dump();
        std::cout << "}";
    }

    void Dump_file(std::ofstream& file) override {
        file << "fun @" << name << "(): " << function_type << " {\n";
        for (int i = 0; i < basic_blocks.size(); i++)
            basic_blocks[i] -> Dump_file(file);
        file << "}";
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

enum LVal_Mode { START = 0, LOAD, STORE };

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
// 定义 visitor 模式
class Visitor_ast {
  public:
    // 最后会将 program 指向构建的 Koopa IR
    // 因为 ir_init 参数要统一，这里用来传参
    ProgramIR* program;
  private:
    FunctionIR* function;
    BasicBlockIR* basic_block;

    // ir_init 返回值要统一，这里用来记录返回值
    int tmp_symbol = 0;  // 临时符号，这种写法可能埋了个雷：基本块不能太长，否则 int 溢出
    // int integer; 暂时用不到
    char ch;
    // 算术表达式添加：需要一个栈来存放数值和临时符号
    std::stack<std::string> stk;
    // 进入 LVal 后有两种模式，load 或 store
    LVal_Mode lval_mode = START;
    
  public:
    void ir_init(CompUnitAST& comp_unit);

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
    void ir_init(FuncTypeAST& func_type);
    void ir_init(BlockAST& block);
    void ir_init(BlockItemListAST& block_item_list);
    void ir_init(BlockItemAST_1& block_item);
    void ir_init(BlockItemAST_2& block_item);
    void ir_init(StmtAST_1& stmt);
    void ir_init(StmtAST_2& stmt);
    void ir_init(StmtAST_3& stmt);
    void ir_init(StmtAST_4& stmt);
    void ir_init(ExpAST& exp);
    void ir_init(LValAST& lval);
    void ir_init(PrimaryExpAST_1& primary_exp);
    void ir_init(PrimaryExpAST_2& primary_exp);
    void ir_init(PrimaryExpAST_3& primary_exp);
    void ir_init(NumberAST& number);
    void ir_init(UnaryExpAST_1& unary_exp);
    void ir_init(UnaryExpAST_2& unary_exp);
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