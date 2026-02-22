#include "koopa.hpp"
#include "parser.hpp"
#include <vector>

// 先不用智能指针unique_ptr<FunctionIR>
ProgramIR* Generate_Program(CompUnitAST* ast) {
    FuncDefAST* fd = dynamic_cast<FuncDefAST*>(ast->func_def.get());

    auto koopa_ir = new ProgramIR();
    if (fd) {
        (koopa_ir -> function).push_back(Generate_Function(fd));
    }

    // 未来全局变量列表在这里补
    
    return koopa_ir;
}

FunctionIR* Generate_Function(FuncDefAST* func_def) {
    BlockAST* fd = dynamic_cast<BlockAST*>(func_def->block.get());

    auto function_ir = new FunctionIR();
    std::string str1 = "i32";
    std::string str2 = func_def->ident;
    function_ir -> name = str2;
    function_ir -> function_type = str1;
    
    if (fd) {
        (function_ir -> basic_block).push_back(Generate_BasicBlock(fd));
    }

    return function_ir;
}

BasicBlockIR* Generate_BasicBlock(BlockAST* block) {
    StmtAST* fd = dynamic_cast<StmtAST*>(block->stmt.get());

    auto basic_block_ir = new BasicBlockIR();
    std::string str = "%entry";
    basic_block_ir -> name = str;

    if (fd) {
        (basic_block_ir -> value).push_back(Generate_Value(fd));
    }

    return basic_block_ir;
}

ValueIR* Generate_Value(StmtAST* stmt) {
    NumberAST* fd = dynamic_cast<NumberAST*>(stmt->number.get());

    auto value_ir = new ValueIR();
    std::string str1 = "ret";
    value_ir -> opcode = str1;
    if (fd) {
        value_ir -> operand = fd -> num;
    }

    return value_ir;
}