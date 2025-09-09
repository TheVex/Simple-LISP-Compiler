#ifndef SLISPCOMP_TOKEN_H
#define SLISPCOMP_TOKEN_H

#include <regex>
#include <string>

enum TokenType {
  // symbols:
  TOKEN_BRACKET_L,
  TOKEN_BRACKET_R,
  TOKEN_APOSTROPHE, // '
  TOKEN_WHITESPACE,
  TOKEN_EOF,
  TOKEN_EOL,

  // datatypes:
  TOKEN_REAL,
  TOKEN_BOOL,
  TOKEN_NULL,
  TOKEN_INT, // integer (many decimals)
  TOKEN_STR,
  TOKEN_VAR,
  // TOKEN_DECIMAL, // one symbol
  // TOKEN_DOT,

  // keywords:
  TOKEN_QUOTE,
  TOKEN_SETQ,
  TOKEN_FUNC,
  TOKEN_LAMBDA,
  TOKEN_PROG,
  TOKEN_COND,
  TOKEN_WHILE,
  TOKEN_RETURN,
  TOKEN_BREAK,
  TOKEN_PLUS,
  TOKEN_MINUS,

  UNDEFINED,
};

class Token {
public:
  std::string value;
  int position;

  TokenType type;

  Token(std::string value, TokenType type, int position);
};

TokenType Str2Token(std::string str);
std::string Token2TokenName(TokenType type);

static const std::regex reg_bracket_l("\\(");
static const std::regex reg_bracket_r("\\)");
static const std::regex reg_apostrophe("'");
static const std::regex reg_whitespace("( |\t)");
static const std::regex reg_eol("\\n");
static const std::regex reg_eof("$");

static const std::regex reg_real("-?\\d+.\\d+");
static const std::regex reg_bool("true|false");
static const std::regex reg_null("null");
static const std::regex reg_int("\\d+");
static const std::regex reg_str("\"[^\"]*\"");
static const std::regex reg_var("[^\"]+");

static const std::regex reg_quote("quote");
static const std::regex reg_setq("setq");
static const std::regex reg_func("func");
static const std::regex reg_lambda("lambda");
static const std::regex reg_prog("prog");
static const std::regex reg_cond("cond");
static const std::regex reg_while("while");
static const std::regex reg_return("return");
static const std::regex reg_break("break");
static const std::regex reg_plus("plus");
static const std::regex reg_minus("minus");

#endif // SLISPCOMP_TOKEN_H
