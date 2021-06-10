#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../compiler/compiler.h"
#include "../parser/parser.h"
#include "../test/test.h"
#include "../utils/colors.h"

enum ExpectedTypes { EXP_INT, EXP_BOOL, EXP_NULL, EXP_STR };

typedef struct Expected {
  int type;
  union {
    int i;
    bool b;
    char* s;
  } v;
} Expected;

typedef struct VmTest {
  char* input;
  Expected expected;
} VmTest;

Expected expect_int(int expected_int);
Expected expect_bool(bool boolean);
Expected expect_str(char* string);
Expected expect_null();
void run_vm_tests(int len, VmTest tests[len], const char* test);
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
    {.input = "-5", .expected = expect_int(-5)},                   //
    {.input = "-10", .expected = expect_int(-10)},                 //
    {.input = "-50 + 100 + -50", .expected = expect_int(0)},       //
    {.input = "(5 + 10 / 3) * -2", .expected = expect_int(-16)},   //
  };
  run_vm_tests(LEN(tests), tests, "integer_arithmetic");
}

void test_boolean_expressions(void) {
  VmTest tests[] = {
    {.input = "true", .expected = expect_bool(true)},                      //
    {.input = "false", .expected = expect_bool(false)},                    //
    {.input = "1 < 2", .expected = expect_bool(true)},                     //
    {.input = "1 > 2", .expected = expect_bool(false)},                    //
    {.input = "1 < 1", .expected = expect_bool(false)},                    //
    {.input = "1 > 1", .expected = expect_bool(false)},                    //
    {.input = "1 == 1", .expected = expect_bool(true)},                    //
    {.input = "1 != 1", .expected = expect_bool(false)},                   //
    {.input = "1 == 2", .expected = expect_bool(false)},                   //
    {.input = "1 != 2", .expected = expect_bool(true)},                    //
    {.input = "true == true", .expected = expect_bool(true)},              //
    {.input = "false == false", .expected = expect_bool(true)},            //
    {.input = "true == false", .expected = expect_bool(false)},            //
    {.input = "true != false", .expected = expect_bool(true)},             //
    {.input = "false != true", .expected = expect_bool(true)},             //
    {.input = "(1 < 2) == true", .expected = expect_bool(true)},           //
    {.input = "(1 < 2) == false", .expected = expect_bool(false)},         //
    {.input = "(1 > 2) == true", .expected = expect_bool(false)},          //
    {.input = "(1 > 2) == false", .expected = expect_bool(true)},          //
    {.input = "!true", expect_bool(false)},                                //
    {.input = "!false", expect_bool(true)},                                //
    {.input = "!5", expect_bool(false)},                                   //
    {.input = "!!true", expect_bool(true)},                                //
    {.input = "!!false", expect_bool(false)},                              //
    {.input = "!!5", expect_bool(true)},                                   //
    {.input = "!(if (false) { 5; })", expect_bool(true)},                  //
    {.input = "if ((if (false) { 1 })) { 1 } else { 2 }", expect_int(2)},  //
  };
  run_vm_tests(LEN(tests), tests, "boolean_expressions");
}

void test_conditionals(void) {
  VmTest tests[] = {
    {.input = "if (true) { 10 }", .expected = expect_int(10)},                //
    {.input = "if (true) { 10 } else { 20 }", .expected = expect_int(10)},    //
    {.input = "if (false) { 10 } else { 20 } ", .expected = expect_int(20)},  //
    {.input = "if (1) { 10 }", .expected = expect_int(10)},                   //
    {.input = "if (1 < 2) { 10 }", .expected = expect_int(10)},               //
    {.input = "if (1 < 2) { 10 } else { 20 }", .expected = expect_int(10)},   //
    {.input = "if (1 > 2) { 10 } else { 20 }", .expected = expect_int(20)},   //
    {.input = "if (1 > 2) { 10 }", .expected = expect_null()},                //
    {.input = "if (false) { 10 }", .expected = expect_null()},                //
  };
  run_vm_tests(LEN(tests), tests, "conditionals");
}

void test_global_let_statements(void) {
  VmTest tests[] = {
    {.input = "let one = 1; one", .expected = expect_int(1)},                 //
    {.input = "let a = 1; let b = 2; a + b", .expected = expect_int(3)},      //
    {.input = "let a = 1; let b = a + a; a + b", .expected = expect_int(3)},  //
  };
  run_vm_tests(LEN(tests), tests, "global_let_statements");
}

void test_string_expressions(void) {
  VmTest tests[] = {
    {.input = "\"monkey\"", .expected = expect_str("monkey")},          //
    {.input = "\"mon\" + \"key\"", .expected = expect_str("monkey")},   //
    {.input = "\"a\" + \"b\" + \"c\"", .expected = expect_str("abc")},  //
  };
  run_vm_tests(LEN(tests), tests, __func__);
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_string_expressions();
  test_conditionals();
  test_global_let_statements();
  test_boolean_expressions();
  test_integer_arithmetic();
  printf("\n");
  return 0;
}

void run_vm_tests(int len, VmTest tests[len], const char* test) {
  char* err = NULL;
  for (int i = 0; i < len; i++) {
    VmTest t = tests[i];
    Program* program = parse_program(t.input);
    Compiler compiler = compiler_new();
    err = compile(compiler, program, PROGRAM_NODE);
    if (err) {
      fail(ss("compiler error: %s", err), test);
    }

    Vm vm = vm_new(compiler_bytecode(compiler));
    err = vm_run(vm);
    if (err)
      fail(ss("vm error: %s", err), test);

    Object* last_popped = vm_last_popped(vm);
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
    case EXP_NULL:
      assert(obj->type == NULL_OBJ, "null correct", test);
      break;
    case EXP_STR:
      assert(obj->type == STRING_OBJ, "string obj correct type", test);
      assert_str_is(obj->value.str, exp.v.s, "string obj value correct", test);
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

Expected expect_null() {
  return (Expected){.type = EXP_NULL};
}

Expected expect_str(char* string) {
  return (Expected){.type = EXP_STR, .v = {.s = string}};
}
