#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../colors.h"
#include "token.h"

Token *new_token(char *type, char *literal)
{
  Token *token = (Token *)malloc(sizeof(Token));
  token->type = type;
  token->literal = literal;
  return token;
}

void print_token(Token *tok)
{
  printf(
      COLOR_GREY "  -> { type: %s, literal: \"%s\" }\n" COLOR_RESET,
      tok->type,
      tok->literal);
}

bool str_is(char *str, char *compare)
{
  return strcmp(str, compare) == 0;
}

bool token_prop_is(char *prop, char *compare)
{
  return str_is(prop, compare);
}
