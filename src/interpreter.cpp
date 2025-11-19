#include "interpreter.h"
#include <cmath>
#include <iostream>

void Interpreter::interpret(Node *root) {
  control_flow = ControlFlow::NONE;
  try {
    if (root && root->type == NodeType::NT_LIST) {
      // Root is a program (list of statements)
      for (Node *stmt : root->children) {
        eval_node(stmt, global_env);
        if (control_flow != ControlFlow::NONE)
          break;
      }
    } else {
      eval_node(root, global_env);
    }
  } catch (const RuntimeError &e) {
    std::cerr << "Runtime Error: " << e.what() << "\n";
  }
}

Value Interpreter::eval_node(Node *node, Environment &env) {
  if (!node)
    return Value::make_null();

  switch (node->type) {
  case NodeType::NT_INT:
    return Value::make_int(node->ival);
  case NodeType::NT_REAL:
    return Value::make_real(node->dval);
  case NodeType::NT_BOOL:
    return Value::make_bool(node->bval);
  case NodeType::NT_NULL:
    return Value::make_null();
  case NodeType::NT_STRING:
    return Value::make_string(node->str);
  case NodeType::NT_IDENT:
    if (env.has_variable(node->str)) {
      return env.get(node->str);
    }
    throw RuntimeError("Undefined identifier: " + node->str);
  case NodeType::NT_LIST:
    return eval_list(node, env);
  case NodeType::NT_QUOTE:
    return eval_quote(node, env);
  default:
    throw RuntimeError("Unknown node type");
  }
}

Value Interpreter::eval_list(Node *node, Environment &env) {
  if (node->children.empty()) {
    return Value::make_null();
  }

  Node *first = node->children[0];
  if (first->type != NodeType::NT_IDENT) {
    throw RuntimeError("First element of list must be an identifier");
  }

  std::string op = first->str;
  std::vector<Node *> args(node->children.begin() + 1, node->children.end());

  if (op == "setq")
    return eval_setq(args, env);
  if (op == "func")
    return eval_func(args, env);
  if (op == "lambda")
    return eval_lambda(args, env);
  if (op == "prog")
    return eval_prog(args, env);
  if (op == "cond")
    return eval_cond(args, env);
  if (op == "while")
    return eval_while(args, env);
  if (op == "return")
    return eval_return(args, env);
  if (op == "break")
    return eval_break(args, env);
  if (op == "quote")
    return eval_quote(node, env);
  if (op == "eval")
    return eval_eval(args, env);
  if (op == "print")
    return eval_print(args, env);

  if (op == "plus")
    return eval_plus(args, env);
  if (op == "minus")
    return eval_minus(args, env);
  if (op == "times")
    return eval_times(args, env);
  if (op == "divide")
    return eval_divide(args, env);

  if (op == "equal")
    return eval_equal(args, env);
  if (op == "nonequal")
    return eval_nonequal(args, env);
  if (op == "less")
    return eval_less(args, env);
  if (op == "lesseq")
    return eval_lesseq(args, env);
  if (op == "greater")
    return eval_greater(args, env);
  if (op == "greatereq")
    return eval_greatereq(args, env);

  if (op == "isint")
    return eval_isint(args, env);
  if (op == "isreal")
    return eval_isreal(args, env);
  if (op == "isbool")
    return eval_isbool(args, env);
  if (op == "isnull")
    return eval_isnull(args, env);
  if (op == "isatom")
    return eval_isatom(args, env);
  if (op == "islist")
    return eval_islist(args, env);

  if (op == "and")
    return eval_and(args, env);
  if (op == "or")
    return eval_or(args, env);
  if (op == "xor")
    return eval_xor(args, env);
  if (op == "not")
    return eval_not(args, env);

  if (env.has_variable(op)) {
    Value func_val = env.get(op);
    if (func_val.type == ValueType::VAL_LAMBDA) {
      if (args.size() != func_val.params.size())
        throw RuntimeError("Lambda " + op + " expects " +
                           std::to_string(func_val.params.size()) +
                           " arguments, got " + std::to_string(args.size()));

      Environment local_env(&env);
      for (size_t i = 0; i < args.size(); i++) {
        Value arg_val = eval_node(args[i], env);
        local_env.define(func_val.params[i], arg_val);
      }

      ControlFlow prev_flow = control_flow;
      control_flow = ControlFlow::NONE;

      Value result = eval_node(func_val.body, local_env);

      if (control_flow == ControlFlow::RETURN) {
        result = return_value;
      }

      control_flow = prev_flow;
      return result;
    }
  }

  return eval_function_call(op, args, env);
}

Value Interpreter::eval_quote(Node *node, Environment &env) {
  if (node->type == NodeType::NT_QUOTE) {
    if (node->children.empty())
      return Value::make_null();
    return node_to_value(node->children[0]);
  }
  // For (quote x) form
  if (node->children.size() < 2)
    throw RuntimeError("quote requires an argument");
  return node_to_value(node->children[1]);
}

Value Interpreter::eval_setq(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("setq requires exactly 2 arguments");

  if (args[0]->type != NodeType::NT_IDENT)
    throw RuntimeError("First argument to setq must be an identifier");

  std::string name = args[0]->str;
  Value value = eval_node(args[1], env);
  env.set(name, value);
  return value;
}

Value Interpreter::eval_func(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() < 3)
    throw RuntimeError(
        "func requires at least 3 arguments: name, params, body");

  if (args[0]->type != NodeType::NT_IDENT)
    throw RuntimeError("Function name must be an identifier");

  std::string name = args[0]->str;

  if (args[1]->type != NodeType::NT_LIST)
    throw RuntimeError("Function parameters must be a list");

  std::vector<std::string> params;
  for (Node *param : args[1]->children) {
    if (param->type != NodeType::NT_IDENT)
      throw RuntimeError("Function parameter must be an identifier");
    params.push_back(param->str);
  }

  Node *body = args[2];
  Value func_val = Value::make_lambda(params, body);
  global_env.define_function(name, func_val);
  return Value::make_null();
}

Value Interpreter::eval_lambda(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() < 2)
    throw RuntimeError("lambda requires at least 2 arguments: params, body");

  if (args[0]->type != NodeType::NT_LIST)
    throw RuntimeError("Lambda parameters must be a list");

  std::vector<std::string> params;
  for (Node *param : args[0]->children) {
    if (param->type != NodeType::NT_IDENT)
      throw RuntimeError("Lambda parameter must be an identifier");
    params.push_back(param->str);
  }

  Node *body = args[1];
  return Value::make_lambda(params, body);
}

Value Interpreter::eval_prog(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.empty())
    throw RuntimeError("prog requires bindings and body");

  Node *bindings = args[0];
  if (bindings->type != NodeType::NT_LIST)
    throw RuntimeError("prog bindings must be a list");

  Environment local_env(&env);

  for (Node *binding : bindings->children) {
    if (binding->type != NodeType::NT_LIST || binding->children.size() != 2)
      throw RuntimeError("prog binding must be (variable value)");

    if (binding->children[0]->type != NodeType::NT_IDENT)
      throw RuntimeError("prog binding variable must be an identifier");

    std::string var_name = binding->children[0]->str;
    Value var_value = eval_node(binding->children[1], local_env);
    local_env.define(var_name, var_value);
  }

  Value result = Value::make_null();
  for (size_t i = 1; i < args.size(); i++) {
    if (control_flow != ControlFlow::NONE)
      break;
    result = eval_node(args[i], local_env);
  }

  return result;
}

Value Interpreter::eval_cond(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() != 3)
    throw RuntimeError(
        "cond requires exactly 3 arguments: condition, then, else");

  Value condition = eval_node(args[0], env);
  if (condition.is_truthy()) {
    return eval_node(args[1], env);
  } else {
    return eval_node(args[2], env);
  }
}

Value Interpreter::eval_while(const std::vector<Node *> &args,
                              Environment &env) {
  if (args.size() < 2)
    throw RuntimeError("while requires at least 2 arguments");

  Value result = Value::make_null();
  while (true) {
    Value condition = eval_node(args[0], env);
    if (!condition.is_truthy())
      break;

    for (size_t i = 1; i < args.size(); i++) {
      result = eval_node(args[i], env);
      if (control_flow == ControlFlow::BREAK) {
        control_flow = ControlFlow::NONE;
        return result;
      }
      if (control_flow == ControlFlow::RETURN) {
        return result;
      }
    }
  }
  return result;
}

Value Interpreter::eval_return(const std::vector<Node *> &args,
                               Environment &env) {
  Value result = Value::make_null();
  if (!args.empty()) {
    result = eval_node(args[0], env);
  }
  control_flow = ControlFlow::RETURN;
  return_value = result;
  return result;
}

Value Interpreter::eval_break(const std::vector<Node *> &args,
                              Environment &env) {
  control_flow = ControlFlow::BREAK;
  return Value::make_null();
}

Value Interpreter::eval_eval(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("eval requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  Node *node = value_to_node(val);
  Value result = eval_node(node, env);
  delete node;
  return result;
}

Value Interpreter::eval_print(const std::vector<Node *> &args,
                              Environment &env) {
  for (size_t i = 0; i < args.size(); i++) {
    Value val = eval_node(args[i], env);
    print_value(val);
    if (i < args.size() - 1)
      std::cout << " ";
  }
  std::cout << "\n";
  return Value::make_null();
}

Value Interpreter::eval_plus(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("plus requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_int(left.ival + right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_real(l + r);
}

Value Interpreter::eval_minus(const std::vector<Node *> &args,
                              Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("minus requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_int(left.ival - right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_real(l - r);
}

Value Interpreter::eval_times(const std::vector<Node *> &args,
                              Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("times requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_int(left.ival * right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_real(l * r);
}

Value Interpreter::eval_divide(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("divide requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;

  if (r == 0.0)
    throw RuntimeError("Division by zero");

  return Value::make_real(l / r);
}

Value Interpreter::eval_equal(const std::vector<Node *> &args,
                              Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("equal requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type != right.type)
    return Value::make_bool(false);

  switch (left.type) {
  case ValueType::VAL_INT:
    return Value::make_bool(left.ival == right.ival);
  case ValueType::VAL_REAL:
    return Value::make_bool(std::abs(left.dval - right.dval) < 1e-9);
  case ValueType::VAL_BOOL:
    return Value::make_bool(left.bval == right.bval);
  case ValueType::VAL_STRING:
    return Value::make_bool(left.sval == right.sval);
  case ValueType::VAL_NULL:
    return Value::make_bool(true);
  default:
    return Value::make_bool(false);
  }
}

Value Interpreter::eval_nonequal(const std::vector<Node *> &args,
                                 Environment &env) {
  Value result = eval_equal(args, env);
  return Value::make_bool(!result.bval);
}

Value Interpreter::eval_less(const std::vector<Node *> &args,
                             Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("less requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_bool(left.ival < right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_bool(l < r);
}

Value Interpreter::eval_lesseq(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("lesseq requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_bool(left.ival <= right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_bool(l <= r);
}

Value Interpreter::eval_greater(const std::vector<Node *> &args,
                                Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("greater requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_bool(left.ival > right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_bool(l > r);
}

Value Interpreter::eval_greatereq(const std::vector<Node *> &args,
                                  Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("greatereq requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);

  if (left.type == ValueType::VAL_INT && right.type == ValueType::VAL_INT) {
    return Value::make_bool(left.ival >= right.ival);
  }

  double l = (left.type == ValueType::VAL_INT) ? left.ival : left.dval;
  double r = (right.type == ValueType::VAL_INT) ? right.ival : right.dval;
  return Value::make_bool(l >= r);
}

Value Interpreter::eval_isint(const std::vector<Node *> &args,
                              Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("isint requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type == ValueType::VAL_INT);
}

Value Interpreter::eval_isreal(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("isreal requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type == ValueType::VAL_REAL);
}

Value Interpreter::eval_isbool(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("isbool requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type == ValueType::VAL_BOOL);
}

Value Interpreter::eval_isnull(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("isnull requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type == ValueType::VAL_NULL);
}

Value Interpreter::eval_isatom(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("isatom requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type != ValueType::VAL_LIST);
}

Value Interpreter::eval_islist(const std::vector<Node *> &args,
                               Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("islist requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(val.type == ValueType::VAL_LIST);
}

Value Interpreter::eval_and(const std::vector<Node *> &args, Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("and requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  if (!left.is_truthy())
    return Value::make_bool(false);

  Value right = eval_node(args[1], env);
  return Value::make_bool(right.is_truthy());
}

Value Interpreter::eval_or(const std::vector<Node *> &args, Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("or requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  if (left.is_truthy())
    return Value::make_bool(true);

  Value right = eval_node(args[1], env);
  return Value::make_bool(right.is_truthy());
}

Value Interpreter::eval_xor(const std::vector<Node *> &args, Environment &env) {
  if (args.size() != 2)
    throw RuntimeError("xor requires exactly 2 arguments");

  Value left = eval_node(args[0], env);
  Value right = eval_node(args[1], env);
  return Value::make_bool(left.is_truthy() != right.is_truthy());
}

Value Interpreter::eval_not(const std::vector<Node *> &args, Environment &env) {
  if (args.size() != 1)
    throw RuntimeError("not requires exactly 1 argument");

  Value val = eval_node(args[0], env);
  return Value::make_bool(!val.is_truthy());
}

Value Interpreter::eval_function_call(const std::string &name,
                                      const std::vector<Node *> &args,
                                      Environment &env) {
  if (!global_env.has_function(name))
    throw RuntimeError("Undefined function: " + name);

  Value func = global_env.get_function(name);
  if (func.type != ValueType::VAL_LAMBDA)
    throw RuntimeError(name + " is not a function");

  if (args.size() != func.params.size())
    throw RuntimeError("Function " + name + " expects " +
                       std::to_string(func.params.size()) + " arguments, got " +
                       std::to_string(args.size()));

  Environment local_env(&global_env);
  for (size_t i = 0; i < args.size(); i++) {
    Value arg_val = eval_node(args[i], env);
    local_env.define(func.params[i], arg_val);
  }

  ControlFlow prev_flow = control_flow;
  control_flow = ControlFlow::NONE;

  Value result = eval_node(func.body, local_env);

  if (control_flow == ControlFlow::RETURN) {
    result = return_value;
  }

  control_flow = prev_flow;
  return result;
}

Value Interpreter::node_to_value(Node *node) {
  if (!node)
    return Value::make_null();

  switch (node->type) {
  case NodeType::NT_INT:
    return Value::make_int(node->ival);
  case NodeType::NT_REAL:
    return Value::make_real(node->dval);
  case NodeType::NT_BOOL:
    return Value::make_bool(node->bval);
  case NodeType::NT_NULL:
    return Value::make_null();
  case NodeType::NT_STRING:
    return Value::make_string(node->str);
  case NodeType::NT_IDENT:
    return Value::make_string(node->str);
  case NodeType::NT_LIST: {
    std::vector<Value> list_val;
    for (Node *child : node->children) {
      list_val.push_back(node_to_value(child));
    }
    return Value::make_list(list_val);
  }
  case NodeType::NT_QUOTE:
    if (!node->children.empty())
      return node_to_value(node->children[0]);
    return Value::make_null();
  default:
    return Value::make_null();
  }
}

Node *Interpreter::value_to_node(const Value &val) {
  switch (val.type) {
  case ValueType::VAL_INT:
    return make_int_node(val.ival);
  case ValueType::VAL_REAL:
    return make_real_node(val.dval);
  case ValueType::VAL_BOOL:
    return make_bool_node(val.bval);
  case ValueType::VAL_NULL:
    return make_null_node();
  case ValueType::VAL_STRING:
    if (val.sval.length() > 0 && val.sval[0] != '"') {
      return make_ident_node(val.sval);
    }
    return make_string_node(val.sval);
  case ValueType::VAL_LIST: {
    std::vector<Node *> children;
    for (const Value &elem : val.list_val) {
      children.push_back(value_to_node(elem));
    }
    return make_list_node(children);
  }
  default:
    return make_null_node();
  }
}

void Interpreter::print_value(const Value &val) {
  switch (val.type) {
  case ValueType::VAL_INT:
    std::cout << val.ival;
    break;
  case ValueType::VAL_REAL:
    std::cout << val.dval;
    break;
  case ValueType::VAL_BOOL:
    std::cout << (val.bval ? "true" : "false");
    break;
  case ValueType::VAL_NULL:
    std::cout << "null";
    break;
  case ValueType::VAL_STRING:
    std::cout << val.sval;
    break;
  case ValueType::VAL_LIST:
    std::cout << "(";
    for (size_t i = 0; i < val.list_val.size(); i++) {
      if (i > 0)
        std::cout << " ";
      print_value(val.list_val[i]);
    }
    std::cout << ")";
    break;
  default:
    std::cout << "<unknown>";
    break;
  }
}
