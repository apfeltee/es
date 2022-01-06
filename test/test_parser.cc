#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser.h>
#include <test/helper.h>

typedef std::u16string_view string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

using namespace es;

TEST(TestParser, PrimaryExpressionLiteral) {
  // This
  {
    vec_string sources = {
      u"\n \t this",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_THIS, ast->type());
      EXPECT_EQ(u"this", ast->source());
    }
  }

  // Identifier
  { 
    vec_string sources = {
      u"你好", u"_abcDEF$", u"NULL", u"Null", u"True", u"False",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_IDENT, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Null
  {
    vec_string sources = {
      u"null",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_NULL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Bool
  {
    vec_string sources = {
      u"true", u"false",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_BOOL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Number
  {
    vec_string sources = {
      u"0", u"101", u"0.01", u"12.05", u".8" ,u"0xAbC09",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_NUMBER, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // String
  {
    vec_string sources = {
      u"''", u"'\\n\\b\\u1234\\x12'", u"'😊'",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_STRING, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Regex
  {
    vec_string sources = {
      u"/a/", u"/[a-z]*?/", u"/[012]/g", u"/[012]/$", u"/你好/",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_REGEX, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal Literal
  {
    vec_pair_string sources = {
      {u"for", u"for"}, {u"😊", u"\xD83D"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpressionArray) {
  {
    std::vector<std::pair<string, size_t>> sources = {
      {u"[]", 0}, {u"[,]", 1}, {u"[abc, 123,'string', ]", 3}, {u"[1+2*3, ++a]", 2}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      size_t length = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_ARRAY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto array = static_cast<ArrayLiteral*>(ast);
      EXPECT_EQ(length, array->length());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error messsage is not intuitive.
    vec_pair_string sources = {
      {u"[a,", u""}, {u"[", u""},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpressionObject) {
  {
    vec_string sources = {
      u"{}", u"{a: 1}", u"{in: bed}", u"{1: 1}", u"{\"abc\": 1}"
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_OBJ, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"{a,}", u"{a,"}, {u"{a 1}", u"{a 1"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpressionParentheses) {
  {
    vec_pair_string sources = {
      {u"(a)", u"a"}, {u"(a + b)", u"a + b"}, {u"(a + b, a++)", u"a + b, a++"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      test::PrintSource("source:", source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error message is not intuitive.
    vec_pair_string sources = {
      {u"()", u")"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      test::PrintSource("source:", source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Binary) {
  {
    std::vector<
      std::pair<string,
      std::pair<string, string>>> sources = {
      {u"a + b * c", {u"a", u" b * c"}},
      {u"a * b + c", {u"a * b", u" c"}},
      {u"a * b + + c - d", {u"a * b + + c", u" d"}},
      {u"a++ == b && ++c != d", {u"a++ == b", u" ++c != d"}},
      {u"(1 + 3) * 5 - (8 + 16)", {u"(1 + 3) * 5", u" (8 + 16)"}}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto lhs = pair.second.first;
      auto rhs = pair.second.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(AST::AST_EXPR_BINARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto binary = static_cast<Binary*>(ast);
      EXPECT_EQ(lhs, binary->lhs()->source());
      EXPECT_EQ(rhs, binary->rhs()->source());
    }
  }
}

TEST(TestParser, Unary) {
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a ++", u"a"}, {u"++\na", u"\na"}, {u"++ a", u" a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto node = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(AST::AST_EXPR_UNARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto unary = static_cast<Unary*>(ast);
      EXPECT_EQ(node, unary->node()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a\n++", u"a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_NE(AST::AST_EXPR_UNARY, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, TripleCondition) {
  {
    std::vector<vec_string> sources = {
      {u"a ?b:c", u"a", u"b", u"c"},
      {u"a ?c ? d : e : c", u"a", u"c ? d : e", u" c"},
    };
    for (auto vec : sources) {
      auto source = vec[0];
      Parser parser(vec[0]);
      AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_EQ(AST::AST_EXPR_TRIPLE, ast->type());
      EXPECT_EQ(source, ast->source());
      auto cond = static_cast<TripleCondition*>(ast);
      EXPECT_EQ(vec[1], cond->cond()->source());
      EXPECT_EQ(vec[2], cond->lhs()->source());
      EXPECT_EQ(vec[3], cond->rhs()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a ?b c", u"a ?b"}, {u"a ", u"a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_NE(AST::AST_EXPR_TRIPLE, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, FunctionExpression) {
  // TODO(zhuzilin) Check FunctionBody
  {
    std::vector<std::pair<string,
                          vec_string>> sources = {
      {u"function () {}", {u""}},
      {u"function name (a, b) {}", {u"name", u"a", u"b"}},
      {u"function (a, a, c) {}", {u"", u"a", u"a", u"c"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto params = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseFunctionExpression();
      EXPECT_EQ(AST::AST_EXPR_FUNC, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<Function*>(ast);
      EXPECT_EQ(params[0], func->name().source());
      EXPECT_EQ(params.size() - 1, func->params().size());
      for (size_t i = 0; i < func->params().size(); i++) {
        EXPECT_EQ(params[i + 1], func->params()[i].source());
      }
    }
  }

  // invalid
  {
    std::vector<std::pair<string, string>> sources = {
      {u"function (,) {}", u"function (,"}, {u"function (a a) {}", u"function (a a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseFunctionExpression();
      EXPECT_NE(AST::AST_EXPR_FUNC, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Arguments) {
  {
    std::vector<std::pair<string,
                          vec_string>> sources = {
      {u"()", {}},
      {u"(a)", {u"a"}},
      {u"(a, 1+3, function(){})", {u"a", u" 1+3", u" function(){}"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto args = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseArguments();
      EXPECT_EQ(AST::AST_EXPR_ARGS, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<Arguments*>(ast);
      for (size_t i = 0; i < func->args().size(); i++) {
        EXPECT_EQ(args[i], func->args()[i]->source());
      }
    }
  }
}

TEST(TestParser, LeftHandSide) {
  {
    vec_string sources = {
      u"new Object()", u"function(a, b, c){}(c, d)",
      u"new new a[123 + xyz].__ABC['您好']()()"
    };
    for (auto source : sources) {
      Parser parser(source);
      test::PrintSource("source:", source);
      AST* ast = parser.ParseLeftHandSideExpression();
      test::PrintSource("ast:", ast->source());
      EXPECT_EQ(AST::AST_EXPR_LHS, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }
}

TEST(TestParser, Debugger) {
  {
    vec_pair_string sources = {
      {u"\n \t debugger", u"\n \t debugger"},
      {u"debugger;", u"debugger;"},
      {u"debugger\na", u"debugger"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_DEBUG, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
  // Illegal
  {
    vec_pair_string sources = {
      {u"debugger 1", u"debugger"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Continue) {
  {
    vec_pair_string sources = {
      {u"continue ;", u"continue ;"}, {u"continue a ", u"continue a"},
      {u"continue a ;", u"continue a ;"}, {u"continue \n a ;", u"continue"},
      // NOTE(zhuzilin) This may be wrong...
      {u"continue a \n ;", u"continue a \n ;"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_CONTINUE, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"continue 1", u"continue"}, {u"continue a b", u"continue a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Break) {
  {
    vec_pair_string sources = {
      {u"break ;", u"break ;"}, {u"break a ", u"break a"},
      {u"break a ;", u"break a ;"}, {u"break \n a ;", u"break"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_BREAK, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"break 1", u"break"}, {u"break a b", u"break a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}