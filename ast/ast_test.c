#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "../test/test.h"
#include "../token/token.h"

void test_string()
{
  Token myVarLetToken = {TOKEN_LET, "let"};
  Token myVarIdentToken = {TOKEN_IDENTIFIER, "myVar"};
  Token anotherVarIdentToken = {TOKEN_IDENTIFIER, "anotherVar"};
  Identifier letNameIdent = {&myVarIdentToken, "myVar"};
  Expression letValueExpr = {"anotherVar", 1, &anotherVarIdentToken};
  LetStatement letStatement = {&myVarLetToken, &letNameIdent, &letValueExpr};
  Statement statement = {myVarLetToken.literal, {1, 0, 0}, &letStatement};
  Statements statements = {&statement, NULL};
  Program program = {myVarLetToken.literal, &statements};
  assert_str_is(
      program_string(&program),
      "let myVar = anotherVar;\n",
      "hand-constructed AST as correct string representation",
      "test_string");
}

int main(void)
{
  test_string();
  return 0;
}
