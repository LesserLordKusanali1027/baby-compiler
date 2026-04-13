# include "../koopa.hpp"
# include "optimizer.hpp"
# include "dead_block_elim.hpp"

void Optimizer::optimizer(ProgramIR& program_ir) {
    for (int i = 0; i < program_ir.functions.size(); i++) {
        if (dynamic_cast<FunctionIR*>(program_ir.functions[i])) {
            FunctionIR* function_ir = dynamic_cast<FunctionIR*>(program_ir.functions[i]);
            dead_block_elimination(function_ir);
        }
    }
}