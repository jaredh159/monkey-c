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
  IntTest tests[] = {
    {"5", 5},
    {"10", 10},
    {"-5", -5},
    {"-10", -10},
    {"5 + 5 + 5 + 5 - 10", 10},
    {"2 * 2 * 2 * 2 * 2", 32},
    {"-50 + 100 + -50", 0},
    {"5 * 2 + 10", 20},
    {"5 + 2 * 10", 25},
    {"20 + 2 * -10", 0},
    {"50 / 2 * 2 + 10", 60},
    {"2 * (5 + 10)", 30},
    {"3 * 3 * 3 + 10", 37},
    {"3 * (3 * 3) + 10", 37},
    {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
  };
  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_integer_object(
      evaluated, tests[i].expected, "eval_integer_expression");
  }
}

void test_eval_boolean_expression() {
  IntTest tests[] = {
    {"true", true},
    {"false", false},
    {"1 < 2", true},
    {"1 > 2", false},
    {"1 < 1", false},
    {"1 > 1", false},
    {"1 == 1", true},
    {"1 != 1", false},
    {"1 == 2", false},
    {"1 != 2", true},
    {"true == true", true},
    {"false == false", true},
    {"true == false", false},
    {"true != false", true},
    {"false != true", true},
    {"(1 < 2) == true", true},
    {"(1 < 2) == false", false},
    {"(1 > 2) == true", false},
    {"(1 > 2) == false", true},
  };
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

  BoolTest tests[] = {
    {"!true", false},
    {"!false", true},
    {"!5", false},
    {"!!true", true},
    {"!!false", false},
    {"!!5", true},
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object evaluated = eval_test(tests[i].input);
    assert_boolean_object(evaluated, tests[i].expected, "test_bang_operator");
  }
}

void test_if_else_expressions(void) {
  IntTest tests[] = {
    {"if (true) { 10 }", 10},
    {"if (false) { 10 }", NULL_SENTINAL},
    {"if (1) { 10 }", 10},
    {"if (1 < 2) { 10 }", 10},
    {"if (1 > 2) { 10 }", NULL_SENTINAL},
    {"if (1 > 2) { 10 } else { 20 }", 20},
    {"if (1 < 2) { 10 } else { 20 }", 10},
  };
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
  IntTest tests[] = {
    {"return 10;", 10},
    {"return 10; 9;", 10},
    {"return 2 * 5; 9;", 10},
    {"9; return 2 * 5; 9;", 10},
    {"if (10 > 1) { if (10 > 1) { return 10; } return 1; }", 10},
  };

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
  StrTest tests[] = {
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
    },
    {
      "\"Hello\" - \"World\"",
      "unknown operator: STRING - STRING",
    },
    {
      "len(1)",
      "argument to `len` not supported, got INTEGER",
    },
    {
      "len(\"one\", \"two\")",
      "wrong number of arguments. got=2, want=1",
    },
    {
      "{\"name\": \"Monkey\"}[fn(x) { x }]",
      "unusable as hash key: FUNCTION",
    },
  };

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

void test_string_concatenation(void) {
  char *t = "string_concatenation";
  Object evaluated = eval_test("\"Hello\" + \" \" + \"World!\"");
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

void test_builtin_functions(void) {
  char *t = "builtin_functions";
  IntTest tests[] = {
    {"len(\"\")", 0},
    {"len(\"four\")", 4},
    {"len(\"hello world\")", 11},
    {"len([1, 2])", 2},
    {"len([])", 0},
    {"first([1, 2])", 1},
    {"first([])", NULL_SENTINAL},
    {"last([1, 2])", 2},
    {"last([4,4,4,99])", 99},
    {"last([])", NULL_SENTINAL},
    {"rest([1, 2])[0]", 2},
    {"len(rest([1, 2, 3, 4]))", 3},
    {"len(last([\"foo\", \"123456\"]))", 6},
    {"let a = [1]; let b = push(a, 5); last(b);", 5},
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    if (tests[i].expected == NULL_SENTINAL)
      assert_null_object(res, t);
    else
      assert_integer_object(res, tests[i].expected, t);
  }
}

void test_array_literals(void) {
  char *t = "array_literals";
  Object evaluated = eval_test("[1, 2 * 2, 3 + 3]");
  assert_int_is(ARRAY_OBJ, evaluated.type, "eval'd is array", t);
  List *elements = evaluated.value.list;
  assert_int_is(3, list_count(elements), "array.length = 3", t);
  Object *first = elements->item;
  assert_integer_object(*first, 1, t);
  Object *second = elements->next->item;
  assert_integer_object(*second, 4, t);
  Object *third = elements->next->next->item;
  assert_integer_object(*third, 6, t);
}

void test_array_index_expressions(void) {
  char *t = "array_index_expressions";
  IntTest tests[] = {
    {"[1, 2, 3][0]", 1},
    {"[1, 2, 3][1]", 2},
    {"[1, 2, 3][2]", 3},
    {"let i = 0; [1][i];", 1},
    {"[1, 2, 3][1 + 1];", 3},
    {"let myArray = [1, 2, 3]; myArray[2];", 3},
    {"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6},
    {"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2},
    {"[1, 2, 3][3]", NULL_SENTINAL},
    {"[1, 2, 3][-1]", NULL_SENTINAL},
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    if (tests[i].expected == NULL_SENTINAL)
      assert_null_object(res, t);
    else
      assert_integer_object(res, tests[i].expected, t);
  }
}

void test_hash_literals(void) {
  char *t = "hash_literals";
  char *input =
    "let two = \"two\";\n"
    "{\n"
    "  \"one\": 10 - 9,\n"
    "  two: 1 + 1,\n"
    "  \"thr\" + \"ee\": 6 / 2,\n"
    "  4: 4,\n"
    "  true: 5,\n"
    "  false: 6\n"
    "}";

  Object res = eval_test(input);
  List *pairs = res.value.list;

  assert_int_is(HASH_OBJ, res.type, "result should be hash", t);
  assert_int_is(6, list_count(pairs), "should have 6 pairs", t);

  HashPair *pair1 = pairs->item;
  assert_str_is("one", pair1->key->value.str, "key one is \"one\"", t);
  assert_integer_object(*pair1->value, 1, t);

  HashPair *pair2 = pairs->next->item;
  assert_str_is("two", pair2->key->value.str, "key two is \"two\"", t);
  assert_integer_object(*pair2->value, 2, t);

  HashPair *pair3 = pairs->next->next->item;
  assert_str_is("three", pair3->key->value.str, "key three is \"three\"", t);
  assert_integer_object(*pair3->value, 3, t);

  HashPair *pair4 = pairs->next->next->next->item;
  assert_integer_object(*pair4->key, 4, t);
  assert_integer_object(*pair4->value, 4, t);

  HashPair *pair5 = pairs->next->next->next->next->item;
  assert_boolean_object(*pair5->key, true, t);
  assert_integer_object(*pair5->value, 5, t);

  HashPair *pair6 = pairs->next->next->next->next->next->item;
  assert_boolean_object(*pair6->key, false, t);
  assert_integer_object(*pair6->value, 6, t);
}

void test_hash_index_expressions(void) {
  char *t = "hash_index_expressions";
  IntTest tests[] = {
    {
      "{\"foo\": 5}[\"foo\"]",
      5,
    },
    {
      "{\"foo\": 5}[\"bar\"]",
      NULL_SENTINAL,
    },
    {
      "let key = \"foo\"; {\"foo\": 5}[key]",
      5,
    },
    {
      "{}[\"foo\"]",
      NULL_SENTINAL,
    },
    {
      "{5: 5}[5]",
      5,
    },
    {
      "{true: 5}[true]",
      5,
    },
    {
      "{false: 5}[false]",
      5,
    },
  };

  int num_tests = sizeof tests / sizeof(tests[0]);
  for (int i = 0; i < num_tests; i++) {
    Object res = eval_test(tests[i].input);
    if (tests[i].expected == NULL_SENTINAL)
      assert_null_object(res, t);
    else
      assert_integer_object(res, tests[i].expected, t);
  }
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_hash_index_expressions();
  test_hash_literals();
  test_builtin_functions();
  test_array_index_expressions();
  test_array_literals();
  test_string_concatenation();
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
