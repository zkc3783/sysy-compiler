#include "AST.h"
#include "Symbol.h"
#include <memory>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "utils.h"
using namespace std;

KoopaString ks;         // Koopa 字符串
SymbolTableStack st;    // 符号表
BlockController bc;     // 通过一个bool值管理代码块的活动状态（例如遇到break，return）
                        // set设为1，finish设为0，alive检查值
WhileStack wst;         // 用栈管理循环，记录入口、循环体和结束的标签

class ScopeHelper {
private:
    static int depth;  // 静态变量，用于记录当前嵌套层数
    std::string type;
    std::string name;

    // 辅助函数，用于生成当前层次的缩进
    std::string getIndent() const {
        return std::string(depth * 2, ' ');  // 每层缩进两个空格
    }

public:
    ScopeHelper(const std::string& type, const std::string& name = "") : type(type), name(name) {
        std::cout << getIndent();  // 输出当前缩进
        std::cout << type;  // 输出类型名
        if (!name.empty()) {
            std::cout << " (" << name << ")";  // 如果变量名不为空，则输出变量名
        }
        std::cout << "{" << std::endl;
        depth++;  // 进入新的层级
    }
    ~ScopeHelper() {
        depth--;  // 退出当前层级
        std::cout << getIndent() << "}" << std::endl;
    }
};

// 初始化静态成员变量
int ScopeHelper::depth = 0;

void CompUnitAST::Dump()const {
    ScopeHelper scope("CompUnitAST");
    st.alloc(); // 全局作用域
    this->DumpGlobalVar();  // 处理全局变量  
    // 库函数声明
    ks.declLibFunc();
    st.insertFUNC("getint", SysYType::SYSY_FUNC_INT);
    st.insertFUNC("getch", SysYType::SYSY_FUNC_INT);
    st.insertFUNC("getarray", SysYType::SYSY_FUNC_INT);
    st.insertFUNC("putint", SysYType::SYSY_FUNC_VOID);
    st.insertFUNC("putch", SysYType::SYSY_FUNC_VOID);
    st.insertFUNC("putarray", SysYType::SYSY_FUNC_VOID);
    st.insertFUNC("starttime", SysYType::SYSY_FUNC_VOID);
    st.insertFUNC("stoptime", SysYType::SYSY_FUNC_VOID);

    int n = func_defs.size();
    for(int i = 0; i < n; ++i)
        func_defs[i]->Dump();
    st.quit();
}

void CompUnitAST::DumpGlobalVar() const{
     // 全局变量
    for(auto &d : decls){
        if(d->tag == DeclAST::CONST_DECL){
            for(auto &const_def : d->const_decl->const_defs){
                const_def->Dump(true);
            }
        } else {
           for(auto &var_def: d->var_decl->var_defs){
                var_def->Dump(true);
           }
        }
    }
    ks.append("\n");

}

void FuncDefAST::Dump() const {
    ScopeHelper scope("FuncDefAST", ident);
    st.resetNameManager();
    
    // 函数名加到符号表
    st.insertFUNC(ident, btype->tag == BTypeAST::INT ? SysYType::SYSY_FUNC_INT : SysYType::SYSY_FUNC_VOID);

    // fun @main(): i32 {
    ks.append("fun " + st.getName(ident) + "(");

    // 提前进入到函数内的block，之后把参数load到变量中
    st.alloc();
    vector<string> var_names;   //KoopaIR参数列表的名字
    
    // 打印函数变量名，并保存到var_names中
    if(func_params != nullptr){
        auto &fps = func_params->func_f_params;
        int n = fps.size();
        var_names.push_back(st.getVarName(fps[0]->ident));
        ks.append(var_names.back() + ": ");
        ks.append(fps[0]->Dump());
        for(int i = 1; i < n; ++i){
            ks.append(", ");
            var_names.push_back(st.getVarName(fps[i]->ident));
            ks.append(var_names.back() + ": ");
            ks.append(fps[i]->Dump());
        }
    }
    ks.append(")");
    btype->Dump();
    ks.append(" {\n");

    // 进入Block
    bc.set();
    ks.label("%entry");

    // 提前把虚参加载到变量中
    if(func_params != nullptr){
        int i = 0;
        for(auto &fp : func_params->func_f_params){
            string var = var_names[i++];

            st.insertINT(fp->ident);
            string name = st.getName(fp->ident);

            ks.alloc(name);
            ks.store(var, name);
        }
    }

    // 具体内容交给block
    if(func_params != nullptr){
        block->Dump(false);
    }else{
        block->Dump();
    }
    // 特判空块
    if(bc.alive()){
        if(btype->tag == BTypeAST::INT)
            ks.ret("0");
        else
            ks.ret("");
    }
    ks.append("}\n\n");
}

string FuncFParamAST::Dump() const{
    ScopeHelper scope("FuncFParamAST");
    return "i32";
}

void BlockAST::Dump(bool new_symbol_tb) const {
    ScopeHelper scope("BlockAST");
    // into this Block
    if(new_symbol_tb)
        st.alloc();
   
    int n = block_items.size();

    for(int i = 0; i < n; ++i){
        block_items[i]->Dump();
    }
    // out of this block
    st.quit();
}

void BlockItemAST::Dump() const{
    ScopeHelper scope("BlockItemAST", tag == DECL ? "DECL" : "STMT");
    if(!bc.alive()) return;
    if(tag == DECL){
        decl->Dump();
    } else{
        stmt->Dump();
    }
}

void DeclAST::Dump() const{
    ScopeHelper scope("BlockItemAST", tag == CONST_DECL ? "CONST_DECL" : "VAR_DECL");
    if(tag == VAR_DECL)
        var_decl->Dump();
    else
        const_decl->Dump();
}

void StmtAST::Dump() const {
    ScopeHelper scope("StmtAST", [&]() -> const std::string& {
        static const std::string tagNames[] = {
            "RETURN", "ASSIGN", "BLOCK", "EXP", "WHILE", "BREAK", "CONTINUE", "IF"
        };
        return tagNames[tag];
    }());
    if(!bc.alive()) return;
    if(tag == RETURN){
        if(exp){
            string ret_name = exp->Dump();
            ks.ret(ret_name);
        } else{
            ks.ret("");
        }
        bc.finish();
    } else if(tag == ASSIGN){
        string val = exp->Dump();
        string to = lval->Dump(true);
        ks.store(val, to);
    } else if(tag == BLOCK){
        block->Dump();
    } else if(tag == EXP){
        if(exp){
            exp->Dump();
        }
    } else if(tag == WHILE){
        string while_entry = st.getLabelName("while_entry");
        string while_body = st.getLabelName("while_body");
        string while_end = st.getLabelName("while_end");
        
        wst.append(while_entry, while_body, while_end);

        ks.jump(while_entry);

        bc.set();
        ks.label(while_entry);
        string cond = exp->Dump();
        ks.br(cond, while_body, while_end);

        bc.set();
        ks.label(while_body);
        stmt->Dump();
        if(bc.alive())
            ks.jump(while_entry);

        bc.set();
        ks.label(while_end);
        wst.quit(); // 该while处理已结束，退栈
    } else if(tag == BREAK){
        ks.jump(wst.getEndName());  // 跳转到while_end
        bc.finish();                // 当前IR的block设为不活跃
    } else if(tag == CONTINUE){
        ks.jump(wst.getEntryName());// 跳转到while_entry
        bc.finish();                // 当前IR的block设为不活跃
    } else if(tag == IF){
        string s = exp->Dump();
        string t = st.getLabelName("then");
        string e = st.getLabelName("else");
        string j = st.getLabelName("end");
        ks.br(s, t, else_stmt == nullptr ? j : e);

        // IF Stmt
        bc.set();
        ks.label(t);
        if_stmt->Dump();
        if(bc.alive())
            ks.jump(j);

        // else stmt
        if(else_stmt != nullptr){
            bc.set();
            ks.label(e);
            else_stmt->Dump();
            if(bc.alive())
                ks.jump(j);

        }
        // end
        bc.set();
        ks.label(j);
    }
    return;
}

void ConstDeclAST::Dump() const{
    ScopeHelper scope("ConstDeclAST");
    int n = const_defs.size();
    for(int i = 0; i < n; ++i){
        const_defs[i]->Dump();
    }
}

void VarDeclAST::Dump() const {
    ScopeHelper scope("VarDeclAST");
    int n = var_defs.size();
    for(int i = 0; i < n; ++i){
        var_defs[i]->Dump();
    }
}

void BTypeAST::Dump() const{
    ScopeHelper scope("BTypeAST", "i32");
    if(tag == BTypeAST::INT){
        ks.append(": i32");
    }
}

void ConstDefAST::Dump(bool is_global) const{
    ScopeHelper scope("ConstDefAST", ident);
    int v = const_init_val->getValue();
    st.insertINTCONST(ident, v);
}

void VarDefAST::Dump(bool is_global) const{
    ScopeHelper scope("VarDefAST", ident);
    st.insertINT(ident);
    string name = st.getName(ident);
    if(is_global){
        if(init_val == nullptr){
            ks.globalAllocINT(name);
        } else {
            int v = init_val->exp->getValue();
            ks.globalAllocINT(name, to_string(v));
        }
    } else {
        ks.alloc(name);
        if(init_val != nullptr){
            string s = init_val->Dump();
            ks.store(s, name);
        }
    }
    return;
}

string InitValAST::Dump() const{
    return exp->Dump();
}

int ConstInitValAST::getValue(){
    return const_exp->getValue();
}

string LValAST::Dump(bool dump_ptr)const{
    ScopeHelper scope("LValAST", ident);
    SysYType *ty = st.getType(ident);
    if(ty->ty == SysYType::SYSY_INT_CONST)
        return to_string(st.getValue(ident));
    else if(ty->ty == SysYType::SYSY_INT){
        if(dump_ptr == false){
            string tmp = st.getTmpName();
            ks.load(tmp, st.getName(ident));
            return tmp;
        } else {
            return st.getName(ident);
        }
    } else {
        // func(ident)
        if(ty->value == -1){
            string tmp = st.getTmpName();
            ks.load(tmp, st.getName(ident));
            return tmp;
        }
        string tmp = st.getTmpName();
        ks.getelemptr(tmp, st.getName(ident), "0");
        return tmp;
    }
}

int LValAST::getValue(){
    return st.getValue(ident);
}


int ConstExpAST::getValue(){
    return exp->getValue();
}

string ExpAST::Dump() const {
    ScopeHelper scope("ExpAST");
    return l_or_exp->Dump();
}
 
int ExpAST::getValue(){
    return l_or_exp->getValue();
}


string PrimaryExpAST::Dump() const{
    switch (tag)
    {
        case PARENTHESES:
            return exp->Dump();
        case NUMBER:
            return to_string(number);
        case LVAL:
            return lval->Dump();
    }
    return "";
}

int PrimaryExpAST::getValue(){
    switch (tag)
    {
        case PARENTHESES:
            return exp->getValue();
        case NUMBER:
            return number;
        case LVAL:
            return lval->getValue();
    }
    return -1;  // make g++ happy
}

string UnaryExpAST::Dump() const{
    if(tag == OP_UNITARY_EXP ) ScopeHelper scope("UnaryExpAST");

    if(tag == PRIMARY_EXP)return primary_exp->Dump();
    else if(tag == OP_UNITARY_EXP){
        string b = unary_exp->Dump();
        if(unary_op == '+') return b;

        string op = unary_op == '-' ? "sub" : "eq";
        string c = st.getTmpName();
        ks.binary(op, c, "0", b);
        return c;
    }else{
        // Func_Call
        string name = st.getName(ident);
        string tmp = "";
        vector<string> par;
        if(st.getType(ident)->ty == SysYType::SYSY_FUNC_INT){
            tmp = st.getTmpName();
        }
        if(func_params){
            int n = func_params->exps.size();
            for(int i = 0; i < n; ++i){
                par.push_back(func_params->exps[i]->Dump());
            }
        }
        ks.call(tmp, st.getName(ident), par);
        return tmp;
    }
}

int UnaryExpAST::getValue(){ 
    if(tag == PRIMARY_EXP) return primary_exp->getValue();

    int v =unary_exp->getValue();
    if(unary_op == '+') return v;
    return unary_op == '-' ? -v : !v;
}

string MulExpAST::Dump() const{ 
    if(tag != UNARY_EXP)ScopeHelper scope("MulExpAST");
    if(tag == UNARY_EXP)return unary_exp->Dump();
    string a, b, c;
    
    a = mul_exp_1->Dump();
    b = unary_exp_2->Dump();

    string op = mul_op == '*' ? "mul":(mul_op == '/' ?"div" : "mod");
    
    c = st.getTmpName();
    ks.binary(op, c, a, b);
    return c;
}

int MulExpAST::getValue(){
    if(tag == UNARY_EXP) return unary_exp->getValue();

    int a = mul_exp_1->getValue(), b = unary_exp_2->getValue();

    return mul_op == '*' ? a * b : (mul_op == '/' ? a / b : a % b);
}

string AddExpAST::Dump() const{
    if(tag != MUL_EXP)ScopeHelper scope("AddExpAST");
    if(tag == MUL_EXP)return mul_exp->Dump();
    string a, b, c;
    
    a = add_exp_1->Dump();
    b = mul_exp_2->Dump();

    string op = add_op == '+' ? "add" : "sub";
    
    c = st.getTmpName();
    ks.binary(op, c, a, b);
    return c;
}

int AddExpAST::getValue(){
    if(tag == MUL_EXP) return mul_exp->getValue();

    int a = add_exp_1->getValue(), b = mul_exp_2->getValue();
    return add_op == '+' ? a + b : a - b;
}

string RelExpAST::Dump() const {
    if(tag != ADD_EXP)ScopeHelper scope("RelExpAST");
    if(tag == ADD_EXP) return add_exp->Dump();
    string a = rel_exp_1->Dump(), b = add_exp_2->Dump();
    string op = rel_op[1] == '=' ? (rel_op[0] == '<' ? "le" : "ge") : (rel_op[0] == '<' ? "lt" : "gt");
    string dest = st.getTmpName();
    ks.binary(op, dest, a, b);
    return dest;
}

int RelExpAST::getValue(){
    if(tag == ADD_EXP) return add_exp->getValue();

    int a = rel_exp_1->getValue(), b = add_exp_2->getValue();
    if(rel_op[1] == '='){
        return rel_op[0] == '>' ? (a >= b) : (a <= b);
    }
    return rel_op[0] == '>' ? (a > b) : (a < b);
}

string EqExpAST::Dump() const {
    if(tag != REL_EXP) ScopeHelper scope("EqExpAST");
    if(tag == REL_EXP) return rel_exp->Dump();
    string a = eq_exp_1->Dump(), b =rel_exp_2->Dump();
    string op = eq_op == '=' ? "eq" : "ne";
    string dest = st.getTmpName();
    ks.binary(op, dest, a, b);
    return dest;
}

int EqExpAST::getValue(){
    if(tag == REL_EXP) return rel_exp->getValue();
    int a = eq_exp_1->getValue(), b = rel_exp_2->getValue();
    return eq_op == '=' ? (a == b) : (a != b);
}

string LAndExpAST::Dump() const {
    if(tag != EQ_EXP) ScopeHelper scope("LAndExpAST");
    if(tag == EQ_EXP) return eq_exp->Dump();
    
    // 修改支持短路逻辑
    string result = st.getVarName("SCRES");
    ks.alloc(result);
    ks.store("0", result);

    string lhs = l_and_exp_1->Dump();
    string then_s = st.getLabelName("then_sc");
    string end_s = st.getLabelName("end_sc");

    ks.br(lhs, then_s, end_s);

    bc.set();
    ks.label(then_s);
    string rhs = eq_exp_2->Dump();
    string tmp = st.getTmpName();
    ks.binary("ne", tmp, rhs, "0");
    ks.store(tmp, result);
    ks.jump(end_s);

    bc.set();
    ks.label(end_s);
    string ret = st.getTmpName();
    ks.load(ret, result);
    return ret;
}

int LAndExpAST::getValue(){
    if(tag == EQ_EXP) return eq_exp->getValue();
    int a = l_and_exp_1->getValue(), b = eq_exp_2->getValue();
    return a && b;  // 注意是逻辑与
}

string LOrExpAST::Dump() const {
    if(tag != L_AND_EXP) ScopeHelper scope("LOrExpAST");
    if(tag == L_AND_EXP) return l_and_exp->Dump();

    // 修改支持短路逻辑
    string result = st.getVarName("SCRES");
    ks.alloc(result);
    ks.store("1", result);

    string lhs = l_or_exp_1->Dump();

    string then_s = st.getLabelName("then_sc");
    string end_s = st.getLabelName("end_sc");

    ks.br(lhs, end_s, then_s);

    bc.set();
    ks.label(then_s);
    string rhs = l_and_exp_2->Dump();
    string tmp = st.getTmpName();
    ks.binary("ne", tmp, rhs, "0");
    ks.store(tmp, result);
    ks.jump(end_s);

    bc.set();
    ks.label(end_s);
    string ret = st.getTmpName();
    ks.load(ret, result);
    return ret;
}

int LOrExpAST::getValue() {
    if(tag == L_AND_EXP) return l_and_exp->getValue();
    int a = l_or_exp_1->getValue(), b = l_and_exp_2->getValue();
    return a || b;
}