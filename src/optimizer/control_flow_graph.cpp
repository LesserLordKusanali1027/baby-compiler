# ifndef CONTROL_FLOW_GRAPH
# define CONTROL_FLOW_GRAPH

# include <vector>
# include <string>
# include <stack>
# include "control_flow_graph.hpp"

int ControlFlowGraph::get_block_index(std::string& name) {
    int idx = 0;
    for (int i = 0; i < block_name.size(); i++) {
        if (name == block_name[i]) {
            idx = i;
            break;
        }
    }
    return idx;
}

int ControlFlowGraph::get_size() {
    return block_name.size();
}

void ControlFlowGraph::add_node(std::string& name) {
    block_name.push_back(name);

    std::vector<int>* tmp_ptr = new std::vector<int>;
    CFG.push_back(tmp_ptr);
}
void ControlFlowGraph::add_edge(int i, int j) { // 由编号加边，加入 i -> j 的有向边
    CFG[i] -> push_back(j);
}
void ControlFlowGraph::add_edge(int i, std::string& name_j) { // 由基本块编号和名加边，加入 i -> j 的有向边
    int j = get_block_index(name_j);

    add_edge(i, j);
}

std::vector<int> ControlFlowGraph::get_successor_ids(int pre_id) { // 得到所有后继节点
    std::vector<int> succ_ids;

    for (int i = 0; i < CFG[pre_id] -> size(); i++) {
        succ_ids.push_back(CFG[pre_id] -> at(i));
    }

    return succ_ids;
}
std::vector<int> ControlFlowGraph::get_predecessor_ids(int succ_id) { // 得到所有前驱节点
    std::vector<int> pre_ids;

    for (int i = 0; i < CFG.size(); i++) {
        for (int j = 0; j < CFG[i] -> size(); j++) {
            if (CFG[i] -> at(j) == succ_id)
                pre_ids.push_back(i);
        }
    }

    return pre_ids;
}

std::vector<int> ControlFlowGraph::get_sink_ids(int entry_id) { // 得到所有叶节点
    std::vector<int> sink_ids;

    int size = block_name.size();
    std::stack<int> block_stk;
    bool* if_reached = new bool[size];
    for (int i = 0; i < size; i++)
        if_reached[i] = false;
    
    // 开始算法
    block_stk.push(0);
    if_reached[0] = true;
    while(!block_stk.empty()) {
        int curr_id = block_stk.top();
        block_stk.pop();
        // 已经访问过了
        if_reached[curr_id] = true;
        // 得到后继节点
        std::vector<int> succ_ids = get_successor_ids(curr_id);
        // 如果没有后继了，加入 
        if (succ_ids.size() == 0) {
            sink_ids.push_back(curr_id);
            continue;
        }
        for (int i = 0; i < succ_ids.size(); i++) {
            if (!if_reached[succ_ids[i]])
                block_stk.push(succ_ids[i]);
        }
    }

    return sink_ids;
}

// 返回孤立节点的编号
std::vector<int> ControlFlowGraph::get_isolated_nodes() {
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
        for (int i = 0; i < CFG[idx]->size(); i++) {
            next_idx = CFG[idx]->at(i);
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