#include "evaluator.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include "../object/object.h"
#include "../parser/parser.h"
#include "../test/test.h"

#define NULL_SENTINAL INT_MIN

typedef struct {
  char *input;
  int expected;
} IntTest;

typedef struct {
  char *input;
  char *expected;
} StrTest;

Object eval_test(char *input) {
  Program *program = parse_program(input);
  return eval(program, PROGRAM_NODE, env_new());
}

void assert_null_object(Object object, char *test_name) {
  assert_int_is(NULL_OBJ, object.type, "object is type=NULL", test_name);
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
  } BoolTest;

  BoolTest tests[] = {{"!true", false}, {"!false", true}, {"!5", false},
    {"!!true", true}, {"!!false", false}, {"!!5", true}};

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_boolean_object(evaluated, tests[i].expected, "test_bang_operator");
  }
}

void test_if_else_expressions(void) {
  IntTest tests[] = {{"if (true) { 10 }", 10},
    {"if (false) { 10 }", NULL_SENTINAL}, {"if (1) { 10 }", 10},
    {"if (1 < 2) { 10 }", 10}, {"if (1 > 2) { 10 }", NULL_SENTINAL},
    {"if (1 > 2) { 10 } else { 20 }", 20},
    {"if (1 < 2) { 10 } else { 20 }", 10}};
  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evald = eval_test(tests[i].input);
    if (tests[i].expected == NULL_SENTINAL)
      assert_null_object(evald, "if_else_expressions");
    else
      assert_integer_object(evald, tests[i].expected, "if_else_expressions");
  }
}

void test_return_statements(void) {
  char *t = "return_statements";
  IntTest tests[] = {{"return 10;", 10}, {"return 10; 9;", 10},
    {"return 2 * 5; 9;", 10}, {"9; return 2 * 5; 9;", 10},
    {"if (10 > 1) { if (10 > 1) { return 10; } return 1; }", 10}};

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    if (res.type == RETURN_VALUE_OBJ) {
      assert_int_is(RETURN_VALUE_OBJ, res.type, "result is RETURN_VALUE", t);
      assert_int_is(tests[i].expected, res.value.return_value->value.i,
        "return value correct", t);
    } else {
      assert_integer_object(res, tests[i].expected, t);
    }
  }
}

void test_error_handling(void) {
  char *t = "error_handling";
  StrTest tests[] = {//
    {
      "5 + true;",
      "type mismatch: INTEGER + BOOLEAN",
    },
    {
      "5 + true; 5;",
      "type mismatch: INTEGER + BOOLEAN",
    },
    {
      "-true",
      "unknown operator: -BOOLEAN",
    },
    {
      "true + false;",
      "unknown operator: BOOLEAN + BOOLEAN",
    },
    {
      "5; true + false; 5",
      "unknown operator: BOOLEAN + BOOLEAN",
    },
    {
      "if (10 > 1) { true + false; }",
      "unknown operator: BOOLEAN + BOOLEAN",
    },
    {
      "if (10 > 1) { if ( 10 > 1) { return true + false; } return 1; }",
      "unknown operator: BOOLEAN + BOOLEAN",
    },
    {
      "foobar",
      "identifier not found: foobar",
    }};

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    assert_int_is(ERROR_OBJ, res.type, "result object.type=ERROR", t);
    assert_str_is(tests[i].expected, res.value.str, "error msg correct", t);
  }
}

void test_let_statements(void) {
  char *t = "let_statements";
  IntTest tests[] = {
    {"let a = 5; a;", 5},
    {"let a = 5 * 5; a;", 25},
    {"let a = 5; let b = a; b;", 5},
    {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    assert_integer_object(res, tests[i].expected, t);
  }
}

void test_function_object(void) {
  char *t = "function_object";
  Object evaluated = eval_test("fn(x) { x + 2; }");
  Function *fn = evaluated.value.fn;
  assert_int_is(FUNCTION_OBJ, evaluated.type, "function is type=FUNCTION", t);
  assert_int_is(1, list_count(fn->parameters), "has 1 param", t);
  Identifier *ident = fn->parameters->item;
  assert_str_is("x", identifier_string(ident), "param is x", t);
  assert_str_is("(x + 2)", block_statement_string(fn->body), "body correct", t);
}

void test_function_application(void) {
  IntTest tests[] = {
    {"let identity = fn(x) { x; }; identity(5);", 5},
    {"let identity = fn(x) { return x; }; identity(5);", 5},
    {"let double = fn(x) { x * 2; }; double(5);", 10},
    {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
    {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
    {"fn(x) { x; }(5)", 5},
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    assert_integer_object(res, tests[i].expected, "function_application");
  }
}

void test_string_literal(void) {
  char *t = "string_literal";
  Object evaluated = eval_test("\"Hello World!\"");
  assert_int_is(STRING_OBJ, evaluated.type, "object is string", t);
  assert_str_is("Hello World!", evaluated.value.str, "string value correct", t);
}

void test_closures(void) {
  char *input =
    "let newAdder = fn(x) { fn(y) { x + y }; };"
    "let addTwo = newAdder(2);"
    "addTwo(2);";
  assert_integer_object(eval_test(input), 4, "closures");
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_string_literal();
  test_closures();
  test_function_application();
  test_function_object();
  test_let_statements();
  test_error_handling();
  test_return_statements();
  test_if_else_expressions();
  test_bang_operator();
  test_eval_boolean_expression();
  test_eval_integer_expression();
  printf("\n");
  return 0;
}
