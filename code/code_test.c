#include "code.h"
#include <stdio.h>
#include "../test/test.h"
#include "../utils/colors.h"

extern OpCodes OP;

void test_make(void) {
  char* n = "make";

  struct MakeTest {
    Opcode op;
    int* operands;
    Byte* expected;
    int expected_len;
  };

  struct MakeTest tests[] = {
    {
      .op = OP.constant,
      .operands = (int[1]){65534},
      .expected = (Byte[3]){OP.constant, 255, 254},
      .expected_len = 3,
    },
  };

  int num_tests = sizeof(tests) / sizeof(struct MakeTest);
  for (int i = 0; i < num_tests; i++) {
    struct MakeTest t = tests[i];
    BytecodeFragment* code = code_make(t.op, t.operands);

    assert_int_is(t.expected_len, code->length, "num operands", n);

    for (int j = 0; j < t.expected_len; j++) {
      assert_int_is(
        t.expected[j], code->bytes[j], int_embed("byte at pos=%d", j), n);
    }
  }
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_make();
  printf("\n");
  return 0;
}
