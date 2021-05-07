#include "code.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OpCodes OP = {.constant = 0};

BytecodeFragment* code_make(Opcode op, int* operands) {
  BytecodeFragment* code = malloc(sizeof(BytecodeFragment));
  code->length = 0;
  code->bytes = NULL;

  Definition* def = code_opcode_lookup(op);
  if (def == NULL) {
    return code;
  }

  code->length = 1;
  for (int i = 0; i < def->num_operands; i++) {
    code->length += def->operand_widths[i];
  }

  Byte* bytes = malloc(code->length * sizeof(Byte));
  bytes[0] = op;

  for (int i = 0; i < def->num_operands; i++) {
    int width = def->operand_widths[i];
    unsigned int operand = operands[i];
    switch (width) {
      case 2:
        bytes[1] = operand >> 8;
        bytes[2] = operand & 0xff;
    }
  }

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
