#ifndef SLISPCOMP_TOKEN_H
#define SLISPCOMP_TOKEN_H

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
  TOKEN_TIMES,
  TOKEN_DIVIDE,
  TOKEN_EQUAL,
  TOKEN_NONEQUAL,
  TOKEN_LESS,
  TOKEN_LESSEQ,
  TOKEN_GREATER,
  TOKEN_GREATEREQ,
  TOKEN_ISINT,
  TOKEN_ISREAL,
  TOKEN_ISBOOL,
  TOKEN_ISNULL,
  TOKEN_ISATOM,
  TOKEN_ISLIST,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_XOR,
  TOKEN_NOT,
  TOKEN_EVAL,

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

#endif // SLISPCOMP_TOKEN_H
