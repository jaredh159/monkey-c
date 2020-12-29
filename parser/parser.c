#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

static Token *current_token = NULL;
static Token *peek_token = NULL;
static void parser_next_token();
static Statement *parse_statement();
static Statement *parse_let_statement();
static bool is_token_type(Token *token, char *token_type);
static bool expect_peek(char *token_type);
static void append_statement(Program *program, Statement *statement);

Program *parse_program(char *input)
{
  Program *program = malloc(sizeof(Program));
  if (program == NULL)
    return program;

  // set up lexer & initial tokens
  lexer_set(input);
  parser_next_token();
  parser_next_token();

  program->statements = NULL;

  Statement *statement;
  for (; !is_token_type(current_token, TOKEN_EOF);)
  {
    statement = parse_statement();
    if (statement != NULL)
      append_statement(program, statement);
    parser_next_token();
  }
  // TODO, setup program->token_literal (see top of p 44)

  return program;
}

Statement *parse_statement()
{
  if (is_token_type(current_token, TOKEN_LET))
    return parse_let_statement();
  return NULL;
}

Statement *parse_let_statement()
{
  Statement *statement = malloc(sizeof(Statement));
  statement->token_literal = current_token->literal;
  if (statement == NULL)
    return NULL;

  if (!expect_peek(TOKEN_IDENTIFIER))
    return NULL;

  LetStatement *let_statement = malloc(sizeof(LetStatement));
  Identifier *name = malloc(sizeof(Identifier));
  Expression *value = malloc(sizeof(Expression));
  if (let_statement == NULL || name == NULL || value == NULL)
    return NULL;

  name->token = current_token;
  name->value = current_token->literal;
  let_statement->token = current_token;
  let_statement->name = name;
  let_statement->value = value;
  statement->let_statement = let_statement;
  statement->type.is_statement = true;
  statement->type.is_expression = false;

  if (!expect_peek(TOKEN_ASSIGN))
    return NULL;

  // TODO, we're skipping the expressions until we encounter a semicolon
  while (!is_token_type(current_token, TOKEN_SEMICOLON))
    parser_next_token();

  return statement;
}

static void append_statement(Program *program, Statement *statement)
{
  Statements *node = malloc(sizeof(Statements));
  node->statement = statement;
  node->next = NULL;

  if (program->statements == NULL)
  {
    program->statements = node;
    return;
  }

  Statements *current = program->statements;
  while (current->next != NULL)
    current = current->next;
  current->next = node;
}

static void parser_next_token()
{
  current_token = peek_token;
  peek_token = lexer_next_token();
}

static bool is_token_type(Token *token, char *type)
{
  if (token == NULL)
    return false;
  return strcmp(token->type, type) == 0;
}

static bool expect_peek(char *token_type)
{
  if (is_token_type(peek_token, token_type))
  {
    parser_next_token();
    return true;
  }
  return false;
}
