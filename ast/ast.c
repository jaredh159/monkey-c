#include <stdio.h>
#include "ast.h"
#include "../colors.h"

#define INDENT_0 ""
#define INDENT_2 "  "
#define INDENT_4 "    "
#define INDENT_6 "      "
#define INDENT_8 "        "

void print_statement_indented(Statement *statement, char *indent);
void print_expression_indented(Expression *expression, char *indent);
void print_identifier_indented(Identifier *identifier, char *indent);
int num_program_statements(Program *program);

int num_program_statements(Program *program)
{
  int num_statements;
  Statements *current = program->statements;
  for (num_statements = 0; current != NULL; num_statements++)
    current = current->next;
  return num_statements;
}

void print_program(Program *program)
{
  int num_statements = num_program_statements(program);
  printf(COLOR_GREY "Program {\n");
  printf("  address: %p\n", program);
  printf("  num_statements: %d\n", num_statements);
  if (num_statements == 0)
    printf("  statements: []\n");
  else
  {
    printf("  statements: [\n");
    Statements *current = program->statements;
    for (current = program->statements; current != NULL; current = current->next)
      if (current->statement != NULL)
        print_statement_indented(current->statement, INDENT_4);
    printf("  ]\n");
  }
  printf("}\n" COLOR_RESET);
}

void print_statement(Statement *statement)
{
  print_statement_indented(statement, INDENT_0);
}

void print_statement_indented(Statement *statement, char *indent)
{
  printf("%sStatement {\n", indent);
  printf("%s  address: %p\n", indent, statement);
  printf("%s  type: %s\n", indent, statement->type.is_expression ? "expression" : "let");
  printf("%s  token_literal: \"%s\"\n", indent, statement->token_literal);
  if (statement->type.is_statement)
  {
    printf("%s  let_statement->identifier: \"%s\"\n", indent, statement->let_statement->name->value);
  }
  printf("%s}\n", indent);
}

void print_expression(Expression *expression)
{
  printf("expression pointer: %p\n", expression);
}

void print_identifier(Identifier *identifier)
{
  printf("identifier pointer: %p\n", identifier);
}
