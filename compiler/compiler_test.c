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

void test_instructions(Instruct* expected, Instruct* actual, char* test);
void test_constants(ConstantPool* expected, ConstantPool* actual, char* test);
void run_compiler_tests(int len, CompilerTest tests[len], const char* test);

void test_hash_literals(void) {
  CompilerTest tests[] = {
    {
      .input = "{}",                                //
      .expected_constants = make_constant_pool(0),  //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_HASH, 0),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "{1: 2, 3: 4, 5: 6}",                //
      .expected_constants = make_constant_pool(6,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 3}},   //
        (Object){INTEGER_OBJ, .value = {.i = 4}},   //
        (Object){INTEGER_OBJ, .value = {.i = 5}},   //
        (Object){INTEGER_OBJ, .value = {.i = 6}}),  //
      .expected_instructions = code_concat_ins(8,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_CONSTANT, 3),                  //
        code_make(OP_CONSTANT, 4),                  //
        code_make(OP_CONSTANT, 5),                  //
        code_make(OP_HASH, 6),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "{1: 2 + 3, 4: 5 * 6}",              //
      .expected_constants = make_constant_pool(6,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 3}},   //
        (Object){INTEGER_OBJ, .value = {.i = 4}},   //
        (Object){INTEGER_OBJ, .value = {.i = 5}},   //
        (Object){INTEGER_OBJ, .value = {.i = 6}}),  //
      .expected_instructions = code_concat_ins(10,  //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_ADD),                          //
        code_make(OP_CONSTANT, 3),                  //
        code_make(OP_CONSTANT, 4),                  //
        code_make(OP_CONSTANT, 5),                  //
        code_make(OP_MUL),                          //
        code_make(OP_HASH, 4),                      //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_array_literals(void) {
  CompilerTest tests[] = {
    {
      .input = "[]",                                //
      .expected_constants = make_constant_pool(0),  //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_ARRAY, 0),                     //
        code_make(OP_POP)),                         //
    },
    {
      .input = "[1, 2, 3]",                         //
      .expected_constants = make_constant_pool(3,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 3}}),  //
      .expected_instructions = code_concat_ins(5,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_ARRAY, 3),                     //
        code_make(OP_POP)),                         //
    },
    {
      .input = "[1 + 2, 3 - 4, 5 * 6]",             //
      .expected_constants = make_constant_pool(6,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 3}},   //
        (Object){INTEGER_OBJ, .value = {.i = 4}},   //
        (Object){INTEGER_OBJ, .value = {.i = 5}},   //
        (Object){INTEGER_OBJ, .value = {.i = 6}}),  //
      .expected_instructions = code_concat_ins(11,  //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_ADD),                          //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_CONSTANT, 3),                  //
        code_make(OP_SUB),                          //
        code_make(OP_CONSTANT, 4),                  //
        code_make(OP_CONSTANT, 5),                  //
        code_make(OP_MUL),                          //
        code_make(OP_ARRAY, 3),                     //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_string_expressions(void) {
  CompilerTest tests[] = {
    {
      .input = "\"monkey\"",                                //
      .expected_constants = make_constant_pool(1,           //
        (Object){STRING_OBJ, .value = {.str = "monkey"}}),  //
      .expected_instructions = code_concat_ins(2,           //
        code_make(OP_CONSTANT, 0),                          //
        code_make(OP_POP)),                                 //
    },
    {
      .input = "\"mon\" + \"key\"",                      //
      .expected_constants = make_constant_pool(2,        //
        (Object){STRING_OBJ, .value = {.str = "mon"}},   //
        (Object){STRING_OBJ, .value = {.str = "key"}}),  //
      .expected_instructions = code_concat_ins(4,        //
        code_make(OP_CONSTANT, 0),                       //
        code_make(OP_CONSTANT, 1),                       //
        code_make(OP_ADD),                               //
        code_make(OP_POP)),                              //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_integer_arithmetic(void) {
  CompilerTest tests[] = {
    {
      .input = "1 + 2",                             //
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_ADD),                          //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1; 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_POP),                          //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1 - 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_SUB),                          //
        code_make(OP_POP)),                         //
    },
    {
      .input = "1 * 2",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_MUL),                          //
        code_make(OP_POP)),                         //
    },
    {
      .input = "2 / 1",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_DIV),                          //
        code_make(OP_POP)),                         //
    },
    {
      .input = "-1",
      .expected_constants = make_constant_pool(1,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(3,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_MINUS),                        //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, "integer_arithmetic");
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
    {
      .input = "!true",
      .expected_constants = make_constant_pool(0),
      .expected_instructions = code_concat_ins(3,  //
        code_make(OP_TRUE),                        //
        code_make(OP_BANG),                        //
        code_make(OP_POP)),                        //
    },
  };
  run_compiler_tests(LEN(tests), tests, "boolean_expressions");
}

void test_conditionals(void) {
  CompilerTest tests[] = {
    {
      .input = "if (true) { 10 }; 3333;",
      .expected_constants = make_constant_pool(2,      //
        (Object){INTEGER_OBJ, .value = {.i = 10}},     //
        (Object){INTEGER_OBJ, .value = {.i = 3333}}),  //
      .expected_instructions = code_concat_ins(8,      //
        code_make(OP_TRUE),                            // 0000
        code_make(OP_JUMP_NOT_TRUTHY, 10),             // 0001
        code_make(OP_CONSTANT, 0),                     // 0004
        code_make(OP_JUMP, 11),                        // 0007
        code_make(OP_NULL),                            // 0010
        code_make(OP_POP),                             // 0011
        code_make(OP_CONSTANT, 1),                     // 0012
        code_make(OP_POP)),                            // 0015
    },
    {
      .input = "if (true) { 10 } else { 20 }; 3333;",
      .expected_constants = make_constant_pool(3,      //
        (Object){INTEGER_OBJ, .value = {.i = 10}},     //
        (Object){INTEGER_OBJ, .value = {.i = 20}},     //
        (Object){INTEGER_OBJ, .value = {.i = 3333}}),  //
      .expected_instructions = code_concat_ins(8,      //
        code_make(OP_TRUE),                            // 0000
        code_make(OP_JUMP_NOT_TRUTHY, 10),             // 0001
        code_make(OP_CONSTANT, 0),                     // 0004
        code_make(OP_JUMP, 13),                        // 0007
        code_make(OP_CONSTANT, 1),                     // 0010
        code_make(OP_POP),                             // 0013
        code_make(OP_CONSTANT, 2),                     // 0014
        code_make(OP_POP)),                            // 0017
    },
  };
  run_compiler_tests(LEN(tests), tests, "test_conditionals");
}

void test_global_let_statements(void) {
  CompilerTest tests[] = {
    {
      .input = "let one = 1; let two = 2;",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_SET_GLOBAL, 1)),               //
    },
    {
      .input = "let one = 1; one;",
      .expected_constants = make_constant_pool(1,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_GET_GLOBAL, 0),                //
        code_make(OP_POP)),                         //
    },
    {
      .input = "let one = 1; let two = one; two;",
      .expected_constants = make_constant_pool(1,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(6,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_GET_GLOBAL, 0),                //
        code_make(OP_SET_GLOBAL, 1),                //
        code_make(OP_GET_GLOBAL, 1),                //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, "global_let_statements");
}

void test_index_expressions(void) {
  CompilerTest tests[] = {
    {
      .input = "[1, 2, 3][1 + 1]",
      .expected_constants = make_constant_pool(5,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 3}},   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(9,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_ARRAY, 3),                     //
        code_make(OP_CONSTANT, 3),                  //
        code_make(OP_CONSTANT, 4),                  //
        code_make(OP_ADD),                          //
        code_make(OP_INDEX),                        //
        code_make(OP_POP)),                         //
    },
    {
      .input = "{1: 2}[2 - 1]",
      .expected_constants = make_constant_pool(4,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 2}},   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(8,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CONSTANT, 1),                  //
        code_make(OP_HASH, 2),                      //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_CONSTANT, 3),                  //
        code_make(OP_SUB),                          //
        code_make(OP_INDEX),                        //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_index_expressions();
  test_hash_literals();
  test_array_literals();
  test_string_expressions();
  test_global_let_statements();
  test_conditionals();
  test_boolean_expressions();
  test_integer_arithmetic();
  printf("\n");
  return 0;
}

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
      case STRING_OBJ:
        assert_str_is(expected_constant.value.str, actual_constant.value.str,
          "string constant correct", test);
        break;
      default:
        printf(
          "ERROR: unhandled constant type=%s\n", object_type(actual_constant));
        exit(EXIT_FAILURE);
    }
  }
}

void run_compiler_tests(int len, CompilerTest tests[len], const char* test) {
  for (int i = 0; i < len; i++) {
    CompilerTest t = tests[i];
    Program* program = parse_program(t.input);
    Compiler compiler = compiler_new();
    char* err = compile(compiler, program, PROGRAM_NODE);
    if (err) {
      fail(ss("compiler error: %s", err), test);
    }
    Bytecode* bytecode = compiler_bytecode(compiler);
    char test_input[256];
    sprintf(test_input, "%s, input=`%s`", test, t.input);
    test_instructions(
      t.expected_instructions, bytecode->instructions, test_input);
    test_constants(t.expected_constants, bytecode->constants, test_input);
  }
}
