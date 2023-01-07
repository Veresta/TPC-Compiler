#include "nasm.h"


#define LABEL_IF "startIF_"
#define LABEL_WHILE "startWHILE_"
#define LABEL_COND "labelCond_"
#define LABEL_ELSE "labelElse_"
#define LABEL_CODE "endIF_" 
#define LABEL_EXPR "labelExpr_"
#define LABEL_TRUE "TRUE"
#define LABEL_FALSE "FALSE"

#define LABEL_SWITCH_COND "labelCondSwitch_"
#define LABEL_CASE "labelCase_"
#define LABEL_DEFAULT "label_default:"
#define SWITCH_CHECK ".CHECK_"
#define LABEL_CODE_SWITCH "ENDWITCH"

#define OPERATOR 0
#define VAR 1
#define CONST 2
#define FUNCTION 3




FILE *f;
static int firstCall = 1;
static int labelId;
static int defaultId;


void nasmTranslateParsing(ListTable list, Node *root, Symbol_table *global_var_table, Symbol_table *localTable);
void functionCallInstr(Node *fCallNode, char *calledId, char *callerId, ListTable list);




static void initBss(int globalBssSize, ListTable listTable){
    

    fprintf(f, "section .bss\n");
    fprintf(f, "%s: resq %d\n", GLOBAL, globalBssSize);
    fprintf(f, "number: resq 1\n");
}

void writeNasmHeader(int globalBssSize, ListTable listTable){
    

    fprintf(f, "section .data\n");
    fprintf(f, "\t formatInt: db \"%s\", 10, 0\n", "%d");
    fprintf(f, "\t formatIntIn: db \"%s\", 0\n", "%d");
    fprintf(f, "\t fmtChar: db \"%s\", 0\n", "%c");
    initBss(globalBssSize, listTable);
    


    fprintf(f, "section  .text\n \
    global  _start\n \
    extern my_putchar\n \
    extern show_registers\n \
    extern my_getint\n \
    extern printf\n \
    extern scanf\n");   
    
}




void call_safe(char *content){
    SUB("rsp", "8");
    CALL(content);
    ADD("rsp", "8");
}

void end_asm(){
    fprintf(f, "_start:\n");

    
    fprintf(f, "\tcall main\n");

    fprintf(f, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
    fclose(f);
}


void writeTestRegisters(){
    call_safe("show_registers");
}

void write_global_eval(Symbol_table *global_table, Node *assign_node){

    
    Node *l_value = assign_node->firstChild;
    Node *r_value = l_value->nextSibling;

    if(!isSymbolInTable(global_table, l_value->u.ident) || isSymbolInTable(global_table, r_value->u.ident)){
        return;
    }

}   



/**
 * @brief 
 * 
 * @param content 
 */
static void callPrintf(char *content){
    
    MOV("rsi", content);
    
    MOV("rax", "0");
    MOV("rdi", "formatInt");
    call_safe("printf");
}

int isArityOne(Node *negNode){
    return (negNode->label == Addsub && (NULL == SECONDCHILD(negNode)));
}

int negInstr(Node *node, Symbol_table *globalTable, Symbol_table *funTable, ListTable list){
    char buf[BUFSIZ];
    if(!isArityOne(node)){
        return 0;
    }
    Node *child = FIRSTCHILD(node);
    Symbol varSym = getSymbolInTableByName(funTable, child->u.ident);
    int priority; 
    switch (child->label){
        case Variable:
            priority = symbolPriority(list, funTable, varSym.symbol_name);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (varSym.offset >= 0) ? "+" : "", varSym.offset);
            break;
        case Int:
            sprintf(buf, "%d", (child->u.num));
            break;
        case Character:
            sprintf(buf, "%d", (child->u.byte));
            break;
        case FunctionCall:


            functionCallInstr(child, child->u.ident, funTable->name_table, list);
            if(node->u.byte == '-') NEG("rax");
            


            return 1;
        default:
            break;
    }
    MOV("rax", buf);
    if(node->u.byte == '-') NEG("rax");
    return 1;
}

/* We suppose node parameter in the body of the main */
/**
 * @brief 
 * 
 * @param addSubNode 
 * @param symbolTable 
 */
void opTranslate(Node* addSubNode, Symbol_table *symbolTable, ListTable list, int stage){
    assert(addSubNode);
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int denominator;
    Symbol s;
    int a, b;
    int offset;
    int priority;
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    Symbol_table *currentFunTable;
    int isLeftInGlobal = 0, isRightInGlobal = 0;
    if(negInstr(addSubNode, globalTable, symbolTable, list)){
        return;
    }
    if(FIRSTCHILD(addSubNode)->label == Addsub || FIRSTCHILD(addSubNode)->label == divstar){
        opTranslate(FIRSTCHILD(addSubNode), symbolTable, list, stage+1);
    } else {
        switch (FIRSTCHILD(addSubNode)->label){
            case Int:
            case Character: 
                a = (addSubNode->firstChild->label == Character) ? addSubNode->firstChild->u.byte : addSubNode->firstChild->u.num;
                sprintf(buf, "%d", a);
                PUSH(buf);
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, FIRSTCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, FIRSTCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, FIRSTCHILD(addSubNode)->u.ident);
                }

                sprintf(buf, "qword [%s %s %d]",(priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);

                
                PUSH(buf);
                break;
            default:
                break;
        }
        
    }
    if(SECONDCHILD(addSubNode)->label == Addsub || SECONDCHILD(addSubNode)->label == divstar){
        opTranslate(SECONDCHILD(addSubNode), symbolTable, list, stage+1);
    }
    if(addSubNode && (addSubNode->label == Addsub || addSubNode->label == divstar)){

        switch (SECONDCHILD(addSubNode)->label){
            case Int:
            case Character:
                a = (SECONDCHILD(addSubNode)->label == Character) ? SECONDCHILD(addSubNode)->u.byte : SECONDCHILD(addSubNode)->u.num;
                if((addSubNode->u.byte == '/' || addSubNode->u.byte == '%')){
                    if(a == 0){
                        raiseError(addSubNode->lineno, "Trying to divide by 0\n");
                        exit(EXIT_FAILURE);
                    } 
                }
                sprintf(buf2, "%d", a);
                PUSH(buf2);
                
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, SECONDCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, SECONDCHILD(addSubNode)->u.ident);

                }

                sprintf(buf2, "qword [%s %s %d]",(priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);

                PUSH(buf2);
                break;
            default:
                break;
        }
        
    }
    if(addSubNode->label == Addsub || addSubNode->u.byte == '*'){
        POP("rcx");
        POP("rax");
    } else {
        POP("rbx");
        PUSH("0");
        POP("rdx");
        POP("rax");
    }

    switch(addSubNode->u.byte){
        case '-':
            SUB("rax", "rcx");
            break;
        case '+':
            ADD("rax", "rcx");
            break;
        case '*':
            MUL("rax", "rcx");
            break;
        case '/':
        case '%':
            DIV("rbx");
            break;
    }
    if(addSubNode->u.byte == '%'){
        PUSH("rdx"); 
        MOV("rax", "rdx");
        MOV("r12", "rdx");
    } else {
        PUSH("rax");
    }
    MOV("r12", "rax"); //Pour vérifier l'affichage avec show registers
    if(stage == 0){
        POP("rax");
    }
    
}

/*=================== Function setup ============ */

void callScanf(char *buf, char *format){
    MOV("r9", "rsp");
    AND("spl", "241");
    SUB("rsp", "16");
    PUSH("r9");
    MOV("rsi", buf);
    MOV("rdi", format);
    MOV("rax", "0");
    call_safe("scanf");

}

void callGetint(char *buf){
    callScanf("number", FMTINT);
    MOV("rax", "qword [number]");
    MOV(buf, "rax");
    MOV("rax", "0");

}

int check_primary_function(char *f_name){
    
    int ret;

    if(strcmp(f_name, "putint") == 0){
        ret = PUTINT;
    } else if(strcmp(f_name, "putchar") == 0)
    {
        ret = PUTCHAR;
    } else if(strcmp(f_name, "getint") == 0) {
        ret = GETINT;
    } else if(strcmp(f_name, "getchar") == 0){
        ret = GETCHAR;
    } else {
        ret = -1;
    }
    
    return ret;
    
}

/*=====================================================*/
void assignInstr(ListTable list, Node *assign_node, Symbol_table *globalTable, Symbol_table *localTable){
    int i;
    char c;
    int pos;
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int priority;
    int isGlobalLayer = 0;
    int checkPrimaryFun;
    Symbol lVar, rVal;
    Node *lValue = FIRSTCHILD(assign_node);
    Node *rValue = SECONDCHILD(assign_node);
    if(lValue->label == Int || lValue->label == Character){
        raiseError(assign_node->lineno, "trying to assign numeric or character\n");
        return;
    }
    if(isSymbolInTable(globalTable, lValue->u.ident)){
        lVar = getSymbolInTableByName(globalTable, lValue->u.ident);
        isGlobalLayer = 1;
    }
    //Local var
    if(isSymbolInTable(localTable, lValue->u.ident)){
        lVar = getSymbolInTableByName(localTable, lValue->u.ident);
        isGlobalLayer = 0;

    } else {
        isGlobalLayer = 1;
    }
    switch(rValue->label){
        case Character:
            c = rValue->u.byte;
            sprintf(buf, "%d", c);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, buf);
            break;
        case Int:
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, buf);
            break;
        case Addsub:
        case divstar:
            opTranslate(rValue, localTable, list, 0); // Put into r12 value of addition
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, "rax");
            MOV("rax", "0");
            MOV("rbx", "0");
            break;
        /*case Getint:
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            callGetint(buf2);
            break;
        case Getchar:
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            callScanf("number", FMTCHAR);
            MOV("rax", "qword [number]");
            MOV(buf2, "rax");
            MOV("rax", "0");
            break;*/
        case FunctionCall:
            COMMENT("[START] Assign to a function call");
            checkPrimaryFun = check_primary_function(rValue->u.ident);
            if(checkPrimaryFun >= 0){
                switch(checkPrimaryFun){
                    case GETINT:   
                        sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
                        callGetint(buf2);
                        break;
                    case GETCHAR:
                            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
                            callScanf("number", FMTCHAR);
                            MOV("rax", "qword [number]");
                            MOV(buf2, "rax");
                            MOV("rax", "0");
                        break;
                    default:

                        break;
                }
                break;
            } else {
                functionCallInstr(rValue, rValue->u.ident,  localTable->name_table, list);
                sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
                MOV(buf2, "rax");
            }

            

            COMMENT("[END] Assign to a function call");
            break;
        case Variable:
            COMMENT("assign variable");
            priority = symbolPriority(list, localTable, rValue->u.ident);
            rVal = getSymbolInTableByName((priority == IN_GLOBAL) ? globalTable : localTable, rValue->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_FUNCTION) ? "rbp" : GLOBAL, (rVal.offset >= 0) ? "+" : "", rVal.offset);
            MOV("rax", buf);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, "rax");
            break;
        default:
            return;
    }
}



int checkNodeContent(Node *n){
    switch(n->label){
        case Addsub:
        case divstar:
            return OPERATOR;
        case Variable:
            return VAR;
        case Int:
        case Character:
            return CONST;
        case FunctionCall:
            return FUNCTION;
        default:
            return -1;
    }
}



static void writePutint(Node *putIntNode, ListTable list, Symbol_table *globalTable, Symbol_table *funTable){
    char buf[BUFSIZ];
    int n;
    Symbol s;
    int priority;
    Node *child = FIRSTCHILD(putIntNode);
    switch(checkNodeContent(child)){
        case CONST:
            COMMENT("putint to const");
            n = child->u.num;
            break;
        case VAR:
            priority = symbolPriority(list, funTable, child->u.ident);
            s = getSymbolInTableByName((priority == IN_FUNCTION) ? funTable : globalTable, child->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_FUNCTION) ? "rbp" : GLOBAL, (s.offset >= 0) ? "+" : "", s.offset);
            callPrintf(buf);
            
            return;
        case OPERATOR:
            opTranslate(child, funTable, list, 0);
            callPrintf("rax");
            return;
        case FUNCTION:
            COMMENT("functionCall");
            functionCallInstr(child, child->u.ident, funTable->name_table, list);
            
            callPrintf("rax");
            COMMENT("END functionCall");
            return;

    }
    sprintf(buf, "%d", n);
    callPrintf(buf);
}

static void writePutchar(Node *putcharNode, ListTable list, Symbol_table *globalTable, Symbol_table *localTable){
    char buf[BUFSIZ];
    char c = FIRSTCHILD(putcharNode)->u.byte;
    Symbol variable;
    int contentLayer;
    switch (checkNodeContent(FIRSTCHILD(putcharNode))){
        case CONST:
            c = FIRSTCHILD(putcharNode)->u.byte;
            switch(c){
                case '\n':
                    sprintf(buf, "`\\n`");
                    MOV("rdi", buf);
                    CALL("my_putchar");
                    return;
                default:
                    sprintf(buf, "'%c'", c);
                    break;
            }
            break;
        case VAR:
            if(isSymbolInTable(globalTable, FIRSTCHILD(putcharNode)->u.ident)){
                variable = getSymbolInTableByName(globalTable, FIRSTCHILD(putcharNode)->u.ident);
                contentLayer = IN_GLOBAL;
            }
            if (isSymbolInTable(localTable, FIRSTCHILD(putcharNode)->u.ident)){
                variable = getSymbolInTableByName(localTable, FIRSTCHILD(putcharNode)->u.ident);
                contentLayer = IN_FUNCTION;
            } else {
                contentLayer = IN_GLOBAL;
            }
            
            sprintf(buf, "qword [%s %s %d]", (contentLayer == IN_GLOBAL) ? GLOBAL : "rbp", (variable.offset >= 0) ? "+" : "", variable.offset);
            break;
    }

    MOV("rdi", buf);
    CALL("my_putchar");
}



int compareInstrAux(Node *condNode, ListTable list, Symbol_table *funTable){
    Node *opLeft, *opRight;
    int lValue, rValue;
    Symbol_table *global;
    Symbol s;
    
    opLeft = FIRSTCHILD(condNode);
    opRight = SECONDCHILD(condNode);
    char buf[BUFSIZ];
    int isGlobalLayer;

    global = getTableInListByName(GLOBAL, list);


    switch(checkNodeContent(opLeft)){
        case OPERATOR: {
            opTranslate(condNode, funTable, list, 0);  //rax
            MOV("r14", "rax");
        }
        break;
        case VAR: {
            if(isSymbolInTable(global, opLeft->u.ident)){
                s = getSymbolInTableByName(global, opLeft->u.ident);
            } //TODO : remove to refract local variables
            if(isSymbolInTable(funTable, opLeft->u.ident)){
                s = getSymbolInTableByName(funTable, opLeft->u.ident);
                isGlobalLayer = 0;
            } 
            else {
                isGlobalLayer = 1;
            }

            sprintf(buf, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r14", buf);
        }
        break;
        case CONST: {
            if(opLeft->label == Character){
                lValue = opLeft->u.byte;
            }
            else {
                lValue = opLeft->u.num;
            }
            sprintf(buf, "%d", lValue);
            MOV("r14", buf);
        }
        break;
    }

    switch(checkNodeContent(opRight)){
        case OPERATOR: {
            opTranslate(condNode, funTable, list, 0); 
            MOV("r15", "rax");

        }
        break;
        case VAR: {
            if(isSymbolInTable(global, opRight->u.ident)){
                s = getSymbolInTableByName(global, opRight->u.ident);
            } 
            if(isSymbolInTable(funTable, opRight->u.ident)){
                s = getSymbolInTableByName(funTable, opRight->u.ident);
                isGlobalLayer = 0;
            } 
            else {
                isGlobalLayer = 1;
            }

            sprintf(buf, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r15", buf);
        }
        break;
        case CONST: {
            if(opRight->label == Character){
                rValue = opRight->u.byte;
            }
            else {
                rValue = opRight->u.num;
            }
            sprintf(buf, "%d", rValue);
            MOV("r15", buf);
        }
    }
}

void logicalInstr(Node *conditionNode, ListTable list, Symbol_table *funTable, char *labelIf, char *labelElse, char *labelCode, int hasElse){
    Symbol s;
    NasmFunCall compFun;
    Symbol_table* globalTable;
    char buf[BUFSIZ];
    switch(conditionNode->label){
        case FunctionCall:
            functionCallInstr(conditionNode, conditionNode->u.ident, funTable->name_table, list);
            MOV("rbx", "0");
            ADD("rbx", "rax");
            JG(labelIf);
            
            break;
        case Variable:
            globalTable = getTableInListByName(GLOBAL, list);
            //Global
            if(isSymbolInTable(globalTable, conditionNode->u.ident)){
                s = getSymbolInTableByName(globalTable, conditionNode->u.ident);
                sprintf(buf, "qword [%s + %d]", GLOBAL, s.offset);
            }
            //Local
            if(isSymbolInTable(funTable, conditionNode->u.ident)){
                s = getSymbolInTableByName(funTable, conditionNode->u.ident);
                sprintf(buf, "qword [%s %s %d]", "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            }
            MOV("rbx", "0");
            ADD("rbx", buf);
            JG(labelIf);
            break;
        case Int:
            sprintf(buf, "%d", conditionNode->u.num);
            MOV("rbx", "0");
            ADD("rbx", buf);
            JG(labelIf);
            break;
        case Eq:
        case Order:

            compareInstrAux(conditionNode, list, funTable);
            CMP("r14", "r15");
            
            if(strcmp(conditionNode->u.ident, "<=") == 0){
                compFun = JLE;
            }
            if (strcmp(conditionNode->u.ident, ">=") == 0){
                compFun = JGE;
            }
            if(strcmp(conditionNode->u.ident, "<") == 0){
                compFun = JL;
            }
            if(strcmp(conditionNode->u.ident, ">") == 0){
                compFun = JG;
            }
            if(strcmp(conditionNode->u.ident, "==") == 0){
                compFun = JE;
            }
            if(strcmp(conditionNode->u.ident, "!=") == 0){
                compFun = JNE;
            }

            nasmCall(compFun, labelIf, NULL);
            MOV("r14", "0");
            MOV("r15", "0");
            break;
        case Addsub:
        case divstar:
            opTranslate(conditionNode, funTable, list, 0); 
            ADD("rbx", "rax");
            JG(labelIf);

            break;
        case Or:

            sprintf(buf, "%s%d:", LABEL_EXPR, labelId);

            if((conditionNode->firstChild->label != Or && conditionNode->firstChild->label != And) &&
            (conditionNode->firstChild->nextSibling->label != Or && conditionNode->firstChild->nextSibling->label != And)){
                fprintf(f, "%s\n", buf);
            }
            labelId += 1;
            sprintf(buf, "%s%d:\n", LABEL_EXPR, labelId);
            logicalInstr(conditionNode->firstChild, list, funTable, labelIf, buf, labelCode, hasElse);
            sprintf(buf, "%s%d:\n", LABEL_EXPR, labelId);
            labelId += 1;
            fprintf(f, "%s", buf);
            logicalInstr(conditionNode->firstChild->nextSibling, list, funTable, labelIf, labelElse, labelCode, hasElse);
            break;
        case And:
            sprintf(buf, "%s%d", LABEL_EXPR, labelId);
            labelId += 1;
            fprintf(f, "%s:\n", buf);
            sprintf(buf, "%s%d", LABEL_EXPR, labelId);
            logicalInstr(conditionNode->firstChild->nextSibling, list, funTable, buf, labelElse, labelCode, hasElse);
            JMP(labelCode);
            fprintf(f, "%s:\n", buf);
            labelId += 1;
            logicalInstr(conditionNode->firstChild, list, funTable, labelIf, labelElse, labelCode, hasElse);
            break;
        default:
            break;
    }
}
void whileInstr(ListTable list, Node *whileNode, Symbol_table *globalTable, Symbol_table *localTable){
    char bufWhile[BUFSIZ];
    char bufCond[BUFSIZ];
    char bufEnd[BUFSIZ];
    labelId += 1;
    int currentId = labelId;
    Node *condNode = FIRSTCHILD(whileNode);
    sprintf(bufWhile, "%s%d", LABEL_WHILE, labelId);
    sprintf(bufCond, "%s%d", LABEL_COND, labelId);
    sprintf(bufEnd, "%s%d", LABEL_CODE, currentId);
    fprintf(f, "%s:\n", bufCond);
    logicalInstr(condNode, list, localTable, bufWhile, NULL, bufEnd, 0);
    JMP(bufEnd);
    fprintf(f, "%s:\n", bufWhile);
    nasmTranslateParsing(list, SECONDCHILD(whileNode), globalTable, localTable);
    JMP(bufCond);
    fprintf(f, "%s:\n", bufEnd);
}

void ifInstr(Node *ifInstr, ListTable list, Symbol_table *globalTable, Symbol_table *funTable){
    char buf[BUFSIZ];
    char bufLabel[BUFSIZ];
    char bufElse[BUFSIZ];
    char bufCode[BUFSIZ];
    Node *elseNode = NULL;
    int hasElse = 0; 
    int checkIf = 0;
    int currentId = labelId;
    NasmFunCall compFun;
    Node *cond = FIRSTCHILD(ifInstr);
    Symbol s;
    int value;
    for(Node *childIf = ifInstr->firstChild; childIf; childIf = childIf->nextSibling){
        hasElse = (childIf->label == Else) ? 1 : 0;
        if(hasElse){
            elseNode = childIf;
        }
        
    }
    sprintf(bufLabel, "%s%d", LABEL_IF, labelId);
    sprintf(bufCode, "%s%d", LABEL_CODE, currentId);
    if(hasElse) sprintf(bufElse, "%s%d", LABEL_ELSE, labelId);
    logicalInstr(cond, list, funTable, bufLabel, bufElse, bufCode, hasElse);
    if(hasElse) {
        JMP(bufElse);
    } else {
        JMP(bufCode);
    }
    
    LABEL(bufLabel);
    nasmTranslateParsing(list, SECONDCHILD(ifInstr), globalTable, funTable); //Parsing du IF
    JMP(bufCode);
    if(hasElse){
        fprintf(f, "%s:\n", bufElse);
        nasmTranslateParsing(list, FIRSTCHILD(elseNode), globalTable, funTable);
    }
    LABEL(bufCode);
}

void returnInstr(Node *returnNode, ListTable list, Symbol_table *globalTable, Symbol_table *localTable){
    int priority;
    char buf[BUFSIZ];

    if(!returnNode->firstChild){
        sprintf(buf, "end%s", localTable->name_table);
        JMP(buf);
        return;
    }

    Symbol funSym = localTable->self;
    Symbol valueSym;
    Node *valueNode = returnNode->firstChild;
    
    if(!valueNode){
        return;
    }

    switch (checkNodeContent(valueNode))
    {
    case OPERATOR:
        opTranslate(valueNode, localTable, list, 0);
        sprintf(buf, "%d", (valueNode->label == Int) ? valueNode->u.num : valueNode->u.byte);
        break;
    case VAR:
        priority = symbolPriority(list, localTable, valueNode->u.ident);
        valueSym = getSymbolInTableByName((priority == IN_GLOBAL) ? globalTable : localTable, valueNode->u.ident);
        sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (valueSym.offset >= 0) ? "+" : "", valueSym.offset);
        MOV("rax", buf);
        break;
    case CONST:
        sprintf(buf, "%d", (valueNode->label == Int) ? valueNode->u.num : valueNode->u.byte);
        MOV("rax", buf);
        break;
    case FUNCTION:
        functionCallInstr(valueNode, valueNode->u.ident, localTable->name_table, list);
        POP("rax");
        break;
    default:
        break;
    }

    sprintf(buf, "end%s", localTable->name_table);
    JMP(buf);

}

int hasSibling(Node *n, label_t label){
    for(Node *sibling = n->nextSibling; sibling; sibling = sibling->nextSibling){
        if(sibling->label == label){
            return 1;
        }
    }
    return 0;
}

int isLastCase(Node *caseNode){
    for(Node *sibling = caseNode->nextSibling; sibling; sibling = sibling->nextSibling){
        if(!sibling || sibling->label == Case){
            return 0;
        }
    }
    return 1;
}

void switchInstr(Node *switchNode, Symbol_table *globalTable, Symbol_table *localTable, ListTable list){
    char bufCond[BUFSIZ];
    char bufCode[BUFSIZ];
    char buf[BUFSIZ];
    char bufCase[BUFSIZ];
    char bufCheck[BUFSIZ];
    int alreadyInDefault;
    Node * hasDefault;
    int priority;
    int tmp, tmp2;
    int cpt = 0, cpt2 = 0;
    labelId += 1;
    int saveLabelId = labelId;
    defaultId += 1;
    int saveDefaultId = defaultId;
    int isFirstCondition = 0;
    int i = 0;
    hasDefault = hasChildLabel(switchNode, Default);

    MOV("r13", "0"); //Notre boolean

    sprintf(bufCheck, "%s%d%d", SWITCH_CHECK, saveLabelId, cpt); //.CHECK_XY
    sprintf(bufCond, "%s%d%d", LABEL_SWITCH_COND, saveLabelId, cpt);
    sprintf(bufCode, "%s%d", LABEL_CODE_SWITCH, saveLabelId);
    sprintf(bufCase, "%s%d%d", LABEL_CASE, saveLabelId, cpt);
    fprintf(f, "%s%d:\n", LABEL_SWITCH_COND, saveLabelId);
    
    //récupérer l'endroit où est stocker la fils du switch,  on fout le resultat dans r14
    switch(checkNodeContent(switchNode->firstChild)){
        case OPERATOR:
            opTranslate(switchNode->firstChild, localTable, list, 0);
            MOV("r14", "rax");
            break;
        case VAR:   
            priority = symbolPriority(list, localTable, switchNode->firstChild->u.ident);
            Symbol s = getSymbolInTableByName((priority == IN_FUNCTION) ? localTable : globalTable, switchNode->firstChild->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r14", buf);
            break;
        case CONST:
            if(switchNode->firstChild->label == Int){
                sprintf(buf, "%d", switchNode->firstChild->u.num);
            } else {
                sprintf(buf, "%c", switchNode->firstChild->u.byte);
            }
            MOV("r14", buf);

            break;
        default:
            raiseError(switchNode->lineno, "can't evaluate orders in switch\n");
            check_sem_err = 1;
            break;
    }

    nasmTranslateParsing(list, FIRSTCHILD(SECONDCHILD(switchNode)), globalTable, localTable);

    int whichSwitch[BUFSIZ];
    i = 0;
    //premier parsing
    for(Node *child = THIRDCHILD(switchNode); child; child = child->nextSibling){
        
        if(child->label == Case){
            cpt2 += 1;
            int res = saveLabelId * 10 + cpt2;
            whichSwitch[i] = res;
            //sprintf(buf, "");
            sprintf(bufCase, "%s%d", LABEL_CASE, res);
            Node *caseValue = child->firstChild;
            if(caseValue->label == Int){
                sprintf(buf, "%d", caseValue->u.num);
            } 
            if(caseValue->label == Character){
                sprintf(buf, "%d", caseValue->u.byte);
            }
            //TODO case Operator
            CMP("r14", buf);
            JE(bufCase);
            i++;
        }
    }
    if(hasDefault){
        sprintf(buf, "%s%d%d\n", "label_default", saveLabelId, saveDefaultId);
        JMP(buf);
    } else {
        JMP(bufCode);
    }

    i = 0;
    //deuxieme parsing
    for(Node *child = THIRDCHILD(switchNode); child; child = child->nextSibling){
        
        isFirstCondition = (i == 1);
        //Pour chaque case qu'on rencontre
        if(child->label == Case){
            sprintf(bufCase, "%s%d", LABEL_CASE, whichSwitch[i]);
            LABEL(bufCase);
            i++;
        }
        if(child->label == Default){
            sprintf(buf, "%s%d%d", "label_default", saveLabelId, saveDefaultId);
            LABEL(buf);
            alreadyInDefault = 1;
        }
        if(child->label == Break){
            if(hasDefault && !alreadyInDefault){
                sprintf(buf, "%s%d%d\n", "label_default", saveLabelId, saveDefaultId);
                JMP(buf);
            } else {
                JMP(bufCode);
            }
        }
        nasmTranslateParsing(list, child->firstChild, globalTable, localTable);
    }
    MOV("r14", "0");
    LABEL(bufCode);
}


void functionCallInstr(Node *fCallNode, char *calledId, char *callerId, ListTable list){
    Symbol paramSymbol;
    int priority;
    char buf[BUFSIZ];
    if(strcmp(calledId, callerId) == 0){
//mov rsp, rbp
        MOV("rsp", "rbp");
    }
    Symbol_table *callerTable = getTableInListByName(callerId, list);
    Symbol_table *calledTable = getTableInListByName(calledId, list);
    Symbol_table *globalTable;
    if(!(fCallNode->firstChild->label == Void)){  
        for(Node *paramNode = fCallNode->firstChild; paramNode; paramNode = paramNode->nextSibling){
            switch (paramNode->label)
            {
            case Variable:
                globalTable = getTableInListByName(GLOBAL, list);
                priority = symbolPriority(list, callerTable, paramNode->u.ident);
                paramSymbol = getSymbolInTableByName((priority == IN_GLOBAL) ? globalTable : callerTable, paramNode->u.ident);
                sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (paramSymbol.offset >= 0) ? "+" : "", paramSymbol.offset);
                PUSH(buf);
                break;
            case Int:
                sprintf(buf, "%d", paramNode->u.num);
                PUSH(buf);
                break;
            case Character:
                if(paramNode->u.byte == '\n'){
                    sprintf(buf, "\\n");
                } else {
                    sprintf(buf, "%d", paramNode->u.byte);
                }
                
                PUSH(buf);
                break;
            case FunctionCall:
                functionCallInstr(paramNode, paramNode->u.ident, callerId, list);
                PUSH("rax");
                break;
            case Addsub:
            case divstar:
                opTranslate(paramNode, callerTable, list, 0);
                PUSH("rax");
                break;
            default:
                break;
            }
        }
    }
    
    CALL(calledId);
}



void nasmTranslateParsing(ListTable list, Node *root, Symbol_table *global_var_table, Symbol_table *localTable){
    int primaryId;
    if(!root) {
        return;
    }

    switch(root->label){
        case Else:
            return;
        case Assign:
            COMMENT("Start assignement");
            assignInstr(list, root, global_var_table, localTable);
            COMMENT("End assignement");
            nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);
            return;
        /*case Putchar:
            writePutchar(root, list, global_var_table, localTable);
            break;
        case Putint:
            writePutint(root, list, global_var_table, localTable);
            nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);
            return;*/
        case If:
            labelId += 1;

            ifInstr(root, list, global_var_table, localTable);
            nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);
            return;
        case While:
            COMMENT("While instr start");
            whileInstr(list, root, global_var_table, localTable);
            COMMENT("While instr end");
            nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);
            return;
        case Return:
            COMMENT("Return instr start");
            returnInstr(root, list, global_var_table, localTable);
            COMMENT("Return instr end");
            break;
        case FunctionCall:
            COMMENT("Function call start");
            primaryId = check_primary_function(root->u.ident);
            if(primaryId >= 0){
                switch(primaryId){
                    case PUTINT:
                       
                        writePutint(root, list, global_var_table, localTable);
                        break;
                    case PUTCHAR:
                        writePutchar(root, list, global_var_table, localTable);
                        break;
                    case GETINT:
                        break;
                    case GETCHAR:
                        break;
                    default:
                        raiseError(-1, "Invalid primary funId\n");
                        break;
                }
            } else {
                functionCallInstr(root, root->u.ident, localTable->name_table, list);
            }
            
            COMMENT("End function call");
            break;
        case Switch:
            COMMENT("Switch start");
            switchInstr(root, global_var_table, localTable, list);
            COMMENT("Switch end");

            nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);
            return;
        default:
            break;
    }
    nasmTranslateParsing(list, root->firstChild, global_var_table, localTable);
    nasmTranslateParsing(list, root->nextSibling, global_var_table, localTable);

}




static void translateTree(Node *root, ListTable list){
    Node *funs = SECONDCHILD(root);
    char buf[BUFSIZ];
    char numeric[10];
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    for(Node *n = FIRSTCHILD(funs); n; n = n->nextSibling){
        char *nameFun = getFuncNameFromDecl(n);
        Symbol_table *funTable = getTableInListByName(nameFun, list);
        Symbol sTable = funTable->self;
        LABEL(nameFun);
        PUSH("rbp");
        MOV("rbp", "rsp");
        sprintf(numeric, "%d", sTable.u.f_type.nb_local * 8);
        SUB("rsp", numeric);
        nasmTranslateParsing(list, SECONDCHILD(n), globalTable, funTable);
        sprintf(numeric, "%d", (sTable.u.f_type.nb_local + sTable.u.f_type.nb_args) * 8);
        sprintf(buf, "end%s", nameFun);
        LABEL(buf);
        ADD("rsp", numeric);
        MOV("rsp", "rbp");
        POP("rbp");
        fprintf(f, "\tret\n");
    }
}


void buildNasmFile(Node *root, ListTable list, char *fname){
    if(fname){
        strcat(fname, ".asm");
    }
    f = fopen((fname) ? fname : "_anonymous.asm", "wr+");
    Symbol_table *table, *mainTable;
    table = getTableInListByName(GLOBAL, list);
    mainTable = getTableInListByName("main", list);
    writeNasmHeader(table->total_size, list);
    translateTree(root, list);
    end_asm();
}


void makeExecutable(char *fname){
    char *src;
    char buf[100];
    sprintf(buf, "nasm -f elf64 %s", (fname) ? fname : "_anonymous.asm");
    system(buf);
    if(fname){
        src = strtok(fname, ".");
    }
    sprintf(buf, "gcc -o %s my_putchar.o %s.o -nostartfiles -no-pie", (fname) ? src : "out", (fname) ? src : "_anonymous");
    system(buf);
    sprintf(buf, "rm -f %s.o", (fname) ? src : "_anonymous");
    system(buf);
    printf("Compilation succeed.\nExecutable file : %s\n", (fname) ? fname : "out");
}