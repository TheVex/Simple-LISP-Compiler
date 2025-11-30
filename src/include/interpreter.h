#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class RuntimeError : public std::runtime_error {
public:
  explicit RuntimeError(const std::string &msg) : std::runtime_error(msg) {}
};

enum class ValueType {
  VAL_INT,
  VAL_REAL,
  VAL_BOOL,
  VAL_NULL,
  VAL_STRING,
  VAL_LIST,
  VAL_FUNCTION,
  VAL_LAMBDA
};

struct Value {
  ValueType type;
  long long ival = 0;
  double dval = 0.0;
  bool bval = false;
  std::string sval;
  std::vector<Value> list_val;

  // For lambda/function
  std::vector<std::string> params;
  Node *body = nullptr;

  Value() : type(ValueType::VAL_NULL) {}
  explicit Value(ValueType t) : type(t) {}

  static Value make_int(long long v) {
    Value val(ValueType::VAL_INT);
    val.ival = v;
    return val;
  }

  static Value make_real(double v) {
    Value val(ValueType::VAL_REAL);
    val.dval = v;
    return val;
  }

  static Value make_bool(bool v) {
    Value val(ValueType::VAL_BOOL);
    val.bval = v;
    return val;
  }

  static Value make_null() { return Value(ValueType::VAL_NULL); }

  static Value make_string(const std::string &s) {
    Value val(ValueType::VAL_STRING);
    val.sval = s;
    return val;
  }

  static Value make_list(const std::vector<Value> &lst) {
    Value val(ValueType::VAL_LIST);
    val.list_val = lst;
    return val;
  }

  static Value make_lambda(const std::vector<std::string> &params, Node *body) {
    Value val(ValueType::VAL_LAMBDA);
    val.params = params;
    val.body = body;
    return val;
  }

  bool is_truthy() const {
    if (type == ValueType::VAL_BOOL)
      return bval;
    if (type == ValueType::VAL_NULL)
      return false;
    if (type == ValueType::VAL_INT)
      return ival != 0;
    if (type == ValueType::VAL_REAL)
      return dval != 0.0;
    return true;
  }
};

class Environment {
private:
  std::map<std::string, Value> variables;
  std::map<std::string, Value> functions;
  Environment *parent;

public:
  explicit Environment(Environment *p = nullptr) : parent(p) {}

  void define(const std::string &name, const Value &value) {
    variables[name] = value;
  }

  void define_function(const std::string &name, const Value &value) {
    functions[name] = value;
  }

  Value get(const std::string &name) {
    if (variables.find(name) != variables.end()) {
      return variables[name];
    }
    if (parent) {
      return parent->get(name);
    }
    throw RuntimeError("Undefined variable: " + name);
  }

  Value get_function(const std::string &name) {
    if (functions.find(name) != functions.end()) {
      return functions[name];
    }
    if (parent) {
      return parent->get_function(name);
    }
    throw RuntimeError("Undefined function: " + name);
  }

  void set(const std::string &name, const Value &value) {
    if (variables.find(name) != variables.end()) {
      variables[name] = value;
      return;
    }
    if (parent) {
      parent->set(name, value);
      return;
    }
    variables[name] = value;
  }

  bool has_variable(const std::string &name) {
    if (variables.find(name) != variables.end())
      return true;
    if (parent)
      return parent->has_variable(name);
    return false;
  }

  bool has_function(const std::string &name) {
    if (functions.find(name) != functions.end())
      return true;
    if (parent)
      return parent->has_function(name);
    return false;
  }
};

enum class ControlFlow { NONE, RETURN, BREAK };

class Interpreter {
private:
  Environment global_env;
  ControlFlow control_flow;
  Value return_value;

  Value eval_node(Node *node, Environment &env);
  Value eval_list(Node *node, Environment &env);
  Value eval_quote(Node *node, Environment &env);

  // Built-in operations
  Value eval_setq(const std::vector<Node *> &args, Environment &env);
  Value eval_func(const std::vector<Node *> &args, Environment &env);
  Value eval_lambda(const std::vector<Node *> &args, Environment &env);
  Value eval_prog(const std::vector<Node *> &args, Environment &env);
  Value eval_cond(const std::vector<Node *> &args, Environment &env);
  Value eval_while(const std::vector<Node *> &args, Environment &env);
  Value eval_return(const std::vector<Node *> &args, Environment &env);
  Value eval_break(const std::vector<Node *> &args, Environment &env);
  Value eval_eval(const std::vector<Node *> &args, Environment &env);
  Value eval_print(const std::vector<Node *> &args, Environment &env);

  // Arithmetic
  Value eval_plus(const std::vector<Node *> &args, Environment &env);
  Value eval_minus(const std::vector<Node *> &args, Environment &env);
  Value eval_times(const std::vector<Node *> &args, Environment &env);
  Value eval_divide(const std::vector<Node *> &args, Environment &env);

  // Comparison
  Value eval_equal(const std::vector<Node *> &args, Environment &env);
  Value eval_nonequal(const std::vector<Node *> &args, Environment &env);
  Value eval_less(const std::vector<Node *> &args, Environment &env);
  Value eval_lesseq(const std::vector<Node *> &args, Environment &env);
  Value eval_greater(const std::vector<Node *> &args, Environment &env);
  Value eval_greatereq(const std::vector<Node *> &args, Environment &env);

  // Type checking
  Value eval_isint(const std::vector<Node *> &args, Environment &env);
  Value eval_isreal(const std::vector<Node *> &args, Environment &env);
  Value eval_isbool(const std::vector<Node *> &args, Environment &env);
  Value eval_isnull(const std::vector<Node *> &args, Environment &env);
  Value eval_isatom(const std::vector<Node *> &args, Environment &env);
  Value eval_islist(const std::vector<Node *> &args, Environment &env);

  // Logical
  Value eval_and(const std::vector<Node *> &args, Environment &env);
  Value eval_or(const std::vector<Node *> &args, Environment &env);
  Value eval_xor(const std::vector<Node *> &args, Environment &env);
  Value eval_not(const std::vector<Node *> &args, Environment &env);

  // Calls
  Value eval_function_call(const std::string &name,
                           const std::vector<Node *> &args, Environment &env);

  // Utils
  Value node_to_value(Node *node);
  Node *value_to_node(const Value &val);
  void print_value(const Value &val);

public:
  Interpreter() : control_flow(ControlFlow::NONE) {}

  void interpret(Node *root);
};

#endif // INTERPRETER_H
