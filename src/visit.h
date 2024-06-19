#pragma once
#include "koopa.h"
#include "Symbol.h"

class RiscvString{
private:
    std::string riscv_str;
    /**
     * 默认只用t0 t1 t2
     * t3 t4 t5作为备用，临时的，随时可能被修改，不安全
    */
public:
    bool immediate(int i){ return -2048 <= i && i < 2048; }

    void binary(const std::string &op, const std::string &rd, const std::string &rs1, const std::string &rs2){
    riscv_str += "  " + op + std::string(6-op.length(),' ') + rd + ", " + rs1 + ", " + rs2 + "\n";
    }
    
    void two(const std::string &op, const std::string &a, const std::string &b){
        riscv_str += "  " + op + std::string(6 - op.length(), ' ') + a + ", " + b + "\n";
    }

    void append(const std::string &s){
        riscv_str += s;
    }

    void mov(const std::string &from, const std::string &to){
        riscv_str += "  mv    " + to + ", "  + from + '\n';
    }

    void ret(){
        riscv_str += "  ret\n";
    }

    void li(const std::string &to, int im){
        riscv_str += "  li    " + to + ", " + std::to_string(im) + "\n";
    }

    void load(const std::string &to, const std::string &base ,int offset){
        if(offset >= -2048 && offset < 2048)
            riscv_str += "  lw    " + to + ", " + std::to_string(offset) + "(" + base + ")\n";    
        else{
            this->li("t3", offset);
            this->binary("add", "t3", "t3", base);
            riscv_str += "  lw    " + to + ", " + "0" + "(" + "t3" + ")\n";    
        }
    }


    void store(const std::string &from, const std::string &base ,int offset){
        if(offset >= -2048 && offset < 2048)
            riscv_str += "  sw    " + from + ", " + std::to_string(offset) + "(" + base + ")\n";    
        else{
            this->li("t3", offset);
            this->binary("add", "t3", "t3", base);
            riscv_str += "  sw    " + from + ", " + "0" + "(" + "t3" + ")\n";  
        }
    }

    void sp(int delta){
        if(delta >= -2048 && delta < 2048){
            this->binary("addi", "sp", "sp", std::to_string(delta));
        }else{
            this->li("t0", delta);
            this->binary("add", "sp", "sp", "t0");
        }
    }
    
    void label(const std::string &name){
        this->append(name + ":\n");
    }

    void bnez(const std::string &rs, const std::string &target){
        this->two("bnez", rs, target);
    }

    void jump(const std::string &target){
        this->append("  j     " + target + "\n");
    }

    void call(const std::string &func){
        this->append("  call " + func + "\n");
    }

    void zeroInitInt(){
        this->append("  .zero 4\n");
    }

    void word(int i){
        this->append("  .word " + std::to_string(i) + "\n");
    }

    void la(const std::string &to, const std::string &name){
        this->append("  la    " + to + ", " + name + "\n");
    }

    const char* c_str(){
        return riscv_str.c_str();
    }

};

// 后端riscv生成时，使用到的临时标号
class TempLabelManager{
private:
    int cnt;
public:
    TempLabelManager():cnt(0){ }
    std::string getTmpLabel(){
        return "Label" + std::to_string(cnt++);
    }
};

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice) ;
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &value);
int Visit(const koopa_raw_integer_t &value);
void Visit(const koopa_raw_binary_t &value);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_get_elem_ptr_t& get_elem_ptr);
void Visit(const koopa_raw_get_ptr_t& get_ptr);


void VisitGlobalVar(koopa_raw_value_t value);
void initGlobalArray(koopa_raw_value_t init);

void allocLocal(const koopa_raw_function_t &func);

size_t getTypeSize(koopa_raw_type_t ty);