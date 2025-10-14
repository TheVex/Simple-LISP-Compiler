#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

enum class NodeType {
    NT_INT,
    NT_REAL,
    NT_BOOL,
    NT_NULL,
    NT_IDENT,   // atom / variable
    NT_STRING,
    NT_LIST,    // list of elements
    NT_QUOTE    // quoted element (optional separate node)
};

struct Node {
    NodeType type;
    std::string str;            // for IDENT, STRING
    long long  ival = 0;        // for INT
    double     dval = 0.0;      // for REAL
    bool       bval = false;    // for BOOL
    std::vector<Node*> children; // for lists

    Node(NodeType t) : type(t) {}
    ~Node() {
        for (auto c : children) delete c;
    }
};

// Фабрики
inline Node* make_int_node(long long v) {
    Node* n = new Node(NodeType::NT_INT); n->ival = v; return n;
}
inline Node* make_real_node(double v) {
    Node* n = new Node(NodeType::NT_REAL); n->dval = v; return n;
}
inline Node* make_bool_node(bool v) {
    Node* n = new Node(NodeType::NT_BOOL); n->bval = v; return n;
}
inline Node* make_null_node() {
    return new Node(NodeType::NT_NULL);
}
inline Node* make_ident_node(const std::string& s) {
    Node* n = new Node(NodeType::NT_IDENT); n->str = s; return n;
}
inline Node* make_string_node(const std::string& s) {
    Node* n = new Node(NodeType::NT_STRING); n->str = s; return n;
}
inline Node* make_list_node(const std::vector<Node*>& elems) {
    Node* n = new Node(NodeType::NT_LIST);
    n->children = elems;
    return n;
}
inline Node* make_quote_node(Node* child) {
    Node* n = new Node(NodeType::NT_QUOTE);
    n->children.push_back(child);
    return n;
}

// Утилита для печати AST (простая)
inline void print_node(Node* n, int depth = 0) {
    for (int i = 0; i < depth; i++) std::cout << "  ";
    switch (n->type) {
    case NodeType::NT_INT:    std::cout << "INT(" << n->ival << ")\n"; break;
    case NodeType::NT_REAL:   std::cout << "REAL(" << n->dval << ")\n"; break;
    case NodeType::NT_BOOL:   std::cout << "BOOL(" << (n->bval ? "true" : "false") << ")\n"; break;
    case NodeType::NT_NULL:   std::cout << "NULL\n"; break;
    case NodeType::NT_IDENT:  std::cout << "IDENT(" << n->str << ")\n"; break;
    case NodeType::NT_STRING: std::cout << "STRING(" << n->str << ")\n"; break;
    case NodeType::NT_QUOTE:  
        std::cout << "QUOTE\n"; 
        for (auto c : n->children) print_node(c, depth + 1);
        break;
    case NodeType::NT_LIST:
        std::cout << "LIST\n";
        for (auto c : n->children) print_node(c, depth + 1);
        break;
    }
}

#endif // AST_H
