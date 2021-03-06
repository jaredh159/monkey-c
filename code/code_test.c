#include "code.h"
#include <stdio.h>
#include "../test/test.h"
#include "../utils/colors.h"

void test_make(void) {
  char* n = "make";

  struct MakeTest {
    OpCode op;
    IntBag operands;
    Byte* expected;
    int expected_len;
  };

  struct MakeTest tests[] = {
    {
      .op = OP_CONSTANT,
      .operands = i(65534),
      .expected = (Byte[]){OP_CONSTANT, 255, 254},
      .expected_len = 3,
    },
    {
      .op = OP_ADD,
      .operands = (IntBag){0},
      .expected = (Byte[]){OP_ADD},
      .expected_len = 1,
    },
    {
      .op = OP_GET_LOCAL,
      .operands = i(255),
      .expected = (Byte[]){OP_GET_LOCAL, 255},
      .expected_len = 2,
    },
    {
      .op = OP_CLOSURE,
      .operands = ii(65534, 255),
      .expected = (Byte[]){OP_CLOSURE, 255, 254, 255},
      .expected_len = 4,
    },
  };

  for (int i = 0; i < LEN(tests); i++) {
    struct MakeTest t = tests[i];
    Instruct* code = code_make_nv(t.op, t.operands);
    assert_int_is(t.expected_len, code->length, "num operands", n);
    for (int j = 0; j < t.expected_len; j++) {
      assert_int_is(t.expected[j], code->bytes[j], si("byte at pos=%d", j), n);
    }
  }
}

void test_read_operands(void) {
  char* n = "read_operands";
  struct ReadOpTest {
    OpCode op;
    IntBag operands;
    int bytes_read;
  };

  struct ReadOpTest tests[] = {
    {
      .op = OP_CONSTANT,
      .operands = i(65535),
      .bytes_read = 2,
    },
    {
      .op = OP_GET_LOCAL,
      .operands = i(255),
      .bytes_read = 1,
    },
  };

  for (int i = 0; i < LEN(tests); i++) {
    struct ReadOpTest t = tests[i];
    Instruct* instruction = code_make_nv(t.op, t.operands);
    Definition* def = code_opcode_lookup(t.op);
    assert(def != NULL, si("opcode=%d definition missing", t.op), n);
    ReadOpResult res = code_read_operands(*def, *instruction);
    char* failMsg =
      si("wrong num bytes read. want=%d, got=%d", t.bytes_read, res.bytes_read);
    assert(res.bytes_read == t.bytes_read, failMsg, n);
    for (int j = 0; j < t.operands.len; j++) {
      int exp_op = t.operands.arr[j];
      int act_op = res.operands.arr[j];
      char* failMsg = si("operand wrong. want=%d, got=%d", exp_op, act_op);
      assert_int_is(exp_op, act_op, failMsg, n);
    }
  }
}

void test_instructions_string(void) {
  char* n = "instructions_string";
  Instruct* ins = code_concat_ins(5,   //
    code_make(OP_ADD),                 //
    code_make(OP_GET_LOCAL, 1),        //
    code_make(OP_CONSTANT, 2),         //
    code_make(OP_CONSTANT, 65535),     //
    code_make(OP_CLOSURE, 65535, 255)  //
  );

  char* expected =
    "0000 OpAdd\n"
    "0001 OpGetLocal 1\n"
    "0003 OpConstant 2\n"
    "0006 OpConstant 65535\n"
    "0009 OpClosure 65535 255\n";

  assert_str_is(
    expected, instructions_str(*ins), "instruction string correct", n);
}

void test_another_instructions_string(void) {
  char* n = "another_instructions_string";
  Instruct* ins = code_concat_ins(4,  //
    code_make(OP_CONSTANT, 0),        //
    code_make(OP_CONSTANT, 1),        //
    code_make(OP_SUB),                //
    code_make(OP_POP)                 //
  );

  char* expected =
    "0000 OpConstant 0\n"
    "0003 OpConstant 1\n"
    "0006 OpSub\n"
    "0007 OpPop\n";

  assert_str_is(
    expected, instructions_str(*ins), "instruction string correct", n);
}

int main(int argc, char** argv) {
  test_make();
  pass_argv(argc, argv);
  test_read_operands();
  test_instructions_string();
  test_another_instructions_string();
  printf("\n");
  return 0;
}
