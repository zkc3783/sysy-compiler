#pragma once
#include <string>
#include <memory>
#include <vector>
// 所有类的声明
class BaseAST; 
class CompUnitAST;
class FuncDefAST;
class FuncFParamsAST;
class FuncFParamAST;

class BlockAST;
class BlockItemAST;

class DeclAST;
class StmtAST;// Stmt


// Decl
class ConstDeclAST;
class VarDeclAST;
class BTypeAST;
class ConstDefAST;
class VarDefAST;
class InitValAST;
class ConstInitValAST;
class LValAST;
class ConstExpAST;

// Expression
class ExpAST;
class PrimaryExpAST;
class UnaryExpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;

class FuncRParamsAST;

/*
    cmh
    BaseAST 是语法树的节点类，它的所有子类表示不同的节点种类
    虽然继承只有一层，但是例如 CompUnitAST 中定义了 std::vector<std::unique_ptr<FuncDefAST>> func_defs;
    用类间关系来说就是关联关系
*/

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;
    // 完成LR的打印，并返回计算结果（临时变量或立即数）所在的变量
    // 删掉了，不用多态。
    // virtual std::string Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {     // 编译开始符
public:
    std::vector<std::unique_ptr<FuncDefAST>> func_defs; // 所有函数定义
    std::vector<std::unique_ptr<DeclAST>> decls;    // 所有全局变量
    void Dump()const;
    void DumpGlobalVar() const;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {         // 定义函数的节点
public:
    std::unique_ptr<BTypeAST> btype;    // 返回值类型
    std::string ident;                  // 函数名标识符
    std::unique_ptr<FuncFParamsAST> func_params;    // 函数参数, nullptr则无参数
    std::unique_ptr<BlockAST> block;    // 函数体
    void Dump() const;
};

class FuncFParamsAST : public BaseAST {     // 函数参数列表
public:
    std::vector<std::unique_ptr<FuncFParamAST>> func_f_params;
    void Dump() const;
};


class FuncFParamAST : public BaseAST {      // 函数参数
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::unique_ptr<BTypeAST> btype;
    std::string ident;
    std::vector<std::unique_ptr<ConstExpAST>> const_exps;   // a[][3]
    std::string Dump() const; // 返回参数类型，如i32, *[i32, 4]
    void getIndex(std::vector<int> &len);
};


// Block 也是 BaseAST
class BlockAST : public BaseAST {       // 单入口单出口的基本块
public:
    std::vector<std::unique_ptr<BlockItemAST>> block_items; //基本块中有很多元素，vector装 zkc
    void Dump(bool new_symbol_tb = true) const;
};

class BlockItemAST : public BaseAST {   // 块中的一个元素
public:
    enum TAG {DECL, STMT};  //zkc 可以是常量变量定义（decl），或者语句（stmt）
    TAG tag;
    std::unique_ptr<DeclAST> decl;
    std::unique_ptr<StmtAST> stmt;
    void Dump() const;
};

class DeclAST : public BaseAST {        // 定义
public:
    enum TAG {CONST_DECL, VAR_DECL};
    TAG tag;
    std::unique_ptr<ConstDeclAST> const_decl;
    std::unique_ptr<VarDeclAST> var_decl;
    void Dump() const;
};

class StmtAST : public BaseAST {
public:
    enum TAG {RETURN, ASSIGN, BLOCK, EXP, WHILE, BREAK, CONTINUE, IF};
    TAG tag; //语句可能是以上的某一种，是哪种就用下面的所需要的类
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<LValAST> lval;
    std::unique_ptr<BlockAST> block;
    std::unique_ptr<StmtAST> stmt;
    std::unique_ptr<StmtAST> if_stmt;
    std::unique_ptr<StmtAST> else_stmt;
/*
    StmtAST::Dump 方法处理不同类型的语句格式，每种语句类型都对应一个格式：
    RETURN语句: "return [Exp];"
        - 如果有表达式exp，则处理 exp->Dump()。
        - 如果没有表达式，则处理 "return;"。

    ASSIGN语句: "LVal = Exp;"
        - 分别处理左值 lval->Dump(true) 和表达式 exp->Dump()。

    BLOCK语句: "Block { ... }"
        - 直接调用 block->Dump() 处理内部的语句块。

    EXP语句: "[Exp];"
        - 处理 exp->Dump()。

    WHILE语句: "while (Exp) Stmt"
        - 分别处理条件表达式 exp->Dump() 和循环体 stmt->Dump()。

    BREAK语句: "break;"
        - 处理 "break;"。

    CONTINUE语句: "continue;"
        - 处理 "continue;"。

    IF语句: "if (Exp) Stmt [else Stmt]"
        - 条件表达式 exp->Dump()，然后是 if_stmt->Dump()，
          可选的 else_stmt->Dump() 处理else部分。
*/
    void Dump() const;
};

class ConstDeclAST : public BaseAST {
public:
    std::vector<std::unique_ptr<ConstDefAST>> const_defs;
    std::unique_ptr<BTypeAST> btype;
    void Dump() const;
};

class VarDeclAST : public BaseAST {
public:
    std::vector<std::unique_ptr<VarDefAST>> var_defs;
    std::unique_ptr<BTypeAST> btype;
    void Dump() const;
};

class BTypeAST : public BaseAST {
public:
    enum TAG {VOID, INT};
    TAG tag;
    void Dump() const;
};

class ConstDefAST : public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::unique_ptr<ConstInitValAST> const_init_val;
    void Dump(bool is_global = false) const;
    void DumpArray(bool is_global = false) const;
};

class VarDefAST: public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::unique_ptr<InitValAST> init_val;   // nullptr implies no init_val
    void Dump(bool is_global = false) const;
    void DumpArray(bool is_global = false) const;
};

class InitValAST : public BaseAST{
public:
    enum TAG { EXP, INIT_LIST};
    TAG tag;
    std::unique_ptr<ExpAST> exp;
    std::vector<std::unique_ptr<InitValAST>> inits; // can be 0, 1, 2,....
    std::string Dump() const;
    void getInitVal(std::string *ptr, const std::vector<int> &len, bool is_global = false) const;
};

class ConstInitValAST : public BaseAST {
public:
    enum TAG { CONST_EXP, CONST_INIT_LIST };
    TAG tag;
    std::unique_ptr<ConstExpAST> const_exp;
    std::vector<std::unique_ptr<ConstInitValAST>> inits;    // size can be 0, 1, ...
    // 表达式求值，计算结果放在pi所指的int内存地址
    int getValue();
    void getInitVal(std::string *ptr, const std::vector<int> &len) const;
};


class LValAST : public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::string Dump(bool dump_ptr = false) const;   // 默认返回的是i32而非指针。
    int getValue();
};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<ExpAST> exp;
    int getValue();
};

// Exp
class ExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> l_or_exp;
    std::string Dump() const;
    int getValue();
};

class PrimaryExpAST : public BaseAST { //表达式成员，可以是另一个表达式exp，一个数number，或一个量lval
public:
    enum TAG { PARENTHESES, NUMBER, LVAL};
    TAG tag;
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<LValAST> lval;
    int number;
    std::string Dump() const ;
    int getValue();
};


class UnaryExpAST : public BaseAST { //处理一元运算+,-,!正负非，否则交给上层primary_exp模块
public:
    enum TAG { PRIMARY_EXP, OP_UNITARY_EXP, FUNC_CALL};
    TAG tag;
    std::unique_ptr<PrimaryExpAST> primary_exp;
    char unary_op;
    std::unique_ptr<UnaryExpAST> unary_exp;
    std::string ident;
    std::unique_ptr<FuncRParamsAST> func_params;
    std::string Dump() const;
    int getValue();
};

// MulExp
class MulExpAST : public BaseAST { //处理乘,除,取模运算，否则交给上层unary_exp模块
public:
    enum TAG {UNARY_EXP, OP_MUL_EXP};
    TAG tag;
    std::unique_ptr<UnaryExpAST> unary_exp;
    std::unique_ptr<MulExpAST> mul_exp_1;
    std::unique_ptr<UnaryExpAST> unary_exp_2;
    char mul_op;
    std::string Dump() const;
    int getValue();
};

// AddExp
class AddExpAST : public BaseAST { //处理加减，否则交给上层mul_exp模块
public:
    enum TAG {MUL_EXP, OP_ADD_EXP};
    TAG tag;
    std::unique_ptr<MulExpAST> mul_exp;
    std::unique_ptr<AddExpAST> add_exp_1;
    std::unique_ptr<MulExpAST> mul_exp_2;
    char add_op;
    std::string Dump() const;
    int getValue();
};

// RelExp
class RelExpAST : public BaseAST { //处理大小比较，否则交给上层add_exp模块
public:
    enum TAG {ADD_EXP, OP_REL_EXP};
    TAG tag;
    std::unique_ptr<AddExpAST> add_exp;
    std::unique_ptr<RelExpAST> rel_exp_1;
    std::unique_ptr<AddExpAST> add_exp_2;
    char rel_op[2];     // <,>,<=,>=
    std::string Dump() const;
    int getValue();
};

class EqExpAST : public BaseAST {//处理相等或不等，否则给上层rel_exp模块
public:
    enum TAG {REL_EXP, OP_EQ_EXP};
    TAG tag;
    std::unique_ptr<RelExpAST> rel_exp;
    std::unique_ptr<EqExpAST> eq_exp_1;
    std::unique_ptr<RelExpAST> rel_exp_2;
    char eq_op;     // =,!
    std::string Dump() const;
    int getValue();
};

class LAndExpAST : public BaseAST { //处理与，否则给上层eq_exp模块
public:
    enum TAG {EQ_EXP, OP_L_AND_EXP};
    TAG tag;
    std::unique_ptr<EqExpAST> eq_exp;
    std::unique_ptr<LAndExpAST> l_and_exp_1;
    std::unique_ptr<EqExpAST> eq_exp_2;
    std::string Dump() const;
    int getValue();
};

class LOrExpAST : public BaseAST { //处理或，否则给上层l_and_exp模块
public:
    enum TAG {L_AND_EXP, OP_L_OR_EXP};
    TAG tag;
    std::unique_ptr<LAndExpAST> l_and_exp;
    std::unique_ptr<LOrExpAST> l_or_exp_1;
    std::unique_ptr<LAndExpAST> l_and_exp_2;
    std::string Dump() const;
    int getValue();
};

class FuncRParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<ExpAST>> exps;  // 函数表达式的参数
    std::string Dump() const;
};
