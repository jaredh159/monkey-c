#include <stdio.h>
#include <stdlib.h>
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
