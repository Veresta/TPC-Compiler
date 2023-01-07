#include "symbols-table.h"



unsigned long hash(unsigned char *str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = hash + c; 

    return hash;
}

Symbol_table *newSymbolTable(char *name_table){
    Symbol_table *table;
    int i;
    if(!(table = malloc(sizeof(Symbol_table)))){
        raiseError(-1, "Failed to allocate the table\n");
        return NULL;
    }
    if(!(table->name_table = malloc(sizeof(char) * strlen(name_table)))){
        raiseError(-1, "Failed to allocate the table's name\n");
        return NULL;
    }
    if(!(table->s = malloc(sizeof(Symbol) * INIT_TABLE_SIZ))){
        DEBUG("Failed to allocated the symbol table : %s\n", name_table);
        return NULL;
    }
    strcpy(table->name_table, name_table);
    table->nb_symbol = 0;
    table->total_size = 0;
    table->size = INIT_TABLE_SIZ;
    table->nb_parameter = 0;
    table->u.number_globals = 0;
    table->self = calloc_symbol();
    for(i = 0; i < INIT_TABLE_SIZ; i++){
        table->s[i] = calloc_symbol();
    }
    return table;
}

Symbol getSymbolInTableByName(Symbol_table *table, char *symbolName){
    return table->s[hash(symbolName)];
}

int isSymbolInTable(Symbol_table *table, char *symbol_name){
    return table->size > hash(symbol_name) && 
    table->s[hash(symbol_name)].symbol_name != NULL && 
    strcmp(table->s[hash(symbol_name)].symbol_name, symbol_name) == 0;
}

static int realloc_table(Symbol_table *table, unsigned long hashkey){
    Symbol *s;
    int i;
    if(table->size <= hashkey){
        long int old_size = table->size;
        table->size += hashkey;
        if(!(s = (Symbol*)realloc(table->s, (sizeof(Symbol)) * table->size))){
            raiseError(-1, "Error while realloc table->symbol | size : %ld\n", table->size);
            exit(EXIT_FAILURE);
        }
        table->s = s;
        for(i = old_size; i < hashkey; i++){
            table->s[i] = calloc_symbol();
        }
    }
    return 1;
}

int insertSymbol(Symbol symbol, Symbol_table *table){
    Symbol *s;
    int i;
    unsigned long hashKey = hash(symbol.symbol_name);
    realloc_table(table, hashKey);
    if(table->s[hash(symbol.symbol_name)].symbol_name && !strcmp(table->s[hash(symbol.symbol_name)].symbol_name, symbol.symbol_name)){
        raiseError(-1, "In function '%s', symbol '%s' already declared as parameter or local variable\n", table->name_table, symbol.symbol_name);
        return 0;
    } else {
        table->s[hashKey] = symbol;
        table->nb_symbol += 1;
    }
    return 1;
}

int isPrimLabelNode(Node *n){
    switch(n->label){
        case Int:
        case Character:
            return 1;
        default:
            return 0;
    }
}

PrimType labelToPrim(label_t label){
    switch(label){
        case Int:
            return INT;
        case Character:
            return CHAR;
        default:
            return NONE;
    }
}

/* We suppose there is a var node */
Symbol_table *buildGlobalVariableSymbolTable(Node *root){
    Symbol symbol;
    PrimType type;
    Symbol_table *table = newSymbolTable(GLOBAL);
    int currentOffset = 0, nbGlobals = 0;
    if (!(root->firstChild)){
        return NULL;
    }
    Node *nodeVars = FIRSTCHILD(root); /* Node of DeclVars according to our tree */
    
    //Variable globaux
    for (Node *child = nodeVars->firstChild; child; child = child->nextSibling) {
        Kind kind = VARIABLE;
        type = (strcmp("int", child->u.ident) == 0) ? INT : CHAR;
        for(Node *grandChild = child->firstChild; grandChild; grandChild = grandChild->nextSibling){
            Symbol s = newSymbol(grandChild->u.ident, kind, type, currentOffset, grandChild->lineno);
            insertSymbol(s, table);
            nbGlobals += 1;
            currentOffset += 8;
        }
    }

    table->total_size = nbGlobals;
    return table;
}



void printSymbolTable(Symbol_table *tab){
    int pos;
    if(tab == NULL){
        printf("NULL \n");
        return;
    }

    printf("TAB NAME : %s\n",tab->name_table);
    
    if(strcmp(tab->name_table, GLOBAL) != 0){
        printf("self symbol : \n");
        print_symbol(tab->self);
        printf(" -------------------------\n");
    }
    
    printf("number of symbols : %d\n",tab->nb_symbol);
    for(pos = 0; pos < tab->size;pos++){
        if(tab->s[pos].symbol_name == NULL) continue;;
        print_symbol(tab->s[pos]);
        
    }
}

//Symbol newFunctionSymbol(char *name_func, PrimType return_type, PrimType arg_types[], int n_args, int is_void, int number_locals)

Symbol_table *buildPrimaryFunction(int funId){
    Symbol_table *funTable;
    Symbol s;
    int n_args;
    int is_void;
    PrimType args[10];
    Kind kind = FUNCTION;
    PrimType returnType;
    switch(funId){
        case PUTINT:
            funTable = newSymbolTable("putint");
            args[0] = INT;
            s = newFunctionSymbol("putint", NONE, args, 1, 1, 0);
            break;
        case PUTCHAR:
            funTable = newSymbolTable("putchar");
            n_args = 1;
            is_void = 1;
            args[0] = CHAR;
            returnType = NONE;
            s = newFunctionSymbol("putchar", NONE, args, 1, 1, 0);
            break;
        case GETCHAR:
            funTable = newSymbolTable("getchar");
            n_args = 0;
            is_void  = 0;
            returnType = CHAR;
            s = newFunctionSymbol("getchar", CHAR, args, 0, 0, 0);
            break;
        case GETINT:
            funTable = newSymbolTable("getint");
            n_args = 0;
            is_void = 0;
            returnType = INT;
            s = newFunctionSymbol("getint", INT, args, 0, 0, 0);
            break;
        default:
            DEBUG("Unvailable primary function id\n");
            return NULL;
    }
    funTable->self = s;
    //insertSymbol(s, funTable);
    return funTable;
}