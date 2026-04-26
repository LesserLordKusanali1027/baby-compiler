# include <vector>
# include <string>
# include <stack>
# include "../koopa.hpp"
# include "dead_code_elim.hpp"
# include "control_flow_graph.hpp"

DeadCodeElim::~DeadCodeElim() {
    if (control_flow_graph) {
        delete control_flow_graph;
        control_flow_graph = NULL;
    }
    if (block_info) {
        delete []block_info;
        block_info = NULL;
    }
}

void DeadCodeElim::build_CFG() {
    control_flow_graph = new ControlFlowGraph();

    // 添加节点
    for (int i = 0; i < (function_ir->basic_blocks).size(); i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir->basic_blocks)[i]);
        control_flow_graph -> add_node(basic_block_ir -> name);
    }
    // 在基本块之间添加边
    int block_nums = (function_ir -> basic_blocks).size();
    for (int i = 0; i < block_nums; i++) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>( (function_ir -> basic_blocks)[i] );

        if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back())) {
            ValueIR_1* value = dynamic_cast<ValueIR_1*>((basic_block_ir -> values).back());
            if (value -> opcode == "jump")
                control_flow_graph -> add_edge(i, value -> operand);
            else if (value -> opcode != "ret" && i != block_nums-1)
                control_flow_graph -> add_edge(i, i+1);
        }
        else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back())) {
            ValueIR_5* value = dynamic_cast<ValueIR_5*>((basic_block_ir -> values).back());
            if (value -> opcode == "br") {
                control_flow_graph -> add_edge(i, value -> operand2);
                control_flow_graph -> add_edge(i, value -> operand3);
            }
        }
        else if (dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back())) {
            ValueIR_7* value = dynamic_cast<ValueIR_7*>((basic_block_ir -> values).back());
            if (value -> opcode != "ret" && i != block_nums-1)
                control_flow_graph -> add_edge(i, i+1);
        }
        else if (i != block_nums-1) {
            control_flow_graph -> add_edge(i, i+1);
        }
    }

    // 得到从 %entry 出发的所有叶节点
    std::vector<int> sink_ids = control_flow_graph -> get_sink_ids();
    
    // 添加附加节点 exit
    std::string exit_str = "exit";
    control_flow_graph -> add_node(exit_str);

    // 加入叶节点 -> exit 的边
    for (int i = 0; i < sink_ids.size(); i++)
        control_flow_graph -> add_edge(sink_ids[i], block_nums);
}

void DeadCodeElim::build_info() {
    // 为记录 use-def、IN-OUT 的数据结构分配空间
    block_info = new BlockInfo[(function_ir -> basic_blocks).size() + 1];
    // 准备记录哪些是局部数组
    array_symbol.clear();
    // 记录会产生影响的符号
    std::unordered_set<std::string> useful_symbol;
    // 逐个基本块看
    for (int i = (function_ir -> basic_blocks).size() - 1; i >= 0; i--) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir -> basic_blocks)[i]);

        // 记录在使用之前就被赋值的变量
        std::unordered_set<std::string> no_use;
        // 工具，记录被使用了的变量，为了能让 no_use 正确记录
        std::unordered_set<std::string> be_use;
        // 先从前往后遍历指令
        for (int j = 0; j < (basic_block_ir -> values).size(); j++) {
            if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
                ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);
                if (value_ir -> opcode == "load" && (value_ir -> operand)[0] == '@') {
                    // 如果 load 到 @ 开头的，不是全局量、不是数组参数、不是局部数组，而是局部变量、变量参数
                    // 就加入 be_use
                    if (array_symbol.count(value_ir -> operand) == 0 && ext_symbol.count(value_ir -> operand) == 0) {
                        be_use.insert(value_ir -> operand);
                    }
                }
            }
            else if (dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j])) {
                ValueIR_4* value_ir = dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j]);
                if ((value_ir -> operand2)[0] == '@') {
                    // 如果 store 的目标不是 % 指针
                    // 是 @ 开头的局部变量，记入 def
                    // 当没有 use 过时，再记入 no_use，即没使用就被赋值了的变量
                    if (array_symbol.count(value_ir -> operand2) == 0 && ext_symbol.count(value_ir -> operand2) == 0) {
                        block_info[i].def.insert(value_ir -> operand2);
                        if (be_use.count(value_ir -> operand2) == 0)
                            no_use.insert(value_ir -> operand2);
                    }
                }
            }
            else if (dynamic_cast<ValueIR_8*>((basic_block_ir -> values)[j])) {
                ValueIR_8* value_ir = dynamic_cast<ValueIR_8*>((basic_block_ir -> values)[j]);
                // 不用说了，直接记入局部数组
                array_symbol.insert(value_ir -> target);
            }
        }

        // 再从后向前遍历指令
        // 开始
        for (int j = (basic_block_ir -> values).size() - 1; j >= 0; j--) {
            if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j])) {
                ValueIR_1* value_ir = dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j]);
                if (value_ir -> opcode == "ret" && (value_ir -> operand)[0] == '%')
                    useful_symbol.insert(value_ir -> operand);
            }
            else if (dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j])) {
                ValueIR_2* value_ir = dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j]);
                // 目标符号是有用的
                if (useful_symbol.count(value_ir -> target) == 1) {
                    if ((value_ir -> operand1)[0] == '%')
                        useful_symbol.insert(value_ir -> operand1);
                    else if ((value_ir -> operand1)[0] == '@') { // 如果不是全局、不是参数，是局部数组，就加入 use
                        if (ext_symbol.count(value_ir -> operand1) == 0) {
                            block_info[i].use.insert(value_ir -> operand1);
                            useful_symbol.insert(value_ir -> operand1);
                        }
                    }
                    if ((value_ir -> operand2)[0] == '%') {
                        useful_symbol.insert(value_ir -> operand2);
                        // std::cout << "Useful symbol: " << value_ir -> operand2 << '\n';
                    }
                }
            }
            else if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
                ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);
                // 对于 load，当 target 有用时
                // 如果右侧是 % 的指针，记入 useful
                // 如果右侧是 @ ，为全局量不用管，为数组参数也不用管
                // 为局部变量若没被赋值就加入 use，局部数组直接加入 use
                if (value_ir -> opcode == "load") {
                    if (useful_symbol.count(value_ir -> target) == 1) {
                        if ((value_ir -> operand)[0] == '%')
                            useful_symbol.insert(value_ir -> operand);
                        else if (ext_symbol.count(value_ir -> operand) == 0 && no_use.count(value_ir -> operand) == 0) {
                            block_info[i].use.insert(value_ir -> operand);
                            useful_symbol.insert(value_ir -> operand);
                        }
                    }
                }
            }
            else if (dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j])) {
                ValueIR_4* value_ir = dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j]);
                // 对于 store
                // 目标为 % 指针时，直接加入 useful，这么做会漏掉一个可以优化的地方，但不漏的话太复杂，妥协了
                // 目标为 @ 时，如果为全局，认为产生了影响加入 useful；如果为局部变量，认为没有影响
                if ((value_ir -> operand2)[0] == '%' && (value_ir -> operand1)[0] == '%') {
                    useful_symbol.insert(value_ir -> operand1);
                    useful_symbol.insert(value_ir -> operand2);
                }
                else {
                    if (ext_symbol.count(value_ir -> operand2) == 1 && (value_ir -> operand1)[0] == '%')
                        useful_symbol.insert(value_ir -> operand1);
                }
            }
            else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j])) {
                ValueIR_5* value_ir = dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j]);
                // br，控制流指令，临时符号参数必须有用
                if ((value_ir -> operand1)[0] == '%')
                    useful_symbol.insert(value_ir -> operand1);
            }
            else if (dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j])) {
                ValueIR_6* value_ir = dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j]);
                // call 这个简单，所有临时符号参数都认为是有用的
                for (int k = 0; k < (value_ir -> parameters).size(); k++) {
                    if ((value_ir -> parameters)[k].at(0) == '%')
                        useful_symbol.insert((value_ir -> parameters)[k]);
                }
            }
        }
    }
    // 至此，use-def 已经求好
}

// 重新计算单个块的 use，只是把对 OUT 有影响的加入
void DeadCodeElim::re_build_use(int idx) {
    BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir -> basic_blocks)[idx]);

    // 把 build_info 中的部分内容重走一遍
    // 记录在使用之前就被赋值的变量
    std::unordered_set<std::string> no_use;
    // 工具，记录被使用了的变量，为了能让 no_use 正确记录
    std::unordered_set<std::string> be_use;
    // 先从前往后遍历指令
    for (int j = 0; j < (basic_block_ir -> values).size(); j++) {
        if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
            ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);
            if (value_ir -> opcode == "load" && (value_ir -> operand)[0] == '@') {
                // 如果 load 到 @ 开头的，不是全局量、不是数组参数、不是局部数组，而是局部变量、变量参数
                // 就加入 be_use
                if (array_symbol.count(value_ir -> operand) == 0 && ext_symbol.count(value_ir -> operand) == 0) {
                    be_use.insert(value_ir -> operand);
                }
            }
        }
        else if (dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j])) {
            ValueIR_4* value_ir = dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j]);
            if ((value_ir -> operand2)[0] == '@') {
                // 如果 store 的目标不是 % 指针
                // 当没有 use 过时，再记入 no_use，即没使用就被赋值了的变量
                if (array_symbol.count(value_ir -> operand2) == 0 && ext_symbol.count(value_ir -> operand2) == 0) {
                    if (be_use.count(value_ir -> operand2) == 0)
                        no_use.insert(value_ir -> operand2);
                }
            }
        }
    }

    // 再从后向前遍历指令
    // 记录会产生影响的符号
    std::unordered_set<std::string> useful_symbol;
    // 开始
    for (int j = (basic_block_ir -> values).size() - 1; j >= 0; j--) {
        if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j])) {
            ValueIR_1* value_ir = dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j]);
            if (value_ir -> opcode == "ret" && (value_ir -> operand)[0] == '%')
                useful_symbol.insert(value_ir -> operand);
        }
        else if (dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j])) {
            ValueIR_2* value_ir = dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j]);
            // 目标符号是有用的
            if (useful_symbol.count(value_ir -> target) == 1) {
                if ((value_ir -> operand1)[0] == '%')
                    useful_symbol.insert(value_ir -> operand1);
                else if ((value_ir -> operand1)[0] == '@') { // 如果不是全局、不是参数，是局部数组，就加入 use
                    if (ext_symbol.count(value_ir -> operand1) == 0) {
                        block_info[idx].use.insert(value_ir -> operand1);
                        useful_symbol.insert(value_ir -> operand1);
                    }
                }
                if ((value_ir -> operand2)[0] == '%')
                    useful_symbol.insert(value_ir -> operand2);
            }
        }
        else if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
            ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);
            // 对于 load，当 target 有用时
            // 如果右侧是 % 的指针，记入 useful
            // 如果右侧是 @ ，为全局量不用管，为数组参数也不用管
            // 为局部变量若没被赋值就加入 use，局部数组直接加入 use
            if (value_ir -> opcode == "load") {
                if (useful_symbol.count(value_ir -> target) == 1) {
                    if ((value_ir -> operand)[0] == '%')
                        useful_symbol.insert(value_ir -> operand);
                    else if (ext_symbol.count(value_ir -> operand) == 0 && no_use.count(value_ir -> operand) == 0) {
                        block_info[idx].use.insert(value_ir -> operand);
                        useful_symbol.insert(value_ir -> operand);
                    }
                }
            }
        }
        else if (dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j])) {
            ValueIR_4* value_ir = dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j]);
            // 对于 store
            // 目标为 % 指针时，直接加入 useful，这么做会漏掉一个可以优化的地方，但不漏的话太复杂，妥协了
            // 目标为 @ 时，如果为全局，认为产生了影响加入 useful；如果为 OUT 中变量，也认为有影响，其他没有影响
            if ((value_ir -> operand2)[0] == '%' && (value_ir -> operand1)[0] == '%')
                useful_symbol.insert(value_ir -> operand1);
            else {
                if (ext_symbol.count(value_ir -> operand2) == 1 && (value_ir -> operand1)[0] == '%')
                    useful_symbol.insert(value_ir -> operand1);
                else if (block_info[idx].OUT.count(value_ir -> operand2) == 1 && (value_ir -> operand1)[0] == '%') // 多出的内容
                    useful_symbol.insert(value_ir -> operand1);
            }
        }
        else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j])) {
            ValueIR_5* value_ir = dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j]);
            // br，控制流指令，临时符号参数必须有用
            if ((value_ir -> operand1)[0] == '%')
                useful_symbol.insert(value_ir -> operand1);
        }
        else if (dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j])) {
            ValueIR_6* value_ir = dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j]);
            // call 这个简单，所有临时符号参数都认为是有用的
            for (int k = 0; k < (value_ir -> parameters).size(); k++) {
                if ((value_ir -> parameters)[k].at(0) == '%')
                    useful_symbol.insert((value_ir -> parameters)[k]);
            }
        }
    }
}

void DeadCodeElim::data_flow_alg() {
    int block_num = control_flow_graph -> get_size();
    // 记录 IN 是否改变，决定是否终止
    std::unordered_set<std::string>* pre_IN = new std::unordered_set<std::string>[block_num];
    // 开始迭代
    while(1) {
        // 记下迭代之前的 IN
        for (int i = 0; i < block_num; i++) {
            pre_IN[i].clear();
            pre_IN[i] = block_info[i].IN;
        }
        // 计算每个块
        for (int i = 0; i < block_num - 1; i++) {
            // 求 OUT[B]
            // 不要忘记清空
            block_info[i].OUT.clear();
            std::vector<int> succ_ids = control_flow_graph -> get_successor_ids(i);
            for (int j = 0; j < succ_ids.size(); j++) {
                int id = succ_ids[j];
                block_info[i].OUT.insert(block_info[id].IN.begin(), block_info[id].IN.end());
            }
            // 更新 use
            re_build_use(i);
            // 求 IN[B]
            block_info[i].IN.clear();
            for (const auto& elem : block_info[i].OUT) {
                if (block_info[i].def.find(elem) == block_info[i].def.end())
                    block_info[i].IN.insert(elem);
            }
            for (const auto& elem : block_info[i].use)
                block_info[i].IN.insert(elem);
        }

        // 查看 IN 是否到达不动点
        bool if_changed = false;
        for (int i = 0; i < block_num; i++) {
            if (block_info[i].IN != pre_IN[i]) {
                if_changed = true;
                break;
            }
        }
        // 如果没变，就退出迭代
        if (!if_changed)  break;
    }
    // 清空 new 的内存
    for (int i = 0; i < block_num; i++)
        pre_IN[i].clear();
    delete []pre_IN;
}

void DeadCodeElim::code_elim() {
    // 记录会产生影响的符号
    std::unordered_set<std::string> useful_symbol;
    // // 输出 def
    // std::cout << "def: \n";
    // for (int i = 0; i < (function_ir -> basic_blocks).size(); i++) {
    //     std::cout << i << ". ";
    //     for (const auto& elem : block_info[i].def) {
    //         std::cout << elem << ' ';
    //     }
    //     if (block_info[i].def.size() == 0)
    //         std::cout << "empty";
    //     std::cout << '\n';
    // }
    // // 输出 use
    // std::cout << "use: \n";
    // for (int i = 0; i < (function_ir -> basic_blocks).size(); i++) {
    //     std::cout << i << ". ";
    //     for (const auto& elem : block_info[i].use) {
    //         std::cout << elem << ' ';
    //     }
    //     if (block_info[i].use.size() == 0)
    //         std::cout << "empty";
    //     std::cout << '\n';
    // }
    // // 输出 IN
    // std::cout << "IN: \n";
    // for (int i = 0; i < (function_ir -> basic_blocks).size(); i++) {
    //     std::cout << i << ". ";
    //     for (const auto& elem : block_info[i].IN) {
    //         std::cout << elem << ' ';
    //     }
    //     if (block_info[i].IN.size() == 0)
    //         std::cout << "empty";
    //     std::cout << '\n';
    // }
    // // 输出 OUT
    // std::cout << "OUT: \n";
    // for (int i = 0; i < (function_ir -> basic_blocks).size(); i++) {
    //     std::cout << i << ". ";
    //     for (const auto& elem : block_info[i].OUT) {
    //         std::cout << elem << ' ';
    //     }
    //     if (block_info[i].OUT.size() == 0)
    //         std::cout << "empty";
    //     std::cout << '\n';
    // }

    // 记录需要 alloc 的变量，为所有 use 的并，即所有被用来产生影响的变量
    std::unordered_set<std::string> var_array_alloc;
    for (int i = 0; i < (function_ir -> basic_blocks).size(); i++) {
        for (const auto& elem : block_info[i].use)
            var_array_alloc.insert(elem);
    }
    
    // 两次遍历，先正向再反向
    for (int i = (function_ir -> basic_blocks).size() - 1; i >= 0; i--) {
        BasicBlockIR* basic_block_ir = dynamic_cast<BasicBlockIR*>((function_ir -> basic_blocks)[i]);
        
        // 记录每个块中的全局变量、全局数组、数组参数、被用到的 (在 OUT、use 中的) 局部数组产生的指针，用于处理 store
        std::unordered_set<std::string> useful_ptr;

        // 先正向遍历，记录 useful 的指针
        for (int j = 0; j < (basic_block_ir -> values).size(); j++) {
            if (dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j])) {
                ValueIR_2* value_ir = dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j]);
                
                if (value_ir -> opcode == "getelemptr") {
                    if (ext_symbol.count(value_ir -> operand1) == 1 ||
                        (array_symbol.count(value_ir -> operand1) == 1 && block_info[i].OUT.count(value_ir -> operand1) == 1) ||
                        (array_symbol.count(value_ir -> operand1) == 1 && block_info[i].use.count(value_ir -> operand1) == 1) ||
                        useful_ptr.count(value_ir -> operand1) == 1)
                        useful_ptr.insert(value_ir -> target);
                }
                else if (value_ir -> opcode == "getptr") {
                    if (useful_ptr.count(value_ir -> operand1))
                        useful_ptr.insert(value_ir -> target);
                }
            }
            else if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
                ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);

                // 只有数组参数会通过这种方法得到指针
                // 但实际上我这里不只记录了数组参数，也许会出 Bug ，也许不会
                if (value_ir -> opcode == "load") {
                    if (ext_symbol.count(value_ir -> operand) == 1)
                        useful_ptr.insert(value_ir -> target);
                }
            }
        }

        // 再后向遍历，并记下要删除指令的编号
        std::vector<int> ready_del;
        // 记录该块中的局部变量、变量参数后面是否被使用了，如果用了 store 就不删，否则删
        std::unordered_map<std::string, bool> var_if_used;
        // 开始循环
        for (int j = (basic_block_ir -> values).size() - 1; j >= 0; j--) {
            if (dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j])) {
                ValueIR_1* value_ir = dynamic_cast<ValueIR_1*>((basic_block_ir -> values)[j]);

                if (value_ir -> opcode == "ret" && (value_ir -> operand)[0] == '%')
                    useful_symbol.insert(value_ir -> operand);
            }
            else if (dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j])) {
                ValueIR_2* value_ir = dynamic_cast<ValueIR_2*>((basic_block_ir -> values)[j]);

                if (useful_symbol.count(value_ir -> target) == 1) {
                    if ((value_ir -> operand1)[0] == '%')
                        useful_symbol.insert(value_ir -> operand1);
                    else if ((value_ir -> operand1)[0] == '@') { // 是局部数组，就加入 useful_symbol
                        if (ext_symbol.count(value_ir -> operand1) == 0)
                            useful_symbol.insert(value_ir -> operand1);
                    }
                    if ((value_ir -> operand2)[0] == '%')
                        useful_symbol.insert(value_ir -> operand2);
                }
                else {
                    ready_del.push_back(j);
                }
            }
            else if (dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j])) {
                ValueIR_3* value_ir = dynamic_cast<ValueIR_3*>((basic_block_ir -> values)[j]);

                if (value_ir -> opcode == "alloc") { // 局部变量，如果不在 OUT 中且块中没被使用，就认为没用
                    if (var_array_alloc.count(value_ir -> target) == 0 && !var_if_used.count(value_ir -> target)) {
                        ready_del.push_back(j);
                    }
                }
                else { // load 指令
                    // target 有用的时候
                    if (useful_symbol.count(value_ir -> target) == 1) {
                        // 后面可能是 全局变量、数组参数、变量参数、局部变量、指针
                        // % 指针，直接加入
                        // @ 全局变脸、数组参数，直接加入
                        // @ 变量参数、局部变量，加入 var_if_used 并记一次使用
                        if ((value_ir -> operand)[0] == '%')
                            useful_symbol.insert(value_ir -> operand);
                        else if (ext_symbol.count(value_ir -> operand) == 1)
                            useful_symbol.insert(value_ir -> operand);
                        else {
                            var_if_used[value_ir -> operand] = true;
                            useful_symbol.insert(value_ir -> operand);
                            // alloc 要保留
                            var_array_alloc.insert(value_ir -> operand);
                        }
                    }
                    else {
                        ready_del.push_back(j);
                    }
                }
            }
            else if (dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j])) {
                ValueIR_4* value_ir = dynamic_cast<ValueIR_4*>((basic_block_ir -> values)[j]);
                // store 指令，后面可能是
                // @：全局变量、变量参数、局部变量
                // 如果是全局变量，保留
                // 如果是变量参数或局部变量，仅当 OUT 中有或 var_if_used 中有时保留
                // %：指针，仅当 useful_ptr 中有时保留
                if ((value_ir -> operand2)[0] == '@') {
                    if (ext_symbol.count(value_ir -> operand2)) {
                        if ((value_ir -> operand1)[0] == '%')
                            useful_symbol.insert(value_ir -> operand1);
                    }
                    else if (block_info[i].OUT.count(value_ir -> operand2)) {
                        if (!(var_if_used.count(value_ir -> operand2)))
                            var_if_used[value_ir -> operand2] = false;
                        if ((value_ir -> operand1)[0] == '%')
                            useful_symbol.insert(value_ir -> operand1);
                    }
                    else if (var_if_used.count(value_ir -> operand2) && var_if_used[value_ir -> operand2]) {
                        var_if_used[value_ir -> operand2] = false;
                        if ((value_ir -> operand1)[0] == '%')
                            useful_symbol.insert(value_ir -> operand1);
                    }
                    else
                        ready_del.push_back(j);
                }
                else {
                    if (useful_ptr.count(value_ir -> operand2)) {
                        useful_symbol.insert(value_ir -> operand2);
                        if ((value_ir -> operand1)[0] == '%')
                            useful_symbol.insert(value_ir -> operand1);
                    }
                    else
                        ready_del.push_back(j);
                }
            }
            else if (dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j])) {
                ValueIR_5* value_ir = dynamic_cast<ValueIR_5*>((basic_block_ir -> values)[j]);
                if ((value_ir -> operand1)[0] == '%')
                    useful_symbol.insert(value_ir -> operand1);
            }
            else if (dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j])) {
                ValueIR_6* value_ir = dynamic_cast<ValueIR_6*>((basic_block_ir -> values)[j]);
                for (int k = 0; k < (value_ir -> parameters).size(); k++) {
                    if ((value_ir -> parameters)[k].at(0) == '%')
                        useful_symbol.insert((value_ir -> parameters)[k]);
                }
            }
            else if (dynamic_cast<ValueIR_8*>((basic_block_ir -> values)[j])) {
                ValueIR_8* value_ir = dynamic_cast<ValueIR_8*>((basic_block_ir -> values)[j]);
                // 如果局部数组在 useful 或 OUT 中，保留，否则删除
                if (useful_symbol.count(value_ir -> target) == 0 && block_info[i].OUT.count(value_ir -> target) == 0)
                    ready_del.push_back(j);
            }
        }

        // 最后删除冗余指令
        for (int j = 0; j < ready_del.size(); j++) {
            delete (basic_block_ir -> values)[ready_del[j]];
            (basic_block_ir -> values).erase((basic_block_ir -> values).begin() + ready_del[j]);
        }
    }
}

void DeadCodeElim::do_elimination() {
    // 构建 CFG 图
    build_CFG();
    // 算好 use、def
    build_info();
    // 迭代!
    data_flow_alg();
    // 执行死代码消除
    code_elim();
}