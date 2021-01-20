#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../utils/colors.h"
#include "../token/token.h"
#include "../utils/argv.h"

static bool verbose = false;
static char embedded[500];

void pass_argv(int argc, char *argv[])
{
  if (argv_has_flag('v', argc, argv) || argv_idx("--verbose", argc, argv) != -1)
    verbose = true;
}

char *int_embed(char *format, int integer)
{
  sprintf(embedded, format, integer);
  return embedded;
}

char *str_embed(char *format, char *str)
{
  sprintf(embedded, format, str);
  return embedded;
}

bool token_literal_is(Token *token, char *literal)
{
  return token_prop_is(token->literal, literal);
}

void fail(char *msg, char *test_name)
{
  printf(COLOR_RED "%sX %s: %s\n" COLOR_RESET, verbose ? "" : "\n", test_name, msg);
  exit(1);
}

void assert(bool predicate, char *msg, char *test_name)
{
  if (!predicate)
  {
    fail(msg, test_name);
    return;
  }

  if (!verbose)
  {
    printf(COLOR_GREEN "•" COLOR_RESET);
    return;
  }
  printf(COLOR_GREEN "√" COLOR_RESET COLOR_GREY " %s: %s\n" COLOR_RESET, test_name, msg);
}

void assert_str_is(char *expected, char *actual, char *msg, char *test_name)
{
  if (strcmp(expected, actual) == 0)
    assert(true, msg, test_name);
  else
  {
    char failmsg[200];
    sprintf(failmsg, "expected string to be `%s`, got `%s` instead", expected, actual);
    fail(failmsg, test_name);
  }
}

void assert_int_is(int expected, int actual, char *msg, char *test_name)
{
  if (expected == actual)
    assert(true, msg, test_name);
  else
  {
    char failmsg[200];
    sprintf(failmsg, "expected `%d`, got `%d` --  %s", expected, actual, msg);
    fail(failmsg, test_name);
  }
}
