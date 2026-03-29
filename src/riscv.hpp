# ifndef RISCV_HPP
# define RISCV_HPP

# include <fstream>
# include <string>
# include <unordered_map>

class ProgramIR;
class GlobalIR_1;
class GlobalIR_2;
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
class ValueIR_8;

// 数组名称 + 对应维度
struct ArraySymbolInfo {
    std::string array_name;
    int symbol_dim;
};
// 记录数组信息
class ArrayInfo {
  private:
    // 数组大小
    std::unordered_map<std::string, std::vector<int>> array_size;
    // 符号和数组名称的对应关系
    std::unordered_map<std::string, ArraySymbolInfo*> symbol_array;
    
  public:
    ArrayInfo() { }
    ~ArrayInfo() {
        array_size.clear();
        symbol_array.clear();
    }

    // 每经过一个函数就要把临时符号清空一次，否则会重复
    void CleanSymbol() {
        for (auto it = symbol_array.begin(); it != symbol_array.end(); ) {
            if (!it->first.empty() && it->first[0] == '%') {
                delete it->second; // 释放 new 出来的内存
                it = symbol_array.erase(it); // erase 返回下一个有效迭代器
            } 
            else  it++;
        }
    }

    // 添加新数组的名字和大小信息
    void Add_Array(std::vector<int>& new_array_size, std::string& new_array_name) {
        for (int i = 0; i < new_array_size.size(); i++) {
            array_size[new_array_name].push_back(new_array_size[i]);
        }
    }

    // 加入数组符号，它自然对应着最高维度
    void Add_Symbol(std::string& new_array_name) {
        ArraySymbolInfo* symbol_info = new ArraySymbolInfo();
        symbol_info -> symbol_dim = 1;
        symbol_info -> array_name = new_array_name;
        symbol_array[new_array_name] = symbol_info;
    }
    // 加入临时符号，根据其上级符号确定其对应的维度
    void Add_Symbol(std::string& new_symbol_name, std::string& former_symbol_name) {
        // 当超出了索引，就不放了，对应最低维的 getelemptr 指令
        int tmp_symbol_dim = symbol_array[former_symbol_name] -> symbol_dim;
        int tmp_array_dim = array_size[ symbol_array[former_symbol_name]->array_name].size();
        if (tmp_symbol_dim >= tmp_array_dim)
            return;

        ArraySymbolInfo* symbol_info = new ArraySymbolInfo();
        symbol_info -> symbol_dim = (symbol_array[former_symbol_name]->symbol_dim) + 1;
        symbol_info -> array_name = symbol_array[former_symbol_name] -> array_name;
        symbol_array[new_symbol_name] = symbol_info;
    }

    // 返回某个符号的单位偏移量
    int Get_Size(std::string& symbol_name) {
        int size = 4;

        int start_dim = symbol_array[symbol_name]->symbol_dim;
        int array_dim = array_size[symbol_array[symbol_name]->array_name].size();
        for (int i = start_dim; i < array_dim; i++) {
            size *= array_size[symbol_array[symbol_name]->array_name][i];
        }

        return size;
    }
};

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

    // 主要为了处理 getelemptr
    ArrayInfo array_info;
    // 记录所有全局数组
    std::unordered_map<std::string, int> global_arrays;

  public:
    Visitor_ir(const char* output) {
        file.open(output);
        if (!file.is_open())
            std::cout << "Failed to open file " << output << std::endl;
    }
    ~Visitor_ir() {
        file.close();
    }

    // 处理特定指令的函数
    void riscv_lw(std::string target_register, int offset);
    void riscv_sw(std::string target_register, int offset, std::string tool_register);

    // 为重复代码设置的工具函数
    int ParamReg(std::string param); // 获取、分配参数对应的寄存器
    int TargetReg(int param1, int param2); // target 应给存到哪个寄存器
    int ThreeOp(ValueIR_2& value, std::string opcode); // 执行三操作数指令

    // 用来布置函数栈空间的函数
    void stack_setup(FunctionIR& function);

    // RISC-V 生成函数
    void riscv_get(ProgramIR& program);
    void riscv_get(GlobalIR_1& global);
    void riscv_get(GlobalIR_2& global);
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
    void riscv_get(ValueIR_8& value);
};

# endif