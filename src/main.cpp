#include "include/lexer.h"
#include "include/token.h"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Wrong count of arguments\n";
    return 1;
  }
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Could not open the file '" << argv[1] << "'\n";
    return 2;
  }

  std::string fileContent((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
  file.close();

  Lexer lexer(fileContent.begin(), fileContent.end());

  Token token = lexer.next_token();
  int tokenCount = 0;

  std::cout << "Tokens in file '" << argv[1] << "':\n";
  std::cout << "----------------------------------------------\n";
  std::cout << "| # | Type | Position | Value |\n";
  std::cout << "----------------------------------------------\n";

  while (token.type != TOKEN_EOF) {
    tokenCount++;
    std::cout << "| " << tokenCount << " | " << Token2TokenName(token.type)
              << " | " << token.position << " | '" << token.value << "' |\n";
    token = lexer.next_token();
  }

  std::cout << "----------------------------------------------\n";
  std::cout << "Total tokens: " << tokenCount << "\n";

  return 0;
}
