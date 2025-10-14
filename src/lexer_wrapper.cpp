#include "ast.h"
#include "parser.tab.h"
#include "parser_interface.h"
#include <cstdlib>
#include <iostream>
#include <vector>

Lexer *g_lexer = nullptr;
std::string g_yylval_str;
long long g_yylval_int = 0;
double g_yylval_num = 0.0;
bool g_yylval_bool = false;

void yyerror(const char *s) { std::cerr << "Parse error: " << s << "\n"; }

int yylex() {
  if (!g_lexer)
    return 0;
  Token token = g_lexer->next_token();

  switch (token.type) {
  case TokenType::TOKEN_BRACKET_L:
    return '(';
  case TokenType::TOKEN_BRACKET_R:
    return ')';
  case TokenType::TOKEN_APOSTROPHE:
    return '\''; // quote short form
  case TokenType::TOKEN_STR:
    g_yylval_str = token.value;
    return STRING;
  case TokenType::TOKEN_INT:
    try {
      g_yylval_int = std::stoll(token.value);
    } catch (...) {
      g_yylval_int = 0;
    }
    return INT;
  case TokenType::TOKEN_REAL:
    try {
      g_yylval_num = std::stod(token.value);
    } catch (...) {
      g_yylval_num = 0.0;
    }
    return REAL;
  case TokenType::TOKEN_BOOL:
    g_yylval_bool =
        (token.value == "true" || token.value == "True" || token.value == "t");
    return BOOL;
  case TokenType::TOKEN_NULL:
    return NULL_TOK;
  case TokenType::TOKEN_VAR:
    g_yylval_str = token.value;
    return IDENT;

  case TokenType::TOKEN_QUOTE:
    return QUOTE;
  case TokenType::TOKEN_SETQ:
    return SETQ;
  case TokenType::TOKEN_FUNC:
    return FUNC;
  case TokenType::TOKEN_LAMBDA:
    return LAMBDA;
  case TokenType::TOKEN_PROG:
    return PROG;
  case TokenType::TOKEN_COND:
    return COND;
  case TokenType::TOKEN_WHILE:
    return WHILE;
  case TokenType::TOKEN_RETURN:
    return RETURN_TOK;
  case TokenType::TOKEN_BREAK:
    return BREAK_TOK;
  case TokenType::TOKEN_PLUS:
    return PLUS;
  case TokenType::TOKEN_MINUS:
    return MINUS;
  case TokenType::TOKEN_TIMES:
    return TIMES;
  case TokenType::TOKEN_DIVIDE:
    return DIVIDE;
  case TokenType::TOKEN_EQUAL:
    return EQUAL;
  case TokenType::TOKEN_NONEQUAL:
    return NONEQUAL;
  case TokenType::TOKEN_LESS:
    return LESS;
  case TokenType::TOKEN_LESSEQ:
    return LESSEQ;
  case TokenType::TOKEN_GREATER:
    return GREATER;
  case TokenType::TOKEN_GREATEREQ:
    return GREATEREQ;
  case TokenType::TOKEN_ISINT:
    return ISINT;
  case TokenType::TOKEN_ISREAL:
    return ISREAL;
  case TokenType::TOKEN_ISBOOL:
    return ISBOOL;
  case TokenType::TOKEN_ISNULL:
    return ISNULL;
  case TokenType::TOKEN_ISATOM:
    return ISATOM;
  case TokenType::TOKEN_ISLIST:
    return ISLIST;
  case TokenType::TOKEN_AND:
    return AND;
  case TokenType::TOKEN_OR:
    return OR;
  case TokenType::TOKEN_XOR:
    return XOR;
  case TokenType::TOKEN_NOT:
    return NOT;
  case TokenType::TOKEN_EVAL:
    return EVAL;
  default:
    if (token.type == TokenType::TOKEN_EOF)
      return 0;
    if (!token.value.empty())
      return token.value[0];
    return 0;
  }
}
