#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>
// zyq
/*
NameManager 处理重复的变量名
Symbol表 表示一个表项 包括标识符 ident、名称 name
SymbolTable 表示一个大表 有标识符 ident、名称 name、类型 type 和值 value 
SymbolTableStack 用来处理符号表栈 
*/
class NameManager{
private:
    int cnt;
    std::unordered_map<std::string, int> no;
public:
    NameManager():cnt(0){}
    void reset();
    std::string getTmpName(); // 生成一个新的变量名
    std::string getName(const std::string &s); // @ 
    std::string getLabelName(const std::string &s); // % 
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

        SysYType(); // 初始化
        SysYType(TYPE _t);
        SysYType(TYPE _t, int _v);
        SysYType(TYPE _t, const std::vector<int> &len);

        ~SysYType();
        //void buildFromArrayType(const std::vector<int> &len, bool is_const);
        // 创建数组的符号表
        //void getArrayType(std::vector<int> &len);
        // 用链表查找，获取数组的维度信息，然后存到len向量中
};

class Symbol{
public:
    std::string ident;   // SysY标识符，x,y
    std::string name;    // KoopaIR中的具名变量，@x_1, @y_1, ..., @n_2
    SysYType *ty;
    Symbol(const std::string &_ident, const std::string &_name, SysYType *_t); // 构造函数：标识符 _ident、名称 _name 和类型指针 _t 
    ~Symbol();
};

class SymbolTable{
public:
    const int UNKNOWN = -1;
    std::unordered_map<std::string, Symbol *> symbol_tb;  // ident -> Symbol 
    SymbolTable() = default;
    ~SymbolTable();
    void insert(Symbol *symbol);
    // insert 函数重载：根据标识符 ident、名称 name、类型 _type 和值 value 创建一个新的符号并插入符号表
    void insert(const std::string &ident, const std::string &name, SysYType::TYPE _type, int value);
    // 在符号表中插入
    void insertINT(const std::string &ident, const std::string &name);
    void insertINTCONST(const std::string &ident, const std::string &name, int value);
    void insertFUNC(const std::string &ident, const std::string &name, SysYType::TYPE _t);
    //void insertArray(const std::string &ident, const std::string &name, const std::vector<int> &len, SysYType::TYPE _t);
    bool exists(const std::string &ident); // 给出标识符查找是否存在
    Symbol *Search(const std::string &ident);
    // 根据标识符 ident 在符号表中查找并返回对应的 Symbol 对象指针
    int getValue(const std::string &ident);
    // 返回value 值
    SysYType *getType(const std::string &ident);
    // 返回SysYType 指针
    std::string getName(const std::string &ident);
    // 返回 name 
};

class SymbolTableStack{
private:
    std::deque<std::unique_ptr<SymbolTable>> sym_tb_st;
    NameManager nm;
public:
    const int UNKNOWN = -1;
    void alloc();// 在栈顶分配一个新的符号表
    void quit();// 从栈顶弹出一个符号表
    void resetNameManager();
    void insert(Symbol *symbol);// 插入一个符号
    void insert(const std::string &ident, SysYType::TYPE _type, int value);
    void insertINT(const std::string &ident);
    void insertINTCONST(const std::string &ident, int value);
    void insertFUNC(const std::string &ident, SysYType::TYPE _t);
    //void insertArray(const std::string &ident, const std::vector<int> &len, SysYType::TYPE _t);
    // 上述为插入各个类型的符号
    bool exists(const std::string &ident);// 一个标识符是否存在于符号表栈中的任何一个作用域
    int getValue(const std::string &ident);// 查找值
    SysYType *getType(const std::string &ident);// 查找符号的类型
    std::string getName(const std::string &ident);// 查找name

    std::string getTmpName();   // 继承 name manager
    std::string getLabelName(const std::string &label_ident); // 继承 name manager
    std::string getVarName(const std::string& var);   // 获取 var name
};