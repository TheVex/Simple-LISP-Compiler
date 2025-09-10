#include "include/lexer.h"
#include "include/token.h"

Lexer::Lexer(std::string::iterator begin, std::string::iterator end) {
  this->begin = begin;
  this->end = end;
  this->current_it = begin;
  this->position = 0;
  this->current_char = (current_it != end) ? *current_it : EOF;
}

void Lexer::advance() {
  if (current_it != end) {
    ++current_it;
    ++position;
    current_char = (current_it != end) ? *current_it : EOF;
  }
}

char Lexer::peek(int offset) {
  auto peek_it = current_it;
  for (int i = 0; i < offset && peek_it != end; ++i) {
    ++peek_it;
  }

  return (peek_it != end) ? *peek_it : EOF;
}

void Lexer::skip_whitespace() {
  while (current_char == ' ' || current_char == '\t' || current_char == '\n' ||
         current_char == '\r') {
    advance();
  }
}

Token Lexer::parse_number() {
  std::string buffer;
  while (std::isdigit(current_char)) {
    buffer += current_char;
    advance();
  }
  if (current_char == '.') { // It's real (WoW)
    buffer += current_char;
    advance();
    while (std::isdigit(current_char)) {
      buffer += current_char;
      advance();
    }
  }
  return Token(buffer, Str2Token(buffer), position);
}

Token Lexer::parse_word() {
  std::string buffer;
  while (std::isalpha(current_char)) {
    buffer += current_char;
    advance();
  }
  return Token(buffer, Str2Token(buffer), position);
}

Token Lexer::parse_string() {
  std::string buffer;
  advance(); // skip the opening quote
  while (current_char != '"' && current_char != EOF) {
    buffer += current_char;
    advance();
  }
  advance(); // skip the closing quote
  return Token(buffer, TokenType::TOKEN_STR, position);
}

Token Lexer::next_token() {
  while (current_char != EOF) {
    skip_whitespace();

    if (current_char == EOF)
      break;

    if (std::isdigit(current_char)) {
      return parse_number(); // integer or real
    } else if (std::isalpha(current_char)) {
      return parse_word(); // either keyword (including null, bool) or variable
    } else if (current_char == '"') {
      return parse_string();
    } else {
      std::string buffer(1, current_char);
      advance();
      Token token(buffer, Str2Token(buffer), position);
      return token;
    }
  }

  return Token("", TokenType::TOKEN_EOF, position);
}
