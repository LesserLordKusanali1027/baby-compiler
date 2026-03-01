# ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <memory>
# include "koopa.hpp"

// 所有 AST 的基类
class BaseAST {
  public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual void accept(Visitor_ast& visitor){};
};

// 非终结符的类均从基类继承
// CompUnit  ::= FuncDef;
class CompUnitAST : public BaseAST {
  public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
  public:
    std::string func_type;

    void Dump() const override {
        std::cout << "FuncTypeAST { " << func_type << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// Block     ::= "{" Stmt "}";
class BlockAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "BlockAST { ";
        stmt -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// Stmt        ::= "return" Exp ";";
class StmtAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "StmtAST { ";
        exp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// Exp         ::= LOrExp;
class ExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lorexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        lorexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "PrimaryExpAST { ";
        exp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class PrimaryExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> number;

    void Dump() const override {
        std::cout << "PrimaryExpAST { ";
        number->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// Number    ::= INT_CONST;
class NumberAST : public BaseAST {
  public:
    int num;

    void Dump() const override {
        std::cout << num;
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> primaryexp;

    void Dump() const override {
        std::cout << "UnaryExpAST { ";
        primaryexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class UnaryExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> unaryop;
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "UnaryExpAST { ";
        unaryop -> Dump();
        std::cout << ", ";
        unaryexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// UnaryOp     ::= "+" | "-" | "!";
// 表面上有三种选择，实际就是一个 char
class UnaryOpAST : public BaseAST {
  public:
    char ch;

    void Dump() const override{
        std::cout << ch;
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "MulExpAST { ";
        unaryexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class MulExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> mulexp;
    char ch;
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "MulExpAST { ";
        mulexp -> Dump();
        std::cout << ", " << ch << ", ";
        unaryexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> mulexp;

    void Dump() const override {
        std::cout << "AddExpAST { ";
        mulexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class AddExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> addexp;
    char ch;
    std::unique_ptr<BaseAST> mulexp;

    void Dump() const override {
        std::cout << "AddExpAST { ";
        addexp -> Dump();
        std::cout << ", " << ch << ", ";
        mulexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> addexp;

    void Dump() const override {
        std::cout << "RelExpAST { ";
        addexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class RelExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> relexp;
    std::string cmp_str;
    std::unique_ptr<BaseAST> addexp;

    void Dump() const override {
        std::cout << "RelExpAST { ";
        relexp -> Dump();
        std::cout << ", " << cmp_str << ", ";
        addexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> relexp;

    void Dump() const override {
        std::cout << "EqExpAST { ";
        relexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class EqExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> eqexp;
    std::string cmp_str;
    std::unique_ptr<BaseAST> relexp;

    void Dump() const override {
        std::cout << "EqExpAST { ";
        eqexp -> Dump();
        std::cout << ", " << cmp_str << ", ";
        relexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> eqexp;

    void Dump() const override {
        std::cout << "LAndExpAST { ";
        eqexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class LAndExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> landexp;
    std::string cmp_str;
    std::unique_ptr<BaseAST> eqexp;

    void Dump() const override {
        std::cout << "LAndExpAST { ";
        landexp -> Dump();
        std::cout << ", " << cmp_str << ", ";
        eqexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> landexp;

    void Dump() const override {
        std::cout << "LOrExpAST { ";
        landexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};
class LOrExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lorexp;
    std::string cmp_str;
    std::unique_ptr<BaseAST> landexp;

    void Dump() const override {
        std::cout << "LOrExpAST { ";
        lorexp -> Dump();
        std::cout << ", " << cmp_str << ", ";
        landexp -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }
};

# endif