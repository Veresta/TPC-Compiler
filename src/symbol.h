#ifndef __SYMBOL
#define __SYMBOL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


#define MAX_FUNC_NAME_SIZ 64
#define MAX_ARGUMENT_FUNC 15

typedef enum{

    FUNCTION, VARIABLE, VOID_KIND, NONE_KIND, PARAM

}Kind;

typedef enum {

    CHAR,
    INT,
    NONE
    
}PrimType;

typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    VOID_TYPE,
    NONE_TYPE
}Type;

typedef struct {
    
    int is_void; 
    PrimType args_types[MAX_ARGUMENT_FUNC];
    PrimType return_type;
    int nb_args;
    int nb_local;
}FunctionType;

typedef struct {
    PrimType type;
    int isGlobal;

}VariableType;

typedef struct symbol{


    char *symbol_name; /* name of the symbol  */
    Kind kind;         /* Kind of the symbol [function, variable or parameter] */
    int offset;
    int lineno;
    union {
        FunctionType f_type; /* Function  symbol */
        PrimType p_type;     /* Primitive symbol */
    }u;

}Symbol;

/**
 * @brief 
 * 
 * @return Symbol 
 */
Symbol calloc_symbol();

/**
 * @brief 
 * 
 * @param symbol 
 * @return int 
 */
int is_symbol_null(Symbol symbol);

/**
 * @brief 
 * 
 * @param name 
 * @param kind 
 * @param type 
 * @param offset 
 * @param lineno 
 * @return Symbol 
 */
Symbol newSymbol(char *name, Kind kind, PrimType type, int offset, int lineno);

/**
 * @brief 
 * 
 * @param type 
 * @return PrimType 
 */
PrimType stringOfTpcType(char* type);

/**
 * @brief 
 * 
 * @param s 
 */
void print_symbol(Symbol s);

/**
 * @brief 
 * 
 * @param t 
 * @return char* 
 */
char * stringOfType(PrimType t);

/**
 * @brief 
 * 
 * @param s 
 */
void free_symbol(Symbol *s);

/**
 * @brief Initialize a symbol for functions
 * 
 * @param name_func 
 * @param return_type 
 * @param arg_types 
 * @param n_args 
 * @param is_void 
 * @return Symbol 
 */
Symbol newFunctionSymbol(char *name_func, PrimType return_type, PrimType arg_types[], int n_args, int is_void, int local_variable);




#endif