#ifndef SEM_PARSER__H
#define SEM_PARSER__H


#include "table-parser.h"

/**
 * @brief 
 * 
 * @param node 
 * @param table 
 * @return int 
 */
int parseSemError(Node *node, ListTable table);
int computeOpNode(Node *opNode);
int checkVariable(Node *prog);
void checkMain(ListTable table);
#endif