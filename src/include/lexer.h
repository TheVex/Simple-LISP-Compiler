#ifndef SLISPCOMP_LEXER_H
#define SLISPCOMP_LEXER_H

#ifndef EOF
#define EOF '\0'
#endif

#include "token.h"
#include <iterator>
#include <string>

class Lexer {
public:
  Lexer(std::string::iterator begin, std::string::iterator end);

  void advance();
  void skip_whitespace();
  char peek(int offset);
  Token next_token();
  Token parse_number();
  Token parse_word();
  Token parse_string();

private:
  std::string::iterator begin;
  std::string::iterator end;
  std::string::iterator current_it;
  char current_char;
  int position;
};

#endif // SLISPCOMP_LEXER_H
