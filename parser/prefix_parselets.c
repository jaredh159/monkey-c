#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Expression *parse_identifier()
{
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

Expression *parse_integer_literal()
{
  Expression *exp = malloc(sizeof(Expression));
  IntegerLiteral *int_literal = malloc(sizeof(IntegerLiteral));
  if (exp == NULL || int_literal == NULL)
    return NULL;

  char *token_literal = parser_current_token()->literal;
  int value = atoi(token_literal);
  if (value == 0 && !str_is(token_literal, "0"))
  {
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

Expression *parse_prefix_expression()
{
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
  prefix->right = parse_expression(/* PRECEDENCE_PREFIX */);
  return exp;
}

PrefixParselet get_prefix_parselet(char *token_type)
{
  if (str_is(token_type, TOKEN_IDENTIFIER))
    return parse_identifier;
  if (str_is(token_type, TOKEN_INTEGER))
    return parse_integer_literal;
  if (str_is(token_type, TOKEN_MINUS))
    return parse_prefix_expression;
  if (str_is(token_type, TOKEN_BANG))
    return parse_prefix_expression;
  return NULL;
}
