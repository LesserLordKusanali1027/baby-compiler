# ifndef KOOPA_HPP
# define KOOPA_HPP

# include <vector>
# include <iostream>
# include <fstream>
# include "riscv.hpp"

class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class ExpAST;
class PrimaryExpAST_1;
class PrimaryExpAST_2;
class NumberAST;
class UnaryExpAST_1;
class UnaryExpAST_2;
class UnaryOpAST;

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

// CompUnit    ::= FuncDef;
// FuncDef     ::= FuncType IDENT "(" ")" Block;
// FuncType    ::= "int";
// Block       ::= "{" Stmt "}";
// Stmt        ::= "return" Exp ";";
// Exp         ::= UnaryExp;
// PrimaryExp  ::= "(" Exp ")" | Number;
// Number      ::= INT_CONST;
// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp     ::= "+" | "-" | "!";
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
    int tmp_symbol = -1;  // 临时符号，这种写法可能埋了个雷：基本块不能太长，否则 int 溢出
    int integer;
    char ch;
    
  public:
    void ir_init(CompUnitAST& comp_unit);
    void ir_init(FuncDefAST& func_def);
    void ir_init(FuncTypeAST& func_type);
    void ir_init(BlockAST& block);
    void ir_init(StmtAST& stmt);
    void ir_init(ExpAST& exp);
    void ir_init(PrimaryExpAST_1& primary_exp);
    void ir_init(PrimaryExpAST_2& primary_exp);
    void ir_init(NumberAST& number);
    void ir_init(UnaryExpAST_1& unary_exp);
    void ir_init(UnaryExpAST_2& unary_exp);
    void ir_init(UnaryOpAST& unary_op);

    void Dump() {
        program -> Dump();
    }
    void Dump_file(std::ofstream& file) {
        program -> Dump_file(file);
    }
};

# endif