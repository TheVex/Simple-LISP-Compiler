#include "semantic_analyzer.h"
#include <iostream>
#include <sstream>

// ============================================================================
// Public Methods
// ============================================================================

Node *SemanticAnalyzer::analyze(Node *root) {
  if (!root) {
    return nullptr;
  }

  errors_.clear();
  warnings_.clear();
  context_ = AnalysisContext();

  // Initialize built-in functions
  context_.add_function("plus", FunctionSignature("plus", 2, {"a", "b"}));
  context_.add_function("minus", FunctionSignature("minus", 2, {"a", "b"}));
  context_.add_function("times", FunctionSignature("times", 2, {"a", "b"}));
  context_.add_function("divide", FunctionSignature("divide", 2, {"a", "b"}));
  context_.add_function("equal", FunctionSignature("equal", 2, {"a", "b"}));
  context_.add_function("nonequal",
                        FunctionSignature("nonequal", 2, {"a", "b"}));
  context_.add_function("less", FunctionSignature("less", 2, {"a", "b"}));
  context_.add_function("lesseq", FunctionSignature("lesseq", 2, {"a", "b"}));
  context_.add_function("greater", FunctionSignature("greater", 2, {"a", "b"}));
  context_.add_function("greatereq",
                        FunctionSignature("greatereq", 2, {"a", "b"}));
  context_.add_function("and", FunctionSignature("and", 2, {"a", "b"}));
  context_.add_function("or", FunctionSignature("or", 2, {"a", "b"}));
  context_.add_function("xor", FunctionSignature("xor", 2, {"a", "b"}));
  context_.add_function("not", FunctionSignature("not", 1, {"a"}));
  context_.add_function("isint", FunctionSignature("isint", 1, {"a"}));
  context_.add_function("isreal", FunctionSignature("isreal", 1, {"a"}));
  context_.add_function("isbool", FunctionSignature("isbool", 1, {"a"}));
  context_.add_function("isnull", FunctionSignature("isnull", 1, {"a"}));
  context_.add_function("isatom", FunctionSignature("isatom", 1, {"a"}));
  context_.add_function("islist", FunctionSignature("islist", 1, {"a"}));
  context_.add_function("head", FunctionSignature("head", 1, {"list"}));
  context_.add_function("tail", FunctionSignature("tail", 1, {"list"}));
  context_.add_function("cons", FunctionSignature("cons", 2, {"elem", "list"}));
  context_.add_function("eval", FunctionSignature("eval", 1, {"expr"}));
  context_.add_function("quote", FunctionSignature("quote", 1, {"expr"}));

  // First pass: collect function definitions and check semantics
  analyze_node(root);

  // Second pass: optimize if enabled
  if (enable_optimizations_ && !has_errors()) {
    Node *optimized = optimize_node(root);
    return optimized;
  }

  return root;
}

void SemanticAnalyzer::print_errors() const {
  for (const auto &error : errors_) {
    std::cerr << "Semantic Error: " << error.message;
    if (!error.context.empty()) {
      std::cerr << " (context: " << error.context << ")";
    }
    std::cerr << "\n";
  }
}

void SemanticAnalyzer::print_warnings() const {
  for (const auto &warning : warnings_) {
    std::cerr << "Warning: " << warning.message;
    if (!warning.context.empty()) {
      std::cerr << " (context: " << warning.context << ")";
    }
    std::cerr << "\n";
  }
}

// ============================================================================
// Semantic Analysis Methods
// ============================================================================

void SemanticAnalyzer::analyze_node(Node *node) {
  if (!node) {
    return;
  }

  switch (node->type) {
  case NodeType::NT_INT:
  case NodeType::NT_REAL:
  case NodeType::NT_BOOL:
  case NodeType::NT_NULL:
  case NodeType::NT_STRING:
    // Literals are always valid
    break;

  case NodeType::NT_IDENT:
    // Will be checked in context (function call or variable reference)
    break;

  case NodeType::NT_QUOTE:
    // Quoted expressions are not evaluated
    for (auto child : node->children) {
      analyze_node(child);
    }
    break;

  case NodeType::NT_LIST:
    analyze_list(node);
    break;
  }
}

void SemanticAnalyzer::analyze_list(Node *node) {
  if (node->children.empty()) {
    return;
  }

  Node *first = node->children[0];
  if (first->type != NodeType::NT_IDENT) {
    // Not a function call, analyze all children
    for (auto child : node->children) {
      analyze_node(child);
    }
    return;
  }

  const std::string &name = first->str;

  // Check special forms
  if (name == "setq") {
    check_setq(node);
  } else if (name == "func") {
    check_func(node);
  } else if (name == "lambda") {
    check_lambda(node);
  } else if (name == "prog") {
    check_prog(node);
  } else if (name == "cond") {
    check_cond(node);
  } else if (name == "while") {
    check_while(node);
  } else if (name == "return") {
    check_return(node);
  } else if (name == "break") {
    check_break(node);
  } else {
    // Regular function call
    check_function_call(node);
  }
}

void SemanticAnalyzer::check_setq(Node *node) {
  // (setq variable value)
  if (node->children.size() != 3) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "setq requires exactly 2 arguments (variable and value)", "setq");
    return;
  }

  Node *var = node->children[1];
  if (var->type != NodeType::NT_IDENT) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "setq first argument must be a variable name", "setq");
    return;
  }

  // Add variable to context
  context_.add_variable(var->str);

  // Analyze the value expression
  analyze_node(node->children[2]);
}

void SemanticAnalyzer::check_func(Node *node) {
  // (func name (param1 param2 ...) body)
  if (node->children.size() < 4) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "func requires name, parameters, and body", "func");
    return;
  }

  Node *name_node = node->children[1];
  Node *params_node = node->children[2];

  if (name_node->type != NodeType::NT_IDENT) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "func name must be an identifier", "func");
    return;
  }

  if (params_node->type != NodeType::NT_LIST) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "func parameters must be a list", "func");
    return;
  }

  std::string func_name = name_node->str;
  std::vector<std::string> params;
  std::set<std::string> param_set;

  // Extract parameter names
  for (auto param : params_node->children) {
    if (param->type != NodeType::NT_IDENT) {
      add_error(SemanticErrorType::TYPE_MISMATCH,
                "function parameters must be identifiers", func_name);
      continue;
    }
    if (param_set.find(param->str) != param_set.end()) {
      add_error(SemanticErrorType::DUPLICATE_PARAMETER,
                "duplicate parameter name: " + param->str, func_name);
    }
    params.push_back(param->str);
    param_set.insert(param->str);
  }

  // Check if function already defined
  if (context_.is_function_defined(func_name)) {
    add_error(SemanticErrorType::DUPLICATE_FUNCTION,
              "function already defined: " + func_name, func_name);
  }

  // Store function body (just the body expression, not the whole list)
  Node *body = node->children[3];
  FunctionSignature sig(func_name, params.size(), params, body);
  context_.add_function(func_name, sig);

  // Analyze function body in new scope
  context_.enter_scope();
  bool old_in_function = context_.in_function;
  context_.in_function = true;
  context_.function_depth++;

  // Add parameters to scope
  for (const auto &param : params) {
    context_.add_variable(param);
  }

  // Analyze body
  for (size_t i = 3; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.function_depth--;
  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_lambda(Node *node) {
  // (lambda (param1 param2 ...) body)
  if (node->children.size() < 3) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "lambda requires parameters and body", "lambda");
    return;
  }

  Node *params_node = node->children[1];
  if (params_node->type != NodeType::NT_LIST) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "lambda parameters must be a list", "lambda");
    return;
  }

  std::vector<std::string> params;
  std::set<std::string> param_set;

  for (auto param : params_node->children) {
    if (param->type != NodeType::NT_IDENT) {
      add_error(SemanticErrorType::TYPE_MISMATCH,
                "lambda parameters must be identifiers", "lambda");
      continue;
    }
    if (param_set.find(param->str) != param_set.end()) {
      add_error(SemanticErrorType::DUPLICATE_PARAMETER,
                "duplicate parameter name: " + param->str, "lambda");
    }
    params.push_back(param->str);
    param_set.insert(param->str);
  }

  // Analyze lambda body in new scope
  context_.enter_scope();
  bool old_in_function = context_.in_function;
  context_.in_function = true;
  context_.function_depth++;

  for (const auto &param : params) {
    context_.add_variable(param);
  }

  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.function_depth--;
  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_prog(Node *node) {
  // (prog ((var1 val1) (var2 val2) ...) body...)
  if (node->children.size() < 2) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "prog requires variable bindings and body", "prog");
    return;
  }

  Node *bindings = node->children[1];
  if (bindings->type != NodeType::NT_LIST) {
    add_error(SemanticErrorType::TYPE_MISMATCH, "prog bindings must be a list",
              "prog");
    return;
  }

  // Enter new scope for prog
  context_.enter_scope();
  bool old_in_function = context_.in_function;
  context_.in_function = true;
  context_.function_depth++;

  // Process bindings
  for (auto binding : bindings->children) {
    if (binding->type != NodeType::NT_LIST || binding->children.size() != 2) {
      add_error(SemanticErrorType::TYPE_MISMATCH,
                "prog binding must be (variable value)", "prog");
      continue;
    }
    if (binding->children[0]->type != NodeType::NT_IDENT) {
      add_error(SemanticErrorType::TYPE_MISMATCH,
                "prog binding variable must be an identifier", "prog");
      continue;
    }
    context_.add_variable(binding->children[0]->str);
    analyze_node(binding->children[1]);
  }

  // Analyze body
  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.function_depth--;
  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_cond(Node *node) {
  // (cond condition then-branch else-branch)
  if (node->children.size() < 3 || node->children.size() > 4) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "cond requires condition and at least one branch (then, "
              "optionally else)",
              "cond");
    return;
  }

  // Analyze all branches
  for (size_t i = 1; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }
}

void SemanticAnalyzer::check_while(Node *node) {
  // (while condition body...)
  if (node->children.size() < 3) {
    add_error(SemanticErrorType::TYPE_MISMATCH,
              "while requires condition and body", "while");
    return;
  }

  // Analyze condition
  analyze_node(node->children[1]);

  // Analyze body with in_loop flag set
  bool old_in_loop = context_.in_loop;
  context_.in_loop = true;

  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.in_loop = old_in_loop;
}

void SemanticAnalyzer::check_return(Node *node) {
  // Check: return should only be inside a function
  if (!context_.in_function) {
    add_error(SemanticErrorType::RETURN_OUTSIDE_FUNCTION,
              "return statement outside of function/lambda/prog", "return");
  }

  // Analyze return value if present
  if (node->children.size() > 1) {
    analyze_node(node->children[1]);
  }
}

void SemanticAnalyzer::check_break(Node *node) {
  // Check: break should only be inside a loop
  if (!context_.in_loop) {
    add_error(SemanticErrorType::BREAK_OUTSIDE_LOOP,
              "break statement outside of loop", "break");
  }
}

void SemanticAnalyzer::check_function_call(Node *node) {
  if (node->children.empty()) {
    return;
  }

  Node *func_node = node->children[0];
  if (func_node->type != NodeType::NT_IDENT) {
    // Not a simple function call, analyze all children
    for (auto child : node->children) {
      analyze_node(child);
    }
    return;
  }

  std::string func_name = func_node->str;

  // Check if function is defined
  if (!context_.is_function_defined(func_name)) {
    // Could be a variable holding a lambda
    if (!context_.is_variable_defined(func_name)) {
      add_warning(SemanticErrorType::UNDEFINED_FUNCTION,
                  "function or variable not defined: " + func_name, func_name);
    }
  } else {
    // Check arity
    FunctionSignature *sig = context_.get_function(func_name);
    if (sig) {
      int expected = sig->arity;
      int actual = node->children.size() - 1; // exclude function name
      if (expected != actual && expected >= 0) {
        std::ostringstream oss;
        oss << "function " << func_name << " expects " << expected
            << " arguments but got " << actual;
        add_error(SemanticErrorType::FUNCTION_ARITY_MISMATCH, oss.str(),
                  func_name);
      }
    }
  }

  // Analyze arguments
  for (size_t i = 1; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }
}

// ============================================================================
// Optimization Methods
// ============================================================================

Node *SemanticAnalyzer::optimize_node(Node *node) {
  if (!node) {
    return nullptr;
  }

  // First, recursively optimize children
  for (size_t i = 0; i < node->children.size(); i++) {
    Node *optimized_child = optimize_node(node->children[i]);
    if (optimized_child != node->children[i]) {
      delete node->children[i];
      node->children[i] = optimized_child;
    }
  }

  // Then apply optimizations to this node
  Node *result = node;

  // Constant folding
  if (is_constant_expression(result)) {
    Node *folded = constant_fold(result);
    if (folded && folded != result) {
      result = folded;
    }
  }

  // Remove unreachable code
  if (result->type == NodeType::NT_LIST) {
    result = remove_unreachable_code(result);
  }

  // Function inlining
  if (enable_function_inlining_ && result->type == NodeType::NT_LIST &&
      !result->children.empty()) {
    Node *first = result->children[0];
    if (first->type == NodeType::NT_IDENT) {
      FunctionSignature *sig = context_.get_function(first->str);
      if (sig && can_inline_function(sig)) {
        Node *inlined = inline_function_call(result, first->str);
        if (inlined && inlined != result) {
          result = inlined;
        }
      }
    }
  }

  return result;
}

Node *SemanticAnalyzer::constant_fold(Node *node) {
  if (node->type != NodeType::NT_LIST || node->children.empty()) {
    return node;
  }

  Node *first = node->children[0];
  if (first->type != NodeType::NT_IDENT) {
    return node;
  }

  const std::string &op = first->str;

  // Binary arithmetic operations
  if ((op == "plus" || op == "minus" || op == "times" || op == "divide") &&
      node->children.size() == 3) {
    Node *left = node->children[1];
    Node *right = node->children[2];
    Node *result = nullptr;

    if (evaluate_arithmetic(op, left, right, result)) {
      return result;
    }
  }

  // Logical operations
  if (op == "not" && node->children.size() == 2) {
    Node *arg = node->children[1];
    if (arg->type == NodeType::NT_BOOL) {
      return make_bool_node(!arg->bval);
    }
  }

  if ((op == "and" || op == "or" || op == "xor") &&
      node->children.size() == 3) {
    Node *left = node->children[1];
    Node *right = node->children[2];

    if (left->type == NodeType::NT_BOOL && right->type == NodeType::NT_BOOL) {
      bool result_val;
      if (op == "and") {
        result_val = left->bval && right->bval;
      } else if (op == "or") {
        result_val = left->bval || right->bval;
      } else { // xor
        result_val = left->bval != right->bval;
      }
      return make_bool_node(result_val);
    }
  }

  // Comparison operations
  if ((op == "equal" || op == "nonequal" || op == "less" || op == "lesseq" ||
       op == "greater" || op == "greatereq") &&
      node->children.size() == 3) {
    Node *left = node->children[1];
    Node *right = node->children[2];

    if (left->type == NodeType::NT_INT && right->type == NodeType::NT_INT) {
      bool result_val = false;
      if (op == "equal")
        result_val = left->ival == right->ival;
      else if (op == "nonequal")
        result_val = left->ival != right->ival;
      else if (op == "less")
        result_val = left->ival < right->ival;
      else if (op == "lesseq")
        result_val = left->ival <= right->ival;
      else if (op == "greater")
        result_val = left->ival > right->ival;
      else if (op == "greatereq")
        result_val = left->ival >= right->ival;
      return make_bool_node(result_val);
    }

    if (left->type == NodeType::NT_REAL && right->type == NodeType::NT_REAL) {
      bool result_val = false;
      if (op == "equal")
        result_val = left->dval == right->dval;
      else if (op == "nonequal")
        result_val = left->dval != right->dval;
      else if (op == "less")
        result_val = left->dval < right->dval;
      else if (op == "lesseq")
        result_val = left->dval <= right->dval;
      else if (op == "greater")
        result_val = left->dval > right->dval;
      else if (op == "greatereq")
        result_val = left->dval >= right->dval;
      return make_bool_node(result_val);
    }
  }

  // Type checking predicates
  if (node->children.size() == 2) {
    Node *arg = node->children[1];
    if (op == "isint") {
      return make_bool_node(arg->type == NodeType::NT_INT);
    } else if (op == "isreal") {
      return make_bool_node(arg->type == NodeType::NT_REAL);
    } else if (op == "isbool") {
      return make_bool_node(arg->type == NodeType::NT_BOOL);
    } else if (op == "isnull") {
      return make_bool_node(arg->type == NodeType::NT_NULL);
    } else if (op == "isatom") {
      return make_bool_node(
          arg->type == NodeType::NT_IDENT || arg->type == NodeType::NT_INT ||
          arg->type == NodeType::NT_REAL || arg->type == NodeType::NT_BOOL ||
          arg->type == NodeType::NT_STRING);
    } else if (op == "islist") {
      return make_bool_node(arg->type == NodeType::NT_LIST);
    }
  }

  return node;
}

Node *SemanticAnalyzer::remove_unreachable_code(Node *node) {
  if (node->type != NodeType::NT_LIST || node->children.empty()) {
    return node;
  }

  // Check for return statement and remove everything after it
  for (size_t i = 0; i < node->children.size(); i++) {
    Node *child = node->children[i];
    if (child->type == NodeType::NT_LIST && !child->children.empty()) {
      Node *first = child->children[0];
      if (first->type == NodeType::NT_IDENT && first->str == "return") {
        // Remove all statements after this return
        if (i + 1 < node->children.size()) {
          // Delete unreachable nodes
          for (size_t j = i + 1; j < node->children.size(); j++) {
            delete node->children[j];
          }
          // Resize to remove unreachable code
          node->children.resize(i + 1);
          break;
        }
      }
    }
  }

  return node;
}

Node *SemanticAnalyzer::inline_function_call(Node *node,
                                             const std::string &func_name) {
  FunctionSignature *sig = context_.get_function(func_name);
  if (!sig || !sig->body) {
    return node;
  }

  // Check if we have the right number of arguments
  if (node->children.size() - 1 != sig->params.size()) {
    return node;
  }

  // Extract arguments
  std::vector<Node *> args;
  for (size_t i = 1; i < node->children.size(); i++) {
    args.push_back(node->children[i]);
  }

  // Clone and substitute
  Node *inlined_body = substitute_parameters(sig->body, sig->params, args);
  return inlined_body;
}

bool SemanticAnalyzer::can_inline_function(const FunctionSignature *sig) const {
  if (!sig || !sig->body) {
    return false;
  }

  // Check if function is simple enough to inline
  int stmt_count = count_statements(sig->body);
  return stmt_count <= inline_threshold_;
}

int SemanticAnalyzer::count_statements(Node *node) const {
  if (!node) {
    return 0;
  }

  if (node->type == NodeType::NT_LIST) {
    int count = 1; // The list itself is a statement
    for (auto child : node->children) {
      count += count_statements(child);
    }
    return count;
  }

  return 1;
}

bool SemanticAnalyzer::is_constant_expression(Node *node) const {
  if (!node) {
    return false;
  }

  // Literals are constant
  if (node->type == NodeType::NT_INT || node->type == NodeType::NT_REAL ||
      node->type == NodeType::NT_BOOL || node->type == NodeType::NT_NULL ||
      node->type == NodeType::NT_STRING) {
    return true;
  }

  // Lists with constant expressions
  if (node->type == NodeType::NT_LIST && !node->children.empty()) {
    Node *first = node->children[0];
    if (first->type == NodeType::NT_IDENT) {
      // Check if it's a pure operation with constant arguments
      if (is_pure_function(first->str)) {
        for (size_t i = 1; i < node->children.size(); i++) {
          if (!is_constant_expression(node->children[i])) {
            return false;
          }
        }
        return true;
      }
    }
  }

  return false;
}

bool SemanticAnalyzer::is_pure_function(const std::string &name) const {
  // Pure functions that can be evaluated at compile time
  return name == "plus" || name == "minus" || name == "times" ||
         name == "divide" || name == "equal" || name == "nonequal" ||
         name == "less" || name == "lesseq" || name == "greater" ||
         name == "greatereq" || name == "and" || name == "or" ||
         name == "xor" || name == "not" || name == "isint" ||
         name == "isreal" || name == "isbool" || name == "isnull" ||
         name == "isatom" || name == "islist";
}

bool SemanticAnalyzer::evaluate_arithmetic(const std::string &op, Node *left,
                                           Node *right, Node *&result) {
  // Integer arithmetic
  if (left->type == NodeType::NT_INT && right->type == NodeType::NT_INT) {
    long long res_val = 0;
    if (op == "plus") {
      res_val = left->ival + right->ival;
    } else if (op == "minus") {
      res_val = left->ival - right->ival;
    } else if (op == "times") {
      res_val = left->ival * right->ival;
    } else if (op == "divide") {
      if (right->ival == 0) {
        return false; // Division by zero
      }
      res_val = left->ival / right->ival;
    } else {
      return false;
    }
    result = make_int_node(res_val);
    return true;
  }

  // Real arithmetic
  if ((left->type == NodeType::NT_REAL || left->type == NodeType::NT_INT) &&
      (right->type == NodeType::NT_REAL || right->type == NodeType::NT_INT)) {
    double left_val =
        (left->type == NodeType::NT_REAL) ? left->dval : (double)left->ival;
    double right_val =
        (right->type == NodeType::NT_REAL) ? right->dval : (double)right->ival;
    double res_val = 0.0;

    if (op == "plus") {
      res_val = left_val + right_val;
    } else if (op == "minus") {
      res_val = left_val - right_val;
    } else if (op == "times") {
      res_val = left_val * right_val;
    } else if (op == "divide") {
      if (right_val == 0.0) {
        return false; // Division by zero
      }
      res_val = left_val / right_val;
    } else {
      return false;
    }
    result = make_real_node(res_val);
    return true;
  }

  return false;
}

// ============================================================================
// Utility Methods
// ============================================================================

void SemanticAnalyzer::add_error(SemanticErrorType type,
                                 const std::string &message,
                                 const std::string &context) {
  errors_.emplace_back(type, message, context);
}

void SemanticAnalyzer::add_warning(SemanticErrorType type,
                                   const std::string &message,
                                   const std::string &context) {
  warnings_.emplace_back(type, message, context);
}

std::string SemanticAnalyzer::node_to_string(Node *node) const {
  if (!node) {
    return "null";
  }

  std::ostringstream oss;
  switch (node->type) {
  case NodeType::NT_INT:
    oss << node->ival;
    break;
  case NodeType::NT_REAL:
    oss << node->dval;
    break;
  case NodeType::NT_BOOL:
    oss << (node->bval ? "true" : "false");
    break;
  case NodeType::NT_NULL:
    oss << "null";
    break;
  case NodeType::NT_IDENT:
    oss << node->str;
    break;
  case NodeType::NT_STRING:
    oss << "\"" << node->str << "\"";
    break;
  case NodeType::NT_LIST:
    oss << "(list)";
    break;
  case NodeType::NT_QUOTE:
    oss << "(quote)";
    break;
  }
  return oss.str();
}

Node *SemanticAnalyzer::clone_node(Node *node) const {
  if (!node) {
    return nullptr;
  }

  Node *cloned = nullptr;

  switch (node->type) {
  case NodeType::NT_INT:
    cloned = make_int_node(node->ival);
    break;
  case NodeType::NT_REAL:
    cloned = make_real_node(node->dval);
    break;
  case NodeType::NT_BOOL:
    cloned = make_bool_node(node->bval);
    break;
  case NodeType::NT_NULL:
    cloned = make_null_node();
    break;
  case NodeType::NT_IDENT:
    cloned = make_ident_node(node->str);
    break;
  case NodeType::NT_STRING:
    cloned = make_string_node(node->str);
    break;
  case NodeType::NT_LIST: {
    std::vector<Node *> cloned_children;
    for (auto child : node->children) {
      cloned_children.push_back(clone_node(child));
    }
    cloned = make_list_node(cloned_children);
    break;
  }
  case NodeType::NT_QUOTE: {
    if (!node->children.empty()) {
      cloned = make_quote_node(clone_node(node->children[0]));
    } else {
      cloned = new Node(NodeType::NT_QUOTE);
    }
    break;
  }
  }

  return cloned;
}

Node *
SemanticAnalyzer::substitute_parameters(Node *body,
                                        const std::vector<std::string> &params,
                                        const std::vector<Node *> &args) {
  if (!body) {
    return nullptr;
  }

  // Create a mapping from parameter names to argument nodes
  std::map<std::string, Node *> substitution_map;
  for (size_t i = 0; i < params.size() && i < args.size(); i++) {
    substitution_map[params[i]] = args[i];
  }

  // Clone body and substitute parameters
  Node *cloned = clone_node(body);

  // Recursive substitution helper
  std::function<void(Node *)> substitute = [&](Node *node) {
    if (!node) {
      return;
    }

    if (node->type == NodeType::NT_IDENT) {
      auto it = substitution_map.find(node->str);
      if (it != substitution_map.end()) {
        // Replace this identifier with the argument
        Node *replacement = clone_node(it->second);
        // Copy replacement data into node
        node->type = replacement->type;
        node->str = replacement->str;
        node->ival = replacement->ival;
        node->dval = replacement->dval;
        node->bval = replacement->bval;
        node->children = replacement->children;
        replacement->children.clear(); // Prevent double deletion
        delete replacement;
      }
    }

    for (auto child : node->children) {
      substitute(child);
    }
  };

  substitute(cloned);
  return cloned;
}
