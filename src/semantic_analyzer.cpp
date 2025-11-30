#include "semantic_analyzer.h"
#include <iostream>
#include <sstream>
#include <functional>

Node *SemanticAnalyzer::analyze(Node *root) {
  if (!root) {
    return nullptr;
  }

  errors_.clear();
  warnings_.clear();
  context_ = AnalysisContext();

  context_.add_function(
      "plus", FunctionSignature("plus", LispType::TYPE_ANY, {"a", "b"},
                                {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "minus", FunctionSignature("minus", LispType::TYPE_ANY, {"a", "b"},
                                 {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "times", FunctionSignature("times", LispType::TYPE_ANY, {"a", "b"},
                                 {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "divide", FunctionSignature("divide", LispType::TYPE_ANY, {"a", "b"},
                                  {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "equal", FunctionSignature("equal", LispType::TYPE_BOOL, {"a", "b"},
                                 {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "nonequal", FunctionSignature("nonequal", LispType::TYPE_BOOL, {"a", "b"},
                                    {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "less", FunctionSignature("less", LispType::TYPE_BOOL, {"a", "b"},
                                {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "lesseq", FunctionSignature("lesseq", LispType::TYPE_BOOL, {"a", "b"},
                                  {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "greater", FunctionSignature("greater", LispType::TYPE_BOOL, {"a", "b"},
                                   {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "greatereq",
      FunctionSignature("greatereq", LispType::TYPE_BOOL, {"a", "b"},
                        {LispType::TYPE_ANY, LispType::TYPE_ANY}));
  context_.add_function(
      "and", FunctionSignature("and", LispType::TYPE_BOOL, {"a", "b"},
                               {LispType::TYPE_BOOL, LispType::TYPE_BOOL}));
  context_.add_function(
      "or", FunctionSignature("or", LispType::TYPE_BOOL, {"a", "b"},
                              {LispType::TYPE_BOOL, LispType::TYPE_BOOL}));
  context_.add_function(
      "xor", FunctionSignature("xor", LispType::TYPE_BOOL, {"a", "b"},
                               {LispType::TYPE_BOOL, LispType::TYPE_BOOL}));
  context_.add_function("not", FunctionSignature("not", LispType::TYPE_BOOL,
                                                 {"a"}, {LispType::TYPE_BOOL}));

  analyze_node(root);

  if (!has_errors()) {
    Node *optimized = optimize_node(root);
    return optimized;
  }

  return root;
}

void SemanticAnalyzer::print_errors() const {
  for (const auto &error : errors_) {
    std::cerr << "Semantic Error: " << error << "\n";
  }
}

void SemanticAnalyzer::print_warnings() const {
  for (const auto &warning : warnings_) {
    std::cerr << "Warning: " << warning << "\n";
  }
}

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
    break;

  case NodeType::NT_IDENT:
    break;

  case NodeType::NT_QUOTE:
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
    for (auto child : node->children) {
      analyze_node(child);
    }
    return;
  }

  const std::string &name = first->str;

  if (name == "setq") {
    check_setq(node);
  } else if (name == "func") {
    check_func(node);
  } else if (name == "lambda") {
    check_lambda(node);
  } else if (name == "prog") {
    check_prog(node);
  } else if (name == "while") {
    check_while(node);
  } else if (name == "return") {
    check_return(node);
  } else if (name == "break") {
    check_break(node);
  } else if (name == "cond") {
    // Analyze all cases
    for (size_t i = 1; i < node->children.size(); i++) {
      analyze_node(node->children[i]);
    }
  } else {
    check_function_call(node);
  }
}

void SemanticAnalyzer::check_setq(Node *node) {
  if (node->children.size() != 3) {
    add_error("setq requires exactly 2 arguments (variable and value)");
    return;
  }

  Node *var = node->children[1];
  if (var->type != NodeType::NT_IDENT) {
    add_error("setq first argument must be a variable name");
    return;
  }

  Node *value = node->children[2];
  analyze_node(value);

  // Type check
  LispType value_type = infer_type(value);

  // check compatibility
  if (context_.is_variable_defined(var->str)) {
    LispType existing_type = context_.get_variable_type(var->str);
    if (!check_type_compatibility(existing_type, value_type)) {
      std::ostringstream oss;
      oss << "Type mismatch: variable '" << var->str << "' has type "
          << type_to_string(existing_type) << " but assigned value has type "
          << type_to_string(value_type);
      add_warning(oss.str());
    }
  }

  context_.add_variable(var->str, value_type);
}

void SemanticAnalyzer::check_func(Node *node) {
  if (node->children.size() < 4) {
    add_error("func requires name, parameters, and body");
    return;
  }

  Node *name_node = node->children[1];
  Node *params_node = node->children[2];

  if (name_node->type != NodeType::NT_IDENT) {
    add_error("func name must be an identifier");
    return;
  }

  if (params_node->type != NodeType::NT_LIST) {
    add_error("func parameters must be a list");
    return;
  }

  std::string func_name = name_node->str;
  std::vector<std::string> params;
  std::vector<LispType> param_types;

  for (auto param : params_node->children) {
    if (param->type != NodeType::NT_IDENT) {
      add_error("function parameters must be identifiers");
      continue;
    }
    params.push_back(param->str);
    param_types.push_back(LispType::TYPE_ANY);
  }

  Node *body = node->children[3];
  FunctionSignature sig(func_name, LispType::TYPE_ANY, params, param_types,
                        body);
  context_.add_function(func_name, sig);

  context_.enter_scope();
  bool old_in_function = context_.in_function;
  LispType old_return_type = context_.current_function_return_type;
  context_.in_function = true;
  context_.current_function_return_type = LispType::TYPE_ANY;

  for (const auto &param : params) {
    context_.add_variable(param, LispType::TYPE_ANY);
  }

  for (size_t i = 3; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.current_function_return_type = old_return_type;
  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_lambda(Node *node) {
  if (node->children.size() < 3) {
    add_error("lambda requires parameters and body");
    return;
  }

  Node *params_node = node->children[1];
  if (params_node->type != NodeType::NT_LIST) {
    add_error("lambda parameters must be a list");
    return;
  }

  std::vector<std::string> params;
  for (auto param : params_node->children) {
    if (param->type != NodeType::NT_IDENT) {
      add_error("lambda parameters must be identifiers");
      continue;
    }
    params.push_back(param->str);
  }

  context_.enter_scope();
  bool old_in_function = context_.in_function;
  context_.in_function = true;

  for (const auto &param : params) {
    context_.add_variable(param, LispType::TYPE_ANY);
  }

  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_prog(Node *node) {
  if (node->children.size() < 2) {
    add_error("prog requires variable bindings and body");
    return;
  }

  Node *bindings = node->children[1];
  if (bindings->type != NodeType::NT_LIST) {
    add_error("prog bindings must be a list");
    return;
  }

  context_.enter_scope();
  bool old_in_function = context_.in_function;
  context_.in_function = true;

  for (auto binding : bindings->children) {
    if (binding->type != NodeType::NT_LIST || binding->children.size() != 2) {
      add_error("prog binding must be (variable value)");
      continue;
    }
    if (binding->children[0]->type != NodeType::NT_IDENT) {
      add_error("prog binding variable must be an identifier");
      continue;
    }

    Node *value = binding->children[1];
    analyze_node(value);
    LispType value_type = infer_type(value);
    context_.add_variable(binding->children[0]->str, value_type);
  }

  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.in_function = old_in_function;
  context_.exit_scope();
}

void SemanticAnalyzer::check_while(Node *node) {
  if (node->children.size() < 3) {
    add_error("while requires condition and body");
    return;
  }

  analyze_node(node->children[1]);

  // break inside loops
  bool old_in_loop = context_.in_loop;
  context_.in_loop = true;

  for (size_t i = 2; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }

  context_.in_loop = old_in_loop;
}

void SemanticAnalyzer::check_return(Node *node) {
  //  return inside functions
  if (!context_.in_function) {
    add_error("return statement must be inside a function, lambda, or prog");
  }

  if (node->children.size() > 1) {
    Node *return_value = node->children[1];
    analyze_node(return_value);

    // return type match function
    LispType return_type = infer_type(return_value);
    if (context_.current_function_return_type != LispType::TYPE_ANY) {
      if (!check_type_compatibility(context_.current_function_return_type,
                                    return_type)) {
        std::ostringstream oss;
        oss << "Return type mismatch: expected "
            << type_to_string(context_.current_function_return_type)
            << " but got " << type_to_string(return_type);
        add_warning(oss.str());
      }
    }
  }
}

void SemanticAnalyzer::check_break(Node *node) {
  if (!context_.in_loop) {
    add_error("break statement must be inside a loop");
  }
}

void SemanticAnalyzer::check_function_call(Node *node) {
  if (node->children.empty()) {
    return;
  }

  Node *func_node = node->children[0];
  if (func_node->type != NodeType::NT_IDENT) {
    for (auto child : node->children) {
      analyze_node(child);
    }
    return;
  }

  std::string func_name = func_node->str;

  if (context_.is_function_defined(func_name)) {
    FunctionSignature *sig = context_.get_function(func_name);
    if (sig) {
      size_t expected_params = sig->params.size();
      size_t actual_args = node->children.size() - 1;

      if (expected_params != actual_args) {
        std::ostringstream oss;
        oss << "Function '" << func_name << "' expects " << expected_params
            << " arguments but got " << actual_args;
        add_error(oss.str());
      } else {
        for (size_t i = 0; i < actual_args; i++) {
          Node *arg = node->children[i + 1];
          analyze_node(arg);

          LispType arg_type = infer_type(arg);
          LispType expected_type = sig->param_types[i];

          if (expected_type != LispType::TYPE_ANY &&
              !check_type_compatibility(expected_type, arg_type)) {
            std::ostringstream oss;
            oss << "Type mismatch in function '" << func_name << "' argument "
                << (i + 1) << ": expected " << type_to_string(expected_type)
                << " but got " << type_to_string(arg_type);
            add_warning(oss.str());
          }
        }
      }
    }
  }

  for (size_t i = 1; i < node->children.size(); i++) {
    analyze_node(node->children[i]);
  }
}

LispType SemanticAnalyzer::infer_type(Node *node) {
  if (!node) {
    return LispType::TYPE_UNKNOWN;
  }

  switch (node->type) {
  case NodeType::NT_INT:
    return LispType::TYPE_INT;
  case NodeType::NT_REAL:
    return LispType::TYPE_REAL;
  case NodeType::NT_BOOL:
    return LispType::TYPE_BOOL;
  case NodeType::NT_STRING:
    return LispType::TYPE_STRING;
  case NodeType::NT_NULL:
    return LispType::TYPE_NULL;
  case NodeType::NT_LIST:
    return LispType::TYPE_LIST;
  case NodeType::NT_IDENT:
    return context_.get_variable_type(node->str);
  case NodeType::NT_QUOTE:
    return LispType::TYPE_LIST;
  default:
    return LispType::TYPE_UNKNOWN;
  }
}

bool SemanticAnalyzer::check_type_compatibility(LispType expected,
                                                LispType actual) {
  if (expected == LispType::TYPE_ANY || actual == LispType::TYPE_ANY) {
    return true;
  }
  if (expected == actual) {
    return true;
  }
  if (expected == LispType::TYPE_REAL && actual == LispType::TYPE_INT) {
    return true;
  }
  return false;
}

std::string SemanticAnalyzer::type_to_string(LispType type) {
  switch (type) {
  case LispType::TYPE_INT:
    return "Integer";
  case LispType::TYPE_REAL:
    return "Real";
  case LispType::TYPE_BOOL:
    return "Boolean";
  case LispType::TYPE_STRING:
    return "String";
  case LispType::TYPE_LIST:
    return "List";
  case LispType::TYPE_NULL:
    return "Null";
  case LispType::TYPE_ANY:
    return "Any";
  default:
    return "Unknown";
  }
}

// Optimization
Node *SemanticAnalyzer::optimize_node(Node *node) {
  if (!node) {
    return nullptr;
  }

  for (size_t i = 0; i < node->children.size(); i++) {
    Node *optimized_child = optimize_node(node->children[i]);
    if (optimized_child != node->children[i]) {
      node->children[i] = optimized_child;
    }
  }

  Node *result = node;

  if (result->type == NodeType::NT_LIST) {
    result = remove_unreachable_code(result);
  }

  if (enable_function_inlining_ && result->type == NodeType::NT_LIST &&
      !result->children.empty()) {
    Node *first = result->children[0];
    if (first->type == NodeType::NT_IDENT) {
      FunctionSignature *sig = context_.get_function(first->str);
      if (sig && should_inline_function(sig)) {
        Node *inlined = inline_function_call(result, first->str);
        if (inlined && inlined != result) {
          result = inlined;
        }
      }
    }
  }

  return result;
}

Node *SemanticAnalyzer::remove_unreachable_code(Node *node) {
  if (node->type != NodeType::NT_LIST || node->children.empty()) {
    return node;
  }

  for (size_t i = 0; i < node->children.size(); i++) {
    Node *child = node->children[i];
    if (child->type == NodeType::NT_LIST && !child->children.empty()) {
      Node *first = child->children[0];
      if (first->type == NodeType::NT_IDENT && first->str == "return") {
        if (i + 1 < node->children.size()) {
          for (size_t j = i + 1; j < node->children.size(); j++) {
            delete node->children[j];
          }
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

  if (node->children.size() - 1 != sig->params.size()) {
    return node;
  }

  std::vector<Node *> args;
  for (size_t i = 1; i < node->children.size(); i++) {
    args.push_back(node->children[i]);
  }

  Node *inlined_body = substitute_parameters(sig->body, sig->params, args);
  return inlined_body;
}

bool SemanticAnalyzer::should_inline_function(const FunctionSignature *sig) {
  if (!sig || !sig->body) {
    return false;
  }

  if (sig->body->type != NodeType::NT_LIST) {
    return true;
  }

  int stmt_count = 0;
  std::function<void(Node *)> count = [&](Node *n) {
    if (!n)
      return;
    if (n->type == NodeType::NT_LIST)
      stmt_count++;
    for (auto child : n->children) {
      count(child);
    }
  };
  count(sig->body);

  return stmt_count <= 3;
}

Node *SemanticAnalyzer::clone_node(Node *node) {
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

  std::map<std::string, Node *> substitution_map;
  for (size_t i = 0; i < params.size() && i < args.size(); i++) {
    substitution_map[params[i]] = args[i];
  }

  Node *cloned = clone_node(body);

  std::function<void(Node *)> substitute = [&](Node *node) {
    if (!node) {
      return;
    }

    if (node->type == NodeType::NT_IDENT) {
      auto it = substitution_map.find(node->str);
      if (it != substitution_map.end()) {
        Node *replacement = clone_node(it->second);
        node->type = replacement->type;
        node->str = replacement->str;
        node->ival = replacement->ival;
        node->dval = replacement->dval;
        node->bval = replacement->bval;
        node->children = replacement->children;
        replacement->children.clear();
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

void SemanticAnalyzer::add_error(const std::string &message) {
  errors_.push_back(message);
}

void SemanticAnalyzer::add_warning(const std::string &message) {
  warnings_.push_back(message);
}
