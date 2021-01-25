#include "evaluator.h"
#include <stdbool.h>
#include <stdio.h>
#include "../object/object.h"
#include "../parser/parser.h"
#include "../test/test.h"

Object eval_test(char *input) {
  Program *program = parse_program(input);
  return eval(program, PROGRAM_NODE);
}

void assert_boolean_object(Object object, bool expected, char *test_name) {
  assert_int_is(BOOLEAN_OBJ, object.type, "object is type=BOOLEAN", test_name);
  assert_int_is(
    (int)expected, object.value.b, "bool has correct value", test_name);
}

void assert_integer_object(Object object, int expected, char *test_name) {
  assert_int_is(INTEGER_OBJ, object.type, "object is type=INTEGER", test_name);
  assert_int_is(
    expected, object.value.i, "integer has correct value", test_name);
}

void test_eval_integer_expression() {
  typedef struct {
    char *input;
    int expected;
  } IntTest;

  IntTest tests[] = {{"5", 5}, {"10", 10}};
  for (int i = 0; i < 2; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_integer_object(
      evaluated, tests[i].expected, "eval_integer_expression");
  }
}

void test_eval_boolean_expression() {
  typedef struct {
    char *input;
    int expected;
  } IntTest;

  IntTest tests[] = {{"true", true}, {"false", false}};
  for (int i = 0; i < 2; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_boolean_object(
      evaluated, tests[i].expected, "eval_boolean_expression");
  }
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_eval_boolean_expression();
  test_eval_integer_expression();
  printf("\n");
  return 0;
}
