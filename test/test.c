#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../object/object.h"
#include "../token/token.h"
#include "../utils/argv.h"
#include "../utils/colors.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
#define debug(a, args...) printf("<%s:%d> " a "\n", __FILE__, __LINE__, ##args)
#pragma clang diagnostic push

static bool verbose = false;
static char embedded[500];

void pass_argv(int argc, char *argv[]) {
  if (argv_has_flag('v', argc, argv) || argv_idx("--verbose", argc, argv) != -1)
    verbose = true;
}

char *int_embed(char *format, int integer) {
  sprintf(embedded, format, integer);
  return embedded;
}

char *str_embed(char *format, char *str) {
  sprintf(embedded, format, str);
  return embedded;
}

bool token_literal_is(Token *token, char *literal) {
  return token_prop_is(token->literal, literal);
}

void fail(char *msg, char *test_name) {
  printf(
    COLOR_RED "%sX %s: %s\n" COLOR_RESET, verbose ? "" : "\n", test_name, msg);
  exit(1);
}

void assert(bool predicate, char *msg, char *test_name) {
  if (!predicate) {
    fail(msg, test_name);
    return;
  }

  if (!verbose) {
    printf(COLOR_GREEN "•" COLOR_RESET);
    return;
  }
  printf(COLOR_GREEN "√" COLOR_RESET COLOR_GREY " %s: %s\n" COLOR_RESET,
    test_name, msg);
}

void assert_str_is(char *expected, char *actual, char *msg, char *test_name) {
  if (strcmp(expected, actual) == 0)
    assert(true, msg, test_name);
  else {
    char failmsg[200];
    sprintf(failmsg, "expected string to be `%s`, got `%s` instead", expected,
      actual);
    fail(failmsg, test_name);
  }
}

void assert_int_is(int expected, int actual, char *msg, char *test_name) {
  if (expected == actual)
    assert(true, msg, test_name);
  else {
    char failmsg[200];
    sprintf(failmsg, "expected `%d`, got `%d` --  %s", expected, actual, msg);
    fail(failmsg, test_name);
  }
}

void assert_integer_object(int expected_int, Object actual, char *test_name) {
  if (actual.type != INTEGER_OBJ) {
    char failmsg[200];
    sprintf(
      failmsg, "object was not type=INTEGER, got=%s", object_type(actual));
    fail(failmsg, test_name);
    return;
  }

  if (actual.value.i == expected_int) {
    assert(true, "integer object correct", test_name);
    return;
  }

  char failmsg[200];
  sprintf(failmsg, "incorrect integer object value, want=%d, got=%d",
    expected_int, actual.value.i);
  fail(failmsg, test_name);
}
