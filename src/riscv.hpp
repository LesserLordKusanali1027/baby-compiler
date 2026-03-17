# ifndef RISCV_HPP
# define RISCV_HPP

# include <fstream>
# include <string>
# include <unordered_map>

class ProgramIR;
class GlobalIR;
class FunctionIR;
class FunctionDeclIR;
class BasicBlockIR;
class ValueIR_1;
class ValueIR_2;
class ValueIR_3;
class ValueIR_4;
class ValueIR_5;
class ValueIR_6;
class ValueIR_7;

class Visitor_ir {
  private:
    std::ofstream file;

    // 寄存器相关
    std::string register_name[16] = {"x0","t0","t1","t2","t3","t4","t5","t6","a0","a1","a2","a3","a4","a5","a6","a7"}; // 通用寄存器
    std::string param_register[8] = {"a0","a1","a2","a3","a4","a5","a6","a7"}; // 传参寄存器
    int current_register = 1; // 最靠前的空闲寄存器
    
    // 函数栈帧相关
    std::unordered_map<std::string, int> var_offset; // 记录每个变量、临时符号在栈上的相对偏移
    int stack_frame; // 整个帧空间大小，对齐到 16 byte
    // 为函数布置栈帧需要的量如下
    int var_space; // 需要为局部变量和临时符号分配的空间
    int ra_space; // 出现 call 则需要为原本的返回地址分配 4 byte 帧空间
    int param_space; // call 的函数超过 8 个参数，需要为其分配帧空间

    // 记录参数名称、是第几个参数，用于处理 store %x, @x 这种将参数存入局部变量的 IR 语句
    std::unordered_map<std::string, int> param_name;

    // 记录所有的全局变量
    std::unordered_map<std::string, int> global_vars;

  public:
    Visitor_ir(const char* output) {
        file.open(output);
        if (!file.is_open())
            std::cout << "Failed to open file " << output << std::endl;
    }
    ~Visitor_ir() {
        file.close();
    }

    // 为重复代码设置的工具函数
    int ParamReg(std::string param); // 获取、分配参数对应的寄存器
    int TargetReg(int param1, int param2); // target 应给存到哪个寄存器
    int ThreeOp(ValueIR_2& value, std::string opcode); // 执行三操作数指令

    // 用来布置函数栈空间的函数
    void stack_setup(FunctionIR& function);

    // RISC-V 生成函数
    void riscv_get(ProgramIR& program);
    void riscv_get(GlobalIR& global);
    void riscv_get(FunctionIR& function);
    void riscv_get(FunctionDeclIR& function_decl) {return;} // 不用管
    void riscv_get(BasicBlockIR& basic_block);
    void riscv_get(ValueIR_1& value);
    void riscv_get(ValueIR_2& value);
    void riscv_get(ValueIR_3& value);
    void riscv_get(ValueIR_4& value);
    void riscv_get(ValueIR_5& value);
    void riscv_get(ValueIR_6& value);
    void riscv_get(ValueIR_7& value);

};

# endif