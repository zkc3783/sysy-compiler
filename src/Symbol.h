#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>

class NameManager{
private:
    int cnt;
    std::unordered_map<std::string, int> no;
public:
    NameManager():cnt(0){}
    void reset();// name map初始化
    std::string getTmpName();// 生成一个新的变量名
    std::string getName(const std::string &s);// 标签名 + @ 的字符（变量）
    std::string getLabelName(const std::string &s);// 标签名 + % 的字符（标签）
};


class SysYType{
    public:
        enum TYPE{
            SYSY_INT, SYSY_INT_CONST, SYSY_FUNC_VOID, SYSY_FUNC_INT,
            SYSY_ARRAY_CONST, SYSY_ARRAY // SYSY的种类
        };
        TYPE ty;
        int value;
        SysYType *next;

        SysYType();// 初始化为整数类型 SYSY_INT，值为 -1，下一节点为空
        SysYType(TYPE _t);// 使用给定类型 _t 初始化，值为 -1，下一节点为空
        SysYType(TYPE _t, int _v);// 使用给定类型 _t 和值 _v 初始化，下一节点为空
        SysYType(TYPE _t, const std::vector<int> &len);
        // 使用给定类型 _t 和维度长度数组 len 初始化。如果 _t 是常量数组类型，则 is_const 为 true。调用 buildFromArrayType 函数来初始化多维数组类型

        ~SysYType();
        void buildFromArrayType(const std::vector<int> &len, bool is_const);
        void getArrayType(std::vector<int> &len);
};

class Symbol{
public:
    std::string ident;   // SysY标识符，诸如x,y
    std::string name;    // KoopaIR中的具名变量，诸如@x_1, @y_1, ..., @n_2
    SysYType *ty;
    Symbol(const std::string &_ident, const std::string &_name, SysYType *_t);
    ~Symbol();
};

class SymbolTable{
public:
    const int UNKNOWN = -1;
    std::unordered_map<std::string, Symbol *> symbol_tb;  // ident -> Symbol *
    SymbolTable() = default;
    ~SymbolTable();
    void insert(Symbol *symbol);

    void insert(const std::string &ident, const std::string &name, SysYType::TYPE _type, int value);

    void insertINT(const std::string &ident, const std::string &name);

    void insertINTCONST(const std::string &ident, const std::string &name, int value);

    void insertFUNC(const std::string &ident, const std::string &name, SysYType::TYPE _t);

    void insertArray(const std::string &ident, const std::string &name, const std::vector<int> &len, SysYType::TYPE _t);

    bool exists(const std::string &ident);

    Symbol *Search(const std::string &ident);

    int getValue(const std::string &ident);

    SysYType *getType(const std::string &ident);

    std::string getName(const std::string &ident);

};

class SymbolTableStack{
private:
    std::deque<std::unique_ptr<SymbolTable>> sym_tb_st;
    NameManager nm;
public:
    const int UNKNOWN = -1;
    void alloc();
    void quit();
    void resetNameManager();
    void insert(Symbol *symbol);
    void insert(const std::string &ident, SysYType::TYPE _type, int value);
    void insertINT(const std::string &ident);
    void insertINTCONST(const std::string &ident, int value);
    void insertFUNC(const std::string &ident, SysYType::TYPE _t);
    void insertArray(const std::string &ident, const std::vector<int> &len, SysYType::TYPE _t);
    bool exists(const std::string &ident);
    int getValue(const std::string &ident);
    SysYType *getType(const std::string &ident);
    std::string getName(const std::string &ident);

    std::string getTmpName();   // inherit from name manager
    std::string getLabelName(const std::string &label_ident); // inherit from name manager
    std::string getVarName(const std::string& var);   // aux var name, such as @short_circuit_res,shouldn't insert it into Symbol table.
};