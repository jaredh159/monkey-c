#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../colors.h"
#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

static Token *current_token = NULL;
static Token *peek_token = NULL;
static void parser_next_token();
static Statement *parse_statement();
static Statement *parse_let_statement();
static Statement *parse_return_statement();
static Statement *parse_expression_statement();
static bool is_token_type(Token *token, char *token_type);
static bool expect_peek(char *token_type);
static void append_statement(Program *program, Statement *statement);
static void clear_error_stack();

Program *parse_program(char *input)
{
  clear_error_stack();

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
  if (is_token_type(current_token, TOKEN_RETURN))
    return parse_return_statement();
  return parse_expression_statement();
}

Expression *parse_expression(int precedence)
{
  PrefixParselet prefix = get_prefix_parselet(current_token->type);
  if (prefix == NULL)
    return NULL;
  Expression *left_exp = prefix();
  return left_exp;
}

Statement *parse_expression_statement()
{
  Statement *statement = malloc(sizeof(Statement));
  Token *initial_token = current_token;
  statement->token_literal = initial_token->literal;
  if (statement == NULL)
    return NULL;

  ExpressionStatement *expression_statement = malloc(sizeof(ExpressionStatement));
  if (expression_statement == NULL)
    return NULL;

  expression_statement->token = current_token;
  Expression *expression = parse_expression(PRECEDENCE_LOWEST);
  if (expression == NULL)
    return NULL;

  expression_statement->expression = expression;
  statement->node = expression_statement;
  statement->type = STATEMENT_EXPRESSION;

  if (is_token_type(peek_token, TOKEN_SEMICOLON))
    parser_next_token();

  return statement;
}

Statement *parse_return_statement()
{
  Statement *statement = malloc(sizeof(Statement));
  Token *initial_token = current_token;
  statement->token_literal = initial_token->literal;
  if (statement == NULL)
    return NULL;

  ReturnStatement *return_statement = malloc(sizeof(ReturnStatement));
  Expression *return_value = malloc(sizeof(Expression));
  if (return_statement == NULL || return_value == NULL)
    return NULL;

  return_statement->token = initial_token;
  return_statement->return_value = return_value;
  statement->node = return_statement;
  statement->type = STATEMENT_RETURN;

  // move past return token
  parser_next_token();

  // skip parsing expression for now
  while (!is_token_type(current_token, TOKEN_SEMICOLON))
    parser_next_token();

  return statement;
}

Statement *parse_let_statement()
{
  Statement *statement = malloc(sizeof(Statement));
  Token *initial_token = current_token;
  statement->token_literal = initial_token->literal;
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
  let_statement->token = initial_token;
  let_statement->name = name;
  let_statement->value = value;
  statement->node = let_statement;
  statement->type = STATEMENT_LET;

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

  char msg[100];
  sprintf(msg, "expected next token to be %s, got %s instead\n", token_type, peek_token->type);
  parser_push_error(msg);
  return false;
}

#define MAX_ERRORS 50
static char *errors[MAX_ERRORS];
static int error_index = 0;

void parser_push_error(char *error_msg)
{
  if (error_index < MAX_ERRORS)
  {
    errors[error_index] = strdup(error_msg);
    error_index += 1;
  }
}

bool parser_has_error()
{
  return error_index > 0;
}

void parser_print_errors()
{
  for (int i = 0; i < error_index; i++)
    printf(COLOR_RED "  -> PARSE ERROR! %s" COLOR_RESET, errors[i]);
}

int parser_num_errors()
{
  return error_index;
}

static void clear_error_stack()
{
  error_index = 0;
}

Token *parser_current_token()
{
  return current_token;
}

Token *parser_peek_token()
{
  return peek_token;
}
