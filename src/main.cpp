#include "lexer.h"
#include "semantic_analyzer.h"

#include "parser.tab.h"
#include "parser_interface.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

int yyparse();
int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: slisp <source-file> [--no-optimize]\n";
    return 1;
  }

  bool enable_optimizations = true;
  if (argc == 3 && std::string(argv[2]) == "--no-optimize") {
    enable_optimizations = false;
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
    std::cout << "Parsing completed successfully\n";
    if (g_root) {
      std::cout << "\n=== Original AST ===\n";
      print_node(g_root);

      SemanticAnalyzer analyzer(enable_optimizations);
      Node *analyzed_root = analyzer.analyze(g_root);

      if (analyzer.has_errors()) {
        std::cout << "\n=== Semantic Errors ===\n";
        analyzer.print_errors();
      } else {
        std::cout << "No semantic errors found.\n";
      }

      analyzer.print_warnings();

      if (!analyzer.has_errors()) {
        std::cout << "\n=== Optimized AST ===\n";
        print_node(analyzed_root);
      }

      if (analyzed_root && analyzed_root != g_root) {
        delete analyzed_root;
      }
      delete g_root;
      g_root = nullptr;
    }
  } else {
    std::cerr << "Parsing failed\n";
  }

  g_lexer = nullptr;
  return result;
}
