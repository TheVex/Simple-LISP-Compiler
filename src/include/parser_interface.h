#ifndef PARSER_INTERFACE_H
#define PARSER_INTERFACE_H

#include "include/lexer.h"
#include "include/token.h"
#include "include/ast.h"

extern Lexer* g_lexer;

extern std::string g_yylval_str;
extern long long   g_yylval_int;
extern double      g_yylval_num;
extern bool        g_yylval_bool;

int yylex();

void yyerror(const char* s);

#endif
