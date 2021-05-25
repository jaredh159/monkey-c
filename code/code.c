#include "code.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Instruct* code_make(int op_int, ...) {
  // can't `va_start()` with type smaller than a word
  OpCode op = op_int;

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
  code->bytes = bytes;
  bytes[0] = op;

  if (def->num_operands == 0) {
    return code;
  }

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
  return code;
}

Definition* code_opcode_lookup(OpCode op) {
  Definition* def = malloc(sizeof(Definition));
  def->num_operands = 0;
  memcpy(def->operand_widths, (int[3]){0, 0, 0}, 3 * sizeof(int));
  switch (op) {
    case OP_CONSTANT:
      def->operand_widths[0] = 2;
      def->num_operands = 1;
      def->name = "OpConstant";
      break;
    case OP_ADD:
      def->name = "OpAdd";
      break;
    case OP_POP:
      def->name = "OpPop";
      break;
    case OP_SUB:
      def->name = "OpSub";
      break;
    case OP_MUL:
      def->name = "OpMul";
      break;
    case OP_DIV:
      def->name = "OpDiv";
      break;
    default:
      free(def);
      return NULL;
  }
  return def;
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

char* instructions_str(Instruct ins) {
  char* str = malloc(50 * sizeof(char) * ins.length);
  int pos = 0;
  int total_length = ins.length;

  for (int i = 0; i < total_length;) {
    Definition* def = code_opcode_lookup(*ins.bytes);
    if (def == NULL) {
      pos += sprintf(&str[pos], "ERROR: no def. for opcode=%d\n", ins.bytes[i]);
      continue;
    }

    pos += sprintf(&str[pos], "%04d ", i);

    ReadOpResult res = code_read_operands(*def, ins);
    if (res.operands.len != def->num_operands) {
      pos +=
        sprintf(&str[pos], "ERROR: operand len %d does not match defined %d\n",
          res.operands.len, def->num_operands);
      continue;
    }

    switch (def->num_operands) {
      case 0:
        pos += sprintf(&str[pos], "%s\n", def->name);
        break;
      case 1:
        pos += sprintf(&str[pos], "%s %d\n", def->name, res.operands.arr[0]);
        break;
      default:
        pos += sprintf(
          &str[pos], "ERROR: unhandled operand count for %s\n", def->name);
        break;
    }

    int consumed_bytes = 1 + res.bytes_read;
    i += consumed_bytes;
    ins.length -= consumed_bytes;
    ins.bytes = ins.bytes + consumed_bytes;
  }

  str[pos + 1] = '\0';
  return str;
}

UInt16 read_uint16(Byte* byte) {
  Byte first = *byte;
  Byte second = *(byte + 1);
  return (first << 8) + second;
}

ReadOpResult code_read_operands(Definition def, Instruct instructions) {
  IntBag operands;
  operands.len = def.num_operands;

  int bytes_read = 0;
  for (int i = 0; i < def.num_operands; i++) {
    int width = def.operand_widths[i];
    switch (width) {
      case 2:
        operands.arr[i] = (int)read_uint16(&instructions.bytes[bytes_read + 1]);
        break;
    }
    bytes_read += width;
  }

  return (ReadOpResult){.operands = operands, .bytes_read = bytes_read};
}

Instruct* code_make_nv(int op_int, IntBag operands) {
  switch (operands.len) {
    case 0:
      return code_make(op_int);
    case 1:
      return code_make(op_int, operands.arr[0]);
    case 2:
      return code_make(op_int, operands.arr[0], operands.arr[1]);
    case 3:
      return code_make(
        op_int, operands.arr[0], operands.arr[1], operands.arr[2]);
    default:
      printf("ERROR: unexpected num int operands: %d\n", operands.len);
      exit(EXIT_FAILURE);
  }
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
