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

PrefixParselet get_prefix_parselet(char *token_type)
{
  if (str_is(token_type, TOKEN_IDENTIFIER))
    return parse_identifier;
  return NULL;
}
