#include "symbol.h"



Symbol newSymbol(char *name, Kind kind, PrimType type, int offset, int lineno){
    Symbol symbol;

    symbol.symbol_name = (char*) malloc(sizeof(char) * strlen(name));
    if(!symbol.symbol_name){
        fprintf(stderr, "Error while allocating symbol name\n");
        exit(EXIT_FAILURE);
    }
    strcpy(symbol.symbol_name, name);
    symbol.kind = kind;
    symbol.u.p_type = type;
    symbol.offset = offset;
    symbol.lineno = lineno;
    return symbol;

}

Symbol newFunctionSymbol(char *name_func, PrimType return_type, PrimType arg_types[], int n_args, int is_void, int number_locals){
    Symbol func_sym;
    int i;
    func_sym.symbol_name = (char*) malloc(sizeof(char) * strlen(name_func));
    if(!func_sym.symbol_name){
        fprintf(stderr, "Error while allocating symbol name\n");
        exit(EXIT_FAILURE);
    }
    strcpy(func_sym.symbol_name, name_func);

    func_sym.kind = FUNCTION;
    func_sym.u.f_type.is_void = is_void;

    func_sym.u.f_type.return_type = return_type;
    for(i = 0; i < n_args; i++){
        func_sym.u.f_type.args_types[i] = arg_types[i];
        
    }
    func_sym.u.f_type.nb_args = n_args;
    func_sym.u.f_type.nb_local = number_locals;
    
    return func_sym;
}

Symbol calloc_symbol(){
    Symbol nullSymbol;

    nullSymbol.kind = NONE_KIND;
    nullSymbol.symbol_name = NULL;

    return nullSymbol;
} 

int is_symbol_null(Symbol symbol){
    return (symbol.kind == NONE_KIND) ? 1 : 0;
}

void print_symbol(Symbol s){
    printf("======= SYMBOL : %s========\n", s.symbol_name);
    
    fprintf(stderr, "Kind : ");
    switch(s.kind){
        case FUNCTION:
            fprintf(stderr, "function\n");
            printf("is void : %s\n", (s.u.f_type.is_void) ? "yes" : "no");
            
            printf("Return Type : %s\n", (s.u.f_type.return_type == CHAR) ? "Char\n" : "Int\n");
            printf("Number of argument : %d | Number of local variable : %d\n", s.u.f_type.nb_args, s.u.f_type.nb_local);
            
            break;
        case VARIABLE:
            fprintf(stderr, "variable | offset : %d\n", s.offset);
            break;
        case PARAM:
            fprintf(stderr, "parametre | offset : %d\n", s.offset);
            break;
        default:
            fprintf(stderr, "Unknown\n");
            break;
    }
    printf("==============================\n");

}

void free_symbol(Symbol *s){
    free(s->symbol_name);
}

PrimType stringOfTpcType(char* type){

    if(strcmp(type, "int") == 0){
        return INT;
    }
    else if(strcmp(type, "char") == 0){
        return CHAR;
    }
    else {
        return NONE;
    }
}

char * stringOfType(PrimType t){
    switch (t){
        case INT:
            return "int";
            break;
        case CHAR:
            return "char";
            break;
        default:
            return "none";
            break;
    }
}