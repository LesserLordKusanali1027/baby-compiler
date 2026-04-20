# include <vector>
# include <string>
# include <stack>
# include "../koopa.hpp"
# include "dead_block_elim.hpp"
# include "control_flow_graph.hpp"

DeadBlockElim::~DeadBlockElim() {
    if (!control_flow_graph) {
        delete control_flow_graph;
        control_flow_graph = NULL;
    }
}

// 构建函数的数据流图
void DeadBlockElim::build_CFG() {
    control_flow_graph = new ControlFlowGraph();

    // 添加节点
    for (int i = 0; i < (function_ir->basic_blocks).size(); i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir->basic_blocks)[i]);
        control_flow_graph -> add_node(basic_block_ir -> name);
    }

    // 在基本块之间添加边
    int size = (function_ir -> basic_blocks).size();
    for (int i = 0; i < size; i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>( (function_ir -> basic_blocks)[i] );

        if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back())) {
            ValueIR_1* value = dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back());
            if (value -> opcode == "jump")
                control_flow_graph -> add_edge(i, value -> operand);
            else if (value -> opcode != "ret" && i != size-1)
                control_flow_graph -> add_edge(i, i+1);
        }
        else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back())) {
            ValueIR_5* value = dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back());
            if (value -> opcode == "br") {
                control_flow_graph -> add_edge(i, value -> operand2);
                control_flow_graph -> add_edge(i, value -> operand3);
            }
        }
        else if (dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back())) {
            ValueIR_7* value = dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back());
            if (value -> opcode != "ret" && i != size-1)
                control_flow_graph -> add_edge(i, i+1);
        }
        else if (i != size-1) {
            control_flow_graph -> add_edge(i, i+1);
        }
    }
}

void DeadBlockElim::do_elimination() {
    build_CFG();
    // 取得孤立的基本块
    std::vector<int> isolated_blocks = control_flow_graph -> get_isolated_nodes();
    // 删除
    for (int i = 0; i < isolated_blocks.size(); i++) {
        delete (function_ir -> basic_blocks)[isolated_blocks[i] - i];
        (function_ir -> basic_blocks).erase((function_ir -> basic_blocks).begin() + isolated_blocks[i] - i);
    }
}