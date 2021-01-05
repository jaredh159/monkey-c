#ifndef __TEST_H__
#define __TEST_H__

#include <stdbool.h>
#include "../token/token.h"

bool token_is(Token *token, char *type);
bool token_literal_is(Token *token, char *literal);
void assert(bool predicate, char *msg, char *test_name);
void assert_str_is(char *str1, char *str2, char *msg, char *test_name);
void fail(char *msg, char *test_name);
void assert_int_is(int expected, int actual, char *msg, char *test_name);

#endif // __TEST_H__
