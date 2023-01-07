#include "utils.h"




void DEBUG(char *format, ... ){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

}

void raiseError(int lineno, char *format, ...){

    DEBUG("\033[0;31m");
    DEBUG("[ERROR] ");
    DEBUG("\033[0m");
    if(lineno >= 0){
        DEBUG("near line ", lineno);
        DEBUG("\033[0;35m");
        DEBUG("%d ", lineno);
        DEBUG("\033[0m"); 
        DEBUG(">>> ");
    }
    
    raise(SIGUSR1);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void raiseWarning(int lineno, char *format, ...){
    va_list args;
    DEBUG("\033[0;35m");
    DEBUG("[WARNING] ");
    DEBUG("\033[0m"); 
    if(lineno >= 0){
        DEBUG("near line ", lineno);
        DEBUG("\033[0;35m");
        DEBUG("%d ", lineno);
        DEBUG("\033[0m"); 
        DEBUG(">>> ");
    }
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}