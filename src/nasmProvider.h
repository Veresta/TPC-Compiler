#ifndef __PROVIDER__
#define __PROVIDER__


#include "table-parser.h"

typedef enum {
    PUSH,
    POP,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    NOT,
    SYSCALL,
    JMP,
    JG,
    JE,
    RET,
    N_AND,
    N_OR,
    CALL,
    CMP,
    JLE,
    JGE,
    JL,
    JNE,
    LEAVE,
    COMMENT,
    AND_N,
    NEG
}NasmFunCall;

extern FILE* f;


int nasmCall(NasmFunCall nasmFunCall, char *var1, char *var2);

#define POP(r) nasmCall(POP, r, NULL)
#define PUSH(r) nasmCall(PUSH, r, NULL)
#define ADD(op, op2) nasmCall(ADD, op, op2)
#define MOV(c1, c2) nasmCall(MOV, c1, c2)
#define SUB(c1, c2) nasmCall(SUB, c1, c2)
#define MUL(c1, c2) nasmCall(MUL, c1, c2)
#define DIV(c1) nasmCall(DIV, c1, NULL)
#define SYSCALL(c) nasmCall(SYSCALL, c, NULL)
#define JMP(c) nasmCall(JMP, c, NULL)
#define JG(c) nasmCall(JG, c, NULL)
#define JE(c) nasmCall(JE, c, NULL)
#define JNE(c) nasmCall(JNE, c, NULL)
#define JLE(c) nasmCall(JLE, c, NULL)
#define JGE(c) nasmCall(JGE, c, NULL)
#define JNE(c) nasmCall(JNE, c, NULL)
#define LEAVE(c) nasmCall(LEAVE, c, NULL)
#define COMMENT(c) nasmCall(COMMENT, c, NULL)
#define AND(c1, c2) nasmCall(AND_N, c1, c2)
#define NEG(c1) nasmCall(NEG, c1, NULL)
#define CALL(c) nasmCall(CALL, c, NULL)
#define CMP(c, c2) nasmCall(CMP, c, c2)
#define LABEL(l) fprintf(f, "%s:\n", l);


#endif