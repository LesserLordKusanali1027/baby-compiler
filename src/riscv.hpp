# ifndef RISCV_HPP
# define RISCV_HPP

# include <fstream>
# include <string>
# include <unordered_map>

class ProgramIR;
class FunctionIR;
class BasicBlockIR;
class ValueIR_1;
class ValueIR_2;
class ValueIR_3;
class ValueIR_4;

class Visitor_ir {
  private:
    std::string register_name[16] = {"x0","t0","t1","t2","t3","t4","t5","t6","a0","a2","a3","a4","a5","a6","a7"};
    int current_register = 1; // 最靠前的空闲寄存器
    std::ofstream file;
    // 算术表达式增加，但如果都放在内存上就不需要这个了
    // std::unordered_map<std::string, int> symbol_register;
    // 记录每个变量、临时符号在栈上的相对偏移
    std::unordered_map<std::string, int> stack_usage;
  public:
    Visitor_ir(const char* output) {
        file.open(output);
        if (!file.is_open())
            std::cout << "Failed to open file " << output << std::endl;
        
        // symbol_register["0"] = 0;
    }
    ~Visitor_ir() {
        file.close();
    }

    // 为重复代码设置的工具函数
    int ParamReg(std::string param); // 获取/分配参数对应的寄存器
    int TargetReg(int param1, int param2); // target 应给存到哪个寄存器
    int ThreeOp(ValueIR_2& value, std::string opcode); // 执行三操作数指令
    // 用来布置函数栈空间的函数
    int stack_setup(FunctionIR& function);

    void riscv_get(ProgramIR& program);
    void riscv_get(FunctionIR& function);
    void riscv_get(BasicBlockIR& basic_block);
    void riscv_get(ValueIR_1& value);
    void riscv_get(ValueIR_2& value);
    void riscv_get(ValueIR_3& value);
    void riscv_get(ValueIR_4& value);
    
};

# endif