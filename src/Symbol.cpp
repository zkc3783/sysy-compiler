#include "Symbol.h"
#include <iostream>
using namespace std;

void NameManager::reset(){  // name map的初始化
    cnt = 0;
}

std::string NameManager::getTmpName(){ // 生成一个新的变量名
    return "%" + std::to_string(cnt++);
}

std::string NameManager::getName(const std::string &s){ // 标签名 + @ 的字符（变量）
    auto i = no.find(s);
    if(i == no.end()){
        no.insert(make_pair(s, 1)); // map中没有当前变量名，插入这个变量，返回0
        return "@" + s + "_0";
    }
    return "@" + s + "_"  + std::to_string(i->second++); // 否则返回计数器，并且计数器++
}
std::string NameManager::getLabelName(const std::string &s){ // 标签名 + % 的字符（标签）
    auto i = no.find(s);
    if(i == no.end()){
        no.insert(make_pair(s, 1));
        return "%" + s + "_0"; // map中没有当前标签名，插入这个变量，返回0
    }
    return "%" + s + "_"  + std::to_string(i->second++); // 搜索到的话返回计数器，并且计数器++
}

// SysYType 表示 SysY 语言中的类型信息，下面四个为构造函数
// TYPE ty：表示类型的枚举值
// int value：对于数组类型，表示维度的大小
// SysYType* next：指向下一个类型节点的指针，用于表示多维数组等复杂类型
SysYType::SysYType():ty(SYSY_INT), value(-1), next(nullptr){} // 初始化为整数类型 SYSY_INT，值为 -1，下一节点为空

SysYType::SysYType(TYPE _t):ty(_t), value(-1), next(nullptr){} // 使用给定类型 _t 初始化，值为 -1，下一节点为空

SysYType::SysYType(TYPE _t, int _v): ty(_t), value(_v), next(nullptr){} // 使用给定类型 _t 和值 _v 初始化，下一节点为空

SysYType::SysYType(TYPE _t, const std::vector<int> &len):ty(_t), value(-1), next(nullptr){ 
    // 使用给定类型 _t 和维度长度数组 len 初始化。如果 _t 是常量数组类型，则 is_const 为 true。调用 buildFromArrayType 函数来初始化多维数组类型
    buildFromArrayType(len, _t == SYSY_ARRAY_CONST);
}

SysYType::~SysYType() { 
    auto p = next;
    if(p != nullptr){
        auto prev = p;
        p = p->next;
        delete prev;
    } // p若不是空指针，删除当前p，属于析构函数
}

void SysYType::buildFromArrayType(const std::vector<int> &len, bool is_const){
    TYPE t = is_const ? SYSY_ARRAY_CONST : SYSY_ARRAY;
    SysYType *p = this;
    for(int i : len){
        p->ty = t;
        p->value = i;
        p->next = new SysYType();
        p = p->next;
    }
    p->next = new SysYType();
    p = p->next;
    p->ty = is_const ? SYSY_INT_CONST : SYSY_INT;
}
// buildFromArrayType 函数用于根据给定的维度长度数组 len 构建数组类型链表。如果 is_const 为 true，则类型为常量数组类型，否则为普通数组类型。每个节点表示一个数组维度，最后一个节点表示数组的基本类型。
void SysYType::getArrayType(std::vector<int> &len){
    len.clear();
    SysYType *p = this;
    while(p->next != nullptr && (p->ty == SYSY_ARRAY_CONST || p->ty == SYSY_ARRAY)){
        len.push_back(value);
        p = p->next;
    }
    return;
} // 用链表查找，获取数组的维度信息，然后存到len向量中

Symbol::Symbol(const std::string &_ident, const std::string &_name, SysYType *_t): ident(_ident), name(_name), ty(_t){
}
// 构造函数：使用标识符 _ident、名称 _name 和类型指针 _t 初始化 Symbol 对象。
Symbol::~Symbol(){
    if(ty) delete ty;
} // 析构函数

SymbolTable::~SymbolTable(){
    for(auto &p : symbol_tb){
        delete p.second; 
    } 
}
// 析构函数

void SymbolTable::insert(Symbol *symbol){
    symbol_tb.insert({symbol->ident, symbol});
} // 将一个函数插入符号表

void SymbolTable::insert(const std::string &ident, const std::string &name, SysYType::TYPE _type, int value){
    SysYType *ty = new SysYType(_type, value);
    Symbol *sym = new Symbol(ident, name, ty);
    insert(sym);
}
// insert 函数重载：根据标识符 ident、名称 name、类型 _type 和值 value 创建一个新的符号并插入符号表

void SymbolTable::insertINT(const std::string &ident, const std::string &name){
    insert(ident, name, SysYType::SYSY_INT, UNKNOWN);
}
// 在符号表中插入一个 int，初始值为 UNKNOWN
void SymbolTable::insertINTCONST(const std::string &ident, const std::string &name, int value){
    insert(ident, name, SysYType::SYSY_INT_CONST, value);
}
// 在符号表中插入一个 int const，初始值为 value
void SymbolTable::insertFUNC(const std::string &ident, const std::string &name, SysYType::TYPE _t){
    insert(ident, name, _t, UNKNOWN);
}
// 插入一个函数类型的符号

void SymbolTable::insertArray(const std::string &ident, const std::string &name, const std::vector<int> &len, SysYType::TYPE _t){
    SysYType *ty = new SysYType(_t, len);
    Symbol *sym = new Symbol(ident, name, ty);
    insert(sym);
}
// 插入一个数组类型的符号

bool SymbolTable::exists(const std::string &ident){
    return symbol_tb.find(ident) != symbol_tb.end();
}
// 查找符号表中是否存在给定标识符的符号

Symbol *SymbolTable::Search(const std::string &ident){
    return symbol_tb.find(ident)->second;
}
// 根据标识符 ident 在符号表中查找并返回对应的 Symbol 对象指针

int SymbolTable::getValue(const std::string &ident){
    return symbol_tb.find(ident)->second->ty->value;
}
// 返回给定标识符 ident 对应的 Symbol 的 value 值

SysYType *SymbolTable::getType(const std::string &ident){
    return symbol_tb.find(ident)->second->ty;
}
// 返回给定标识符 ident 对应的 Symbol 的 SysYType 指针

std::string SymbolTable::getName(const std::string &ident){
    return symbol_tb.find(ident)->second->name;
}
// 返回给定标识符 ident 对应的 Symbol 的 name 属性

void SymbolTableStack::alloc(){
    sym_tb_st.emplace_back(new SymbolTable());
}
// 在栈顶分配一个新的符号表
void SymbolTableStack::quit(){
    sym_tb_st.pop_back();
}
// 从栈顶弹出一个符号表
void SymbolTableStack::resetNameManager(){
    nm.reset();
}
// 重置名字管理器
void SymbolTableStack::insert(Symbol *symbol){
    sym_tb_st.back()->insert(symbol);
}
// 插入一个符号
void SymbolTableStack::insert(const std::string &ident, SysYType::TYPE _type, int value){
    string name = nm.getName(ident);
    sym_tb_st.back()->insert(ident, name, _type, value);
}
// 在当前作用域（栈顶的符号表）中插入一个符号，给定符号标识符、类型和初始值。符号表中的符号名字由 NameManager 生成
void SymbolTableStack::insertINT(const std::string &ident){
    string name = nm.getName(ident);
    sym_tb_st.back()->insertINT(ident, name);
}
// 栈顶的符号表中插入一个int的符号
void SymbolTableStack::insertINTCONST(const std::string &ident, int value){
    string name = nm.getName(ident);
    sym_tb_st.back()->insertINTCONST(ident, name, value);
}
// 栈顶的符号表中插入一个const int的符号
void SymbolTableStack::insertFUNC(const std::string &ident, SysYType::TYPE _t){
    string name = "@" + ident;
    sym_tb_st.back()->insertFUNC(ident, name, _t);
}
// 插入一个函数符号
void SymbolTableStack::insertArray(const std::string &ident, const std::vector<int> &len, SysYType::TYPE _t){
    string name = nm.getName(ident);
    sym_tb_st.back()->insertArray(ident, name, len, _t);
}
// 插入一个数组符号
bool SymbolTableStack::exists(const std::string &ident){
    for(int i = (int)sym_tb_st.size() - 1; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            return true;
    }
    return false;
}
// 一个标识符是否存在于符号表栈中的任何一个作用域
int SymbolTableStack::getValue(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getValue(ident);
}
// 查找并返回符号的值
SysYType *SymbolTableStack::getType(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getType(ident);
}
// 查找并返回符号的类型
std::string SymbolTableStack::getName(const std::string &ident){
    int i = (int)sym_tb_st.size() - 1;
    for(; i >= 0; --i){
        if(sym_tb_st[i]->exists(ident))
            break;
    }
    return sym_tb_st[i]->getName(ident);
}
// 查找并返回符号的名字
std::string SymbolTableStack::getTmpName(){
    return nm.getTmpName();
}
// 生成一个新的临时变量名
std::string SymbolTableStack::getLabelName(const std::string &label_ident){
    return nm.getLabelName(label_ident);
}
// 生成一个标签名，用于标识代码中的跳转位置
std::string SymbolTableStack::getVarName(const std::string& var){
    return nm.getName(var);
}
// 生成或获取一个变量名