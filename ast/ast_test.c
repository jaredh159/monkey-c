#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "../test/test.h"
#include "../parser/parser.h"
#include "../token/token.h"

void test_string()
{
  Token myVarLetToken = {TOKEN_LET, "let"};
  Token myVarIdentToken = {TOKEN_IDENTIFIER, "myVar"};
  Token anotherVarIdentToken = {TOKEN_IDENTIFIER, "anotherVar"};
  Identifier letNameIdent = {&myVarIdentToken, "myVar"};
  Expression letValueExpr = {"anotherVar", EXPRESSION_IDENTIFIER, &anotherVarIdentToken};
  LetStatement letStatement = {&myVarLetToken, &letNameIdent, &letValueExpr};
  Statement statement = {myVarLetToken.literal, STATEMENT_LET, &letStatement};
  Statements statements = {&statement, NULL};
  Program program = {myVarLetToken.literal, &statements};
  assert_str_is(
      program_string(&program),
      "let myVar = anotherVar;\n",
      "hand-constructed AST has correct string representation",
      "test_string");
}

int main(int argc, char **argv)
{
  pass_argv(argc, argv);
  test_string();
  printf("\n");
  return 0;
}
