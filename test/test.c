#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../colors.h"
#include "../token/token.h"

bool token_literal_is(Token *token, char *literal)
{
  return strcmp(token->literal, literal) == 0;
}

bool token_is(Token *token, char *type)
{
  return strcmp(token->type, type) == 0;
}

void fail(char *msg, char *test_name)
{
  printf(COLOR_RED "X %s: %s\n" COLOR_RESET, test_name, msg);
  exit(1);
}

void assert(bool predicate, char *msg, char *test_name)
{
  if (!predicate)
    return fail(msg, test_name);

  printf(COLOR_GREEN "√" COLOR_RESET COLOR_GREY " %s: %s\n" COLOR_RESET, test_name, msg);
}

void assert_str_is(char *str1, char *str2, char *msg, char *test_name)
{
  assert(strcmp(str1, str2) == 0, msg, test_name);
}
