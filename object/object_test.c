#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../test/test.h"
#include "../token/token.h"

void test_object_hash_equality(void) {
  char *t = "object_hash";
  Object hello1 = {.type = STRING_OBJ, .value = {.str = "Hello World"}};
  Object hello2 = {.type = STRING_OBJ, .value = {.str = "Hello World"}};
  Object diff1 = {.type = STRING_OBJ, .value = {.str = "Goat banjo"}};
  Object diff2 = {.type = STRING_OBJ, .value = {.str = "Goat banjo"}};

  assert_str_is(
    object_hash(hello1), object_hash(hello2), "same str content, same hash", t);
  assert_str_is(
    object_hash(diff1), object_hash(diff2), "same str content, same hash", t);
  assert(strcmp(object_hash(diff1), object_hash(hello2)) != 0,
    "different strings have different hashes", t);
}

void test_object_hash_values(void) {
  char *t = "object_hash_values";

  Object hello = {.type = STRING_OBJ, .value = {.str = "hello"}};
  assert_str_is("S=hello", object_hash(hello), "str val correct", t);

  Object integer = {.type = INTEGER_OBJ, .value = {.i = 389}};
  assert_str_is("I=389", object_hash(integer), "int val correct", t);

  Object boolean = {.type = BOOLEAN_OBJ, .value = {.b = true}};
  assert_str_is("B=true", object_hash(boolean), "bool val correct", t);

  Object boolean = {.type = BOOLEAN_OBJ, .value = {.b = false}};
  assert_str_is("B=false", object_hash(boolean), "bool val correct", t);
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_object_hash_equality();
  test_object_hash_values();
  printf("\n");
  return 0;
}
