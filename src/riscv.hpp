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

class Visitor_ir {
  private:
    std::string register_name[16] = {"x0","t0","t1","t2","t3","t4","t5","t6","a0","a2","a3","a4","a5","a6","a7"};
    int current_register = 1; // 最靠前的空闲寄存器
    std::ofstream file;
    // 算术表达式增加:
    std::unordered_map<std::string, int> symbol_register;
    
  public:
    Visitor_ir(const char* output) {
        file.open(output);
        if (!file.is_open())
            std::cout << "Failed to open file " << output << std::endl;
        
        symbol_register["0"] = 0;
    }
    ~Visitor_ir() {
        file.close();
    }

    void riscv_get(ProgramIR& program);
    void riscv_get(FunctionIR& function);
    void riscv_get(BasicBlockIR& basic_block);
    void riscv_get(ValueIR_1& value);
    void riscv_get(ValueIR_2& value);
};

# endif