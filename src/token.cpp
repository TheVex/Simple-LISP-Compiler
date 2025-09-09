#include "include/token.h"

Token::Token(std::string value, TokenType type, int position) {
  this->value = value;
  this->type = type;
  this->position = position;
}

TokenType Str2Token(std::string str) {
  if (std::regex_match(str, reg_bracket_l)) {
    return TokenType::TOKEN_BRACKET_L;
  } else if (std::regex_match(str, reg_bracket_r)) {
    return TokenType::TOKEN_BRACKET_R;
  } else if (std::regex_match(str, reg_apostrophe)) {
    return TokenType::TOKEN_APOSTROPHE;
  } else if (std::regex_match(str, reg_whitespace)) {
    return TokenType::TOKEN_WHITESPACE;
  }

  else if (std::regex_match(str, reg_real)) {
    return TokenType::TOKEN_REAL;
  } else if (std::regex_match(str, reg_bool)) {
    return TokenType::TOKEN_BOOL;
  } else if (std::regex_match(str, reg_null)) {
    return TokenType::TOKEN_NULL;
  } else if (std::regex_match(str, reg_int)) {
    return TokenType::TOKEN_INT;
  } else if (std::regex_match(str, reg_str)) {
    return TokenType::TOKEN_STR;
  }

  else if (std::regex_match(str, reg_quote)) {
    return TokenType::TOKEN_QUOTE;
  } else if (std::regex_match(str, reg_setq)) {
    return TokenType::TOKEN_SETQ;
  } else if (std::regex_match(str, reg_func)) {
    return TokenType::TOKEN_FUNC;
  } else if (std::regex_match(str, reg_lambda)) {
    return TokenType::TOKEN_LAMBDA;
  } else if (std::regex_match(str, reg_prog)) {
    return TokenType::TOKEN_PROG;
  } else if (std::regex_match(str, reg_cond)) {
    return TokenType::TOKEN_COND;
  } else if (std::regex_match(str, reg_while)) {
    return TokenType::TOKEN_WHILE;
  } else if (std::regex_match(str, reg_return)) {
    return TokenType::TOKEN_RETURN;
  } else if (std::regex_match(str, reg_break)) {
    return TokenType::TOKEN_BREAK;
  } else if (std::regex_match(str, reg_plus)) {
    return TokenType::TOKEN_PLUS;
  } else if (std::regex_match(str, reg_minus)) {
    return TokenType::TOKEN_MINUS;
  } else if (std::regex_match(str, reg_var)) {
    return TokenType::TOKEN_VAR;
  }

  return TokenType::UNDEFINED;
}

std::string Token2TokenName(TokenType type) {
  switch (type) {
  case TOKEN_BRACKET_L:
    return "BRACKET_L";
  case TOKEN_BRACKET_R:
    return "BRACKET_R";
  case TOKEN_APOSTROPHE:
    return "APOSTROPHE";
  case TOKEN_WHITESPACE:
    return "WHITESPACE";
  case TOKEN_EOF:
    return "EOF";
  case TOKEN_EOL:
    return "EOL";
  case TOKEN_REAL:
    return "REAL";
  case TOKEN_BOOL:
    return "BOOL";
  case TOKEN_NULL:
    return "NULL";
  case TOKEN_INT:
    return "INT";
  case TOKEN_STR:
    return "STRING";
  case TOKEN_VAR:
    return "VARIABLE";
  case TOKEN_QUOTE:
    return "QUOTE";
  case TOKEN_SETQ:
    return "SETQ";
  case TOKEN_FUNC:
    return "FUNC";
  case TOKEN_LAMBDA:
    return "LAMBDA";
  case TOKEN_PROG:
    return "PROG";
  case TOKEN_COND:
    return "COND";
  case TOKEN_WHILE:
    return "WHILE";
  case TOKEN_RETURN:
    return "RETURN";
  case TOKEN_BREAK:
    return "BREAK";
  case TOKEN_PLUS:
    return "PLUS";
  case TOKEN_MINUS:
    return "MINUS";
  case UNDEFINED:
    return "UNDEFINED";
  default:
    return "UNKNOWN";
  }
}
