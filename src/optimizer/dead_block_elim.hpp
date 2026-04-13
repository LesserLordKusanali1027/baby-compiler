# ifndef DEAD_BLOCK_ELIM
# define DEAD_BLOCK_ELIM

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
class ValueIR_9;

class DataFlowGraph;

void dead_block_elimination(FunctionIR* function_ir);

# endif