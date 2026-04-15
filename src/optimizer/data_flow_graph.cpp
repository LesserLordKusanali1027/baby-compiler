# ifndef DATA_FLOW_GRAPH
# define DATA_FLOW_GRAPH

# include "vector"
# include "string"
# include "stack"
# include "data_flow_graph.hpp"

int DataFlowGraph::get_block_index(std::string& name) {
    int idx = 0;
    for (int i = 0; i < block_name.size(); i++) {
        if (name == block_name[i]) {
            idx = i;
            break;
        }
    }
    return idx;
}

void DataFlowGraph::add_node(std::string& name) {
    block_name.push_back(name);

    std::vector<int>* tmp_ptr = new std::vector<int>;
    DFG.push_back(tmp_ptr);
}
void DataFlowGraph::add_edge(int i, int j) { // 由编号加边，加入 i -> j 的有向边
    DFG[i] -> push_back(j);
}
void DataFlowGraph::add_edge(int i, std::string& name_j) { // 由基本块编号和名加边，加入 i -> j 的有向边
    int j = get_block_index(name_j);

    add_edge(i, j);
}

// 返回孤立节点的编号
std::vector<int> DataFlowGraph::get_isolated_nodes() {
    bool* if_isolated = new bool[block_name.size()];
    for (int i = 0; i < block_name.size(); i++)
        if_isolated[i] = true;
        
    // 算法
    std::stack<int> block_stk;
    block_stk.push(0);
    while(!block_stk.empty()) {
        int idx = block_stk.top();
        block_stk.pop();
        if_isolated[idx] = false;

        int next_idx;
        for (int i = 0; i < DFG[idx]->size(); i++) {
            next_idx = DFG[idx]->at(i);
            if (if_isolated[next_idx])
                block_stk.push(next_idx);
        }
    }

    std::vector<int> isolated_node;
    for (int i = 0; i < block_name.size(); i++) {
        if (if_isolated[i]) {
            isolated_node.push_back(i);
        }
    }

    return isolated_node;
}

# endif