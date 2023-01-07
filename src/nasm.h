#ifndef __ASM_H
#define __ASM_H

#include "nasmProvider.h"


#define FMTINT "formatIntIn"
#define FMTCHAR "fmtChar"


/**
 * @brief Generate the translator as a file.asm
 * Beware, we suppose the file.tpc is semanticly correct 
 * @param root root of the tree which begin the parsing to translate
 * @param list 
 */
void buildNasmFile(Node *root, ListTable list, char *fname);

/**
 * @brief compile the asm file to an executor file
 * 
 * @param fname 
 */
void makeExecutable(char *fname);


#endif