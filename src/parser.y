%{
#include <cstdio>
#include <cstdlib>
#include "parser_interface.h"
#include "include/ast.h"

// результирующий AST root
Node* g_root = nullptr;
%}

/* Ќе используем %union Ч будем строить Node* вручную через глобалы */
/* ќбъ€вл€ем токены */
%token INT REAL BOOL NULL_TOK IDENT STRING
%token SETQ FUNC LAMBDA PROG COND WHILE RETURN_TOK BREAK_TOK
%token PLUS MINUS TIMES DIVIDE EQUAL NONEQUAL LESS LESSEQ GREATER GREATEREQ
%token ISINT ISREAL ISBOOL ISNULL ISATOM ISLIST
%token AND OR XOR NOT
%token EVAL

%start program

%%

program:
    elements                { g_root = make_list_node($1); }
  ;

elements:
    /* пусто */             { $$ = std::vector<Node*>(); }
  | elements element       { $$ = $1; $$.push_back($2); }
  ;

/* Element : Atom | Literal | List */
element:
    atom                    { $$ = $1; }
  | literal                 { $$ = $1; }
  | list                    { $$ = $1; }
  ;

/* atom -> IDENT */
atom:
    IDENT                   { $$ = make_ident_node(g_yylval_str); }
  ;

/* literals */
literal:
    INT                     { $$ = make_int_node(g_yylval_int); }
  | REAL                    { $$ = make_real_node(g_yylval_num); }
  | BOOL                    { $$ = make_bool_node(g_yylval_bool); }
  | NULL_TOK                { $$ = make_null_node(); }
  | STRING                  { $$ = make_string_node(g_yylval_str); }
  ;

/* lists: '(' { element } ')' */
list:
    '(' list_elems ')'      { $$ = make_list_node($2); }
  | '\'' element            { $$ = make_quote_node($2); } /* 'elem is shorthand for (quote elem) */
  ;

list_elems:
    /* пусто */             { $$ = std::vector<Node*>(); }
  | list_elems element      { $$ = $1; $$.push_back($2); }
  ;

%%

/* ¬ажно: Bison вставит сгенерированный код. */
