# include <vector>
# include <string>
# include <stack>

class ControlFlowGraph {
  private:
    std::vector<std::string> block_name;
    std::vector<std::vector<int>*> CFG;

  public:
    ControlFlowGraph(){}
    ~ControlFlowGraph() {
        for (int i = 0; i < CFG.size(); i++) {
            CFG[i] -> clear();
        }
        CFG.clear();
    }

    int get_block_index(std::string& name);

    int get_size();

    void add_node(std::string& name);
    void add_edge(int i, int j); // 由编号加边，加入 i -> j 的有向边
    void add_edge(int i, std::string& name_j); // 由基本块编号和名加边，加入 i -> j 的有向边

    // 自 C++17 起，返回值优化就是强制性的了，std::vector<int> 会直接构造在调用方法帧上，不会有额外的内存拷贝
    std::vector<int> get_successor_ids(int pre_id); // 返回某节点所有后继
    std::vector<int> get_predecessor_ids(int succ_id); // 返回某节点所有前驱

    // 返回从入口开始没有后继的所有基本块
    std::vector<int> get_sink_ids(int entry_id = 0);

    // 返回孤立节点的编号
    std::vector<int> get_isolated_nodes();
};