#include <iostream>
#ifndef PARSER_HPP
#define PARSER_HPP

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
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
};

// FuncDef 也是 BaseAST
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
};

// 我直接硬翻译
// FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
 public:
  std::string func_type;

  void Dump() const override {
    std::cout << "FuncTypeAST { " << func_type << " }";
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
};

// Stmt      ::= "return" Number ";"
class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> number;

  void Dump() const override {
    std::cout << "StmtAST { ";
    number -> Dump();
    std::cout << " }";
  }
};

// Number    ::= INT_CONST;
class NumberAST : public BaseAST {
 public:
  int num;

  void Dump() const override {
    std::cout << num;
  }
};

#endif