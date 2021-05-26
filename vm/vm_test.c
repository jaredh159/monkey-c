#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../compiler/compiler.h"
#include "../parser/parser.h"
#include "../test/test.h"
#include "../utils/colors.h"

enum ExpectedTypes { EXP_INT, EXP_BOOL };

typedef struct Expected {
  int type;
  union {
    int i;
    bool b;
  } v;
} Expected;

typedef struct VmTest {
  char* input;
  Expected expected;
} VmTest;

Expected expect_int(int expected_int);
Expected expect_bool(bool boolean);
void run_vm_tests(int len, VmTest tests[len], char* test);
void test_expected_object(Expected exp, Object* obj, char* test);

void test_integer_arithmetic(void) {
  VmTest tests[] = {
    {.input = "1", .expected = expect_int(1)},                     //
    {.input = "2", .expected = expect_int(2)},                     //
    {.input = "1 + 2", .expected = expect_int(3)},                 //
    {.input = "1 - 2", .expected = expect_int(-1)},                //
    {.input = "1 * 2", .expected = expect_int(2)},                 //
    {.input = "4 / 2", .expected = expect_int(2)},                 //
    {.input = "50 / 2 * 2 + 10 - 5", .expected = expect_int(55)},  //
    {.input = "5 + 5 + 5 + 5 - 10", .expected = expect_int(10)},   //
    {.input = "2 * 2 * 2 * 2 * 2", .expected = expect_int(32)},    //
    {.input = "5 * 2 + 10", .expected = expect_int(20)},           //
    {.input = "5 + 2 * 10", .expected = expect_int(25)},           //
    {.input = "5 * (2 + 10)", .expected = expect_int(60)},         //
  };
  run_vm_tests(LEN(tests), tests, "integer_arithmetic");
}

void test_boolean_expressions(void) {
  VmTest tests[] = {
    {.input = "true", .expected = expect_bool(true)},    //
    {.input = "false", .expected = expect_bool(false)},  //
  };
  run_vm_tests(LEN(tests), tests, "boolean_expressions");
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_boolean_expressions();
  test_integer_arithmetic();
  printf("\n");
  return 0;
}

void run_vm_tests(int len, VmTest tests[len], char* test) {
  char* err = NULL;
  for (int i = 0; i < len; i++) {
    VmTest t = tests[i];
    Program* program = parse_program(t.input);
    compiler_init();
    err = compile(program, PROGRAM_NODE);
    if (err) {
      fail(ss("compiler error: %s", err), test);
    }

    vm_init(compiler_bytecode());
    err = vm_run();
    if (err)
      fail(ss("vm error: %s", err), test);

    Object* last_popped = vm_last_popped();
    test_expected_object(
      t.expected, last_popped, ss("%s, input=`%s`", test, t.input));
  }
}

void test_expected_object(Expected exp, Object* obj, char* test) {
  switch (exp.type) {
    case EXP_INT:
      assert_integer_object(exp.v.i, *obj, test);
      break;
    case EXP_BOOL:
      assert(exp.v.b == ((BooleanLiteral*)obj)->value, "boolean correct", test);
      break;
    default:
      printf("ERROR: unhandled expected object type=%d\n", exp.type);
      exit(EXIT_FAILURE);
  }
}

Expected expect_bool(bool boolean) {
  return (Expected){.type = EXP_BOOL, .v = {.b = boolean}};
}

Expected expect_int(int expected_int) {
  return (Expected){.type = EXP_INT, .v = {.i = expected_int}};
}
