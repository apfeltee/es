#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestProgram, SimpleAssign0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Number* num = static_cast<Number*>(GetValue(e, static_cast<Reference*>(res.value)));
    EXPECT_EQ(1, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, SimpleAssign1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1;a=2;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(2, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, CompoundAssign0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1; a+=1; a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(2, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = function(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(3, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"function a(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(3, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1; function b(){return a;}; b()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(1, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call3) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"function c(){return function() { return 10};}; c()()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(10, num->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, CallFunctionContructor) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = Function('return 5'); a()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_NUMBER, res.value->type());
    Number* num = static_cast<Number*>(res.value);
    EXPECT_EQ(5, num->data());
  }
}

TEST(TestProgram, Object0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: 1}; a.a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(1, num->data());
  }
}

TEST(TestProgram, Object1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: {0: 10}}; a.a[0]");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(10, num->data());
  }
}

TEST(TestProgram, Object2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: 136}; a.a = 5; a.a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(5, num->data());
  }
}

TEST(TestProgram, Object3) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {get b() {return this.c}, set b(x) {this.c = x}}; a.b = 5; a.b");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(5, num->data());
  }
}

TEST(TestProgram, New) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"a = new new function() {\n"
      u"  this.a = 12345;\n"
      u"  return function () {this.b=23456}\n"
      u"}\n"
      u"a.b\n"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(23456, num->data());
  }
}

TEST(TestProgram, If) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"a = 1\n"
      u"if (false)\n"
      u"  a = 4\n"
      u"else {a = 2}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(2, num->data());
  }
}

TEST(TestProgram, Strict0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"a = 1"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_REFERENCE, e->type());
  }
}

TEST(TestProgram, Strict1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"a = 235\n"
      u"var a; a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(235, num->data());
  }
}

TEST(TestProgram, Var0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"var a = 147; a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(147, num->data());
  }
}

TEST(TestProgram, While0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"while (a < n) {\n"
      u" a *= 2\n"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(8, num->data());
  }
}

TEST(TestProgram, While1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"while (a < n) {\n"
      u" if (a % 2 == 0) break\n"
      u" a *= 2\n"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(2, num->data());
  }
}

TEST(TestProgram, While2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0, n = 4, sum = 0\n"
      u"while (a < n) {\n"
      u" a += 1\n"
      u" if (a == 2) continue\n"
      u" sum += a\n"
      u"}\n"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(8, num->data());
  }
}

TEST(TestProgram, DoWhile0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 10, n = 5\n"
      u"do {\n"
      u" a *= 2\n"
      u"} while (a < n)\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(20, num->data());
  }
}

TEST(TestProgram, DoWhile1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"do {\n"
      u" if (a % 2 == 0) break\n"
      u" a *= 2\n"
      u"} while (a < n);\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(2, num->data());
  }
}

TEST(TestProgram, DoWhile2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0, n = 4, sum = 0\n"
      u"do {"
      u" a += 1\n"
      u" if (a == 2) continue\n"
      u" sum += a\n"
      u"} while (a < n)\n"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, ref));
    EXPECT_EQ(8, num->data());
  }
}
