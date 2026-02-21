#ifndef IR_GENERATOR_HPP
#define IR_GENERATOR_HPP

#include "parser.hpp"
#include <vector>
#include <iostream>
#include <fstream>

// 所有 IR 的基类
class BaseIR {
public:
    virtual ~BaseIR() = default;

    virtual void Dump() const = 0;
    virtual void Dump_file(std::ofstream& file) = 0;
};

// ProgramIR 类
// API 设计，在以下 4 种方法中，选择了法 4
// 1. void Generate(CompUnitAST& ast, ProgramIR& koopa_ir);
// 2. ProgramIR& Generate(CompUnitAST& ast);
// 3. void Generate(CompUnitAST* ast, ProgramIR* koopa_ir);
// 4. ProgramIR* Generate(CompUnitAST* ast);
// 最后选择把 API 放到类外面
class ProgramIR : public BaseIR {
public:
    std::vector<BaseIR*> global;
    std::vector<BaseIR*> function;
    void Dump() const override {
        for (int i = 0; i < global.size(); i++)
            global[i] -> Dump();
        for (int i = 0; i < function.size(); i++)
            function[i] -> Dump();
    }

    void Dump_file(std::ofstream& file) override {
        for (int i = 0; i < global.size(); i++)
            global[i] -> Dump_file(file);
        for (int i = 0; i < function.size(); i++)
            function[i] -> Dump_file(file);
    }
};

class FunctionIR : public BaseIR {
public:
    std::string name;
    std::string function_type;
    std::vector<BaseIR*> basic_block;
    void Dump() const override {
        std::cout << "fun " << name << "(): " << function_type << " {\n";
        for (int i = 0; i < basic_block.size(); i++)
            basic_block[i] -> Dump();
        std::cout << "}";
    }

    void Dump_file(std::ofstream& file) override {
        file << "fun " << name << "(): " << function_type << " {\n";
        for (int i = 0; i < basic_block.size(); i++)
            basic_block[i] -> Dump_file(file);
        file << "}";
    }
};

class BasicBlockIR : public BaseIR {
public:
    std::string name;
    std::vector<BaseIR*> value;
    void Dump() const override {
        std::cout << name << ":\n";
        for (int i = 0; i < value.size(); i++)
            value[i] -> Dump();
    }

    void Dump_file(std::ofstream& file) override {
        file << name << ":\n";
        for (int i = 0; i < value.size(); i++)
            value[i] -> Dump_file(file);
    }

};

class ValueIR : public BaseIR {
public:
    std::string opcode;
    int operand;
    void Dump() const override {
        std::cout << "  " << opcode << " " << operand << "\n";
    }

    void Dump_file(std::ofstream& file) override {
        file << "  " << opcode << " " << operand << "\n";
    }
};


ProgramIR* Generate_Program(CompUnitAST* ast);
FunctionIR* Generate_Function(FuncDefAST* func_def);
BasicBlockIR* Generate_BasicBlock(BlockAST* block);
ValueIR* Generate_Value(StmtAST* stmt);

#endif