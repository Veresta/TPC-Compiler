#include "sem_parser.h"


/**
 * @brief To translate the operation 
 * 
 * @param opNode 
 * @return int 
 */
int computeOpNode(Node *opNode){
    switch(opNode->label){
        case Addsub:
            switch(opNode->u.byte){
                case '+':
                    return computeOpNode(FIRSTCHILD(opNode)) + computeOpNode(SECONDCHILD(opNode));
                case '-':
                    return computeOpNode(FIRSTCHILD(opNode)) + computeOpNode(SECONDCHILD(opNode));
                    break;
            }
        case divstar:
            switch(opNode->u.byte){
                case '*':
                    return computeOpNode(FIRSTCHILD(opNode)) * computeOpNode(SECONDCHILD(opNode));
                case '/':
                    return computeOpNode(FIRSTCHILD(opNode)) / computeOpNode(SECONDCHILD(opNode));
                case '%':
                    return computeOpNode(FIRSTCHILD(opNode)) % computeOpNode(SECONDCHILD(opNode));
            }
            break;
        case Int:
            return opNode->u.num;
        case Character:
            
            return opNode->u.byte;
    }
}

static PrimType getTypeOfNode(Node *node, Symbol_table *funTable, Symbol_table *globalTable){
    Symbol var;
    switch (node->label){
        case Int:  
            return INT;
        case Character:
            return CHAR;
        case Variable:
            if(isSymbolInTable(funTable, node->u.ident)){
                var = getSymbolInTableByName(funTable, node->u.ident);
            } else if (isSymbolInTable(globalTable, node->u.ident)){
                var = getSymbolInTableByName(globalTable, node->u.ident);
                
            } else {
                return NONE;
            }
            return var.u.p_type;
        default:
            return NONE;
    }
}


int isSymbolInGlobalAndFunc(char * symbol_name, Symbol_table *funTable, Symbol_table *globalTable)
{
    return isSymbolInTable(globalTable, symbol_name) && isSymbolInTable(funTable, symbol_name);
}

static int variableExistCheck(Node *varcall_node, ListTable table, char *name){
    Symbol_table *globals = getTableInListByName(GLOBAL, table);
    Symbol_table *func = getTableInListByName(name, table);
    int ret_val = isSymbolInTable(globals, varcall_node->u.ident) || isSymbolInTable(func, varcall_node->u.ident);
    
    return ret_val == 1;
}

static int functionCallParamCheck(ListTable list, Symbol_table *fun_caller_table, Symbol_table *function_table, Symbol_table *global_var_table, Node *fc_root){

    int i;
    i = 0;
    Symbol params = function_table->self;
    Symbol s;

    if (function_table->nb_parameter == 0 && fc_root->firstChild->label == Void){
        return 1;
    }
    
    for(Node *n = fc_root->firstChild; n; n = n->nextSibling){
        
        if(n->label == Variable){
            int count = isSymbolInTable(fun_caller_table, n->u.ident) + isSymbolInTable(global_var_table, n->u.ident);
            if(count == 0){
                raiseError(n->lineno, "undefined reference to variable '%s'\n", n->u.ident);
            } 
        }

        if(i >= params.u.f_type.nb_args){
            raiseError(n->lineno, "Too much parameters while trying to call function '%s'\n", fc_root->u.ident);
            break;
        }
        if(isPrimLabelNode(n)){
            if (labelToPrim(n->label) == INT && params.u.f_type.args_types[i] == CHAR){
                raiseWarning(n->lineno, "When trying to call '%s' : Expected type '%s' but the given type was '%s'\n", fc_root->u.ident, stringOfType(params.u.f_type.args_types[i]), stringOfType(labelToPrim(n->label)));
                i++;
                continue;
            }
            else {
                i++;
                continue;
            }
        }   
        if(n->label == FunctionCall){
            Symbol_table *paramFunTable = getTableInListByName(n->u.ident, list);
            Symbol symParamFun = paramFunTable->self;
            if(symParamFun.u.f_type.is_void){
                raiseError(n->lineno, "Attempt to call a void-function as parameter\n");
                return 0;
            }
            i++;
            continue;

            
        }
        if(n->label == Addsub){
            
            i++;
            continue;
        }
        //On récupère le symbol de la fonction pour vérifier ses paramètres
        if(isSymbolInTable(function_table, n->u.ident)){
            s = getSymbolInTableByName(function_table, n->u.ident);
        }  
        else if (isSymbolInTable(global_var_table, n->u.ident)){ //
            s = getSymbolInTableByName(global_var_table, n->u.ident);
        }  
        else if (isSymbolInTable(fun_caller_table, n->u.ident)){
            s = getSymbolInTableByName(fun_caller_table, n->u.ident);
        }
        else {
            raiseError(fc_root->lineno, "Not enough arguments given while calling function %s\n", function_table->name_table);
            return 0;
        }
        PrimType t = (s.kind == FUNCTION) ? s.u.f_type.return_type : s.u.p_type;
        if(t == CHAR && params.u.f_type.args_types[i] == Int){
            raiseWarning(fc_root->lineno, "parameter '%s' has type : %s but type expected was %s \n", i, s.symbol_name, stringOfType(s.u.p_type), stringOfType(params.u.f_type.args_types[i]));
            return 0;
        }
        i++;
    }
    if(i != params.u.f_type.nb_args){
        raiseError(fc_root->lineno, "Expected %d argument but %d were given\n", params.u.f_type.nb_args, i);
        return 0;
    }
    return 1;

}


static int functionCallCheck(Node *fc_node, ListTable table, char *callerFunctionName, char *calledFunctionName){

    Symbol_table *tableGlobal = getTableInListByName(GLOBAL, table);
    Symbol_table *tableCallerFunction = getTableInListByName(callerFunctionName, table);
    Symbol_table *tableCalledFunction = getTableInListByName(calledFunctionName, table);


    if(!tableCalledFunction){
        raiseError(fc_node->lineno, "implicit declaration of function '%s'\n", calledFunctionName);
        
        return 0;
    }

    return functionCallParamCheck(table, tableCallerFunction, tableCalledFunction, tableGlobal, fc_node);
}


static int equalCompareCheck(Node *eq, ListTable tab, char *name_tab){

    Symbol_table* global_tab;
    Symbol_table* function_tab;
    Node *var1 = eq->firstChild;
    Node *var2 = eq->firstChild->nextSibling;

    global_tab = getTableInListByName(GLOBAL, tab);
    function_tab = getTableInListByName(name_tab, tab);

    if(var1->label == Variable){
        char id1[20];
        strcpy(id1, var1->u.ident);
        if(!(isSymbolInTable(global_tab,id1)) && !(isSymbolInTable(function_tab,id1))){
            raiseError(var1->lineno, "variable '%s' neither declared as local in function %s or as globals\n", id1, name_tab);
            
            return 0;
        }
    }

    if(var2->label == Variable){
        char* id2 = var2->u.ident;
        if(!(isSymbolInTable(global_tab,id2)) && !(isSymbolInTable(function_tab,id2))){
            raiseError(var2->lineno, "Undefined reference to Variable '%s'\n", id2);
            
            return 0;
        }
        if(isSymbolInGlobalAndFunc(id2, function_tab, global_tab)){
            raiseError(var2->lineno, "symbol %s in both table function and global var\n", id2);
            
            return 0;
        }
    }

    return 1;
}





int assignCheck(Node *assign, ListTable tab, char *nameTable){
    
    Symbol_table *global_tab;
    Symbol_table *function_tab;
    Symbol_table *calledTable;
    int check = 0;
    global_tab = getTableInListByName(GLOBAL, tab);
    function_tab = getTableInListByName(nameTable, tab);

    Node *lValue = FIRSTCHILD(assign);
    Node *rValue = SECONDCHILD(assign);

    PrimType lType = getTypeOfNode(lValue, function_tab, global_tab);
    PrimType rType = getTypeOfNode(rValue, function_tab, global_tab);

    if(lType == CHAR && rType == INT) {
        raiseWarning(lValue->lineno, "assigning char variable '%s' to an integer\n", lValue->u.ident);
        check_warn = 1;
    }   

    if(lValue->label == Variable){
        //La variable est dans la table des globaux
        check += isSymbolInTable(global_tab, lValue->u.ident);
        if(isSymbolInTable(function_tab, lValue->u.ident)){
            Symbol s = getSymbolInTableByName(function_tab, lValue->u.ident);
            if(s.kind != FUNCTION){
                check += 1;
            } 
            if (rValue->label == FunctionCall){
                if(!functionCallCheck(rValue, tab, nameTable, rValue->u.ident)){
                    raiseError(rValue->lineno, "Function call failed\n");
                    
                    return 0;
                }
                calledTable = getTableInListByName(rValue->u.ident, tab);
                if(!calledTable){
                    raiseError(rValue->lineno, "implicit declaration of function '%s'\n", rValue->u.ident);
                    return 0;
                }
                Symbol fun = calledTable->self;
                if(fun.u.f_type.is_void){
                    raiseError(rValue->lineno, "Attempt to assign to a variable a void-function\n");
                    
                }
                if(lType == CHAR && fun.u.f_type.return_type == INT){
                    raiseWarning(rValue->lineno, "Return type of function '%s' doesn't match with the assigned variable\n Expected char, got int\n", rValue->u.ident);
                }
            }
            if(rValue->label == Variable){
                if(!variableExistCheck(rValue, tab, nameTable)){
                    raiseError(rValue->lineno, "Use undefined reference to variable '%s'\n", rValue->u.ident);
                }
            }
        }
        if(!check){
            raiseError(lValue->lineno, "Trying to assign a non-existing value : '%s'.\n", lValue->u.ident);
            
            return 0;
        }
        return check == 1 || check == 2;
    }
    return 1;
}


void checkDuplicatedCaseContentAux(ListTable listTable, char *tableName, Node *node, int *tab, int *index){
    int value;
    if(!node){
        return;
    }
    if(node->label == Case){
        Node *inCase = FIRSTCHILD(node);
        switch(inCase->label){
            case Int:
                tab[*index] = inCase->u.num;
                break;
            case Character:
                tab[*index] = inCase->u.byte;
                break;
            case Addsub:
            case divstar:
                tab[*index] = computeOpNode(inCase);
                break;
            default:
                break;
        }
        (*index)++;
    }
    checkDuplicatedCaseContentAux(listTable, tableName, node->nextSibling, tab, index);
}


void checkDuplicatedCaseContent(ListTable listTable, char *tableName, Node *node){
    int tab[BUFSIZ];
    int index = 0;
    int checkCaseContent = 0;

    checkDuplicatedCaseContentAux(listTable, tableName, node->firstChild->nextSibling->nextSibling, tab, &index);
    for(int i = 0; i < index - 1; i++){
        for(int j = i+1; j < index; j++){
            if(tab[i] == tab[j]){
                checkCaseContent = 1;
            }
        }
    }
    if(checkCaseContent){
        raiseError(node->lineno, "Duplicate case value\n");
    }
}

void nbDefault(int *cpt, Node *node){
    if(!node){
        return;
    }
    if(node->label == Default){
        (*cpt)++;
    }

    nbDefault(cpt, node->nextSibling);
}

/**
 * @brief 
 * 
 * @param assign 
 * @param tab 
 * @return int 
 */
int switchCheck(ListTable listTable, char * tableName, Node *switchNode){
    int cptDefault = 0;
    checkDuplicatedCaseContent(listTable, tableName, switchNode);
    nbDefault(&cptDefault, switchNode->firstChild);
    if(cptDefault > 1){
        raiseError(switchNode->lineno, "multiple default label in 1 switch\n");
    }
}


/**
 * @brief check if the scope starting in node is return complete or not
 * 
 * @param list 
 * @param node 
 * @return int 
 */
int isReturnComplete(ListTable list, Node *node) {
    Node *elseNode;
    Node *ifNode;
    Node *switchNode;
    if(node->label == Switch){
        goto next;
    }
    if(node->label == DeclFonct){
        //There, the function is return complete if there is a return in the main scope
        if(hasChildLabel(node->firstChild->nextSibling, Return)){
            return 1;
        }
        else {
            //Case if
            ifNode = hasChildLabel(node->firstChild->nextSibling, If);
            if(ifNode && (elseNode = hasChildLabel(ifNode, Else))){
                return isReturnComplete(list, ifNode) && isReturnComplete(list, elseNode);
            }
            if((switchNode = hasChildLabel(node->firstChild->nextSibling, Switch))){
                return isReturnComplete(list, switchNode);
            }
            if((ifNode && (!elseNode))){
                goto next;
            }
            return 0;
        }
    }
    next:
    if(node->label == If || node->label == Else){
        if(hasChildLabel(node, Return)){
            return 1;
        }
        else {
            ifNode = hasChildLabel(node, If);
            if(ifNode && (elseNode = hasChildLabel(ifNode, Else))){
                return isReturnComplete(list, ifNode) && isReturnComplete(list, elseNode);
            } else {
                return 0;
            }
        }
    }
    if(node->label == Switch){
        int checkReturn = 0;
        for(Node *child = node->firstChild; child; child =child->nextSibling){
            if(child->label == Case || child->label == Default){
                if(hasChildLabel(node, Return)){
                    checkReturn = 1;
                } 
                else {
                    checkReturn = isReturnComplete(list, child);
                }
            }
        }
        return checkReturn;
    }
    if(node->label == Case || node->label == Default){
        if(hasChildLabel(node, Return)){
            return 1;
        }
        else {
            for(Node *childCase = node->firstChild; childCase; childCase = childCase->nextSibling){
                if (childCase->label == Switch || childCase->label == If || childCase->label== Else){
                    return isReturnComplete(list, childCase);
                }
            }
            if(!hasChildLabel(node, Return)){
                return 0;
            }
        }
    }
}

static int hasReturnContent(ListTable list, Node *n){
    if(!n){
        return 0;
    }
    if(n->label == Return){
        if(n->firstChild && n->firstChild->label == FunctionCall){
            Symbol_table *table = getTableInListByName(n->firstChild->u.ident, list);
            Symbol sfun = table->self;
            if(sfun.u.f_type.is_void){
                raiseError(n->lineno, "void function not ignored as it ough to be\n");
            }
        }
        if(n->firstChild){
            return 1;
        }

    }

    return hasReturnContent(list, n->firstChild) || hasReturnContent(list, n->nextSibling);
}

void checkReturnsRec(ListTable list, Node *n, PrimType type, Symbol_table *localTable, Symbol_table *globalTable){
    if(!n){
        return;
    }
    
    if(n->label == Return){
        if(!(n->firstChild)){
            raiseWarning(n->lineno, "return with no value, in non-void return function\n");
            
            goto end;
        }
        if(n->firstChild->label == Variable){
            if(type == CHAR){
                int priority = symbolPriority(list, localTable, n->firstChild->u.ident);
                Symbol s = getSymbolInTableByName((priority == IN_FUNCTION) ? localTable : globalTable, n->firstChild->u.ident);
                if(s.u.p_type == INT){
                    raiseWarning(n->lineno, "Return an int in a function returning a char\n");
                }
            }
        }
        if(n->firstChild->label == Int && type == CHAR){
            raiseWarning(n->lineno, "Return an int in a function returning a char\n");
        }
        if(n->firstChild->label == FunctionCall){
            Symbol_table *table = getTableInListByName(n->firstChild->u.ident, list);
            Symbol sfun = table->self;
            if(sfun.u.f_type.is_void){
                raiseError(n->lineno, "void function not ignored as it ough to be\n");
            }
        }
    }
    end:
    checkReturnsRec(list, n->firstChild, type, localTable, globalTable);
    checkReturnsRec(list, n->nextSibling, type, localTable, globalTable);
}

/**
 * @brief We suppose all functions return are return completed
 * We suppose n -> body of function
 * 
 * @param n 
 * @return int 
 */
void checkReturnsValue(ListTable list, Node *n, Symbol_table* table, Symbol sym){
    //
    if(sym.u.f_type.is_void){
        if(hasReturnContent(list, n)){
            raiseWarning(n->lineno, "return with a value, in function returning void\n");
        } 

    } else {
        PrimType type = sym.u.f_type.return_type;
        Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
        checkReturnsRec(list, n, type, table, globalTable);
    }
}

/**
 * @brief parse the tree and check if there is a sem error or not
 * 
 * @param n 
 * @param table 
 * @return int 1 if the parse returned no error, 0 if there is at least 1 sem error
 */
static int parseSemErrorAux(Node *n, ListTable table, char *name_table){
    if (!n){
        return 1;
    }
    if(n->label == EnTeteFonct) return parseSemErrorAux(n->nextSibling, table, name_table);
    if(n->label == DeclFonct) return parseSemErrorAux(n->firstChild->nextSibling, table, name_table);
    switch(n->label){
        case FunctionCall:
            functionCallCheck(n, table, name_table, n->u.ident);
            break;
        case Variable: 
            if(!variableExistCheck(n, table, name_table)){
                raiseError(n->lineno, "Use undefined reference to variable '%s'\n", n->u.ident);
            }
            parseSemErrorAux(n->nextSibling, table, name_table);
            return 1;
        case Assign:
            assignCheck(n, table, name_table);
            parseSemErrorAux(n->nextSibling, table, name_table);
            return 1;
        case Eq:
            equalCompareCheck(n, table, name_table);
            break;
        case Switch:
            switchCheck(table, name_table, n);
            break;
        default:
            parseSemErrorAux(n->nextSibling, table, name_table);
            parseSemErrorAux(n->firstChild, table, name_table);
            return 1;
    }
    parseSemErrorAux(n->nextSibling, table, name_table);
    parseSemErrorAux(n->firstChild, table, name_table);
}

int warnCheckAssign(Node *body, char *symbolId){
    int isAssign = 0;
    if(!body){
        return 0;
    }
    if((body->label == Assign) && strcmp(symbolId, FIRSTCHILD(body)->u.ident) == 0){
        return 1;
    }
    return warnCheckAssign(body->nextSibling, symbolId) || warnCheckAssign(body->firstChild, symbolId);
}

int checkUsedVar(Node *root, Node *body, char *symbolId, int lineno){
    if(!body){
        return 0;
    }
    if(body->label == Addsub || body->label == divstar){
        if(body->firstChild->label == Addsub || body->firstChild->label == divstar){
            checkUsedVar(root, body->firstChild, symbolId, lineno);
        }
        if(SECONDCHILD(body) && (SECONDCHILD(body)->label == Addsub || SECONDCHILD(body)->label == divstar)){
            checkUsedVar(root, body->firstChild, symbolId, lineno);
        }
        if(strcmp(symbolId, body->u.ident) == 0){
            if(!warnCheckAssign(root, symbolId)){
                raiseWarning(lineno, "'%s' is used uninitialized in this function near line %d\n", symbolId, body->lineno);
            }
            return 1;
        }
    }
    return checkUsedVar(root, body->firstChild, symbolId, lineno) || checkUsedVar(root, body->nextSibling, symbolId, lineno);
}

void checkVariableState(Node *curNode, int *isUsed, int *isInitialized, char *symbolId, int baseLineno){
    if(curNode && (curNode->label == Variable) && strcmp(curNode->u.ident, symbolId) == 0){
        (*isUsed) = 1;
        if(!(*isInitialized)){
            raiseWarning(baseLineno, "'%s' is used uninitialized in this function near line %d\n", symbolId, curNode->lineno);
        }
    }
}

void parseVariableUsage(Node *root, Node *currentNode, char *symbolId, int baseLineno, int *isInitialized, int *isUsed){
    if((*isInitialized) > 0 && (*isUsed) > 0){
        return;
    }

    for(Node *child = currentNode->firstChild; child; child = child->nextSibling){
        switch(child->label){
            case Switch:
                checkVariableState(child->firstChild, isUsed, isInitialized, symbolId, baseLineno);
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
            case SuiteInstr:
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
            case If:
                checkVariableState(child->firstChild, isUsed, isInitialized, symbolId, baseLineno);
            case Else:
            case While:
                checkVariableState(child->firstChild, isUsed, isInitialized, symbolId, baseLineno);
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
            case Addsub:
            case divstar:
                if(child->firstChild && (child->firstChild->label == Addsub || child->firstChild->label == divstar)){
                    parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                }
                checkVariableState(child->firstChild, isUsed, isInitialized, symbolId, baseLineno);
                if(SECONDCHILD(child) && (SECONDCHILD(child)->label == Addsub || SECONDCHILD(child)->label == divstar)){
                    parseVariableUsage(root, SECONDCHILD(child), symbolId, baseLineno, isInitialized, isUsed);
                }
                checkVariableState(SECONDCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                break;
            case Order:
            case Eq:
                checkVariableState(child->firstChild, isUsed, isInitialized, symbolId, baseLineno);
                checkVariableState(SECONDCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                break;
            case FunctionCall:  
                for(Node *param = child->firstChild; param; param = param->nextSibling){
                    checkVariableState(param, isUsed, isInitialized, symbolId, baseLineno);
                }
                
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
            case Or:
            case And:
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);

                checkVariableState(FIRSTCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                checkVariableState(SECONDCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                break;
            case Assign:
                //left value
                if(strcmp(symbolId, FIRSTCHILD(child)->u.ident) == 0){
                    (*isInitialized) = 1;
                }
                
                //right Value
                if(SECONDCHILD(child)->label == Addsub || SECONDCHILD(child)->label == divstar || SECONDCHILD(child)->label == FunctionCall){
                    parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                    parseVariableUsage(root, SECONDCHILD(child), symbolId, baseLineno, isInitialized, isUsed);
                }
                checkVariableState(SECONDCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                break;
            case Return:
                checkVariableState(FIRSTCHILD(child), isUsed, isInitialized, symbolId, baseLineno);
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
            case Case:
                parseVariableUsage(root, child, symbolId, baseLineno, isInitialized, isUsed);
                break;
        }
    }

}

int checkVariable(Node *prog){
    int isInitialized = 0, isUsed = 0;
    Node *vars;
    Node *declFoncts = SECONDCHILD(prog);
    for(Node *fonct = declFoncts->firstChild; fonct; fonct = fonct->nextSibling){
        vars = FIRSTCHILD(SECONDCHILD(fonct));
        if(vars != NULL && vars->label == DeclVars){
            
            for(Node *types = vars->firstChild; types; types = types->nextSibling){
                
                for(Node *var = types->firstChild; var; var = var->nextSibling){
                    
                    isInitialized = 0;
                    isUsed = 0;
                    parseVariableUsage(prog, SECONDCHILD(fonct), var->u.ident, var->lineno, &isInitialized, &isUsed);
                    
                    if(isInitialized && !isUsed){
                        
                        raiseWarning(var->lineno, "variable '%s' set but not used\n", var->u.ident);
                    }
                    
                    if(!isInitialized && !isUsed){
                        
                        raiseWarning(var->lineno, "unused variable '%s'\n", var->u.ident);
                    }
                    
                }
            }
        }
        
    }
    
}

void checkMain(ListTable table){
    Symbol_table *mainTable;
    mainTable = getTableInListByName("main", table);
    if(!mainTable){
        raiseError(-1, "Can't find reference to 'main'\n");
    } else {
        Symbol s_main = mainTable->self;
        if(s_main.u.f_type.is_void || s_main.u.f_type.return_type != INT){
            raiseError(-1, "Expected main to return an integer\n");
        }
    }
}

int parseSemError(Node *node, ListTable table){
    if(!node){
        return 1;
    }

    if(node->label != DeclFoncts){
        parseSemError(node->nextSibling, table);
        parseSemError(node->firstChild, table);
    }
    else{
        for(Node *n = node->firstChild; n; n = n->nextSibling){
            char *nameFun = getFuncNameFromDecl(n);
            if(isSymbolInTable(getTableInListByName(GLOBAL, table), nameFun)){
                raiseError(n->lineno, "Invalid symbol id : %s, already referenced as global\n", nameFun);
            }
            Symbol_table *sTable = getTableInListByName(nameFun, table);
            Symbol funS = sTable->self;
            if(!funS.u.f_type.is_void){
                if(!isReturnComplete(table, n)){
                    raiseWarning(n->lineno, "control reaches end of non-void function\n");
                }
            }
            checkReturnsValue(table, SECONDCHILD(n), sTable, funS);
            parseSemErrorAux(n, table, getFuncNameFromDecl(n));
        }
    }
    return 1;
}










