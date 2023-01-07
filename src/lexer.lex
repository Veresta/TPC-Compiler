%{
#include "parser.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int lineno = 1;
int linecharno = 0;
int line_count = 1;
char* current_line = NULL;
%}
%x LONG_COMMENT SHORT_COMMENT
%option nounput
%option noinput
%option noyywrap
%%
"/*" {BEGIN LONG_COMMENT;}
"//"                {BEGIN SHORT_COMMENT;}
<LONG_COMMENT>"*/"        { BEGIN INITIAL; linecharno += yyleng; }
<LONG_COMMENT,SHORT_COMMENT>.        {linecharno += yyleng;}
<LONG_COMMENT>\n        {lineno++;linecharno=0;}
<SHORT_COMMENT>\n        {BEGIN INITIAL; lineno++;linecharno=0;}
"=="|"!="	   {strncpy(yylval.comp, yytext, 3); linecharno = linecharno + yyleng; return EQ;}
"<"|">"|"<="|">="	{strncpy(yylval.comp, yytext, 3); linecharno = linecharno + yyleng; return ORDER;}
"+"|"-" 	        {yylval.byte = yytext[0]; linecharno = linecharno + yyleng; return ADDSUB;}
"/"|"*"|"%" 	    {yylval.byte = yytext[0]; linecharno = linecharno + yyleng; return DIVSTAR;}
void        {linecharno = linecharno + yyleng; return VOID;} 
if 		    {linecharno = linecharno + yyleng; return IF;}
else        {linecharno = linecharno + yyleng; return ELSE;}
while       {linecharno = linecharno + yyleng; return WHILE;}
return      {linecharno = linecharno + yyleng; return RETURN;}
switch      {linecharno = linecharno + yyleng; return SWITCH;}
case        {linecharno = linecharno + yyleng; return CASE;}
default     {linecharno = linecharno + yyleng; return DEFAULT;}
break       {linecharno = linecharno + yyleng; return BREAK;}
[0-9]+  	{yylval.num = atoi(yytext); linecharno = linecharno + yyleng; return NUM;}
"||"        {linecharno = linecharno + yyleng; return OR;}
"&&"        {linecharno = linecharno + yyleng; return AND;}
'\\?.'	    {
    if(yytext[1] == '\\'){
        switch(yytext[2]){
            case 'n':
                yylval.byte = '\n';
                break;
            case 't':
                yylval.byte = '\t';
                break;
            case 'r':
                yylval.byte = '\r';
                break;
            case 'b':
                yylval.byte = '\b';
                break;
            case '0':
                yylval.byte = '\0';
                break;
        }
    } else {
        yylval.byte = yytext[1];
    }
    linecharno = linecharno + yyleng; 
    return CHARACTER;
}
" "|\t     {linecharno = linecharno + yyleng;}
int|char   {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return TYPE;}
[a-zA-Z_][a-zA-Z0-9_]*            {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return IDENT;}
^.*[\n] {current_line = (char*)malloc(sizeof(char)*1024); strcpy(current_line, yytext); REJECT;}
. 	{linecharno = linecharno + yyleng; return yytext[0];}
\n {line_count++; linecharno = 0; lineno++;}
%%