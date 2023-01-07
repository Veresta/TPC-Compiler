/* tree.h */
#ifndef __TREE
#define __TREE
#include "utils.h"
typedef enum {
  Prog,
  DeclVars,
  DeclFoncts,
  types,
  divstar,
  id,
  DeclFonct,
  EnTeteFonct,
  Corps,
  Void,
  Parametres,
  ListTypVar,
  SuiteInstr,
  Exp,
  Variable,
  Assign,
  Body,
  Instr,
  If,
  Else,
  While,
  Or,
  And,
  Eq,
  Plus,
  Minus,
  Order,
  Addsub,
  FunctionCall,
  Neg,
  Int,
  Character,
  Arguments,
  ListExp,
  Return,
  EmptyInstr,
  Switch,
  Case,
  Default,
  Break
  
  /* list all other node labels, if any */
  /* The list must coincide with the string array in tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} label_t;

typedef struct Node {
  label_t label;
  
  struct Node *firstChild, *nextSibling;
  int lineno;
  union{
    char ident[64];
    int num;
    char byte;
    char comp[3];
  }u;
} Node;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);
char *stringFromLabel(int label);
char *getFuncNameFromDecl(Node *declFonctRoot);
int isPrimLabel();


#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
#endif