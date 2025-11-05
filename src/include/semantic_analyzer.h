#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <map>
#include <set>
#include <string>
#include <vector>

// Semantic error types
enum class SemanticErrorType {
  UNDEFINED_VARIABLE,
  UNDEFINED_FUNCTION,
  BREAK_OUTSIDE_LOOP,
  RETURN_OUTSIDE_FUNCTION,
  FUNCTION_ARITY_MISMATCH,
  TYPE_MISMATCH,
  DUPLICATE_PARAMETER,
  DUPLICATE_FUNCTION
};

// Semantic error information
struct SemanticError {
  SemanticErrorType type;
  std::string message;
  std::string context;

  SemanticError(SemanticErrorType t, const std::string &msg,
                const std::string &ctx = "")
      : type(t), message(msg), context(ctx) {}
};

// Function signature for type checking
struct FunctionSignature {
  std::string name;
  int arity;                       // number of parameters
  std::vector<std::string> params; // parameter names
  Node *body;                      // function body for inlining

  FunctionSignature() : arity(0), body(nullptr) {}
  FunctionSignature(const std::string &n, int a,
                    const std::vector<std::string> &p, Node *b = nullptr)
      : name(n), arity(a), params(p), body(b) {}
};

// Semantic analysis context
struct AnalysisContext {
  std::map<std::string, FunctionSignature> functions; // defined functions
  std::set<std::string> variables; // defined variables in current scope
  std::vector<std::set<std::string>> scopes; // scope stack
  bool in_loop;                              // are we inside a loop?
  bool in_function;                          // are we inside a function?
  int function_depth;                        // nesting level of functions

  AnalysisContext() : in_loop(false), in_function(false), function_depth(0) {}

  void enter_scope() { scopes.push_back(std::set<std::string>()); }

  void exit_scope() {
    if (!scopes.empty()) {
      scopes.pop_back();
    }
  }

  void add_variable(const std::string &name) {
    if (!scopes.empty()) {
      scopes.back().insert(name);
    }
    variables.insert(name);
  }

  bool is_variable_defined(const std::string &name) const {
    // Check current scopes
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      if (it->find(name) != it->end()) {
        return true;
      }
    }
    // Check global variables
    return variables.find(name) != variables.end();
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

// Main Semantic Analyzer class
class SemanticAnalyzer {
private:
  std::vector<SemanticError> errors_;
  std::vector<SemanticError> warnings_;
  AnalysisContext context_;
  bool enable_optimizations_;
  bool enable_function_inlining_;
  int inline_threshold_; // max statements to inline

  // Helper methods for semantic checks
  void analyze_node(Node *node);
  void analyze_list(Node *node);
  void check_setq(Node *node);
  void check_func(Node *node);
  void check_lambda(Node *node);
  void check_prog(Node *node);
  void check_cond(Node *node);
  void check_while(Node *node);
  void check_return(Node *node);
  void check_break(Node *node);
  void check_function_call(Node *node);

  // Helper methods for optimizations
  Node *optimize_node(Node *node);
  Node *constant_fold(Node *node);
  Node *remove_unreachable_code(Node *node);
  Node *inline_function_call(Node *node, const std::string &func_name);
  bool is_pure_function(const std::string &name) const;
  bool can_inline_function(const FunctionSignature *sig) const;
  int count_statements(Node *node) const;

  // Helper methods for node evaluation
  bool is_constant_expression(Node *node) const;
  Node *evaluate_constant_expression(Node *node);
  bool evaluate_arithmetic(const std::string &op, Node *left, Node *right,
                           Node *&result);

  // Utility methods
  void add_error(SemanticErrorType type, const std::string &message,
                 const std::string &context = "");
  void add_warning(SemanticErrorType type, const std::string &message,
                   const std::string &context = "");
  std::string node_to_string(Node *node) const;
  Node *clone_node(Node *node) const;
  Node *substitute_parameters(Node *body,
                              const std::vector<std::string> &params,
                              const std::vector<Node *> &args);

public:
  SemanticAnalyzer(bool enable_optimizations = true,
                   bool enable_function_inlining = false,
                   int inline_threshold = 3)
      : enable_optimizations_(enable_optimizations),
        enable_function_inlining_(enable_function_inlining),
        inline_threshold_(inline_threshold) {}

  // Main analysis entry point
  Node *analyze(Node *root);

  // Error and warning accessors
  const std::vector<SemanticError> &get_errors() const { return errors_; }
  const std::vector<SemanticError> &get_warnings() const { return warnings_; }
  bool has_errors() const { return !errors_.empty(); }

  // Print errors and warnings
  void print_errors() const;
  void print_warnings() const;
};

#endif // SEMANTIC_ANALYZER_H
