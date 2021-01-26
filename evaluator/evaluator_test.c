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

  IntTest tests[] = {{"5", 5}, {"10", 10}, {"-5", -5}, {"-10", -10},
    {"5 + 5 + 5 + 5 - 10", 10}, {"2 * 2 * 2 * 2 * 2", 32},
    {"-50 + 100 + -50", 0}, {"5 * 2 + 10", 20}, {"5 + 2 * 10", 25},
    {"20 + 2 * -10", 0}, {"50 / 2 * 2 + 10", 60}, {"2 * (5 + 10)", 30},
    {"3 * 3 * 3 + 10", 37}, {"3 * (3 * 3) + 10", 37},
    {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50}};
  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
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

  IntTest tests[] = {{"true", true}, {"false", false}, {"1 < 2", true},
    {"1 > 2", false}, {"1 < 1", false}, {"1 > 1", false}, {"1 == 1", true},
    {"1 != 1", false}, {"1 == 2", false}, {"1 != 2", true},
    {"true == true", true}, {"false == false", true}, {"true == false", false},
    {"true != false", true}, {"false != true", true}, {"(1 < 2) == true", true},
    {"(1 < 2) == false", false}, {"(1 > 2) == true", false},
    {"(1 > 2) == false", true}};
  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_boolean_object(
      evaluated, tests[i].expected, "eval_boolean_expression");
  }
}

void test_bang_operator() {
  typedef struct {
    char *input;
    bool expected;
  } IntTest;

  IntTest tests[] = {{"!true", false}, {"!false", true}, {"!5", false},
    {"!!true", true}, {"!!false", false}, {"!!5", true}};

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_boolean_object(evaluated, tests[i].expected, "test_bang_operator");
  }
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_bang_operator();
  test_eval_boolean_expression();
  test_eval_integer_expression();
  printf("\n");
  return 0;
}
