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

void check_parser_errors(char *test_name);
void assert_let_statement(Statement *, char *identifier, char *test_name);
void assert_return_statement(Statement *stmt, char *test_name);

void test_parses_identifier_expression()
{
  char *t = "parses_identifier_expression";
  Program *program = parse_program("foobar;");
  if (program == NULL)
    fail("parse_program() returned NULL", t);

  check_parser_errors(t);
  assert_int_is(1, num_program_statements(program), "program has 1 statement", t);

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
  assert_int_is(1, num_program_statements(program), "program has 1 statement", t);

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
  assert_int_is(3, num_program_statements(program), "program has 3 statements", t);

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
  assert_int_is(3, num_program_statements(program), "program has 3 statements", t);

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
  assert_int_is(1, num_program_statements(program), "program has 1 statement", t);
  Statement *stmt = program->statements->statement;
  assert(stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_int_is(exp->type, EXPRESSION_INTEGER_LITERAL, "expression is integer literal", t);
  IntegerLiteral *int_literal = exp->node;
  assert_int_is(5, int_literal->value, "int literal is int 5", t);
  assert_str_is("5", int_literal->token->literal, "int_literal->token->literal is \"5\"", t);
}

int main(int argc, char **argv)
{
  pass_argv(argc, argv);
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
