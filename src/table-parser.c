#include "table-parser.h"


ListTable newSymbolTableListTable(Symbol_table *table){
    ListTable ls_table;
    
    if(!(ls_table = (ListTable)malloc(sizeof(Table)))){
        fprintf(stderr, "Error while allocating ListTable of symbol table");
        return NULL; 
    }

    ls_table->table = table;
    ls_table->next = NULL;

    return ls_table;

}

int insertSymbolTableInListTable(ListTable listTable, Symbol_table *table){

    ListTable tmp;
    tmp = listTable;

    if(tmp->table == NULL){
        listTable->table = table;
        return 1;
    }
    for(tmp; tmp->next; tmp = tmp->next){}
    tmp->next = newSymbolTableListTable(table);
    if(!tmp->next){
        raiseError(-1, "Error while insert the table\n");
        return 0;
    }

    return 1;
}

void printSymbolTableList(ListTable lst){

    ListTable tmp = lst;
    if(!lst){
        return;
    }
    while(tmp != NULL){
        printSymbolTable(tmp->table);
        tmp = tmp->next;
    }
}

ListTable buildSymbolListTableFromRoot(Node *root){

    ListTable ListTable;
    Node* functions_root;
    PrimType function_t;
    int i = 0, is_void, nb_args = 0, global_offset = 0;
    int totalLocalVariable = 0;
    int offsetLocalVar = 0, offsetParam = 0;
    Symbol_table *globals_table = buildGlobalVariableSymbolTable(root);
    Symbol_table * putintTable, *putcharTable, *getcharTable, *getintTable;
    putintTable = buildPrimaryFunction(PUTINT);
    putcharTable = buildPrimaryFunction(PUTCHAR);
    getintTable = buildPrimaryFunction(GETINT);
    getcharTable = buildPrimaryFunction(GETCHAR);

    ListTable = newSymbolTableListTable(NULL);
    insertSymbolTableInListTable(ListTable, globals_table);
    insertSymbolTableInListTable(ListTable, putintTable);
    insertSymbolTableInListTable(ListTable, putcharTable);
    insertSymbolTableInListTable(ListTable, getintTable);
    insertSymbolTableInListTable(ListTable, getcharTable);


    functions_root = root->firstChild->nextSibling; //On DeclFoncts
    //parse of the DeclFonct
    for(Node* function_root = functions_root->firstChild; function_root; function_root = function_root->nextSibling){
        
        nb_args = 0;
        totalLocalVariable = 0;
        
        // =============== Management of the functions's header ==================
        
        Node* header_function = function_root->firstChild;
        PrimType param_types[MAX_ARGUMENT_FUNC];
        Node *function_type = header_function->firstChild;
        if(getTableInListByName(function_type->firstChild->u.ident, ListTable)){
            raiseError(function_type->lineno, "Function already defined\n");
        }
        is_void = header_function->firstChild->label == Void ? 1 : 0; 
        function_t = stringOfTpcType(function_type->u.ident);

        Symbol_table *table = newSymbolTable(function_type->firstChild->u.ident);
        Node *params = function_type->nextSibling;
        Symbol s;
        
        if(params->firstChild->label != Void){
            
            for (Node *paramType = params->firstChild; paramType; paramType = paramType->nextSibling){
                table->nb_parameter += 1;

            }
            offsetParam = table->nb_parameter * 8 + 8;
            for (Node *paramType = params->firstChild; paramType; paramType = paramType->nextSibling){
                //parametres
                Kind k = PARAM;
                PrimType type = stringOfTpcType(paramType->u.ident);
                Node* id = paramType->firstChild;
                s = newSymbol(id->u.ident, k, type, offsetParam, id->lineno);
                insertSymbol(s, table);
                

                //symbol de structure de fonction
                param_types[nb_args] = type;
                nb_args += 1;
                offsetParam -= 8;
            }
        }
        //=========================== Function's body ===========================

        Node* body = header_function->nextSibling;
        //Function's local variable :
        Node* local = body->firstChild;
        offsetLocalVar = -8;
        for(Node* localVarNode = local->firstChild; localVarNode; localVarNode = localVarNode->nextSibling){
            
            PrimType type = stringOfTpcType(localVarNode->u.ident); // type's variable
            Kind kind = VARIABLE;
            for(Node *id = localVarNode->firstChild; id; id = id->nextSibling){
                totalLocalVariable += 1;
                s = newSymbol(id->u.ident, kind, type, offsetLocalVar, id->lineno);
                insertSymbol(s, table);
                offsetLocalVar -= 8;
            }
        }
        table->total_size = totalLocalVariable;
        //TODO remove this
        Symbol params_sym = newFunctionSymbol(function_type->firstChild->u.ident, function_t, param_types, nb_args, is_void, totalLocalVariable);
        table->self = params_sym;
        //insertSymbol(params_sym, table);
        insertSymbolTableInListTable(ListTable, table);
    }
    return ListTable;
}


Node * hasChildLabel(Node *root, label_t label){
    for(Node *child = root->firstChild; child; child = child->nextSibling){
        if(label == child->label){
            return child;   
        }
    }
    return NULL;
}

/**
 * @brief To get the symbol table stored in the ListTable
 * @warning may return null if the table doesn't exist
 * @param name_table 
 * @param SymbolTableListTable 
 * @return Symbol_table* 
 */
Symbol_table *getTableInListByName(char *name_table, ListTable SymbolTableListTable){
    
    ListTable tmp = SymbolTableListTable;
    if(!SymbolTableListTable){
        return NULL;
    }
    
    while(1){

        if(!strcmp(name_table, tmp->table->name_table)){
            return tmp->table;
        }
        if(!tmp) return NULL;
        tmp = tmp->next;
        if(!tmp) return NULL;
    }
    
    return (tmp) ? tmp->table : NULL;
}
int symbolPriority(ListTable list, Symbol_table *functionTable, char *nameSymbol){
    Symbol_table *global;
    int isGlobalLayer = 0;
    global = getTableInListByName(GLOBAL, list);
    
    if(isSymbolInTable(global, nameSymbol)){
        isGlobalLayer = 1;
    }
    if (isSymbolInTable(functionTable, nameSymbol)){
        isGlobalLayer = 0;
    }
    return (isGlobalLayer) ? IN_GLOBAL : IN_FUNCTION;

}
