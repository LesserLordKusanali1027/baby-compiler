# include <vector>
# include <string>
# include <stack>
# include "../koopa.hpp"
# include "dead_block_elim.hpp"

class DataFlowGraph {
  private:
    std::vector<std::string> block_name;
    std::vector<std::vector<int>*> DFG;

  public:
    DataFlowGraph(){}
    ~DataFlowGraph() {
        for (int i = 0; i < DFG.size(); i++) {
            DFG[i] -> clear();
        }
        DFG.clear();
    }

    int get_block_index(std::string& name) {
        int idx = 0;
        for (int i = 0; i < block_name.size(); i++) {
            if (name == block_name[i]) {
                idx = i;
                break;
            }
        }
        return idx;
    }

    void add_node(std::string& name) {
        block_name.push_back(name);

        std::vector<int>* tmp_ptr = new std::vector<int>;
        DFG.push_back(tmp_ptr);
    }
    void add_edge(int i, int j) { // 由编号加边，加入 i -> j 的有向边
        DFG[i] -> push_back(j);
    }
    void add_edge(int i, std::string& name_j) { // 由基本块编号和名加边，加入 i -> j 的有向边
        int j = get_block_index(name_j);

        add_edge(i, j);
    }

    // 返回孤立节点的编号
    std::vector<int> get_isolated_nodes() {
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
};

void dead_block_elimination(FunctionIR* function_ir) {
    DataFlowGraph data_flow_graph;

    // 添加节点
    for (int i = 0; i < (function_ir->basic_blocks).size(); i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir->basic_blocks)[i]);
        data_flow_graph.add_node(basic_block_ir -> name);
    }

    // 在基本块之间添加边
    int size = (function_ir -> basic_blocks).size();
    for (int i = 0; i < size; i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>( (function_ir -> basic_blocks)[i] );

        if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back())) {
            ValueIR_1* value = dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back());
            if (value -> opcode == "jump")
                data_flow_graph.add_edge(i, value -> operand);
            else if (value -> opcode != "ret" && i != size-1)
                data_flow_graph.add_edge(i, i+1);
        }
        else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back())) {
            ValueIR_5* value = dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back());
            if (value -> opcode == "br") {
                data_flow_graph.add_edge(i, value -> operand2);
                data_flow_graph.add_edge(i, value -> operand3);
            }
        }
        else if (dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back())) {
            ValueIR_7* value = dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back());
            if (value -> opcode != "ret" && i != size-1)
                data_flow_graph.add_edge(i, i+1);
        }
        else if (i != size-1) {
            data_flow_graph.add_edge(i, i+1);
        }
    }

    // 取得孤立的基本块
    std::vector<int> isolated_blocks = data_flow_graph.get_isolated_nodes();
    // 删除
    int deleted = 0; // 注意删除时下标会跟着移动
    for (int i = 0; i < isolated_blocks.size(); i++) {
        delete (function_ir -> basic_blocks)[isolated_blocks[i] - deleted];
        (function_ir -> basic_blocks).erase((function_ir -> basic_blocks).begin() + isolated_blocks[i] - deleted);
        deleted++;
    }
}