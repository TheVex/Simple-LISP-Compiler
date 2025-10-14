#include "token.h"
#include <regex>

// Regex definitions
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
static const std::regex reg_times("times");
static const std::regex reg_divide("divide");
static const std::regex reg_equal("equal");
static const std::regex reg_nonequal("nonequal");
static const std::regex reg_less("less");
static const std::regex reg_lesseq("lesseq");
static const std::regex reg_greater("greater");
static const std::regex reg_greatereq("greatereq");
static const std::regex reg_isint("isint");
static const std::regex reg_isreal("isreal");
static const std::regex reg_isbool("isbool");
static const std::regex reg_isnull("isnull");
static const std::regex reg_isatom("isatom");
static const std::regex reg_islist("islist");
static const std::regex reg_and("and");
static const std::regex reg_or("or");
static const std::regex reg_xor("xor");
static const std::regex reg_not("not");
static const std::regex reg_eval("eval");
static const std::regex reg_var("[a-zA-Z_][a-zA-Z0-9_]*");

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
  } else if (std::regex_match(str, reg_times)) {
    return TokenType::TOKEN_TIMES;
  } else if (std::regex_match(str, reg_divide)) {
    return TokenType::TOKEN_DIVIDE;
  } else if (std::regex_match(str, reg_equal)) {
    return TokenType::TOKEN_EQUAL;
  } else if (std::regex_match(str, reg_nonequal)) {
    return TokenType::TOKEN_NONEQUAL;
  } else if (std::regex_match(str, reg_less)) {
    return TokenType::TOKEN_LESS;
  } else if (std::regex_match(str, reg_lesseq)) {
    return TokenType::TOKEN_LESSEQ;
  } else if (std::regex_match(str, reg_greater)) {
    return TokenType::TOKEN_GREATER;
  } else if (std::regex_match(str, reg_greatereq)) {
    return TokenType::TOKEN_GREATEREQ;
  } else if (std::regex_match(str, reg_isint)) {
    return TokenType::TOKEN_ISINT;
  } else if (std::regex_match(str, reg_isreal)) {
    return TokenType::TOKEN_ISREAL;
  } else if (std::regex_match(str, reg_isbool)) {
    return TokenType::TOKEN_ISBOOL;
  } else if (std::regex_match(str, reg_isnull)) {
    return TokenType::TOKEN_ISNULL;
  } else if (std::regex_match(str, reg_isatom)) {
    return TokenType::TOKEN_ISATOM;
  } else if (std::regex_match(str, reg_islist)) {
    return TokenType::TOKEN_ISLIST;
  } else if (std::regex_match(str, reg_and)) {
    return TokenType::TOKEN_AND;
  } else if (std::regex_match(str, reg_or)) {
    return TokenType::TOKEN_OR;
  } else if (std::regex_match(str, reg_xor)) {
    return TokenType::TOKEN_XOR;
  } else if (std::regex_match(str, reg_not)) {
    return TokenType::TOKEN_NOT;
  } else if (std::regex_match(str, reg_eval)) {
    return TokenType::TOKEN_EVAL;
  } else if (std::regex_match(str, reg_bool)) {
    return TokenType::TOKEN_BOOL;
  } else if (std::regex_match(str, reg_null)) {
    return TokenType::TOKEN_NULL;
  }

  // Check data types
  else if (std::regex_match(str, reg_real)) {
    return TokenType::TOKEN_REAL;
  } else if (std::regex_match(str, reg_int)) {
    return TokenType::TOKEN_INT;
  } else if (std::regex_match(str, reg_str)) {
    return TokenType::TOKEN_STR;
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
  case TOKEN_TIMES:
    return "TIMES";
  case TOKEN_DIVIDE:
    return "DIVIDE";
  case TOKEN_EQUAL:
    return "EQUAL";
  case TOKEN_NONEQUAL:
    return "NONEQUAL";
  case TOKEN_LESS:
    return "LESS";
  case TOKEN_LESSEQ:
    return "LESSEQ";
  case TOKEN_GREATER:
    return "GREATER";
  case TOKEN_GREATEREQ:
    return "GREATEREQ";
  case TOKEN_ISINT:
    return "ISINT";
  case TOKEN_ISREAL:
    return "ISREAL";
  case TOKEN_ISBOOL:
    return "ISBOOL";
  case TOKEN_ISNULL:
    return "ISNULL";
  case TOKEN_ISATOM:
    return "ISATOM";
  case TOKEN_ISLIST:
    return "ISLIST";
  case TOKEN_AND:
    return "AND";
  case TOKEN_OR:
    return "OR";
  case TOKEN_XOR:
    return "XOR";
  case TOKEN_NOT:
    return "NOT";
  case TOKEN_EVAL:
    return "EVAL";
  case UNDEFINED:
    return "UNDEFINED";
  default:
    return "UNKNOWN";
  }
}
