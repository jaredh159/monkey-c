#ifndef __TEST_H__
#define __TEST_H__

#include <stdbool.h>
#include "../token/token.h"

char *str_embed(char *format, char *str);
char *int_embed(char *format, int integer);
void pass_argv(int argc, char *argv[]);
bool token_literal_is(Token *token, char *literal);
void assert(bool predicate, char *msg, char *test_name);
void assert_str_is(char *expected, char *actual, char *msg, char *test_name);
void fail(char *msg, char *test_name);
void assert_int_is(int expected, int actual, char *msg, char *test_name);

#endif // __TEST_H__
