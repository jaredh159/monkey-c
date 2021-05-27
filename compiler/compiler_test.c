#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "../code/code.h"
#include "../parser/parser.h"
#include "../test/test.h"
#include "../utils/colors.h"

typedef struct CompilerTest {
  char* input;
  Instruct* expected_instructions;
  ConstantPool* expected_constants;
} CompilerTest;

void test_instructions(Instruct* expected, Instruct* actual, char* test) {
  if (expected->length != actual->length) {
    fail(ss("wrong instructions length, want=%s, got=%s",
           instructions_str(*expected), instructions_str(*actual)),
      test);
  }
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
    char* err = compile(program, PROGRAM_NODE);
    if (err) {
      fail(ss("compiler error: %s", err), test);
    }
    Bytecode* bytecode = compiler_bytecode();
    char test_input[256];
    sprintf(test_input, "%s, input=`%s`", test, t.input);
    test_instructions(
      t.expected_instructions, bytecode->instructions, test_input);
    test_constants(t.expected_constants, bytecode->constants, test_input);
  }
}

void test_integer_arithmetic(void) {
  char* n = "integer_arithmetic";
  CompilerTest t1 = {
    .input = "1 + 2",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 1}},   //
      (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
    .expected_instructions = code_concat_ins(4,   //
      code_make(OP_CONSTANT, 0),                  //
      code_make(OP_CONSTANT, 1),                  //
      code_make(OP_ADD),                          //
      code_make(OP_POP)),                         //
  };
  CompilerTest t2 = {
    .input = "1; 2",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 1}},   //
      (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
    .expected_instructions = code_concat_ins(4,   //
      code_make(OP_CONSTANT, 0),                  //
      code_make(OP_POP),                          //
      code_make(OP_CONSTANT, 1),                  //
      code_make(OP_POP)),                         //
  };
  CompilerTest t3 = {
    .input = "1 - 2",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 1}},   //
      (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
    .expected_instructions = code_concat_ins(4,   //
      code_make(OP_CONSTANT, 0),                  //
      code_make(OP_CONSTANT, 1),                  //
      code_make(OP_SUB),                          //
      code_make(OP_POP)),                         //
  };
  CompilerTest t4 = {
    .input = "1 * 2",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 1}},   //
      (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
    .expected_instructions = code_concat_ins(4,   //
      code_make(OP_CONSTANT, 0),                  //
      code_make(OP_CONSTANT, 1),                  //
      code_make(OP_MUL),                          //
      code_make(OP_POP)),                         //
  };
  CompilerTest t5 = {
    .input = "2 / 1",
    .expected_constants = make_constant_pool(2,   //
      (Object){INTEGER_OBJ, .value = {.i = 2}},   //
      (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
    .expected_instructions = code_concat_ins(4,   //
      code_make(OP_CONSTANT, 0),                  //
      code_make(OP_CONSTANT, 1),                  //
      code_make(OP_DIV),                          //
      code_make(OP_POP)),                         //
  };
  run_compiler_tests(5, (CompilerTest[]){t1, t2, t3, t4, t5}, n);
}

void test_boolean_expressions(void) {
  CompilerTest tests[] = {
    {
      .input = "true",
      .expected_constants = make_constant_pool(0),
      .expected_instructions = code_concat_ins(2,  //
        code_make(OP_TRUE),                        //
        code_make(OP_POP)),                        //
    },
    {
      .input = "false",
      .expected_constants = make_constant_pool(0),
      .expected_instructions = code_concat_ins(2,  //
        code_make(OP_FALSE),                       //
        code_make(OP_POP)),                        //
    },
    {
      .input = "1 > 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_GREATER_THAN),                 //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1 < 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_GREATER_THAN),                 //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1 == 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_EQUAL),                        //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1 != 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_NOT_EQUAL),                    //
        code_make(OP_POP)),                         //
    },
    {
      .input = "true == false",
      .expected_constants = make_constant_pool(0),
      .expected_instructions = code_concat_ins(4,  //
        code_make(OP_TRUE),                        //
        code_make(OP_FALSE),                       //
        code_make(OP_EQUAL),                       //
        code_make(OP_POP)),                        //
    },
    {
      .input = "true != false",
      .expected_constants = make_constant_pool(0),
      .expected_instructions = code_concat_ins(4,  //
        code_make(OP_TRUE),                        //
        code_make(OP_FALSE),                       //
        code_make(OP_NOT_EQUAL),                   //
        code_make(OP_POP)),                        //
    },
  };
  run_compiler_tests(LEN(tests), tests, "boolean_expressions");
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_boolean_expressions();
  test_integer_arithmetic();
  printf("\n");
  return 0;
}
