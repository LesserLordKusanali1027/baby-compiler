# include "vector"
# include "string"
# include "stack"

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

    int get_block_index(std::string& name);

    void add_node(std::string& name);
    void add_edge(int i, int j); // 由编号加边，加入 i -> j 的有向边
    void add_edge(int i, std::string& name_j); // 由基本块编号和名加边，加入 i -> j 的有向边

    // 返回孤立节点的编号
    std::vector<int> get_isolated_nodes();
};