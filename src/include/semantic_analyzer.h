#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <map>
#include <set>
#include <string>
#include <vector>

enum class LispType {
  TYPE_UNKNOWN,
  TYPE_INT,
  TYPE_REAL,
  TYPE_BOOL,
  TYPE_STRING,
  TYPE_LIST,
  TYPE_NULL,
  TYPE_ANY
};

struct FunctionSignature {
  std::string name;
  LispType return_type;
  std::vector<std::string> params;
  std::vector<LispType> param_types;
  Node *body;

  FunctionSignature() : return_type(LispType::TYPE_ANY), body(nullptr) {}
  FunctionSignature(const std::string &n, LispType ret,
                    const std::vector<std::string> &p,
                    const std::vector<LispType> &pt, Node *b = nullptr)
      : name(n), return_type(ret), params(p), param_types(pt), body(b) {}
};

struct AnalysisContext {
  std::map<std::string, FunctionSignature> functions;
  std::map<std::string, LispType> variables;
  std::vector<std::map<std::string, LispType>> scopes;
  bool in_loop;
  bool in_function;
  LispType current_function_return_type;

  AnalysisContext()
      : in_loop(false), in_function(false),
        current_function_return_type(LispType::TYPE_ANY) {}

  void enter_scope() { scopes.push_back(std::map<std::string, LispType>()); }

  void exit_scope() {
    if (!scopes.empty()) {
      scopes.pop_back();
    }
  }

  void add_variable(const std::string &name, LispType type) {
    if (!scopes.empty()) {
      scopes.back()[name] = type;
    }
    variables[name] = type;
  }

  LispType get_variable_type(const std::string &name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto var_it = it->find(name);
      if (var_it != it->end()) {
        return var_it->second;
      }
    }
    auto var_it = variables.find(name);
    if (var_it != variables.end()) {
      return var_it->second;
    }
    return LispType::TYPE_UNKNOWN;
  }

  bool is_variable_defined(const std::string &name) const {
    return get_variable_type(name) != LispType::TYPE_UNKNOWN;
  }

  void add_function(const std::string &name, const FunctionSignature &sig) {
    functions[name] = sig;
  }

  bool is_function_defined(const std::string &name) const {
    return functions.find(name) != functions.end();
  }

  FunctionSignature *get_function(const std::string &name) {
    auto it = functions.find(name);
    if (it != functions.end()) {
      return &(it->second);
    }
    return nullptr;
  }
};

class SemanticAnalyzer {
private:
  std::vector<std::string> errors_;
  std::vector<std::string> warnings_;
  AnalysisContext context_;
  bool enable_function_inlining_;

  void analyze_node(Node *node);
  void analyze_list(Node *node);
  void check_setq(Node *node);
  void check_func(Node *node);
  void check_lambda(Node *node);
  void check_prog(Node *node);
  void check_while(Node *node);
  void check_return(Node *node);
  void check_break(Node *node);
  void check_function_call(Node *node);

  LispType infer_type(Node *node);
  bool check_type_compatibility(LispType expected, LispType actual);
  std::string type_to_string(LispType type);

  Node *optimize_node(Node *node);
  Node *remove_unreachable_code(Node *node);
  Node *inline_function_call(Node *node, const std::string &func_name);
  bool should_inline_function(const FunctionSignature *sig);

  Node *clone_node(Node *node);
  Node *substitute_parameters(Node *body,
                              const std::vector<std::string> &params,
                              const std::vector<Node *> &args);
  void add_error(const std::string &message);
  void add_warning(const std::string &message);

public:
  SemanticAnalyzer(bool enable_inlining = true)
      : enable_function_inlining_(enable_inlining) {}

  Node *analyze(Node *root);
  bool has_errors() const { return !errors_.empty(); }
  void print_errors() const;
  void print_warnings() const;
};

#endif // SEMANTIC_ANALYZER_H
