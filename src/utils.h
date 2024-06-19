#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <stack>

class KoopaIR{
private:
    std::string koopa_ir;
public:
    void append(const std::string &s){
        koopa_ir += s;
    }

    void binary(const std::string &op, const std::string &rd, const std::string &s1, const std::string &s2){
        koopa_ir += "  " + rd + " = " + op + " " + s1 + ", " + s2 + "\n";
    }

    void label(const std::string &s){
        koopa_ir += s + ":\n";
    }

    void ret(const std::string &name){
        koopa_ir +="  ret " + name + "\n";
    }

    void alloc(const std::string &name){
        koopa_ir += "  " + name + " = alloc i32\n";
    }

    void alloc(const std::string &name, const std::string &type_str){
        koopa_ir += "  " + name + " = alloc " + type_str + "\n";
    }

    void globalAllocINT(const std::string &name, const std::string &init = "zeroinit"){
        koopa_ir += "global " + name + " = alloc i32, " + init + "\n";
    }

    void globalAllocArray(const std::string &name, const std::string &array_type, const std::string &init){
        koopa_ir += "global " + name + " = alloc " + array_type + ", " + init + "\n";
    }

    void load(const std::string & to, const std::string &from){
        koopa_ir += "  " + to + " = load " + from + '\n';
    }

    void store(const std::string &from, const std::string &to){
        koopa_ir += "  store " + from + ", " + to + '\n';
    }

    // Branch ::= "br" Value "," SYMBOL "," SYMBOL;
    void br(const std::string &v, const std::string &then_s, const std::string &else_s){
        koopa_ir += "  br " + v + ", " + then_s + ", " + else_s + '\n';
    }
    
    // Jump ::= "jump" SYMBOL;
    void jump(const std::string &label){
        koopa_ir += "  jump " + label + '\n';
    }

    // FunCall ::= "call" SYMBOL "(" [Value {"," Value}] ")";
    void call(const std::string &to, const std::string &func,const std::vector<std::string>& params){
        if(to.length()){
            koopa_ir += "  " + to + " = ";
        }else{
            koopa_ir += "  ";
        }
        koopa_ir += "call " + func +"(";
        if(params.size()){
            int n = params.size();
            koopa_ir += params[0];
            for(int i = 1; i < n; ++i){
                koopa_ir += ", " + params[i];
            }
        }
        koopa_ir += ")\n";
    }

    void getelemptr(const std::string& to, const std::string &from, const int i){
        koopa_ir += "  " + to + " = getelemptr " + from + ", " + std::to_string(i) + "\n";
    }

    void getelemptr(const std::string& to, const std::string &from, const std::string& i){
        koopa_ir += "  " + to + " = getelemptr " + from + ", " + i + "\n";
    }

    void declLibFunc(){
        this->append("decl @getint(): i32\n");
        this->append("decl @getch(): i32\n");
        this->append("decl @getarray(*i32): i32\n");
        this->append("decl @putint(i32)\n");
        this->append("decl @putch(i32)\n");
        this->append("decl @putarray(i32, *i32)\n");
        this->append("decl @starttime()\n");
        this->append("decl @stoptime()\n");
        this->append("\n");
    }

    const char * c_str(){return koopa_ir.c_str();}
};

class BlockController{
private:
    bool f = true;
public:
    bool alive(){
        return f;
    }

    void finish(){
        f = false;
    }

    void set(){
        f = true;
    }
};

class WhileName{
public:
    std::string entry_name, body_name, end_name;
    WhileName(const std::string &_entry, const std::string & _body, const std::string &_end): entry_name(_entry), body_name(_body), end_name(_end){}
};

class WhileStack{
private:
    std::stack<WhileName> whiles;
public:
    void append(const std::string &_entry, const std::string & _body, const std::string &_end){
        whiles.emplace(_entry, _body, _end);
    }
    
    void quit(){
        whiles.pop();
    }

    std::string getEntryName(){
        return whiles.top().entry_name;
    }

    std::string getBodyName(){
        return whiles.top().body_name;
    }

    std::string getEndName(){
        return whiles.top().end_name;
    }
};