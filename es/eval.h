#ifndef ES_EVALUATOR_H
#define ES_EVALUATOR_H

#include <math.h>

#include <es/parser/character.h>
#include <es/parser/ast.h>
#include <es/types/completion.h>
#include <es/types/reference.h>
#include <es/types/builtin/function_object.h>
#include <es/execution_context.h>
#include <es/helper.h>

namespace es {

Completion EvalProgram(Error* e, AST* ast);
Completion EvalStatement(Error* e, AST* ast);
Completion EvalReturnStatement(Error* e, AST* ast);
Completion EvalExpressionStatement(Error* e, AST* ast);

JSValue* EvalExpression(Error* e, AST* ast);
Reference* EvalIdentifier(AST* ast);
Number* EvalNumber(AST* ast);
String* EvalString(AST* ast);
JSValue* EvalBinaryExpression(Error* e, AST* ast);
JSValue* EvalLeftHandSideExpression(Error* e, AST* ast);
std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast);
JSValue* EvalCallExpression(Error* e, JSValue* ref, std::vector<JSValue*> arg_list);

Completion EvalProgram(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
  auto prog = static_cast<ProgramOrFunctionBody*>(ast);
  auto statements = prog->statements();
  // 12.9 considered syntactically incorrect if it contains
  //      a return statement that is not within a FunctionBody.
  if (ast->type() != AST::AST_FUNC_BODY) {
    for (auto stmt : statements) {
      if (stmt->type() == AST::AST_STMT_RETURN) {
        e = Error::SyntaxError();
        // TODO(zhuzilin) error object.
        return Completion(Completion::THROW, nullptr, nullptr);
      }
    }
  }

  Completion head_result, tail_result;
  if (statements.size() == 0)
    return Completion();
  for (auto stmt : prog->statements()) {
    if (head_result.IsAbruptCompletion())
      break;
    tail_result = EvalStatement(e, stmt);
    head_result = Completion(
      tail_result.type,
      tail_result.value == nullptr? head_result.value : tail_result.value,
      tail_result.target
    );
  }
  return head_result;
}

Completion EvalReturnStatement(Error* e, AST* ast) {
  log::PrintSource("enter EvalReturnStatement");
  assert(ast->type() == AST::AST_STMT_RETURN);
  Return* return_stmt = static_cast<Return*>(ast);
  if (return_stmt->expr() == nullptr) {
    return Completion(Completion::RETURN, Undefined::Instance(), nullptr);
  }
  auto exp_ref = EvalExpression(e, return_stmt->expr());
  return Completion(Completion::RETURN, GetValue(e, exp_ref), nullptr);
}

Completion EvalStatement(Error* e, AST* ast) {
  switch(ast->type()) {
    case AST::AST_STMT_RETURN:
      return EvalReturnStatement(e, ast);
    case AST::AST_STMT_EMPTY:
      return Completion();
    default:
      return EvalExpressionStatement(e, ast);
  }
}

Completion EvalExpressionStatement(Error* e, AST* ast) {
  JSValue* val = EvalExpression(e, ast);
  return Completion(Completion::NORMAL, val, nullptr);
}

JSValue* EvalExpression(Error* e, AST* ast) {
  JSValue* val;
  switch (ast->type()) {
    case AST::AST_EXPR_IDENT:
      val = EvalIdentifier(ast);
      break;
    case AST::AST_EXPR_NULL:
      val = Null::Instance();
      break;
    case AST::AST_EXPR_BOOL:
      val = ast->source() == u"true" ? Bool::True() : Bool::False();
      break;
    case AST::AST_EXPR_NUMBER:
      val = EvalNumber(ast);
      break;
    case AST::AST_EXPR_BINARY:
      val = EvalBinaryExpression(e, ast);
      break;
    case AST::AST_EXPR_LHS:
      val = EvalLeftHandSideExpression(e, ast);
      break;
    case AST::AST_FUNC:
      val = EvalFunction(ast);
      break;
    default:
      std::cout << ast->type() << " " << AST::AST_ILLEGAL << std::endl;
      assert(false);
  }
  return val;
}

Reference* EvalIdentifier(AST* ast) {
  log::PrintSource("enter EvalIdentifier ", ast->source());
  assert(ast->type() == AST::AST_EXPR_IDENT);
  // 10.3.1 Identifier Resolution
  LexicalEnvironment* env = ExecutionContextStack::Global()->Top().lexical_env();
  // TODO(zhuzilin) strict mode code
  return env->GetIdentifierReference(ast->source(), false);
}

Number* EvalNumber(AST* ast) {
  assert(ast->type() == AST::AST_EXPR_NUMBER);
  auto source = ast->source();
  double val = 0;
  double frac = 1;
  size_t pos = 0;
  bool dot = false;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'.':
        dot = true;
        break;
      case u'e':
      case u'E': {
        double exp = 0;
        bool sign = true;
        pos++;  // skip e/E
        if (c == u'-') {
          sign = false;
          pos++;  // skip -
        } else if (c == u'+') {
          sign = true;
          pos++; // skip +;
        }
        while (pos < source.size()) {
          exp *= 10;
          exp += character::Digit(c);
        }
        if (!sign)
          exp = -exp;
        return new Number(val * pow(10.0, exp));
      }
      case u'x':
      case u'X': {
        assert(val == 0);
        pos++;
        while (pos < source.size()) {
          c = source[pos];
          val *= 16;
          val += character::Digit(c);
          pos++;
        }
        return new Number(val);
      }
      default:
        if (dot) {
          frac /= 10;
          val += character::Digit(c) * frac;
        } else {
          val *= 10;
          val += character::Digit(c);
        }
    }
    pos++;
  }
  return new Number(val);
}

String* EvalString(AST* ast) {
  assert(ast->type() == AST::AST_EXPR_STRING);
  auto source = ast->source();
  source = source.substr(1, source.size() - 2);
  size_t pos = 0;
  std::vector<std::u16string> vals;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'\\': {
        pos++;
        c = source[pos];
        switch (c) {
          case u'b':
            pos++;
            vals.emplace_back(u"\b");
            break;
          case u't':
            pos++;
            vals.emplace_back(u"\t");
            break;
          case u'n':
            pos++;
            vals.emplace_back(u"\n");
            break;
          case u'v':
            pos++;
            vals.emplace_back(u"\v");
            break;
          case u'f':
            pos++;
            vals.emplace_back(u"\f");
            break;
          case u'r':
            pos++;
            vals.emplace_back(u"\r");
            break;
          case u'x': {
            pos++;  // skip 'x'
            char16_t hex = 0;
            for (size_t i = 0; i < 2; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          case u'u': {
            pos++;  // skip 'u'
            char16_t hex = 0;
            for (size_t i = 0; i < 4; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          default:
            c = source[pos];
            if (character::IsLineTerminator(c)) {
              pos++;
              continue;
            }
            pos++;
            vals.emplace_back(std::u16string(1, c));
        }
        break;
      }
      default: {
        size_t start = pos;
        while (true) {
          if (pos == source.size() || source[pos] == u'\\')
            break;
          pos++;
        }
        size_t end = pos;
        auto substr = source.substr(start, end - start);
        vals.emplace_back(std::u16string_view(substr.data(), substr.size()));
      }
    }
  }
  if (vals.size() == 0) {
    return new String(u"");
  } else if (vals.size() == 1) {
    return new String(vals[0]);
  }
  return new String(StrCat(vals));
}

JSValue* EvalBinaryExpression(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_BINARY);
  Binary* b = static_cast<Binary*>(ast);
  if (b->op().source() == u"=") {
    JSValue* lref = EvalLeftHandSideExpression(e, b->lhs());
    if (e != nullptr)
      return nullptr;
    JSValue* rref = EvalExpression(e, b->rhs());
    if (e != nullptr)
      return nullptr;
    JSValue* rval = GetValue(e, rref);
    if (e != nullptr)
      return nullptr;
    if (lref->type() == JSValue::JS_REF) {
      Reference* ref = static_cast<Reference*>(lref);
      if (ref->IsStrictReference() && ref->GetBase()->type() == JSValue::JS_ENV_REC &&
          (ref->GetReferencedName() == u"eval" || ref->GetReferencedName() == u"arguments")) {
        e = Error::SyntaxError();
        return nullptr;
      }
    }
    PutValue(e, lref, rval);
    if (e != nullptr)
      return nullptr;
    return rval;
  }
  assert(false);
}

JSValue* EvalLeftHandSideExpression(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_EXPR_LHS);
  LHS* lhs = static_cast<LHS*>(ast);
  // TODO(zhuzilin) support new
  JSValue* base = EvalExpression(e, lhs->base());
  for (size_t i = 0; i < lhs->order().size(); i++) {
    // Only support call for now;
    auto pair = lhs->order()[i];
    switch (pair.second) {
      case LHS::PostfixType::CALL: {
        auto args = lhs->args_list()[pair.first];
        auto arg_list = EvalArgumentsList(e, args);
        if (e != nullptr)
          return nullptr;
        base = EvalCallExpression(e, base, arg_list);
        if (e != nullptr)
          return nullptr;
        break;
      }
        
      default:
        assert(false);
        break;
    }

  }
  return base;
}

std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast) {
  std::vector<JSValue*> arg_list;
  for (AST* ast : ast->args()) {
    JSValue* ref = EvalExpression(e, ast);
    if (e != nullptr)
      return {};
    JSValue* arg = GetValue(e, ref);
    if (e != nullptr)
      return {};
    arg_list.emplace_back(arg);
  }
  return arg_list;
}

// 11.2.3
JSValue* EvalCallExpression(Error* e, JSValue* ref, std::vector<JSValue*> arg_list) {
  log::PrintSource("enter EvalCallExpression");
  JSValue* val = GetValue(e, ref);
  if (e != nullptr)
    return nullptr;
  if (!val->IsObject()) {  // 4
    e = Error::TypeError();
    return nullptr;
  }
  auto obj = static_cast<JSObject*>(val);
  if (!obj->IsCallable()) {  // 5
    e = Error::TypeError();
    return nullptr;
  }
  auto func = static_cast<FunctionObject*>(obj);
  JSValue* this_value;
  if (ref->IsReference()) {
    Reference* r = static_cast<Reference*>(ref);
    JSValue* base = r->GetBase();
    if (r->IsPropertyReference()) {
      this_value = base;
    } else {
      assert(base->IsEnvironmentRecord());
      auto env_rec = static_cast<EnvironmentRecord*>(base);
      this_value = env_rec->ImplicitThisValue();
    }
  } else {
    this_value = Undefined::Instance();
  }
  return func->Call(e, this_value, arg_list);
}

}  // namespace es

#endif  // ES_EVALUATOR_H