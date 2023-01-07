%{
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
void yyerror(char *s);
int yylex(void);
extern int lineno;
extern int line_count;
extern int linecharno;
extern char *current_line;
struct Node* rootProg;
extern int check_sem_err;
extern int check_warn;

%}
%code requires {
#include "nasm.h"
#include "sem_parser.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
Node* root;
}
%expect 1
%union{
  Node *node;
  char ident[64];
  int num;
  char byte;
  char comp[3];
}

%token OR AND RETURN WHILE IF ELSE FOR VOID SWITCH DEFAULT CASE BREAK 
%token <num> NUM
%token <ident> IDENT TYPE
%token <byte>  DIVSTAR ADDSUB CHARACTER
%token <comp> EQ ORDER
%type <node> DeclVars Prog DeclFoncts Declarateurs DeclFonct EnTeteFonct Corps Parametres ListTypVar SuiteInstr Instr LValue Exp TB FB M E T F Arguments ListExp BeginSwitchExpr SwitchExpr 


%%
Prog:  DeclVars DeclFoncts              {$$ = makeNode(Prog); addChild($$, $1); addChild($$, $2); /*printTree($$); deleteTree($$);*/ rootProg = $$;}
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1; Node *t = makeNode(types); strcpy(t->u.ident, $2); addChild($$, t); addChild(t, $3);}
    |  %empty                           {$$ = makeNode(DeclVars);}
    ;
Declarateurs:  
       Declarateurs ',' IDENT           {$$ = $1; Node *n = makeNode(id); strcpy(n->u.ident, $3); addSibling($$, n);}
    |  IDENT                            {$$ = makeNode(id); strcpy($$->u.ident, $1);}
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1; addChild($$, $2);}
    |  DeclFonct                        {$$ = makeNode(DeclFoncts); addChild($$, $1);}
    ;
DeclFonct:
       EnTeteFonct Corps                {$$ = makeNode(DeclFonct); addChild($$, $1); addChild($$, $2);}                        
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(types); strcpy(type->u.ident, $1); addChild($$, type); Node *n = makeNode(id); strcpy(n->u.ident, $2); addChild(type, n); addChild($$, $4);}
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(Void); addChild($$, type); Node *n = makeNode(id); strcpy(n->u.ident, $2); addChild(type, n); addChild($$, $4);}
    ;
Parametres:
       VOID                             {$$ = makeNode(Parametres); addChild($$, makeNode(Void));}
    |  ListTypVar                       {$$ = makeNode(Parametres); addChild($$, $1);}
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT        {$$ = $1; Node *type = makeNode(types); addSibling($$, type); strcpy(type->u.ident, $3); Node *n = makeNode(id); strcpy(n->u.ident, $4); addChild(type, n);}
    |  TYPE IDENT                       {$$ = makeNode(types); strcpy($$->u.ident, $1); Node *n = makeNode(id); strcpy(n->u.ident, $2); addChild($$, n);}
    ;
Corps: '{' DeclVars SuiteInstr '}'      {$$ = makeNode(Body); addChild($$, $2); addSibling($2, $3);}
    ;
SuiteInstr:                             
       SuiteInstr Instr                 {if($1){$$ = $1; addSibling($$, $2);}else{$$=$2;}}
    |  %empty                           {$$ = NULL;}
    ;
Instr:
       LValue '=' Exp ';'                   {$$ = makeNode(Assign); addChild($$, $1); addChild($$, $3);}
    |  IF '(' Exp ')' Instr                 {$$ = makeNode(If); addChild($$, $3); addChild($$, $5);}
    |  IF '(' Exp ')' Instr ELSE Instr      {$$ = makeNode(If); addChild($$, $3); addChild($$, $5); Node *else_n = makeNode(Else); addSibling($3, else_n); addChild(else_n, $7);}
    |  WHILE '(' Exp ')' Instr              {$$ = makeNode(While); addChild($$, $3); addChild($$, $5);}
    |  IDENT '(' Arguments  ')' ';'         {$$ = makeNode(FunctionCall); $$->lineno = lineno; strcpy($$->u.ident, $1); addChild($$, $3);}
    |  SWITCH '(' Exp ')' '{' SuiteInstr BeginSwitchExpr '}' {$$ = makeNode(Switch); addChild($$,$3); Node* instrNode = makeNode(SuiteInstr); addChild(instrNode, $6); addChild($$, instrNode); addChild($$,$7);}
    |  RETURN Exp ';'                       {$$ = makeNode(Return); addChild($$, $2);}
    |  RETURN ';'                            {$$ = makeNode(Return);}
    |  '{' SuiteInstr '}'                   {$$ = $2;}
    |  ';'                                   {$$ = makeNode(EmptyInstr);}
    ;
Exp :  Exp OR TB                            {$$ = makeNode(Or); addChild($$, $1); addChild($$, $3);}
    |  TB                                   {$$ = $1;}
    ;
TB  :  TB AND FB                            {$$ = makeNode(And); addChild($$, $1); addChild($$, $3);}
    |  FB                                   {$$ = $1;}
    ;
FB  :  FB EQ M                              {$$ = makeNode(Eq); strcpy($$->u.comp, $2); addChild($$, $1); addChild($$, $3);}
    |  M                                    {$$ = $1;}
    ;
M   :  M ORDER E                            {$$ = makeNode(Order); strcpy($$->u.comp, $2); addChild($$, $1); addChild($$, $3);}
    |  E                                    {$$ = $1;}
    ;
E   :  E ADDSUB T                           {$$ = makeNode(Addsub); $$->u.byte = $2; addChild($$, $1); addChild($$, $3);}
    |  T                                    {$$ = $1;}
    ;    
T   :  T DIVSTAR F                          {$$ = makeNode(divstar); $$->u.byte = $2; addChild($$, $1); addChild($$, $3);}
    |  F                                    {$$ = $1;}
    ;
F   :  ADDSUB F                             {$$ = makeNode(Addsub); $$->u.byte = $1; addChild($$, $2);}
    |  '!' F                                {$$ = makeNode(Neg);}
    |  '(' Exp ')'                          {$$ = $2;}
    |  NUM                                  {$$ = makeNode(Int); $$->u.num = $1;}
    |  CHARACTER                            {$$ = makeNode(Character); $$->u.byte = $1;}
    |  LValue                               {$$ = $1;}
    |  IDENT '(' Arguments  ')'             {$$ = makeNode(FunctionCall); $$->lineno = line_count;strcpy($$->u.ident, $1); addChild($$, $3);}
    ;
LValue:
       IDENT                { $$ = makeNode(Variable); strcpy($$->u.ident, $1);}
    ;
BeginSwitchExpr:
        BeginSwitchExpr SwitchExpr {if($1){$$ = $1; addSibling($$,$2);}else{$$=$2;}}
    |   %empty                     {$$ = NULL;}
    ;
SwitchExpr: 
        CASE Exp ':' SuiteInstr  {$$ = makeNode(Case); addChild($$, $2); addChild($$, $4);}
    |   DEFAULT ':' SuiteInstr  {$$ = makeNode(Default); addChild($$, $3);}
    |   BREAK ';'   SuiteInstr  {$$ = makeNode(Break); addSibling($$, $3);}
    ;
Arguments:
       ListExp              {$$ = $1;}
    |  %empty               {$$ = makeNode(Void);}
    ;
ListExp:
       ListExp ',' Exp      {$$ = $1; addSibling($$, $3);}      
    |  Exp                  {$$ = $1;}
    ;   
%%
void yyerror(char *s){
    int i;
    fprintf (stderr, "%s near line %d at char : %d\n", s, line_count + 1, linecharno);
    fprintf(stderr, "%s", current_line);
    for(i = 0; i < linecharno - 1; i++){
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^");
    fprintf(stderr, "\n");
    free(current_line);
}

void print_help(){

    printf("=================================\n");
    printf("===========HELP GUIDE============\n");
    printf("=================================\n");
    printf("Command Line: ./tpcas [OPTION]\nWith OPTION equal to -h/--help or -t/--tree\n");
    printf("OPTION 1 : -h/--help , Print usage of the program.\n");
    printf("OPTION 2 : -t/--tree , Print the abstract tree on the standard output\n");
    printf("(You can use both options)\n");
}

void handler(int sig){
    if (sig == SIGUSR1){
        check_sem_err = 1;
    }
}

int main(int argc, char **argv){

    int option, hasArgument;
    int showTree = 0;
    int result;
    int opt = 0;
    int option_index = 0;
    int opt_asm = 0;
    int make_exec = 1;
    int sem_err_res = 0;
    int showTable = 0;
    int hasPath = 0;
    int hasFile = 0;
    char *fileName = NULL;
    char path[100];
    FILE *file;

    static struct option long_option[] = {

        {"help", no_argument,0,'h'},
        {"tree", no_argument,0,'t'},
        {"symtabs", no_argument,0,'s'},
        {"noexec", no_argument,0,'n'},
        {0,0,0,0}
    };
    while((opt = getopt_long(argc, argv,"t h s n", long_option, &option_index)) !=-1 ){

        switch(opt){

            case 'h' : print_help();
                       break;
            case 't' : showTree = 1;
                       break;
            case 's' : showTable = 1;
                       break;
            case 'n' : make_exec = 0;
                        break;

            default : result = 3; break;
        }
    }

    if(result == 3){
        return 3;
    }
    while((option = getopt(argc, argv, ":thsln")) != - 1){
        switch(option){
            case 't':
                showTree = 1;
                break;
            case 'h':
                print_help();
                break;
            case 's' : showTable = 1; 
                       break;
            case 'n' : make_exec = 0;
                       break;
            case '?':
                printf("unknown option, exit(3)\n");
                return 3;
        }
    }
    if(optind == (argc-1)){
        hasFile = 1;
        strcpy(path, argv[optind]);
        file = fopen(path, "r");
        char *token = strtok(argv[optind], "/");
        while(token){
            if(strstr(token, ".tpc") != NULL){
                break;
            }
            token = strtok(NULL, "/");
        }
        fileName = strtok(token, ".");
    }
    if(hasFile){
        

        int checkDup = dup2(fileno(file), STDIN_FILENO);
        if(checkDup == -1){
            perror("dup2");
        }
        fclose(file);

    }

    result = yyparse();
    if(result){
        return result;
    }
    signal(SIGUSR1, handler);

    if(showTree){
        printTree(rootProg);
    }
    ListTable list;

    list = buildSymbolListTableFromRoot(rootProg);
    
    if(showTable){
        printSymbolTableList(list);
    }
    checkMain(list);
    parseSemError(rootProg, list);
    checkVariable(rootProg);
    if (check_sem_err || !list){
        return 2;
    }

    
    
    DEBUG("=== NO SEM-ERRORS DETECTED ===\n");
    if(make_exec){
        buildNasmFile(rootProg, list, fileName);
        DEBUG("Generate executable...\n");
        makeExecutable(fileName); // make ./out
    }

        



    deleteTree(rootProg);
    return result;
}
