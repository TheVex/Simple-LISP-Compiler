%{
#include <cstdio>
#include <cstdlib>
#include "parser_interface.h"
#include "ast.h"
#include <vector>

// Forward declarations
class Node;

// Global AST root
Node* g_root = nullptr;
%}

%locations

%union {
    void* ptr_val;
}

%type <ptr_val> program element atom list literal elements list_elems

%token INT REAL BOOL NULL_TOK IDENT STRING
%token QUOTE SETQ FUNC LAMBDA PROG COND WHILE RETURN_TOK BREAK_TOK
%token PLUS MINUS TIMES DIVIDE EQUAL NONEQUAL LESS LESSEQ GREATER GREATEREQ
%token ISINT ISREAL ISBOOL ISNULL ISATOM ISLIST
%token AND OR XOR NOT
%token EVAL

%start program

%%

program:
    elements                { g_root = make_list_node(*(std::vector<Node*>*)$1); delete (std::vector<Node*>*)$1; }
  ;

elements:
    /* empty */             { $$ = (void*)new std::vector<Node*>(); }
  | elements element        { $$ = $1; ((std::vector<Node*>*)$$)->push_back((Node*)$2); }
  ;

/* Element : Atom | Literal | List */
element:
    atom                    { $$ = $1; }
  | literal                 { $$ = $1; }
  | list                    { $$ = $1; }
  ;

/* atom -> IDENT or any keyword that can be used as identifier */
atom:
    IDENT                   { $$ = (void*)make_ident_node(g_yylval_str); }
  | SETQ                    { $$ = (void*)make_ident_node("setq"); }
  | FUNC                    { $$ = (void*)make_ident_node("func"); }
  | LAMBDA                  { $$ = (void*)make_ident_node("lambda"); }
  | PROG                    { $$ = (void*)make_ident_node("prog"); }
  | COND                    { $$ = (void*)make_ident_node("cond"); }
  | WHILE                   { $$ = (void*)make_ident_node("while"); }
  | RETURN_TOK              { $$ = (void*)make_ident_node("return"); }
  | BREAK_TOK               { $$ = (void*)make_ident_node("break"); }
  | PLUS                    { $$ = (void*)make_ident_node("plus"); }
  | MINUS                   { $$ = (void*)make_ident_node("minus"); }
  | TIMES                   { $$ = (void*)make_ident_node("times"); }
  | DIVIDE                  { $$ = (void*)make_ident_node("divide"); }
  | QUOTE                   { $$ = (void*)make_ident_node("quote"); }
  | EVAL                    { $$ = (void*)make_ident_node("eval"); }
  | EQUAL                   { $$ = (void*)make_ident_node("equal"); }
  | NONEQUAL                { $$ = (void*)make_ident_node("nonequal"); }
  | LESS                    { $$ = (void*)make_ident_node("less"); }
  | LESSEQ                  { $$ = (void*)make_ident_node("lesseq"); }
  | GREATER                 { $$ = (void*)make_ident_node("greater"); }
  | GREATEREQ               { $$ = (void*)make_ident_node("greatereq"); }
  | ISINT                   { $$ = (void*)make_ident_node("isint"); }
  | ISREAL                  { $$ = (void*)make_ident_node("isreal"); }
  | ISBOOL                  { $$ = (void*)make_ident_node("isbool"); }
  | ISNULL                  { $$ = (void*)make_ident_node("isnull"); }
  | ISATOM                  { $$ = (void*)make_ident_node("isatom"); }
  | ISLIST                  { $$ = (void*)make_ident_node("islist"); }
  | AND                     { $$ = (void*)make_ident_node("and"); }
  | OR                      { $$ = (void*)make_ident_node("or"); }
  | XOR                     { $$ = (void*)make_ident_node("xor"); }
  | NOT                     { $$ = (void*)make_ident_node("not"); }
  ;

/* literals */
literal:
    INT                     { $$ = (void*)make_int_node(g_yylval_int); }
  | REAL                    { $$ = (void*)make_real_node(g_yylval_num); }
  | BOOL                    { $$ = (void*)make_bool_node(g_yylval_bool); }
  | NULL_TOK                { $$ = (void*)make_null_node(); }
  | STRING                  { $$ = (void*)make_string_node(g_yylval_str); }
  ;

/* lists: '(' { element } ')' */
list:
    '(' list_elems ')'      { $$ = (void*)make_list_node(*(std::vector<Node*>*)$2); delete (std::vector<Node*>*)$2; }
  | '\'' element            { $$ = (void*)make_quote_node((Node*)$2); } /* 'elem is shorthand for (quote elem) */
  ;

list_elems:
    /* empty */             { $$ = (void*)new std::vector<Node*>(); }
  | list_elems element      { $$ = $1; ((std::vector<Node*>*)$$)->push_back((Node*)$2); }
  ;

%%

/* Note: Bison generates yyparse() function automatically */
