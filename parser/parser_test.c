#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../test/test.h"
#include "../ast/ast.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
// TODO: move this elsewhere
#define debug(a, args...) printf("<%s:%d> " a "\n", __FILE__, __LINE__, ##args)
#pragma clang diagnostic push

typedef struct
{
  int type;
  char *ident_val;
  int int_val;
  char *int_literal;
} LitExpTest;

enum LiteralExpressionType
{
  LITERAL_TYPE_INT,
  LITERAL_TYPE_IDENT,
  LITERAL_TYPE_BOOL,
};

static LitExpTest five = {LITERAL_TYPE_INT, "", 5, "5"};
static LitExpTest _true = {LITERAL_TYPE_BOOL, "", (int)true, "true"};
static LitExpTest _false = {LITERAL_TYPE_BOOL, "", (int)false, "false"};

void assert_literal_expression(Expression *expr, LitExpTest *expected, char *test_name);
void check_parser_errors(char *test_name);
void assert_let_statement(Statement *, char *identifier, char *test_name);
void assert_return_statement(Statement *stmt, char *test_name);
void assert_integer_literal(Expression *exp, int value, char *literal, char *test_name);
void assert_identifier(Expression *exp, char *value, char *test_name);
void assert_infix_expression(Expression *expr, LitExpTest left, char *op, LitExpTest right, char *t);
void assert_boolean_literal(Expression *exp, bool value, char *test_name);

void test_parses_identifier_expression()
{
  char *t = "parses_identifier_expression";
  Program *program = parse_program("foobar;");
  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);

  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "statement is expression", t);

  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_int_is(exp->type, EXPRESSION_IDENTIFIER, "expression is identifier", t);
  assert_str_is(exp->token_literal, "foobar", "expression token_literal is foobar", t);
  Identifier *ident = exp->node;
  assert_str_is(ident->value, "foobar", "identifier value is foobar", t);
  assert_str_is(ident->token->literal, "foobar", "identifier->token->literal is foobar", t);
}

void test_parses_one_let_statement()
{
  char *t = "parses_one_let_statement";
  Program *program = parse_program("let x = 5;");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);

  Statement *stmt = program->statements->statement;
  assert_let_statement(stmt, "x", t);
}

void test_parses_multiple_let_statements()
{
  char *t = "parses_multiple_let_statements";
  Statement *stmt;
  Program *program = parse_program(
      "let x = 5;\n"
      "let y = 10;\n"
      "let foobar = 838383;");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(3, num_statements(program->statements), "program has 3 statements", t);

  stmt = program->statements->statement;
  assert_let_statement(stmt, "x", t);
  stmt = program->statements->next->statement;
  assert_let_statement(stmt, "y", t);
  stmt = program->statements->next->next->statement;
  assert_let_statement(stmt, "foobar", t);
}

void test_parses_return_statements()
{
  char *t = "parses_return_statements";
  Program *program = parse_program(
      "return 5;\n"
      "return 10;\n"
      "return 993322;\n");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(3, num_statements(program->statements), "program has 3 statements", t);

  assert_return_statement(program->statements->statement, t);
  assert_return_statement(program->statements->next->statement, t);
  assert_return_statement(program->statements->next->next->statement, t);
}

void test_parses_integer_literal_expression()
{
  char *t = "parses_integer_literal_expression";
  Program *program = parse_program("5;");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_integer_literal(exp, 5, "5", t);
}

void test_parses_boolean_literal_expression()
{
  char *t = "parses_boolean_literal_expression";
  Program *program = parse_program("true;");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_boolean_literal(exp, true, t);
}

void test_parses_prefix_expressions()
{
  char *t = "parses_prefix_expressions";
  typedef struct
  {
    char *input;
    char *operator;
    LitExpTest value;
  } PrefixTest;

  PrefixTest tests[] = {
      {"!true;", "!", _true},
      {"!false;", "!", _false},
      {"!5;", "!", five},
      {"-15;", "-", {LITERAL_TYPE_INT, "", 15, "15"}}};

  for (int i = 0; i < 2; i++)
  {
    PrefixTest test = tests[i];
    Program *program = parse_program(test.input);
    if (program == NULL)
      fail("parse_program() returned NULL", t);

    check_parser_errors(t);
    assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
    Statement *stmt = program->statements->statement;
    assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
    ExpressionStatement *es = get_expression(stmt);
    Expression *exp = es->expression;
    assert_int_is(exp->type, EXPRESSION_PREFIX, "expression type is PREFIX", t);
    PrefixExpression *prefix = exp->node;
    assert_str_is(test.operator, prefix->operator, str_embed("operator is %s", test.operator), t);
    assert_literal_expression(prefix->right, &test.value, t);
  }
}

void test_parses_infix_expressions()
{
  char *t = "parses_infix_expressions";
  typedef struct
  {
    char *input;
    LitExpTest left;
    char *operator;
    LitExpTest right;
  } InfixTest;

  InfixTest tests[] = {
      {"true == true", _true, "==", _true},
      {"true != false", _true, "!=", _false},
      {"false == false", _false, "==", _false},
      {"5 + 5;", five, "+", five},
      {"5 - 5;", five, "-", five},
      {"5 * 5;", five, "*", five},
      {"5 / 5;", five, "/", five},
      {"5 > 5;", five, ">", five},
      {"5 < 5;", five, "<", five},
      {"5 == 5;", five, "==", five},
      {"5 != 5;", five, "!=", five},
  };

  int num_tests = sizeof(tests) / sizeof(InfixTest);
  for (int i = 0; i < num_tests; i++)
  {
    InfixTest test = tests[i];
    Program *program = parse_program(test.input);
    if (program == NULL)
      fail("parse_program() returned NULL", t);

    check_parser_errors(t);
    assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
    Statement *stmt = program->statements->statement;
    assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
    ExpressionStatement *es = get_expression(stmt);
    Expression *exp = es->expression;

    assert_infix_expression(exp, test.left, test.operator, test.right, t);
  }
}

void test_operator_precedence_parsing()
{
  char *t = "operator_precedence_parsing";
  typedef struct
  {
    char *input;
    char *expected;
  } PrecedenceTest;

  PrecedenceTest tests[] = {
      {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
      {"(5 + 5) * 2", "((5 + 5) * 2)"},
      {"2 / (5 + 5)", "(2 / (5 + 5))"},
      {"-(5 + 5)", "(-(5 + 5))"},
      {"!(true == true)", "(!(true == true))"},
      {"true", "true"},
      {"false", "false"},
      {"3 > 5 == false", "((3 > 5) == false)"},
      {"3 > 5 == true", "((3 > 5) == true)"},
      {"-a * b", "((-a) * b)"},
      {"!-a", "(!(-a))"},
      {"a + b + c", "((a + b) + c)"},
      {"a * b * c", "((a * b) * c)"},
      {"a * b / c", "((a * b) / c)"},
      {"a + b / c", "(a + (b / c))"},
      {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
      {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
      {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
      {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
      {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"}};
  int num_tests = sizeof(tests) / sizeof(PrecedenceTest);

  for (int i = 0; i < num_tests; i++)
  {
    PrecedenceTest test = tests[i];
    Program *program = parse_program(test.input);
    if (program == NULL)
      fail("parse_program() returned NULL", t);

    check_parser_errors(t);
    char *actual = program_string(program);
    assert_str_is(test.expected, actual, "program string is correct", t);
  }
}

void test_parses_if_expression()
{
  char *t = "parses_if_expression";
  Program *program = parse_program("if (x < y) { x }");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert(exp->type == EXPRESSION_IF, "expression is IF", t);
  IfExpression *if_exp = exp->node;
  LitExpTest x = {LITERAL_TYPE_IDENT, "x", 0, ""};
  LitExpTest y = {LITERAL_TYPE_IDENT, "y", 0, ""};
  assert_infix_expression(if_exp->condition, x, "<", y, t);
  assert_int_is(1, num_statements(if_exp->consequence->statements), "consequence has 1 statement", t);
  Statement *consequence = if_exp->consequence->statements->statement;
  assert(consequence->type == STATEMENT_EXPRESSION, "first consequence statement is expression", t);
  ExpressionStatement *cq_exp = get_expression(consequence);
  assert_identifier(cq_exp->expression, "x", t);
  assert(if_exp->alternative == NULL, "alternative is NULL", t);
}

void test_parses_if_else_expression()
{
  char *t = "parses_if_else_expression";
  Program *program = parse_program("if (x < y) { x } else { y }");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_statements(program->statements), "program has 1 statement", t);
  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert(exp->type == EXPRESSION_IF, "expression is IF", t);
  IfExpression *if_exp = exp->node;
  LitExpTest x = {LITERAL_TYPE_IDENT, "x", 0, ""};
  LitExpTest y = {LITERAL_TYPE_IDENT, "y", 0, ""};
  assert_infix_expression(if_exp->condition, x, "<", y, t);

  // test consequence
  assert_int_is(1, num_statements(if_exp->consequence->statements), "consequence has 1 statement", t);
  Statement *consequence = if_exp->consequence->statements->statement;
  assert(consequence->type == STATEMENT_EXPRESSION, "first consequence statement is expression", t);
  ExpressionStatement *cq_exp = get_expression(consequence);
  assert_identifier(cq_exp->expression, "x", t);

  // test alternative
  assert_int_is(1, num_statements(if_exp->alternative->statements), "alternative has 1 statement", t);
  Statement *alternative = if_exp->alternative->statements->statement;
  assert(alternative->type == STATEMENT_EXPRESSION, "first alternative statement is expression", t);
  ExpressionStatement *at_exp = get_expression(alternative);
  assert_identifier(at_exp->expression, "y", t);
}

int main(int argc, char **argv)
{
  pass_argv(argc, argv);
  test_parses_if_expression();
  test_parses_if_else_expression();
  test_parses_boolean_literal_expression();
  test_operator_precedence_parsing();
  test_parses_infix_expressions();
  test_parses_prefix_expressions();
  test_parses_identifier_expression();
  test_parses_return_statements();
  test_parses_one_let_statement();
  test_parses_multiple_let_statements();
  test_parses_integer_literal_expression();
  printf("\n");
  return 0;
}

void assert_let_statement(Statement *stmt, char *identifier, char *test_name)
{
  assert_str_is(
      stmt->token_literal,
      "let",
      "statement->token_literal is \"let\"",
      test_name);

  assert(stmt->type == STATEMENT_LET, "statement TYPE is `let`", test_name);

  char msg[50];
  sprintf(msg, "identifier is \"%s\"", identifier);

  assert_str_is(get_let(stmt)->name->value, identifier, msg, test_name);
}

void assert_return_statement(Statement *stmt, char *test_name)
{
  assert_str_is(
      stmt->token_literal,
      "return",
      "statement->token_literal is \"return\"",
      test_name);

  assert(stmt->type == STATEMENT_RETURN, "statement TYPE is `return`", test_name);
}

void check_parser_errors(char *test_name)
{
  if (!parser_has_error())
    return;

  parser_print_errors();
  char msg[50];
  sprintf(msg, "parser had %i errors\n", parser_num_errors());
  fail(msg, test_name);
}

void assert_integer_literal(Expression *exp, int value, char *literal, char *test_name)
{
  char msg[50];
  assert_int_is(exp->type, EXPRESSION_INTEGER_LITERAL, "expression is integer literal", test_name);
  IntegerLiteral *int_literal = exp->node;

  sprintf(msg, "int literal is int %d", value);
  assert_int_is(value, int_literal->value, msg, test_name);

  sprintf(msg, "int_literal->token->literal is \"%s\"", literal);
  assert_str_is(literal, int_literal->token->literal, msg, test_name);
}

void assert_boolean_literal(Expression *exp, bool value, char *test_name)
{
  assert_int_is(exp->type, EXPRESSION_BOOLEAN_LITERAL, "expression is boolean literal", test_name);
  BooleanLiteral *bool_literal = exp->node;
  assert_int_is((int)value, bool_literal->value, "boolean is correct", test_name);
  assert_str_is(value ? "true" : "false", bool_literal->token->literal, "boolean token literal correct", test_name);
}

void assert_identifier(Expression *exp, char *value, char *test_name)
{
  assert_int_is(EXPRESSION_IDENTIFIER, exp->type, "expression is identifier", test_name);
  Identifier *ident = exp->node;
  assert_str_is(value, ident->value, "identifier value is correct", test_name);
  assert_str_is(value, ident->token->literal, "identifier token literal is correct", test_name);
}

void assert_literal_expression(Expression *expr, LitExpTest *expected, char *test_name)
{
  switch (expected->type)
  {
  case LITERAL_TYPE_BOOL:
    assert_boolean_literal(expr, (bool)expected->int_val, test_name);
    return;
  case LITERAL_TYPE_INT:
    assert_integer_literal(expr, expected->int_val, expected->int_literal, test_name);
    return;
  case LITERAL_TYPE_IDENT:
    assert_identifier(expr, expected->ident_val, test_name);
    return;
  }
  fail("literal expression type not handled", test_name);
}

void assert_infix_expression(
    Expression *expr,
    LitExpTest left,
    char *operand,
    LitExpTest right,
    char *test_name)
{
  assert_int_is(EXPRESSION_INFIX, expr->type, "expression is infix", test_name);
  InfixExpression *infix = expr->node;
  assert_literal_expression(infix->left, &left, test_name);
  assert_str_is(operand, infix->operator, "operand is correct", test_name);
  assert_literal_expression(infix->right, &right, test_name);
}
