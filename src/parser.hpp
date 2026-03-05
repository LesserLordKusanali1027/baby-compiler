# ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <memory>
# include "sema.hpp"
# include "koopa.hpp"

// 所有 AST 的基类
class BaseAST {
  public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual void accept(Visitor_ast& visitor){};

    virtual void accept(Visitor_sema& visitor){};
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// Decl          ::= ConstDecl | VarDecl;
class DeclAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> constdecl;

    void Dump() const override {
        std::cout << "DeclAST { ";
        constdecl->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class DeclAST_2 : public BaseAST {
    public:
    std::unique_ptr<BaseAST> vardecl;

    void Dump() const override {
        std::cout << "DeclAST { ";
        vardecl->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// ConstDecl     ::= "const" BType ConstDefList ";";
class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> constdeflist;

    void Dump() const override {
        std::cout << "ConstDeclAST { ";
        btype->Dump();
        std::cout << ", ";
        constdeflist->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// BType         ::= "int";
class BTypeAST : public BaseAST {
  public:
    std::string btype;

    void Dump() const override {
        std::cout << btype;
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
class ConstDefListAST : public BaseAST {
  public:
    std::vector<std::unique_ptr<BaseAST>> constdefs;

    void push_back(std::unique_ptr<BaseAST> constdef) {
        constdefs.push_back(std::move(constdef));
    }

    void Dump() const override {
        std::cout << "ConstDefListAST { ";
        for (int i = 0; i < constdefs.size(); i++) {
            constdefs[i]->Dump();
            if (i != constdefs.size()-1)
                std::cout << ", ";
        }
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// ConstDef      ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST {
  public:
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;

    void Dump() const override {
        std::cout << "ConstDef { ";
        std::cout << ident << ", " << '=' << ", ";
        constinitval->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// ConstInitVal  ::= ConstExp;
class ConstInitValAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> constexp;

    void Dump() const override {
        std::cout << "ConstInitVal { ";
        constexp->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// VarDecl       ::= BType VarDefList ";";
class VarDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> vardeflist;

    void Dump() const override {
        std::cout << "VarDeclAST { ";
        btype->Dump();
        std::cout << ", ";
        vardeflist->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// VarDefList    ::= VarDef | VarDefList "," VarDef;
class VarDefListAST : public BaseAST {
  public:
    std::vector<std::unique_ptr<BaseAST>> vardefs;

    void push_back(std::unique_ptr<BaseAST> vardef) {
        vardefs.push_back(std::move(vardef));
    }

    void Dump() const override {
        std::cout << "VarDefListAST { ";
        for (int i = 0; i < vardefs.size(); i++) {
            vardefs[i]->Dump();
            if (i != vardefs.size()-1)
                std::cout << ", ";
        }
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// VarDef        ::= IDENT | IDENT "=" InitVal;
class VarDefAST_1 : public BaseAST {
  public:
    std::string ident;

    void Dump() const override {
        std::cout << "VarDefAST { ";
        std::cout << ident << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class VarDefAST_2 : public BaseAST {
  public:
    std::string ident;
    std::unique_ptr<BaseAST> initval;

    void Dump() const override {
        std::cout << "VarDefAST { ";
        std::cout << ident << ", =, ";
        initval->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// InitVal       ::= Exp;
class InitValAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "InitValAST { ";
        exp->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// Block         ::= "{" BlockItemList "}";
class BlockAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> blockitemlist;

    void Dump() const override {
        std::cout << "BlockAST { ";
        blockitemlist -> Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// BlockItemList ::= %empty | BlockItemList BlockItem
class BlockItemListAST : public BaseAST {
  public:
    std::vector<std::unique_ptr<BaseAST>> blockitems;

    void push_back(std::unique_ptr<BaseAST> blockitem) {
        blockitems.push_back(std::move(blockitem)); // 这里好像不能直接用 blockitem，要加上std::move，具体原因还不懂
    }

    void Dump() const override {
        std::cout << "BlockItemListAST { ";
        for (int i = 0; i < blockitems.size(); i++) {
            blockitems[i]->Dump();
            if (i != blockitems.size()-1)
                std::cout << ", ";
        }
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// BlockItem     ::= Decl | Stmt;
class BlockItemAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> decl;

    void Dump() const override {
        std::cout << "BlockItemAST { ";
        decl->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class BlockItemAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "BlockItemAST { ";
        stmt->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// Stmt        ::= LVal "=" Exp ";" | "return" Exp ";";
class StmtAST_1 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "StmtAST { ";
        lval->Dump();
        std::cout << ", =, ";
        exp->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        // visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class StmtAST_2 : public BaseAST {
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// LVal          ::= IDENT;
class LValAST : public BaseAST {
  public:
    std::string ident;

    void Dump() const override {
        std::cout << "LValAST { " << ident << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// PrimaryExp  ::= "(" Exp ")" | LVal | Number;
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class PrimaryExpAST_2 : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lval;

    void Dump() const override {
        std::cout << "PrimaryExpAST { ";
        lval->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};
class PrimaryExpAST_3 : public BaseAST {
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
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

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

// ConstExp      ::= Exp;
class ConstExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "ConstExp { ";
        exp->Dump();
        std::cout << " }";
    }

    void accept(Visitor_ast& visitor) override {
        visitor.ir_init(*this);
    }

    void accept(Visitor_sema& visitor) override {
        visitor.sema_analysis(*this);
    }
};

# endif