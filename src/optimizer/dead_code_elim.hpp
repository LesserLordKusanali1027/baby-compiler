# ifndef DEAD_CODE_ELIM
# define DEAD_CODE_ELIM

# include <unordered_set>

class FunctionIR;

class ControlFlowGraph;

struct BlockInfo {
    std::unordered_set<std::string> use;
    std::unordered_set<std::string> def;
    std::unordered_set<std::string> IN;
    std::unordered_set<std::string> OUT;
};

class DeadCodeElim {
  private:
    FunctionIR* function_ir;
    ControlFlowGraph* control_flow_graph;
    BlockInfo* block_info; // 记录 use-def、IN-OUT
    // 重要的外部符号
    // 包括全局变量、数组参数
    std::unordered_set<std::string> ext_symbol;
    // 记录局部数组
    std::unordered_set<std::string> array_symbol;

  public:
    DeadCodeElim (FunctionIR* function, std::unordered_set<std::string>& symbol_table) {
        function_ir = function;
        control_flow_graph = NULL;
        block_info = NULL;
        ext_symbol = symbol_table;
    }
    ~DeadCodeElim();

    void build_CFG();
    void build_info();
    // 重新计算 use
    void re_build_use(int idx);

    // 迭代算法求 IN-OUT
    void data_flow_alg();

    // 执行指令消除
    void code_elim();

    void do_elimination();
};

# endif