#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <signal.h>


#define GLOBAL_ID "global_vars"

/**
 * @brief print the format into the stderr
 * the call is similar to : fprintf(stderr, format, ...);
 * 
 * @param format 
 * @param ... 
 */
void DEBUG(char *format, ... );

/**
 * @brief send a warning to the stderr, print the line 'lineno' and the format as warning message
 * 
 * @param lineno line of the warning
 * @param format warning message
 * @param ... 
 */
void raiseWarning(int lineno, char *format,...);


/**
 * @brief send an error to the stderr, print the line 'lineno' and the format as error message
 * this error will raise the signal 'SIGSUR1'
 * 
 * @param lineno line of the error
 * @param format error message
 * @param ... 
 */
void raiseError(int lineno, char *format, ...);




#endif