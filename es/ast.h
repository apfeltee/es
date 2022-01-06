#ifndef ES_AST_H
#define ES_AST_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include <es/token.h>

namespace es {

class AST {
 public:
  enum Type {
    AST_EXPR_THIS,
    AST_EXPR_IDENT,

    AST_EXPR_NULL,
    AST_EXPR_BOOL,
    AST_EXPR_NUMBER,
    AST_EXPR_STRING,
    AST_EXPR_REGEX,

    AST_EXPR_ARRAY,
    AST_EXPR_OBJ,

    AST_EXPR_PAREN,  // ( Expression )

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_TRIPLE,

    AST_EXPR_FUNC,

    AST_EXPR_ARGS,
    AST_EXPR_LHS,

    AST_EXPR,

    AST_FUNC_BODY,

    AST_STMT_EMPTY,
    AST_STMT_CONTINUE,
    AST_STMT_BREAK,
    AST_STMT_RETURN,
    AST_STMT_THROW,
    AST_STMT_LABEL,
    AST_STMT_DEBUG,

    AST_PROGRAM,

    AST_ILLEGAL,
  };

  AST(Type type, std::u16string_view source = u"") : type_(type), source_(source) {}
  virtual ~AST() {};

  Type type() { return type_; }
  std::u16string_view source() { return source_; }

  void SetSource(std::u16string_view source) { source_ = source; }

  bool IsIllegal() { return type_ == AST_ILLEGAL; }

 private:
  Type type_;
  std::u16string_view source_;
};

class ArrayLiteral : public AST {
 public:
  ArrayLiteral() : AST(AST_EXPR_ARRAY), len_(0) {}

  ~ArrayLiteral() override {
    for (auto pair : elements_) {
      delete pair.second;
    }
  }

  size_t length() { return len_; }

  void AddElement(AST* element) {
    if (element != nullptr) {
      elements_.emplace_back(len_, element);
    }
    len_++;
  }

 private:
  std::vector<std::pair<size_t, AST*>> elements_;
  size_t len_;
};

class ObjectLiteral : public AST {
 public:
  ObjectLiteral() : AST(AST_EXPR_OBJ) {}

  ~ObjectLiteral() override {
    for (auto pair : properties_) {
      delete pair.second.value;
    }
  }

  struct Property {
    enum Type {
      NORMAL = 0,
      GET,
      SET,
    };

    Property(Token k, AST* v, Type t) : key(k), type(t), value(v) {}

    Token key;
    AST* value;
    Type type;
  };

  void AddProperty(Property p) {
    // When having same key, will keep the last one.
    // but {1.0: 1, 1.00: 2} will have 2 key-val pair.
    properties_.emplace(p.key.source(), p);
  }

 private:
  std::unordered_map<std::u16string_view, Property> properties_;
};

class Binary : public AST {
 public:
  Binary(AST* lhs, AST* rhs, Token op) :
    AST(AST_EXPR_BINARY), lhs_(lhs), rhs_(rhs), op_(op) {}

  ~Binary() override {
    delete lhs_;
    delete rhs_;
  }

  AST* lhs() { return lhs_; }
  AST* rhs() { return rhs_; }
  Token op() { return op_; }

 private:
  AST* lhs_;
  AST* rhs_;
  Token op_;
};

class Unary : public AST {
 public:
  Unary(AST* node, Token op, bool prefix) :
    AST(AST_EXPR_UNARY), node_(node), op_(op), prefix_(prefix) {}

  ~Unary() override {
    delete node_;
  }

  AST* node() { return node_; }
  Token op() { return op_; }
  bool prefix() { return prefix_; }

 private:
  AST* node_;
  Token op_;
  bool prefix_;
};

class TripleCondition : public AST {
 public:
  TripleCondition(AST* cond, AST* lhs, AST* rhs) :
    AST(AST_EXPR_TRIPLE), cond_(cond), lhs_(lhs), rhs_(rhs) {}

  ~TripleCondition() override {
    delete cond_;
    delete lhs_;
    delete rhs_;
  }

  AST* cond() { return cond_; }
  AST* lhs() { return lhs_; }
  AST* rhs() { return rhs_; }

 private:
  AST* cond_;
  AST* lhs_;
  AST* rhs_;
};

class Expression : public AST {
 public:
  Expression() : AST(AST_EXPR) {}
  ~Expression() override {
    for (auto element : elements_) {
      delete element;
    }
  }

  void AddElement(AST* element) { elements_.push_back(element); }

 private:
  std::vector<AST*> elements_;
};

class FunctionBody : public AST {
 public:
  FunctionBody() : AST(AST_FUNC_BODY) {}
};

class Function : public AST {
 public:
  Function(Token name, std::vector<Token> params, FunctionBody* body) :
    AST(AST_EXPR_FUNC), name_(name), params_(params), body_(body) {}

  ~Function() override {
    delete body_;
  }

  Token name() { return name_; }
  std::vector<Token> params() { return params_; }
  FunctionBody* body() { return body_; }

 private:
  Token name_;
  std::vector<Token> params_;
  FunctionBody* body_;
};

class Arguments : public AST {
 public:
  Arguments(std::vector<AST*> args) : AST(AST_EXPR_ARGS), args_(args) {}

  ~Arguments() override {
    for (auto arg : args_)
      delete arg;
  }

  std::vector<AST*> args() { return args_; }

 private:
  std::vector<AST*> args_;
};

class LHS : public AST {
 public:
  LHS(AST* base, size_t new_count) :
    AST(AST_EXPR_LHS), base_(base), new_count_(new_count) {}

  ~LHS() override {
    for (auto args : args_list_)
      delete args;
    for (auto index : index_list_)
      delete index;
  }

  enum PostfixType{
    CALL,
    INDEX,
    PROP,
  };

  void AddArguments(Arguments* args) {
    order_.emplace_back(std::make_pair(args_list_.size(), CALL));
    args_list_.emplace_back(args);
  }

  void AddIndex(AST* index) {
    order_.emplace_back(std::make_pair(index_list_.size(), INDEX));
    index_list_.emplace_back(index);
  }

  void AddProp(Token prop_name) {
    order_.emplace_back(std::make_pair(prop_name_list_.size(), PROP));
    prop_name_list_.emplace_back(prop_name);
  }

 private:
  AST* base_;
  size_t new_count_;

  std::vector<std::pair<size_t, PostfixType>> order_;
  std::vector<Arguments*> args_list_;
  std::vector<AST*> index_list_;
  std::vector<Token> prop_name_list_;
};

class Program : public AST {
 public:
  Program() : AST(AST_PROGRAM) {}
  ~Program() override {
    for (auto element : elements_)
      delete element;
  }

  void AddFunctionDecl(AST* func) {
    // TODO(zhuzilin) check function has name.
    elements_.emplace_back(func);
  }
  void AddStatement(AST* stmt) {
    // TODO(zhuzilin) check stmt is statment.
    elements_.emplace_back(stmt);
  }

 private:
  std::vector<AST*> elements_;
};

class LabelStmt : public AST {
 public:
  LabelStmt(Token ident, AST* stmt) :
    AST(AST_STMT_LABEL), ident_(ident), stmt_(stmt) {}
  ~LabelStmt() {
    delete stmt_;
  }

 private:
  Token ident_;
  AST* stmt_;
};

class Continue : public AST {
 public:
  Continue(Token ident, std::u16string_view source) :
    AST(AST_STMT_CONTINUE, source), ident_(ident) {}

  Token ident() { return ident_; }

 private:
  Token ident_;
};

class Break : public AST {
 public:
  Break(Token ident, std::u16string_view source) :
    AST(AST_STMT_BREAK, source), ident_(ident) {}

  Token ident() { return ident_; }

 private:
  Token ident_;
};

class Return : public AST {
 public:
  Return(AST* expr, std::u16string_view source) :
    AST(AST_STMT_BREAK, source), expr_(expr) {}

  AST* expr() { return expr_; }

 private:
  AST* expr_;
};

}  // namespace es

#endif