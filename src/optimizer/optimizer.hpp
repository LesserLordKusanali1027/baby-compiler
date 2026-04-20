# ifndef OPTIMIZER
# define OPTIMIZER

class ProgramIR;

class Optimizer {
  public:
    void optimizer(ProgramIR& program_ir); // 由这个函数来充当对外的接口，同时构建数据流图并调用一系列优化
};

# endif