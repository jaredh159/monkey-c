#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
static char *expression_string(Expression *exp);

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
  printf("  address: %p\n", (void *)program);
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
  printf("%s  address: %p\n", indent, (void *)statement);
  printf("%s  type: %s\n", indent, statement->type.is_expression ? "expression" : "let");
  printf("%s  token_literal: \"%s\"\n", indent, statement->token_literal);
  if (statement->type.is_let)
  {
    LetStatement *ls = (LetStatement *)(statement->node);
    printf("%s  let_statement->identifier: \"%s\"\n", indent, ls->name->value);
  }
  printf("%s}\n", indent);
}

void print_expression(Expression *expression)
{
  printf("expression pointer: %p\n", (void *)expression);
}

void print_identifier(Identifier *identifier)
{
  printf("identifier pointer: %p\n", (void *)identifier);
}

void statement_invariant(Statement *statement, bool type_predicate, char *type)
{
  if (type_predicate)
    return;
  printf(COLOR_RED "statement not of required type `%s`\n" COLOR_RESET, type);
  print_statement(statement);
  exit(1);
}

ReturnStatement *get_return(Statement *statement)
{
  statement_invariant(statement, statement->type.is_return, "return");
  return (ReturnStatement *)statement->node;
}

LetStatement *get_let(Statement *statement)
{
  statement_invariant(statement, statement->type.is_let, "let");
  return (LetStatement *)statement->node;
}

ExpressionStatement *get_expression(Statement *statement)
{
  statement_invariant(statement, statement->type.is_expression, "expression");
  return (ExpressionStatement *)statement->node;
}

#define MAX_STMT_STR_LEN 100

char *program_string(Program *program)
{
  int num_statements = num_program_statements(program);
  char *prog_str = malloc(MAX_STMT_STR_LEN * num_statements);
  prog_str[0] = '\0';
  if (num_statements == 0)
    return prog_str;

  Statements *current = program->statements;
  for (current = program->statements; current != NULL; current = current->next)
    if (current->statement != NULL)
      strcat(prog_str, statement_string(current->statement));

  return prog_str;
}

char *statement_string(Statement *statement)
{
  if (statement->type.is_let)
    return let_statement_string(statement->node);
  else if (statement->type.is_return)
    return return_statement_string(statement->node);
  else
    return expression_statement_string(statement->node);
}

char *let_statement_string(LetStatement *ls)
{
  char *let_str = malloc(MAX_STMT_STR_LEN);
  sprintf(let_str,
          "%s %s = %s;\n",
          ls->token->literal,
          ls->name->value,
          expression_string(ls->value));
  return let_str;
}

char *return_statement_string(ReturnStatement *rs)
{
  char *ret_str = malloc(MAX_STMT_STR_LEN);
  sprintf(ret_str,
          "%s %s;\n",
          rs->token->literal,
          expression_string(rs->return_value));
  return ret_str;
}

char *expression_statement_string(ExpressionStatement *es)
{
  return identifier_string(es->identifier);
}

char *identifier_string(Identifier *identifier)
{
  return identifier->value;
}

// TODO... is this necessary any more?
static char *expression_string(Expression *exp)
{
  char *exp_str = malloc(MAX_STMT_STR_LEN);
  sprintf(exp_str, "%s", exp->token_literal);
  return exp_str;
}
