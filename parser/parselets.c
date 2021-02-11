#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Expression *parse_identifier() {
  Expression *exp = malloc(sizeof(Expression));
  Identifier *ident = malloc(sizeof(Identifier));
  if (exp == NULL || ident == NULL)
    return NULL;

  ident->token = parser_current_token();
  ident->value = parser_current_token()->literal;

  exp->token_literal = parser_current_token()->literal;
  exp->type = EXPRESSION_IDENTIFIER;
  exp->node = ident;
  return exp;
}

Expression *parse_integer_literal() {
  Expression *exp = malloc(sizeof(Expression));
  IntegerLiteral *int_literal = malloc(sizeof(IntegerLiteral));
  if (exp == NULL || int_literal == NULL)
    return NULL;

  char *token_literal = parser_current_token()->literal;
  int value = atoi(token_literal);
  if (value == 0 && !str_is(token_literal, "0")) {
    char *err_msg_fmt = "could not parse %s as an integer";
    char err_msg[50];
    sprintf(err_msg, err_msg_fmt, token_literal);
    parser_push_error(err_msg);
    return NULL;
  }

  int_literal->token = parser_current_token();
  int_literal->value = value;
  exp->token_literal = token_literal;
  exp->type = EXPRESSION_INTEGER_LITERAL;
  exp->node = int_literal;
  return exp;
}

Expression *parse_string_literal(void) {
  Expression *exp = malloc(sizeof(Expression));
  StringLiteral *str_literal = malloc(sizeof(StringLiteral));
  if (exp == NULL || str_literal == NULL)
    return NULL;

  char *token_literal = parser_current_token()->literal;
  char *value = parser_current_token()->literal;
  str_literal->token = parser_current_token();
  str_literal->value = value;
  exp->token_literal = token_literal;
  exp->type = EXPRESSION_STRING_LITERAL;
  exp->node = str_literal;
  return exp;
}

Expression *parse_boolean_literal() {
  Expression *exp = malloc(sizeof(Expression));
  BooleanLiteral *bool_literal = malloc(sizeof(BooleanLiteral));
  if (exp == NULL || bool_literal == NULL)
    return NULL;

  char *token_literal = parser_current_token()->literal;
  int value = parser_current_token()->type == TOKEN_TRUE;
  bool_literal->token = parser_current_token();
  bool_literal->value = value;
  exp->token_literal = token_literal;
  exp->type = EXPRESSION_BOOLEAN_LITERAL;
  exp->node = bool_literal;
  return exp;
}

Expression *parse_prefix_expression() {
  Expression *exp = malloc(sizeof(Expression));
  PrefixExpression *prefix = malloc(sizeof(PrefixExpression));
  if (exp == NULL || prefix == NULL)
    return NULL;

  Token *initial_token = parser_current_token();
  prefix->token = initial_token;
  prefix->operator= initial_token->literal;
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_PREFIX;
  exp->node = prefix;

  parser_next_token();
  prefix->right = parse_expression(PRECEDENCE_PREFIX);
  return exp;
}

Expression *parse_call_expression(Expression *fn) {
  Token *initial_token = parser_current_token();
  Expression *exp = malloc(sizeof(Expression));
  CallExpression *ce = malloc(sizeof(CallExpression));
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_CALL;
  exp->node = ce;
  ce->fn = fn;
  ce->token = initial_token;
  ce->arguments = parse_expression_list(TOKEN_RIGHT_PAREN);
  return exp;
}

Expression *parse_array_literal(void) {
  Token *initial_token = parser_current_token();
  Expression *exp = malloc(sizeof(Expression));
  ArrayLiteral *array_lit = malloc(sizeof(ArrayLiteral));
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_ARRAY_LITERAL;
  exp->node = array_lit;
  array_lit->token = initial_token;
  array_lit->elements = parse_expression_list(TOKEN_RIGHT_BRACKET);
  return exp;
}

Expression *parse_index_expression(Expression *left) {
  Expression *exp = malloc(sizeof(Expression));
  IndexExpression *ie = malloc(sizeof(IndexExpression));
  if (exp == NULL || ie == NULL)
    return NULL;

  Token *initial_token = parser_current_token();
  ie->token = initial_token;
  ie->left = left;

  parser_next_token();
  ie->index = parse_expression(PRECEDENCE_LOWEST);
  if (!parser_expect_peek(TOKEN_RIGHT_BRACKET)) {
    return NULL;
  }

  exp->type = EXPRESSION_INDEX;
  exp->token_literal = initial_token->literal;
  exp->node = ie;
  return exp;
}

Expression *parse_infix_expression(Expression *left) {
  Expression *exp = malloc(sizeof(Expression));
  InfixExpression *infix = malloc(sizeof(InfixExpression));
  if (exp == NULL || infix == NULL)
    return NULL;

  Token *initial_token = parser_current_token();
  infix->token = initial_token;
  infix->operator= initial_token->literal;
  infix->left = left;
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_INFIX;
  exp->node = infix;
  int precedence = parser_current_precedence();
  parser_next_token();
  infix->right = parse_expression(precedence);
  return exp;
}

Expression *parse_grouped_expression() {
  parser_next_token();
  Expression *exp = parse_expression(PRECEDENCE_LOWEST);
  if (!parser_expect_peek(TOKEN_RIGHT_PAREN)) {
    return NULL;
  }
  return exp;
}

Expression *parse_if_expression() {
  Token *initial_token = parser_current_token();
  Expression *exp = malloc(sizeof(Expression));
  IfExpression *if_exp = malloc(sizeof(IfExpression));
  if (exp == NULL || if_exp == NULL)
    return NULL;

  if (!parser_expect_peek(TOKEN_LEFT_PAREN))
    return NULL;

  parser_next_token();
  if_exp->condition = parse_expression(PRECEDENCE_LOWEST);

  if (!parser_expect_peek(TOKEN_RIGHT_PAREN))
    return NULL;

  if (!parser_expect_peek(TOKEN_LEFT_BRACE))
    return NULL;

  if_exp->consequence = parse_block_statement();

  if (parser_peek_token_is(TOKEN_ELSE)) {
    parser_next_token();
    if (!parser_expect_peek(TOKEN_LEFT_BRACE))
      return NULL;

    if_exp->alternative = parse_block_statement();
  }

  if_exp->token = initial_token;
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_IF;
  exp->node = if_exp;
  return exp;
}

Expression *parse_function_literal() {
  Token *initial_token = parser_current_token();
  Expression *exp = malloc(sizeof(Expression));
  FunctionLiteral *fn = malloc(sizeof(FunctionLiteral));
  if (exp == NULL || fn == NULL)
    return NULL;

  if (!parser_expect_peek(TOKEN_LEFT_PAREN))
    return NULL;

  fn->parameters = parse_function_parameters();

  if (!parser_expect_peek(TOKEN_LEFT_BRACE))
    return NULL;

  fn->body = parse_block_statement();

  fn->token = initial_token;
  exp->token_literal = initial_token->literal;
  exp->type = EXPRESSION_FUNCTION_LITERAL;
  exp->node = fn;
  return exp;
}

PrefixParselet get_prefix_parselet(int token_type) {
  switch (token_type) {
    case TOKEN_FUNCTION:
      return parse_function_literal;
    case TOKEN_IF:
      return parse_if_expression;
    case TOKEN_LEFT_PAREN:
      return parse_grouped_expression;
    case TOKEN_IDENTIFIER:
      return parse_identifier;
    case TOKEN_INTEGER:
      return parse_integer_literal;
    case TOKEN_STRING:
      return parse_string_literal;
    case TOKEN_MINUS:
    case TOKEN_BANG:
      return parse_prefix_expression;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
      return parse_boolean_literal;
    case TOKEN_LEFT_BRACKET:
      return parse_array_literal;
  }
  return NULL;
}

InfixParselet get_infix_parselet(int token_type) {
  switch (token_type) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_SLASH:
    case TOKEN_ASTERISK:
    case TOKEN_EQ:
    case TOKEN_NOT_EQ:
    case TOKEN_LT:
    case TOKEN_GT:
      return parse_infix_expression;
    case TOKEN_LEFT_PAREN:
      return parse_call_expression;
    case TOKEN_LEFT_BRACKET:
      return parse_index_expression;
  }
  return NULL;
}
