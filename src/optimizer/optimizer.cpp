# include <unordered_set>
# include "../koopa.hpp"
# include "optimizer.hpp"
# include "control_flow_graph.hpp"
# include "dead_block_elim.hpp"
# include "dead_code_elim.hpp"

// 由这个函数来充当对外的接口，同时构建数据流图并调用一系列优化
void Optimizer::optimizer(ProgramIR& program_ir) {
    // 为了死代码消除的活跃变量分析准备全局变量表
    std::unordered_set<std::string> global_symbol;
    for (int i = 0; i < program_ir.globals.size(); i++) {
        if (dynamic_cast<GlobalIR_1*>(program_ir.globals[i])) {
            GlobalIR_1* global_ir = dynamic_cast<GlobalIR_1*>(program_ir.globals[i]);
            global_symbol.insert(global_ir -> name);
        }
        else {
            GlobalIR_2* global_ir = dynamic_cast<GlobalIR_2*>(program_ir.globals[i]);
            global_symbol.insert(global_ir -> name);
        }
    }

    for (int i = 0; i < program_ir.functions.size(); i++) {
        if (dynamic_cast<FunctionIR*>(program_ir.functions[i])) {
            FunctionIR* function_ir = dynamic_cast<FunctionIR*>(program_ir.functions[i]);
            // 死代码块消除
            DeadBlockElim dead_block_elim(function_ir);
            dead_block_elim.do_elimination();
            
            // 死代码消除
            // 先准备外部变量表
            std::unordered_set<std::string> ext_symbol = global_symbol; // 全局函数
            for (int i = 0; i < (function_ir -> parameters).size(); i++) { // 加入数组参数
                if((function_ir -> param_dims)[i] != 0) {
                    // 这里把 % 换成 @
                    std::string tmp_str = "@" + (function_ir -> parameters)[i].substr(1);
                    ext_symbol.insert(tmp_str);
                }
            }
            // 开始消除
            DeadCodeElim dead_code_elim(function_ir, ext_symbol);
            dead_code_elim.do_elimination();
        }
    }
}