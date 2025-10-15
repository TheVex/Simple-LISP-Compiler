#ifndef PARSER_INTERFACE_H
#define PARSER_INTERFACE_H

#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <string>

extern Lexer *g_lexer;
extern Node *g_root;

extern std::string g_yylval_str;
extern long long g_yylval_int;
extern double g_yylval_num;
extern bool g_yylval_bool;

extern "C" {
int yylex();
void yyerror(const char *s);
}

#endif
