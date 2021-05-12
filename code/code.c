#include "code.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OpCodes OP = {.constant = 0};

Instruct* code_make(int op_int, ...) {
  // can't `va_start()` with type smaller than a word
  Opcode op = op_int;

  Instruct* code = malloc(sizeof(Instruct));
  code->length = 0;
  code->bytes = NULL;

  Definition* def = code_opcode_lookup(op);
  if (def == NULL) {
    printf("ERROR: unexpected opcode=%d\n", op);
    exit(EXIT_FAILURE);
  }

  code->length = 1;
  for (int i = 0; i < def->num_operands; i++) {
    code->length += def->operand_widths[i];
  }

  Byte* bytes = malloc(code->length * sizeof(Byte));
  bytes[0] = op;

  va_list ap;
  va_start(ap, op_int);

  for (int i = 0; i < def->num_operands; i++) {
    int width = def->operand_widths[i];
    int operand = va_arg(ap, int);
    switch (width) {
      case 2:
        bytes[1] = operand >> 8;
        bytes[2] = operand & 0xff;
    }
  }

  va_end(ap);
  code->bytes = bytes;
  return code;
}

Definition* code_opcode_lookup(Opcode op) {
  Definition* def = malloc(sizeof(Definition));
  switch (op) {
    case 0:
      memcpy(def->operand_widths, (int[3]){2}, 3 * sizeof(int));
      def->num_operands = 1;
      def->name = "OpConstant";
      return def;
    default:
      free(def);
      return NULL;
  }
}

Instruct* code_concat_ins(int len, ...) {
  va_list ap;
  va_start(ap, len);

  Instruct* all_ins[len];

  int total_len = 0;
  for (int i = 0; i < len; i++) {
    Instruct* ins = va_arg(ap, Instruct*);
    all_ins[i] = ins;
    total_len += ins->length;
  }
  va_end(ap);

  Instruct* concatted = malloc(sizeof(Instruct));
  Byte* bytes = malloc(total_len * sizeof(Byte));
  int index = 0;
  for (int i = 0; i < len; i++) {
    for (int j = 0; j < all_ins[i]->length; j++) {
      bytes[index++] = all_ins[i]->bytes[j];
    }
  }

  concatted->length = total_len;
  concatted->bytes = bytes;
  return concatted;
}

IntBag int_bag(int len, ...) {
  va_list ap;
  va_start(ap, len);

  IntBag ints;
  for (int i = 0; i < len && i < 3; i++) {
    int next_int = va_arg(ap, int);
    ints.arr[i] = next_int;
  }
  ints.len = len;

  va_end(ap);
  return ints;
}

IntBag i(int i1) {
  return int_bag(1, i1);
}

IntBag ii(int i1, int i2) {
  return int_bag(2, i1, i2);
}

IntBag iii(int i1, int i2, int i3) {
  return int_bag(3, i1, i2, i3);
}
