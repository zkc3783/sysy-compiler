#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>
// zyq
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

        ~SysYType();// 析构函数
        void buildFromArrayType(const std::vector<int> &len, bool is_const);
        // 创建数组的符号表
        void getArrayType(std::vector<int> &len);// 用链表查找，获取数组的维度信息，然后存到len向量中
};

class Symbol{
public:
    std::string ident;   // SysY标识符，诸如x,y
    std::string name;    // KoopaIR中的具名变量，诸如@x_1, @y_1, ..., @n_2
    SysYType *ty;
    Symbol(const std::string &_ident, const std::string &_name, SysYType *_t);// 构造函数：使用标识符 _ident、名称 _name 和类型指针 _t 初始化 Symbol 对象。
    ~Symbol(); // 析构函数
};

class SymbolTable{
public:
    const int UNKNOWN = -1;
    std::unordered_map<std::string, Symbol *> symbol_tb;  // ident -> Symbol *
    SymbolTable() = default;
    ~SymbolTable();
    void insert(Symbol *symbol);
    // insert 函数重载：根据标识符 ident、名称 name、类型 _type 和值 value 创建一个新的符号并插入符号表
    void insert(const std::string &ident, const std::string &name, SysYType::TYPE _type, int value);

    void insertINT(const std::string &ident, const std::string &name);
    // 在符号表中插入一个 int
    void insertINTCONST(const std::string &ident, const std::string &name, int value);
    // 在符号表中插入一个 int const，初始值为 value
    void insertFUNC(const std::string &ident, const std::string &name, SysYType::TYPE _t);
    // 插入一个函数类型的符号
    void insertArray(const std::string &ident, const std::string &name, const std::vector<int> &len, SysYType::TYPE _t);
    // 插入一个数组类型的符号
    bool exists(const std::string &ident);
    // 查找符号表中是否存在给定标识符的符号
    Symbol *Search(const std::string &ident);
    // 根据标识符 ident 在符号表中查找并返回对应的 Symbol 对象指针
    int getValue(const std::string &ident);
    // 返回给定标识符 ident 对应的 Symbol 的 value 值
    SysYType *getType(const std::string &ident);
    // 返回给定标识符 ident 对应的 Symbol 的 SysYType 指针
    std::string getName(const std::string &ident);
    // 返回给定标识符 ident 对应的 Symbol 的 name 属性
};

class SymbolTableStack{
private:
    std::deque<std::unique_ptr<SymbolTable>> sym_tb_st;
    NameManager nm;
public:
    const int UNKNOWN = -1;
    void alloc();// 在栈顶分配一个新的符号表
    void quit();// 从栈顶弹出一个符号表
    void resetNameManager();// 重置名字管理器
    void insert(Symbol *symbol);// 插入一个符号
    void insert(const std::string &ident, SysYType::TYPE _type, int value);// 在当前作用域（栈顶的符号表）中插入一个符号，给定符号标识符、类型和初始值
    void insertINT(const std::string &ident);// 栈顶的符号表中插入一个int的符号
    void insertINTCONST(const std::string &ident, int value);// 栈顶的符号表中插入一个const int的符号
    void insertFUNC(const std::string &ident, SysYType::TYPE _t);// 插入一个函数符号
    void insertArray(const std::string &ident, const std::vector<int> &len, SysYType::TYPE _t);// 插入一个数组符号
    bool exists(const std::string &ident);// 一个标识符是否存在于符号表栈中的任何一个作用域
    int getValue(const std::string &ident);// 查找并返回符号的值
    SysYType *getType(const std::string &ident);// 查找并返回符号的类型
    std::string getName(const std::string &ident);// 查找并返回符号的名字

    std::string getTmpName();   // 继承name manager
    std::string getLabelName(const std::string &label_ident); // 继承name manager
    std::string getVarName(const std::string& var);   // 获取 var name
};