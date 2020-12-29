#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../test/test.h"
#include "../ast/ast.h"

// TODO: move this elsewhere
#define debug(a, args...) printf("<%s:%d> " a "\n", __FILE__, __LINE__, ##args)

void assert_let_statement(Statement *, char *, bool, bool, char *, char *);

void test_parses_one_let_statement()
{
  char *t = "parses_one_let_statement";
  Program *program = parse_program("let x = 5;");
  if (program == NULL)
    fail("parse_program() returned NULL", t);

  assert(num_program_statements(program) == 1, "program has 1 statement", t);

  Statement *stmt = program->statements->statement;
  assert_let_statement(stmt, "let", true, false, "x", t);
}

void test_parses_multiple_let_statements()
{
  char *t = "parses_multiple_let_statements";
  Statement *stmt;
  Program *program = parse_program("let x = 5;\nlet y = 10;\nlet foobar = 838383;");

  if (program == NULL)
    fail("parse_program() returned NULL", t);

  assert(num_program_statements(program) == 3, "program has 3 statements", t);

  stmt = program->statements->statement;
  assert_let_statement(stmt, "let", true, false, "x", t);
  stmt = program->statements->next->statement;
  assert_let_statement(stmt, "let", true, false, "y", t);
  stmt = program->statements->next->next->statement;
  assert_let_statement(stmt, "let", true, false, "foobar", t);
}

int main(void)
{
  test_parses_one_let_statement();
  test_parses_multiple_let_statements();
  return 0;
}

void assert_let_statement(
    Statement *stmt,
    char *token_literal,
    bool is_statement,
    bool is_expression,
    char *identifier_value,
    char *test_name)
{
  char msg[50];

  sprintf(msg, "statement->token_literal is \"%s\"", token_literal);
  assert_str_is(stmt->token_literal, token_literal, msg, test_name);

  sprintf(msg, "statement TYPE is %sstatement", is_statement ? "" : "NOT ");
  assert(stmt->type.is_statement == is_statement, msg, test_name);

  sprintf(msg, "statement TYPE is %sexpression", is_expression ? "" : "NOT ");
  assert(stmt->type.is_expression == is_expression, msg, test_name);

  sprintf(msg, "identifier is \"%s\"", identifier_value);
  assert_str_is(stmt->let_statement->name->value, identifier_value, msg, test_name);
}
