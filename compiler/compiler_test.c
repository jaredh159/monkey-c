#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "../code/code.h"
#include "../parser/parser.h"
#include "../test/test.h"
#include "../utils/colors.h"

extern OpCodes OP;

typedef struct CompilerTest {
  char* input;
  Instruct* expected_instructions;
  ConstantPool* expected_constants;
} CompilerTest;

void test_instructions(Instruct* expected, Instruct* actual, char* test) {
  assert_int_is(expected->length, actual->length, "instructions length", test);
  for (int i = 0; i < expected->length; i++) {
    assert_int_is(expected->bytes[i], actual->bytes[i],
      si("instruction byte at pos=%d", i), test);
  }
}

void test_constants(ConstantPool* expected, ConstantPool* actual, char* test) {
  assert_int_is(expected->length, actual->length, "constants length", test);
  for (int i = 0; i < expected->length; i++) {
    Object expected_constant = expected->constants[i];
    Object actual_constant = actual->constants[i];
    switch (expected_constant.type) {
      case INTEGER_OBJ:
        assert_integer_object(expected_constant.value.i, actual_constant, test);
        break;
      default:
        printf(
          "ERROR: unhandled constant type=%s\n", object_type(actual_constant));
        exit(EXIT_FAILURE);
    }
  }
}

void run_compiler_tests(int len, CompilerTest tests[len], char* test) {
  for (int i = 0; i < len; i++) {
    CompilerTest t = tests[i];
    Program* program = parse_program(t.input);
    compiler_init();
    compile(program, PROGRAM_NODE);
    Bytecode* bytecode = compiler_bytecode();
    test_instructions(t.expected_instructions, bytecode->instructions, test);
    test_constants(t.expected_constants, bytecode->constants, test);
  }
}

void test_integer_arithmetic(void) {
  char* n = "integer_arithmetic";
  CompilerTest test = {
    .input = "1 + 2",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 1}},   //
      (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
    .expected_instructions = code_concat_ins(2,   //
      code_make(OP.constant, 0),                  //
      code_make(OP.constant, 1)),                 //
  };
  run_compiler_tests(1, (CompilerTest[1]){test}, n);
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_integer_arithmetic();
  printf("\n");
  return 0;
}
