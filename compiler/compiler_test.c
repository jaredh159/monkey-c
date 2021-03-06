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
Object make_compiled_fn_obj(int num_locals, Instruct* instructions);

// these are a bit sad, but he tests the internals of the compiler
// and I don't really want to expose all the guts of the compiler in compiler.h
Instruct* compiler_scope_instructions(Compiler c);
OpCode compiler_scope_last_opcode(Compiler c);
OpCode compiler_scope_prev_opcode(Compiler c);
int compiler_scope_index(Compiler c);
void compiler_test_emit(Compiler c, OpCode op_code);
void compiler_enter_scope(Compiler c);
Instruct* compiler_leave_scope(Compiler c);

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

void test_functions(void) {
  CompilerTest tests[] = {
    {
      .input = "fn() { return 5 + 10 }",
      .expected_constants = make_constant_pool(3,   //
        (Object){INTEGER_OBJ, .value = {.i = 5}},   //
        (Object){INTEGER_OBJ, .value = {.i = 10}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(4,                        //
            code_make(OP_CONSTANT, 0),              //
            code_make(OP_CONSTANT, 1),              //
            code_make(OP_ADD),                      //
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_CLOSURE, 2, 0),                //
        code_make(OP_POP)),                         //
    },
    {
      .input = "fn() { 5 + 10 }",
      .expected_constants = make_constant_pool(3,   //
        (Object){INTEGER_OBJ, .value = {.i = 5}},   //
        (Object){INTEGER_OBJ, .value = {.i = 10}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(4,                        //
            code_make(OP_CONSTANT, 0),              //
            code_make(OP_CONSTANT, 1),              //
            code_make(OP_ADD),                      //
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_CLOSURE, 2, 0),                //
        code_make(OP_POP)),                         //
    },
    {
      .input = "fn() { 1; 2 }",
      .expected_constants = make_constant_pool(3,  //
        (Object){INTEGER_OBJ, .value = {.i = 1}},  //
        (Object){INTEGER_OBJ, .value = {.i = 2}},  //
        make_compiled_fn_obj(0,                    //
          code_concat_ins(4,                       //
            code_make(OP_CONSTANT, 0),             //
            code_make(OP_POP),                     //
            code_make(OP_CONSTANT, 1),             //
            code_make(OP_RETURN_VALUE))            //
          )),                                      //
      .expected_instructions = code_concat_ins(2,  //
        code_make(OP_CLOSURE, 2, 0),               //
        code_make(OP_POP)),                        //
    },
    {
      .input = "fn() { }",
      .expected_constants = make_constant_pool(1,        //
        make_compiled_fn_obj(0, code_make(OP_RETURN))),  //
      .expected_instructions = code_concat_ins(2,        //
        code_make(OP_CLOSURE, 0, 0),                     //
        code_make(OP_POP)),                              //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_let_statement_scopes(void) {
  CompilerTest tests[] = {
    {
      .input = "let num = 55; fn() { num }",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 55}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(2,                        //
            code_make(OP_GET_GLOBAL, 0),            //
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(4,   //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_CLOSURE, 1, 0),                //
        code_make(OP_POP)),                         //
    },
    {
      .input = "fn() { let num = 55; num }",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 55}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(4,                        //
            code_make(OP_CONSTANT, 0),              //
            code_make(OP_SET_LOCAL, 0),             //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_CLOSURE, 1, 0),                //
        code_make(OP_POP)),                         //
    },
    {
      .input = "fn() { let a = 55; let b = 77; a + b }",
      .expected_constants = make_constant_pool(3,   //
        (Object){INTEGER_OBJ, .value = {.i = 55}},  //
        (Object){INTEGER_OBJ, .value = {.i = 77}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(8,                        //
            code_make(OP_CONSTANT, 0),              //
            code_make(OP_SET_LOCAL, 0),             //
            code_make(OP_CONSTANT, 1),              //
            code_make(OP_SET_LOCAL, 1),             //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_GET_LOCAL, 1),             //
            code_make(OP_ADD),                      //
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(2,   //
        code_make(OP_CLOSURE, 2, 0),                //
        code_make(OP_POP)),                         //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_function_calls(void) {
  CompilerTest tests[] = {
    {
      .input = "fn() { 24 }();",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 24}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(2,                        //
            code_make(OP_CONSTANT, 0),              // `24`
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(3,   //
        code_make(OP_CLOSURE, 1, 0),                // the compiled fn
        code_make(OP_CALL, 0),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "let noArg = fn() { 24 }; noArg();",
      .expected_constants = make_constant_pool(2,   //
        (Object){INTEGER_OBJ, .value = {.i = 24}},  //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(2,                        //
            code_make(OP_CONSTANT, 0),              // `24`
            code_make(OP_RETURN_VALUE))             //
          )),                                       //
      .expected_instructions = code_concat_ins(5,   //
        code_make(OP_CLOSURE, 1, 0),                // the compiled fn
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_GET_GLOBAL, 0),                //
        code_make(OP_CALL, 0),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "let oneArg = fn(a) {a}; oneArg(24);",
      .expected_constants = make_constant_pool(2,    //
        make_compiled_fn_obj(0,                      //
          code_concat_ins(2,                         //
            code_make(OP_GET_LOCAL, 0),              //
            code_make(OP_RETURN_VALUE))              //
          ),                                         //
        (Object){INTEGER_OBJ, .value = {.i = 24}}),  //
      .expected_instructions = code_concat_ins(6,    //
        code_make(OP_CLOSURE, 0, 0),                 //
        code_make(OP_SET_GLOBAL, 0),                 //
        code_make(OP_GET_GLOBAL, 0),                 //
        code_make(OP_CONSTANT, 1),                   //
        code_make(OP_CALL, 1),                       //
        code_make(OP_POP)),                          //
    },
    {
      .input = "let manyArg = fn(a, b, c) {a; b; c}; manyArg(24, 25, 26);",
      .expected_constants = make_constant_pool(4,           //
        make_compiled_fn_obj(0,                             //
          code_concat_ins(6,                                //
            code_make(OP_GET_LOCAL, 0),                     //
            code_make(OP_POP), code_make(OP_GET_LOCAL, 1),  //
            code_make(OP_POP), code_make(OP_GET_LOCAL, 2),  //
            code_make(OP_RETURN_VALUE))                     //
          ),                                                //
        (Object){INTEGER_OBJ, .value = {.i = 24}},          //
        (Object){INTEGER_OBJ, .value = {.i = 25}},          //
        (Object){INTEGER_OBJ, .value = {.i = 26}}),         //
      .expected_instructions = code_concat_ins(8,           //
        code_make(OP_CLOSURE, 0, 0),                        //
        code_make(OP_SET_GLOBAL, 0),                        //
        code_make(OP_GET_GLOBAL, 0),                        //
        code_make(OP_CONSTANT, 1),                          //
        code_make(OP_CONSTANT, 2),                          //
        code_make(OP_CONSTANT, 3),                          //
        code_make(OP_CALL, 3),                              //
        code_make(OP_POP)),                                 //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_builtins(void) {
  CompilerTest tests[] = {
    {
      .input = "len([]); push([], 1);",
      .expected_constants = make_constant_pool(1,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(9,   //
        code_make(OP_GET_BUILTIN, BUILTIN_LEN),     //
        code_make(OP_ARRAY, 0),                     //
        code_make(OP_CALL, 1),                      //
        code_make(OP_POP),                          //
        code_make(OP_GET_BUILTIN, BUILTIN_PUSH),    //
        code_make(OP_ARRAY, 0),                     //
        code_make(OP_CONSTANT, 0),                  //
        code_make(OP_CALL, 2),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "fn() { len([]) }",
      .expected_constants = make_constant_pool(1,    //
        make_compiled_fn_obj(0,                      //
          code_concat_ins(4,                         //
            code_make(OP_GET_BUILTIN, BUILTIN_LEN),  //
            code_make(OP_ARRAY, 0),                  //
            code_make(OP_CALL, 1),                   //
            code_make(OP_RETURN_VALUE))              //
          )),                                        //
      .expected_instructions = code_concat_ins(2,    //
        code_make(OP_CLOSURE, 0, 0),                 //
        code_make(OP_POP)),                          //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_closures(void) {
  CompilerTest tests[] = {
    {
      .input = "fn(a) { fn(b) { a + b } }",
      .expected_constants = make_constant_pool(2,  //
        make_compiled_fn_obj(1,                    //
          code_concat_ins(4,                       //
            code_make(OP_GET_FREE, 0),             //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_ADD),                     //
            code_make(OP_RETURN_VALUE))),          //
        make_compiled_fn_obj(1,                    //
          code_concat_ins(3,                       //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_CLOSURE, 0, 1),           //
            code_make(OP_RETURN_VALUE)))           //
        ),
      .expected_instructions = code_concat_ins(2,  //
        code_make(OP_CLOSURE, 1, 0),               //
        code_make(OP_POP)),                        //
    },
    {
      .input = "fn(a) { fn(b) { fn(c) { a + b + c } } }",
      .expected_constants = make_constant_pool(3,  //
        make_compiled_fn_obj(1,                    //
          code_concat_ins(6,                       //
            code_make(OP_GET_FREE, 0),             //
            code_make(OP_GET_FREE, 1),             //
            code_make(OP_ADD),                     //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_ADD),                     //
            code_make(OP_RETURN_VALUE))),          //
        make_compiled_fn_obj(1,                    //
          code_concat_ins(4,                       //
            code_make(OP_GET_FREE, 0),             //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_CLOSURE, 0, 2),           //
            code_make(OP_RETURN_VALUE))),          //
        make_compiled_fn_obj(1,                    //
          code_concat_ins(3,                       //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_CLOSURE, 1, 1),           //
            code_make(OP_RETURN_VALUE)))           //
        ),
      .expected_instructions = code_concat_ins(2,  //
        code_make(OP_CLOSURE, 2, 0),               //
        code_make(OP_POP)),                        //
    },
    {
      .input = "\
        let global = 55;\
        fn() {\
          let a = 66;\
          fn() {\
            let b = 77;\
            fn() {\
              let c = 88;\
              global + a + b + c;\
            }\
          }\
        }",
      .expected_constants = make_constant_pool(7,   //
        (Object){INTEGER_OBJ, .value = {.i = 55}},  //
        (Object){INTEGER_OBJ, .value = {.i = 66}},  //
        (Object){INTEGER_OBJ, .value = {.i = 77}},  //
        (Object){INTEGER_OBJ, .value = {.i = 88}},  //
        make_compiled_fn_obj(1,                     //
          code_concat_ins(10,                       //
            code_make(OP_CONSTANT, 3),              //
            code_make(OP_SET_LOCAL, 0),             //
            code_make(OP_GET_GLOBAL, 0),            //
            code_make(OP_GET_FREE, 0),              //
            code_make(OP_ADD),                      //
            code_make(OP_GET_FREE, 1),              //
            code_make(OP_ADD),                      //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_ADD),                      //
            code_make(OP_RETURN_VALUE))),           //
        make_compiled_fn_obj(1,                     //
          code_concat_ins(6,                        //
            code_make(OP_CONSTANT, 2),              //
            code_make(OP_SET_LOCAL, 0),             //
            code_make(OP_GET_FREE, 0),              //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_CLOSURE, 4, 2),            //
            code_make(OP_RETURN_VALUE))),           //
        make_compiled_fn_obj(1,                     //
          code_concat_ins(5,                        //
            code_make(OP_CONSTANT, 1),              //
            code_make(OP_SET_LOCAL, 0),             //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_CLOSURE, 5, 1),            //
            code_make(OP_RETURN_VALUE)))            //
        ),
      .expected_instructions = code_concat_ins(4,  //
        code_make(OP_CONSTANT, 0),                 //
        code_make(OP_SET_GLOBAL, 0),               //
        code_make(OP_CLOSURE, 6, 0),               //
        code_make(OP_POP)),                        //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_recursive_functions(void) {
  CompilerTest tests[] = {
    {
      .input = "let countDown = fn(x) { countDown(x - 1); }"
               "countDown(1);",
      .expected_constants = make_constant_pool(3,   //
        (Object){INTEGER_OBJ, .value = {.i = 1}},   //
        make_compiled_fn_obj(0,                     //
          code_concat_ins(6,                        //
            code_make(OP_CURRENT_CLOSURE),          //
            code_make(OP_GET_LOCAL, 0),             //
            code_make(OP_CONSTANT, 0),              //
            code_make(OP_SUB),                      //
            code_make(OP_CALL, 1),                  //
            code_make(OP_RETURN_VALUE))             //
          ),                                        //
        (Object){INTEGER_OBJ, .value = {.i = 1}}),  //
      .expected_instructions = code_concat_ins(6,   //
        code_make(OP_CLOSURE, 1, 0),                //
        code_make(OP_SET_GLOBAL, 0),                //
        code_make(OP_GET_GLOBAL, 0),                //
        code_make(OP_CONSTANT, 2),                  //
        code_make(OP_CALL, 1),                      //
        code_make(OP_POP)),                         //
    },
    {
      .input = "let wrapper = fn() {"
               "  let countDown = fn(x) { countDown(x - 1); }"
               "  countDown(1);"
               "};"
               "wrapper();",
      .expected_constants = make_constant_pool(4,  //
        (Object){INTEGER_OBJ, .value = {.i = 1}},  //
        make_compiled_fn_obj(0,                    //
          code_concat_ins(6,                       //
            code_make(OP_CURRENT_CLOSURE),         //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_CONSTANT, 0),             //
            code_make(OP_SUB),                     //
            code_make(OP_CALL, 1),                 //
            code_make(OP_RETURN_VALUE))            //
          ),
        (Object){INTEGER_OBJ, .value = {.i = 1}},  //
        make_compiled_fn_obj(0,                    //
          code_concat_ins(6,                       //
            code_make(OP_CLOSURE, 1, 0),           //
            code_make(OP_SET_LOCAL, 0),            //
            code_make(OP_GET_LOCAL, 0),            //
            code_make(OP_CONSTANT, 2),             //
            code_make(OP_CALL, 1),                 //
            code_make(OP_RETURN_VALUE))            //
          )),                                      //
      .expected_instructions = code_concat_ins(5,  //
        code_make(OP_CLOSURE, 3, 0),               //
        code_make(OP_SET_GLOBAL, 0),               //
        code_make(OP_GET_GLOBAL, 0),               //
        code_make(OP_CALL, 0),                     //
        code_make(OP_POP)),                        //
    },
  };
  run_compiler_tests(LEN(tests), tests, __func__);
}

void test_compiler_scopes(void) {
  const char* t = __func__;
  Compiler c = compiler_new();
  assert_int_is(0, compiler_scope_index(c), "scope index starts at 0", t);
  SymbolTable global_symbols = compiler_symbol_table(c);
  compiler_test_emit(c, OP_MUL);

  compiler_enter_scope(c);
  assert_int_is(1, compiler_scope_index(c), "scope should be 1 after enter", t);
  compiler_test_emit(c, OP_SUB);

  assert_int_is(1, compiler_scope_instructions(c)->length,
    "inner scope instructions length should be 1", t);

  OpCode op = compiler_scope_last_opcode(c);
  assert_int_is(OP_SUB, op,
    ss("last instruction op_code wrong, got=%s, want=%s", "OpSub",
      code_opcode_lookup(op)),
    t);

  if (symbol_table_outer(compiler_symbol_table(c)) != global_symbols) {
    fail("compiler did not enclose symbol table", t);
  }

  compiler_leave_scope(c);
  assert_int_is(0, compiler_scope_index(c), "scope index back to 0", t);

  if (compiler_symbol_table(c) != global_symbols) {
    fail("compiler did not restore global symbol table", t);
  }
  if (symbol_table_outer(compiler_symbol_table(c)) != NULL) {
    fail("compiler modified global symbol table incorrectly", t);
  }

  compiler_test_emit(c, OP_ADD);
  assert_int_is(2, compiler_scope_instructions(c)->length,
    "outer scope instructions length should be 2", t);

  op = compiler_scope_last_opcode(c);
  assert_int_is(OP_ADD, op,
    ss("last instruction op_code wrong, got=%s, want=%s", "OpAdd",
      code_opcode_lookup(op)),
    t);

  op = compiler_scope_prev_opcode(c);
  assert_int_is(OP_MUL, op,
    ss("last instruction op_code wrong, got=%s, want=%s", "OpMul",
      code_opcode_lookup(op)),
    t);
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_recursive_functions();
  test_closures();
  test_builtins();
  test_compiler_scopes();
  test_let_statement_scopes();
  test_function_calls();
  test_functions();
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
      case COMPILED_FUNCTION_OBJ:
        assert_int_is(COMPILED_FUNCTION_OBJ, actual_constant.type,
          "constant must be compiled fn", test);
        char fn_test[512];
        sprintf(fn_test, "%s compiled function constant=%d", test, i);
        test_instructions(expected_constant.value.compiled_fn->instructions,
          actual_constant.value.compiled_fn->instructions, fn_test);
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

Object make_compiled_fn_obj(int num_locals, Instruct* instructions) {
  CompiledFunction* compiled_fn = malloc(sizeof(CompiledFunction));
  compiled_fn->num_locals = num_locals;
  compiled_fn->instructions = instructions;
  return (Object){.type = COMPILED_FUNCTION_OBJ, {.compiled_fn = compiled_fn}};
}
