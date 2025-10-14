#include "lexer.h"

#include "parser.tab.h"
#include "parser_interface.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

int yyparse();
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: slisp <source-file>\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Could not open file '" << argv[1] << "'\n";
    return 2;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  std::unique_ptr<Lexer> lexer =
      std::make_unique<Lexer>(content.begin(), content.end());
  g_lexer = lexer.get();

  std::cout << "Parsing file: " << argv[1] << "\n";
  int result = yyparse();

  if (result == 0) {
    std::cout << "Parsing completed successfully.\n";
    if (g_root) {
      std::cout << "\nAST:\n";
      print_node(g_root);
      delete g_root;
      g_root = nullptr;
    }
  } else {
    std::cerr << "Parsing failed.\n";
  }

  g_lexer = nullptr;
  return result;
}
