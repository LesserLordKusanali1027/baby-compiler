%code requires {
#include <memory>
#include <string>
#include "parser.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "parser.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  ConstDefListAST* constitemlist_val;
  BlockItemListAST* blockitemlist_val;
  VarDefListAST* vardeflist_val;
  CompUnitListAST* compunitlist_val;
  FuncFParamListAST* funcfparamlist_val;
  FuncRParamListAST* funcrparamlist_val;

  ConstSizeListAST* constsizelist_val;
  ConstInitValListAST* constinitvallist_val;
  VarSizeListAST* varsizelist_val;
  InitValListAST* initvallist_val;
  ExpListAST* explist_val;

  ConstExpListAST* constexplist_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN GE LE EQ NE AND OR CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef Block Stmt Number Exp PrimaryExp UnaryExp UnaryOp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl ConstDef BType ConstInitVal BlockItem LVal ConstExp VarDecl VarDef InitVal
%type <constitemlist_val> ConstDefList
%type <blockitemlist_val> BlockItemList
%type <vardeflist_val> VarDefList
%type <ast_val> MatchedStmt UnmatchedStmt
%type <ast_val> FuncFParam
%type <compunitlist_val> CompUnitList
%type <funcfparamlist_val> FuncFParamList
%type <funcrparamlist_val> FuncRParamList
%type <ast_val> CompUnitItem
%type <constsizelist_val> ConstSizeList
%type <constinitvallist_val> ConstInitValList
%type <varsizelist_val> VarSizeList
%type <initvallist_val> InitValList
%type <explist_val> ExpList
%type <constexplist_val> ConstExpList

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
// CompUnit    ::= CompUnitList;
CompUnit
  : CompUnitList {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit -> comp_unit_list = unique_ptr<BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

// CompUnitList  ::= CompUnitItem | CompUnitList CompUnitItem;
CompUnitList
  : CompUnitItem {
    auto ast = new CompUnitListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | CompUnitList CompUnitItem {
    $1 -> push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

// CompUnitItem  ::= FuncDef | Decl;
CompUnitItem
  : FuncDef {
    auto ast = new CompUnitItemAST_1();
    ast -> func_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Decl {
    auto ast = new CompUnitItemAST_2();
    ast -> decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// Decl          ::= ConstDecl | VarDecl;
Decl
  : ConstDecl {
    auto ast = new DeclAST_1();
    ast -> constdecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST_2();
    ast -> vardecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// ConstDecl     ::= "const" BType ConstDefList ";";
ConstDecl
  : CONST BType ConstDefList ';' {
    auto ast = new ConstDeclAST();
    ast -> btype = unique_ptr<BaseAST>($2);
    ast -> constdeflist = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// BType         ::= "int" | "void";
BType
  : INT {
    auto ast = new BTypeAST();
    auto str = new string("int");
    ast -> btype = *unique_ptr<string>(str);
    $$ = ast;
  }
  | VOID {
    auto ast = new BTypeAST();
    auto str = new string("void");
    ast -> btype = *unique_ptr<string>(str);
    $$ = ast;
  }
  ;

// ConstDefList  ::= ConstDef | ConstDefList "," ConstDef;
ConstDefList
  : ConstDef {
    auto ast = new ConstDefListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | ConstDefList ',' ConstDef {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// ConstDef ::= IDENT "=" ConstInitVal | IDENT ConstSizeList "=" ConstInitVal;
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST_1();
    ast -> ident = *unique_ptr<string>($1);
    ast -> constinitval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ConstSizeList '=' ConstInitVal {
    auto ast = new ConstDefAST_2();
    ast -> ident = *unique_ptr<string>($1);
    ast -> constsizelist = unique_ptr<BaseAST>($2);
    ast -> constinitval = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// ConstSizeList ::= "[" ConstExp "]" | ConstSizeList "[" ConstExp "]";
ConstSizeList
  : '[' ConstExp ']' {
    auto ast = new ConstSizeListAST();
    ast -> push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  | ConstSizeList '[' ConstExp ']' {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// ConstInitVal ::= ConstExp | "{" [ConstInitValList] "}";
ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST_1();
    ast -> constexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new ConstInitValAST_2();
    ast -> constinitvallist = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | '{' ConstInitValList '}' {
    auto ast = new ConstInitValAST_2();
    ast -> constinitvallist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// ConstInitValList ::= ConstInitVal | ConstInitValList "," ConstInitVal;
ConstInitValList
  : ConstInitVal {
    auto ast = new ConstInitValListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | ConstInitValList ',' ConstInitVal {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// VarDecl       ::= BType VarDefList ";";
VarDecl
  : BType VarDefList ';' {
    auto ast = new VarDeclAST();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> vardeflist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// VarDefList    ::= VarDef | VarDefList "," VarDef;
VarDefList
  : VarDef {
    auto ast = new VarDefListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | VarDefList ',' VarDef {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// VarDef ::= IDENT | IDENT "=" InitVal | IDENT VarSizeList | IDENT VarSizeList "=" InitVal;
VarDef
  : IDENT {
    auto ast = new VarDefAST_1();
    ast -> ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST_2();
    ast -> ident = *unique_ptr<string>($1);
    ast -> initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT VarSizeList {
    auto ast = new VarDefAST_3();
    ast -> ident = *unique_ptr<string>($1);
    ast -> varsizelist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT VarSizeList '=' InitVal {
    auto ast = new VarDefAST_4();
    ast -> ident = *unique_ptr<string>($1);
    ast -> varsizelist = unique_ptr<BaseAST>($2);
    ast -> initval = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// VarSizeList ::= "[" ConstExp "]" | VarSizeList "[" ConstExp "]";
VarSizeList
  : '[' ConstExp ']' {
    auto ast = new VarSizeListAST();
    ast -> push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  | VarSizeList '[' ConstExp ']' {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// InitVal ::= Exp | "{" [InitValList] "}";
InitVal
  : Exp {
    auto ast = new InitValAST_1();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new InitValAST_2();
    ast -> initvallist = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | '{' InitValList '}' {
    auto ast = new InitValAST_2();
    ast -> initvallist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// InitValList ::= InitVal | InitValList "," InitVal;
InitValList
  : InitVal {
    auto ast = new InitValListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | InitValList ',' InitVal {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// FuncDef     ::= BType IDENT "(" [FuncFParamList] ")" Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, BType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : BType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_param_list = unique_ptr<BaseAST>(nullptr);
    ast -> block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BType IDENT '(' FuncFParamList ')' Block {
    auto ast = new FuncDefAST();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_param_list = unique_ptr<BaseAST>($4);
    ast -> block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

// FuncFParamList ::= FuncFParam | FuncFParamList "," FuncFParam;
FuncFParamList
  : FuncFParam {
    auto ast = new FuncFParamListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | FuncFParamList ',' FuncFParam {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// FuncFParam ::= BType IDENT | BType IDENT "[" "]" [ConstExpList];
FuncFParam
  : BType IDENT {
    auto ast = new FuncFParamAST_1();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    $$ = ast;
  }
  | BType IDENT '[' ']' {
    auto ast = new FuncFParamAST_2();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> constexplist = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | BType IDENT '[' ']' ConstExpList {
    auto ast = new FuncFParamAST_2();
    ast -> btype = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> constexplist = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// ConstExpList ::= "[" ConstExp "]" | ConstExpList "[" ConstExp "]";
ConstExpList
  : '[' ConstExp ']' {
    auto ast = new ConstExpListAST();
    ast -> push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  | ConstExpList '[' ConstExp ']' {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// Block         ::= "{" BlockItemList "}";
Block
  : '{' BlockItemList '}' {
    auto ast = new BlockAST();
    ast -> blockitemlist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// BlockItemList ::= empty | BlockItemList BlockItem
BlockItemList
  : %empty {
    auto ast = new BlockItemListAST();
    $$ = ast;
  }
  | BlockItemList BlockItem {
    $1 -> push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

// BlockItem     ::= Decl | Stmt;
BlockItem
  : Decl {
    auto ast = new BlockItemAST_1();
    ast -> decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST_2();
    ast -> stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// Stmt ::= MatchedStmt | UnmatchedStmt;
Stmt
  : MatchedStmt {
    auto ast = new StmtAST_1();
    ast -> matchedstmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnmatchedStmt {
    auto ast = new StmtAST_2();
    ast -> unmatchedstmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// MatchedStmt ::= LVal "=" Exp ";" | "return" [Exp] ";" | [Exp] ";" | Block | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt | "while" "(" Exp ")" Stmt | "break" ";" | "continue" ";";
MatchedStmt
  : LVal '=' Exp ';' {
    auto ast = new MatchedStmtAST_1();
    ast -> lval = unique_ptr<BaseAST>($1);
    ast -> exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new MatchedStmtAST_2();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new MatchedStmtAST_2();
    ast -> exp = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new MatchedStmtAST_3();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new MatchedStmtAST_3();
    ast -> exp = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | Block {
    auto ast = new MatchedStmtAST_4();
    ast -> block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE MatchedStmt {
    auto ast = new MatchedStmtAST_5();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> matchedstmt1 = unique_ptr<BaseAST>($5);
    ast -> matchedstmt2 = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new MatchedStmtAST_6();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new MatchedStmtAST_7();
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new MatchedStmtAST_8();
    $$ = ast;
  }
  ;

// UnmatchedStmt ::= "if" "(" Exp ")" Stmt | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt;
UnmatchedStmt
  : IF '(' Exp ')' Stmt {
    auto ast = new UnmatchedStmtAST_1();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE UnmatchedStmt {
    auto ast = new UnmatchedStmtAST_2();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> matchedstmt = unique_ptr<BaseAST>($5);
    ast -> unmatchedstmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  ;

// Number    ::= INT_CONST;
Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast -> num = $1;
    $$ = ast;
  }
  ;

// Exp         ::= LOrExp;
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast -> lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// LVal ::= IDENT | IDENT ExpList;
LVal
  : IDENT {
    auto ast = new LValAST_1();
    ast -> ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT ExpList {
    auto ast = new LValAST_2();
    ast -> ident = *unique_ptr<string>($1);
    ast -> explist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// ExpList ::= "[" Exp "]" | ExpList "[" Exp "]";
ExpList
  : '[' Exp ']' {
    auto ast = new ExpListAST();
    ast -> push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  | ExpList '[' Exp ']' {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }

// PrimaryExp  ::= "(" Exp ")" | LVal | Number;
PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST_1();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST_2();
    ast -> lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST_3();
    ast -> number = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp | IDENT "(" [FuncRParamList] ")";
UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST_1();
    ast -> primaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST_2();
    ast -> unaryop = unique_ptr<BaseAST>($1);
    ast -> unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST_3();
    ast -> ident = *unique_ptr<string>($1);
    ast -> func_r_param_list = unique_ptr<BaseAST>(nullptr);
    $$ = ast;
  }
  | IDENT '(' FuncRParamList ')' {
    auto ast = new UnaryExpAST_3();
    ast -> ident = *unique_ptr<string>($1);
    ast -> func_r_param_list = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// FuncRParamList ::= Exp | FuncRParamList "," Exp;
FuncRParamList
  : Exp {
    auto ast = new FuncRParamListAST();
    ast -> push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | FuncRParamList ',' Exp {
    $1 -> push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// UnaryOp     ::= "+" | "-" | "!";
UnaryOp
  : '+' {
    auto ast = new UnaryOpAST();
    ast -> ch = '+';
    $$ = ast;
  }
  | '-' {
    auto ast = new UnaryOpAST();
    ast -> ch = '-';
    $$ = ast;
  }
  | '!' {
    auto ast = new UnaryOpAST();
    ast -> ch = '!';
    $$ = ast;
  }
  ;

// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
MulExp
  : UnaryExp {
    auto ast = new MulExpAST_1();
    ast -> unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST_2();
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> ch = '*';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST_2();
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> ch = '/';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExpAST_2();
    ast -> mulexp = unique_ptr<BaseAST>($1);
    ast -> ch = '%';
    ast -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
AddExp
  : MulExp {
    auto ast = new AddExpAST_1();
    ast -> mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST_2();
    ast -> addexp = unique_ptr<BaseAST>($1);
    ast -> ch = '+';
    ast -> mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExpAST_2();
    ast -> addexp = unique_ptr<BaseAST>($1);
    ast -> ch = '-';
    ast -> mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
RelExp
  : AddExp {
    auto ast = new RelExpAST_1();
    ast -> addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp '<' AddExp {
    auto ast = new RelExpAST_2();
    ast -> relexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "<";
    ast -> addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new RelExpAST_2();
    ast -> relexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = ">";
    ast -> addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp LE AddExp {
    auto ast = new RelExpAST_2();
    ast -> relexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "<=";
    ast -> addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new RelExpAST_2();
    ast -> relexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = ">=";
    ast -> addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
EqExp
  : RelExp {
    auto ast = new EqExpAST_1();
    ast -> relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExpAST_2();
    ast -> eqexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "==";
    ast -> relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp NE RelExp {
    auto ast = new EqExpAST_2();
    ast -> eqexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "!=";
    ast -> relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LAndExp
  : EqExp {
    auto ast = new LAndExpAST_1();
    ast -> eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp AND EqExp {
    auto ast = new LAndExpAST_2();
    ast -> landexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "&&";
    ast -> eqexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
LOrExp
  : LAndExp {
    auto ast = new LOrExpAST_1();
    ast -> landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    auto ast = new LOrExpAST_2();
    ast -> lorexp = unique_ptr<BaseAST>($1);
    ast -> cmp_str = "||";
    ast -> landexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// ConstExp      ::= Exp;
ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
