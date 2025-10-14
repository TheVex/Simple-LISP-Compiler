#pragma once
#include "parser.tab.hpp"
#include "lexer.h"

extern Lexer* lexer;

int yylex(yy::Parser::semantic_type* yylval, yy::Parser::location_type* yylloc);
