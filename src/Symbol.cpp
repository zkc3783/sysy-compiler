#include <bits/stdc++.h>
#include "Symbol.h"
using namespace std;

void NameTable::reset(){  // name map的初始化
    cnt = 0;
}

std::string NameTable::getTmpName(){ // 生成一个新的变量名
    return "%" + std::to_string(cnt++);
}

std::string NameTable::getName(const std::string &s){ // 标签名 + @ 的字符（变量）
    auto i = no.find(s);
    if(i == no.end()){
        no.insert(make_pair(s, 1)); // map中没有当前变量名，插入这个变量，返回0
        return "@" + s + "_0";
    }
    return "@" + s + "_"  + std::to_string(i->second++); // 否则返回计数器，并且计数器++
}
std::string NameTable::getLabelName(const std::string &s){ // 标签名 + % 的字符（标签），同上
    auto i = no.find(s);
    if(i == no.end()){
        no.insert(make_pair(s, 1));
        return "%" + s + "_0"; 
    }
    return "%" + s + "_"  + std::to_string(i->second++);
}

// SysYType 表示 SysY 语言中的类型信息，下面四个为构造函数
// TYPE ty：表示类型的枚举值
// int value：常量的值，对于数组类型，表示维度的大小
// SysYType* next：指向下一个类型节点的指针，用于表示多维数组等复杂类型
SysYType::SysYType():ty(SYSY_INT), value(-1), next(nullptr){} // 初始化为整数类型 SYSY_INT

SysYType::SysYType(TYPE _t):ty(_t), value(-1), next(nullptr){} // 使用给定类型 _t 初始化

SysYType::SysYType(TYPE _t, int _v): ty(_t), value(_v), next(nullptr){} // 使用给定类型 _t 和值 _v 初始化

// p若不是空指针，删除当前p
SysYType::~SysYType() { 
    auto p = next;
    if(p != nullptr){
        auto prev = p;
        p = p->next;
        delete prev;
    } 
}

// 构造函数：使用标识符 _ident、名称 _name 和类型指针 _t 初始化 Symbol
Symbol::Symbol(const std::string &_ident, const std::string &_name, SysYType *_t): ident(_ident), name(_name), ty(_t){
}

Symbol::~Symbol(){
    if(ty) delete ty;
}

STable::~STable(){
    for(auto &p : symbol_tb){
        delete p.second; 
    } 
}
// 插入符号表
void STable::insert(Symbol *symbol){
    symbol_tb.insert({symbol->ident, symbol});
} 
// 创建一个新的符号并插入符号表
void STable::insert(const std::string &ident, const std::string &name, SysYType::TYPE _type, int value){
    SysYType *ty = new SysYType(_type, value);
    Symbol *sym = new Symbol(ident, name, ty);
    insert(sym);
}

// 在符号表中插入
void STable::insertINT(const std::string &ident, const std::string &name){
    insert(ident, name, SysYType::SYSY_INT, UNKNOWN);
}

void STable::insertINTCONST(const std::string &ident, const std::string &name, int value){
    insert(ident, name, SysYType::SYSY_INT_CONST, value);
}

void STable::insertFUNC(const std::string &ident, const std::string &name, SysYType::TYPE _t){
    insert(ident, name, _t, UNKNOWN);
}

// 查找符号表中是否存在标识符
bool STable::exists(const std::string &ident){
    return symbol_tb.find(ident) != symbol_tb.end();
}

// 根据标识符 ident 在符号表中查找并返回对应的 Symbol 对象指针
Symbol *STable::Search(const std::string &ident){
    return symbol_tb.find(ident)->second;
}

// 返回给定标识符 ident 对应的 Symbol 的 value 值
int STable::getValue(const std::string &ident){
    return symbol_tb.find(ident)->second->ty->value;
}

// 返回给定标识符 ident 对应的 Symbol 的 SysYType 指针
SysYType *STable::getType(const std::string &ident){
    return symbol_tb.find(ident)->second->ty;
}

// 返回给定标识符 ident 对应的 Symbol 的 name 属性
std::string STable::getName(const std::string &ident){
    return symbol_tb.find(ident)->second->name;
}

// 在栈顶分配一个新的符号表
void SStack::alloc(){
    sym_tb_st.emplace_back(new STable());
}
// 从栈顶弹出一个符号表
void SStack::quit(){
    sym_tb_st.pop_back();
}
// 重置名字管理器
void SStack::resetNameManager(){
    nt.reset();
}
// 插入一个符号
void SStack::insert(Symbol *symbol){
    sym_tb_st.back()->insert(symbol);
}
// 在当前作用域（栈顶的符号表）中插入一个符号，给定符号标识符、类型和初始值。符号表中的符号名字由 NameTable 生成
void SStack::insert(const std::string &ident, SysYType::TYPE _type, int value){
    string name = nt.getName(ident);
    sym_tb_st.back()->insert(ident, name, _type, value);
}
// 插入int
void SStack::insertINT(const std::string &ident){
    string name = nt.getName(ident);
    sym_tb_st.back()->insertINT(ident, name);
}
// 插入const int
void SStack::insertINTCONST(const std::string &ident, int value){
    string name = nt.getName(ident);
    sym_tb_st.back()->insertINTCONST(ident, name, value);
}
// 插入一个函数符号
void SStack::insertFUNC(const std::string &ident, SysYType::TYPE _t){
    string name = "@" + ident;
    sym_tb_st.back()->insertFUNC(ident, name, _t);
}

// 一个标识符是否存在于符号表栈中的任何一个作用域
bool SStack::exists(const std::string &ident){
    for(int i = (int)sym_tb_st.size() - 1; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            return true;
    }
    return false;
}

// 扫描栈查找并返回符号的值
int SStack::getValue(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getValue(ident);
}
// 扫描栈查找并返回符号的类型
SysYType *SStack::getType(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getType(ident);
}
// 查找符号名
std::string SStack::getName(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getName(ident);
}
// 临时变量名
std::string SStack::getTmpName(){
    return nt.getTmpName();
}
// 标签名
std::string SStack::getLabelName(const std::string &label_ident){
    return nt.getLabelName(label_ident);
}
// 变量名
std::string SStack::getVarName(const std::string& var){
    return nt.getName(var);
}
