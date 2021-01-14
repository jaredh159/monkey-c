#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../utils/colors.h"
#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

static Token *current_token = NULL;
static Token *peek_token = NULL;
static Statement *parse_statement();
static Statement *parse_let_statement();
static Statement *parse_return_statement();
static Statement *parse_expression_statement();
static void append_statement(Program *program, Statement *statement);
static void clear_error_stack();
static void no_prefix_parse_fn_error(int token_type);

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
  for (; current_token->type != TOKEN_EOF;)
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
  if (current_token->type == TOKEN_LET)
    return parse_let_statement();
  if (current_token->type == TOKEN_RETURN)
    return parse_return_statement();
  return parse_expression_statement();
}

Expression *parse_expression(int precedence)
{
  PrefixParselet prefix = get_prefix_parselet(current_token->type);
  if (prefix == NULL)
  {
    no_prefix_parse_fn_error(current_token->type);
    return NULL;
  }
  Expression *left_exp = prefix();

  for (; peek_token->type != TOKEN_SEMICOLON && precedence < parser_peek_precedence();)
  {
    InfixParselet infix = get_infix_parselet(parser_peek_token()->type);
    if (infix == NULL)
      return left_exp;

    parser_next_token();
    left_exp = infix(left_exp);
  }
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

  if (peek_token->type == TOKEN_SEMICOLON)
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
  while (current_token->type != TOKEN_SEMICOLON)
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

  if (!parser_expect_peek(TOKEN_IDENTIFIER))
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

  if (!parser_expect_peek(TOKEN_ASSIGN))
    return NULL;

  // TODO, we're skipping the expressions until we encounter a semicolon
  while (current_token->type != TOKEN_SEMICOLON)
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

void parser_next_token()
{
  current_token = peek_token;
  peek_token = lexer_next_token();
}

bool parser_expect_peek(int token_type)
{
  if (peek_token->type == token_type)
  {
    parser_next_token();
    return true;
  }

  char msg[100];
  sprintf(msg, "expected next token to be %s, got %d instead\n", token_type_name(token_type), peek_token->type);
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

static void no_prefix_parse_fn_error(int token_type)
{
  char *err = malloc(200);
  sprintf(err, "no prefix parse function for token type `%s` found\n", token_type_name(token_type));
  parser_push_error(err);
}

int parser_peek_precedence()
{
  return token_precedence(parser_peek_token()->type);
}

int parser_current_precedence()
{
  return token_precedence(parser_current_token()->type);
}

bool parser_peek_token_is(int token_type)
{
  return peek_token->type == token_type;
}

bool parser_current_token_is(int token_type)
{
  return current_token->type == token_type;
}
