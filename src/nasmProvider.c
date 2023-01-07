#include "nasmProvider.h"


char* stringOfNasmFun(NasmFunCall nasmFunction){
    switch (nasmFunction){
        case PUSH: return "push";
        case POP:  return "pop";
        case MOV:  return "mov";
        case ADD:  return "add";
        case SUB:  return "sub";
        case MUL:  return "imul";
        case DIV:  return "idiv";
        case NOT:  return "neg";
        case SYSCALL: return "syscall";
        case JMP:   return "jmp";
        case JG:    return "jg";
        case JE:    return "je";
        case RET:   return "ret";
        case N_AND: return "and";
        case N_OR:  return "or";
        case CALL:  return "call";
        case CMP :  return "cmp";
        case JLE : return "jle";
        case JGE : return "jge";
        case JL: return "jl";
        case JNE: return "jne";
        case LEAVE: return "leave";
        case COMMENT: return ";";
        case AND_N: return "and";
        case NEG:    return "neg";

    }
}


static int nasmArityOf(NasmFunCall fc){
    switch (fc){
        case PUSH: 
        case POP:    
        case NOT:  
        case JMP:
        case JG:
        case JE:
        case DIV:
        case CALL:
        case JLE:
        case JGE:
        case JL:
        case JNE:
        case COMMENT:
        case NEG:
            return 1;
        case MOV:
        case ADD: 
        case SUB: 
        case MUL: 
        case N_AND: 
        case N_OR:  
        case CMP:
        case AND_N:
            return 2;
        case SYSCALL:
        case RET: 
        case LEAVE:
            return 0;
    }
}


static int nasmFunArityCheck(NasmFunCall nasmFunCall, char *var1, char *var2){
    switch (nasmArityOf(nasmFunCall)){
        case 0:
            return (!var1) && (!var2);
        case 1:
            return var1 && (!var2);
        case 2:
            return var1 && var2;
        default:
            return -1;
    } 
}


int nasmCall(NasmFunCall nasmFunCall, char *var1, char *var2)
{  
    if(!nasmFunArityCheck(nasmFunCall, var1, var2)){
        DEBUG("Fail to write nasm function : incorrect usage\n");
        return 0;
    }

    switch(nasmArityOf(nasmFunCall)){
        case 0:
            fprintf(f, "\t%s\n", stringOfNasmFun(nasmFunCall));
            break;
        case 1:
            fprintf(f, "\t%s %s\n", stringOfNasmFun(nasmFunCall), var1);
            break;
        case 2:
            fprintf(f, "\t%s %s, %s\n", stringOfNasmFun(nasmFunCall), var1, var2);
            break;
        default:
            break;
    }
    return 1;

}
