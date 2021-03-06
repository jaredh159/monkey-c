#ifndef __CODE_H__
#define __CODE_H__

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned char Byte;
typedef unsigned char OpCode;

enum OpCodes {
  OP_CONSTANT,
  OP_ADD,
  OP_POP,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_TRUE,
  OP_FALSE,
  OP_NULL,
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_GREATER_THAN,
  OP_MINUS,
  OP_BANG,
  OP_JUMP,
  OP_JUMP_NOT_TRUTHY,
  OP_SET_GLOBAL,
  OP_GET_GLOBAL,
  OP_SET_LOCAL,
  OP_GET_LOCAL,
  OP_ARRAY,
  OP_HASH,
  OP_INDEX,
  OP_CALL,
  OP_RETURN,
  OP_RETURN_VALUE,
  OP_GET_BUILTIN,
  OP_CLOSURE,
  OP_CURRENT_CLOSURE,
  OP_GET_FREE,
};

typedef struct Instruct {
  UInt8 length;
  Byte* bytes;
} Instruct;

typedef struct IntBag {
  int arr[3];
  int len;
} IntBag;

typedef struct Definition {
  char* name;
  int operand_widths[3];
  int num_operands;
} Definition;

typedef struct ReadOpResult {
  IntBag operands;
  int bytes_read;
} ReadOpResult;

Definition* code_opcode_lookup(OpCode);

/**
 * Variadic version
 */
Instruct* code_make(int, ...);

/**
 * Non-variadic (`nv`) version
 */
Instruct* code_make_nv(int, IntBag);

Instruct* code_concat_ins(int, ...);
ReadOpResult code_read_operands(Definition, Instruct);
char* instructions_str(Instruct instructions);
UInt16 read_uint16(Byte* byte);

IntBag int_bag(int len, ...);
IntBag i(int i1);
IntBag ii(int i1, int i2);
IntBag iii(int i1, int i2, int i3);

#endif  // __CODE_H__
