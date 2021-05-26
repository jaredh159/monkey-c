#ifndef __TEST_H__
#define __TEST_H__

#include <stdbool.h>
#include "../object/object.h"
#include "../token/token.h"

#define LEN(array) ((int)sizeof(array) / (int)sizeof(array[0]))

/**
 * Return a string embedded with strings (up to 3)
 */
char *ss(char *format, ...);

/**
 * Return a string embedded with `int`s (up to 3)
 */
char *si(char *format, ...);

void pass_argv(int argc, char *argv[]);
bool token_literal_is(Token *token, char *literal);
void assert(bool predicate, char *msg, char *test_name);
void assert_str_is(char *expected, char *actual, char *msg, char *test_name);
void fail(char *msg, char *test_name);
void assert_int_is(int expected, int actual, char *msg, char *test_name);
void assert_integer_object(int expected_int, Object actual, char *test_name);

#endif  // __TEST_H__
