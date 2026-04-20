# ifndef DEAD_BLOCK_ELIM
# define DEAD_BLOCK_ELIM

class FunctionIR;

class ControlFlowGraph;

class DeadBlockElim {
  private:
    FunctionIR* function_ir;
    ControlFlowGraph* control_flow_graph;
  public:
    DeadBlockElim(FunctionIR* function) {
        function_ir = function;
        control_flow_graph = NULL;
    }
    ~DeadBlockElim();

    void build_CFG();
    void do_elimination();
};



# endif