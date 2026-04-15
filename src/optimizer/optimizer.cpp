# include "../koopa.hpp"
# include "optimizer.hpp"
# include "data_flow_graph.hpp"
# include "dead_block_elim.hpp"
# include "dead_code_elim.hpp"

Optimizer::~Optimizer() {
    if (!data_flow_graph) {
        delete data_flow_graph;
        data_flow_graph = NULL;
    }
}

// 构建函数的数据流图
void Optimizer::build_DFG(FunctionIR* function_ir) {
    if (!data_flow_graph) {
        delete data_flow_graph;
        data_flow_graph = NULL;
    }
    data_flow_graph = new DataFlowGraph();

    // 添加节点
    for (int i = 0; i < (function_ir->basic_blocks).size(); i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir->basic_blocks)[i]);
        data_flow_graph -> add_node(basic_block_ir -> name);
    }

    // 在基本块之间添加边
    int size = (function_ir -> basic_blocks).size();
    for (int i = 0; i < size; i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>( (function_ir -> basic_blocks)[i] );

        if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back())) {
            ValueIR_1* value = dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back());
            if (value -> opcode == "jump")
                data_flow_graph -> add_edge(i, value -> operand);
            else if (value -> opcode != "ret" && i != size-1)
                data_flow_graph -> add_edge(i, i+1);
        }
        else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back())) {
            ValueIR_5* value = dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back());
            if (value -> opcode == "br") {
                data_flow_graph -> add_edge(i, value -> operand2);
                data_flow_graph -> add_edge(i, value -> operand3);
            }
        }
        else if (dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back())) {
            ValueIR_7* value = dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back());
            if (value -> opcode != "ret" && i != size-1)
                data_flow_graph -> add_edge(i, i+1);
        }
        else if (i != size-1) {
            data_flow_graph -> add_edge(i, i+1);
        }
    }
}

// 由这个函数来充当对外的接口，同时构建数据流图并调用一系列优化
void Optimizer::optimizer(ProgramIR& program_ir) {
    for (int i = 0; i < program_ir.functions.size(); i++) {
        if (dynamic_cast<FunctionIR*>(program_ir.functions[i])) {
            FunctionIR* function_ir = dynamic_cast<FunctionIR*>(program_ir.functions[i]);
            // 构建数据流图
            build_DFG(function_ir);
            
            // 死代码块消除
            dead_block_elimination(function_ir, data_flow_graph);
            // 死代码消除
            dead_code_elimination(function_ir, data_flow_graph);
        }
    }
}