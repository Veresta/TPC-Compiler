#ifndef __SYMBOLS_TABLE
#define __SYMBOLS_TABLE

#include "symbol.h"
#include "tree.h"


#define MAX_SIZE_TABLE 255
#define INIT_NAME_TABLE_SIZE 20
#define INIT_TABLE_SIZ 10000
#define INIT_PARAMETERS_SIZ 10

#define PUTINT 0
#define PUTCHAR 1
#define GETINT 2
#define GETCHAR 3

#define GLOBAL "global_vars"


typedef struct table{
    
    char *name_table;
    unsigned long size;
    Symbol self;
    Symbol *s;          //Hashtable of symbols
    int nb_symbol;
    Type *parameters;   
    int nb_parameter;   //Its a functions's symbol table
    int total_size;

    union {
        int number_globals;
    }u;

}Symbol_table;

int check_sem_err; 

/**
 * @brief Create a symbol table object
 * 
 * @param name_table 
 * @return Symbol_table* 
 */
Symbol_table *newSymbolTable(char *name_table);

/**
 * @brief Get the symbol by name object
 * 
 * @param table 
 * @param symbolName 
 * @return Symbol 
 */
Symbol getSymbolInTableByName(Symbol_table *table, char *symbolName);

/**
 * @brief 
 * 
 * @param symbol 
 * @param table 
 * @return int 
 */
int insertSymbol(Symbol symbol, Symbol_table *table);

/**
 * @brief Create a global variable table object
 * 
 * @param tree 
 * @return Symbol_table* 
 */
Symbol_table *buildGlobalVariableSymbolTable(Node *tree);

/**
 * @brief 
 * 
 * @param tab 
 */
void printSymbolTable(Symbol_table *tab);

/**
 * @brief 
 * 
 * @param table 
 * @param symbol_name 
 * @return int 
 */
int isSymbolInTable(Symbol_table *table, char *symbol_name);

/**
 * @brief 
 * 
 * @param n 
 * @return int 
 */
int isPrimLabelNode(Node *n);

/**
 * @brief 
 * 
 * @param label 
 * @return PrimType 
 */
PrimType labelToPrim(label_t label);

/**
 * @brief 
 * 
 * @return Symbol_table* 
 */
Symbol_table *buildPrimaryFunction(int funId);

#endif