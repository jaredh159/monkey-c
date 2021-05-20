#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../object/object.h"
#include "../token/token.h"
#include "../utils/argv.h"
#include "../utils/colors.h"

static bool verbose = false;
static char embedded[500];

void pass_argv(int argc, char *argv[]) {
  if (argv_has_flag('v', argc, argv) || argv_idx("--verbose", argc, argv) != -1)
    verbose = true;
}

char *ss(char *format, ...) {
  int args_len = 0;

  for (char *p = format; *p; p++) {
    if (*p == '%' && *(p + 1) != '%') {
      args_len++;
    }
  }

  va_list ap;
  va_start(ap, format);

  char *strs[args_len];
  for (int i = 0; i < args_len; i++) {
    strs[i] = va_arg(ap, char *);
  }

  va_end(ap);

  switch (args_len) {
    case 1:
      sprintf(embedded, format, strs[0]);
      break;
    case 2:
      sprintf(embedded, format, strs[0], strs[1]);
      break;
    case 3:
      sprintf(embedded, format, strs[0], strs[1], strs[2]);
      break;
    default:
      printf("Unhandled arg count=%d\n", args_len);
      exit(EXIT_FAILURE);
  }

  return embedded;
}

char *si(char *format, ...) {
  int args_len = 0;

  for (char *p = format; *p; p++) {
    if (*p == '%' && *(p + 1) != '%') {
      args_len++;
    }
  }

  va_list ap;
  va_start(ap, format);

  int ints[args_len];
  for (int i = 0; i < args_len; i++) {
    ints[i] = va_arg(ap, int);
  }

  va_end(ap);

  switch (args_len) {
    case 1:
      sprintf(embedded, format, ints[0]);
      break;
    case 2:
      sprintf(embedded, format, ints[0], ints[1]);
      break;
    case 3:
      sprintf(embedded, format, ints[0], ints[1], ints[2]);
      break;
    default:
      printf("Unhandled arg count=%d\n", args_len);
      exit(EXIT_FAILURE);
  }

  return embedded;
}

bool token_literal_is(Token *token, char *literal) {
  return token_prop_is(token->literal, literal);
}

void fail(char *msg, char *test_name) {
  printf(
    COLOR_RED "%sX %s: %s\n" COLOR_RESET, verbose ? "" : "\n", test_name, msg);
  exit(EXIT_FAILURE);
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
