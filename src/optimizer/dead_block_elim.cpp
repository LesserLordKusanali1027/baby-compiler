# include <vector>
# include <string>
# include <stack>
# include "../koopa.hpp"
# include "dead_block_elim.hpp"
# include "data_flow_graph.hpp"

void dead_block_elimination(FunctionIR* function_ir, DataFlowGraph* data_flow_graph) {
    // 取得孤立的基本块
    std::vector<int> isolated_blocks = data_flow_graph -> get_isolated_nodes();
    // 删除
    int deleted = 0; // 注意删除时下标会跟着移动
    for (int i = 0; i < isolated_blocks.size(); i++) {
        delete (function_ir -> basic_blocks)[isolated_blocks[i] - deleted];
        (function_ir -> basic_blocks).erase((function_ir -> basic_blocks).begin() + isolated_blocks[i] - deleted);
        deleted++;
    }
}