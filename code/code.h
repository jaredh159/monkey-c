#ifndef __CODE_H__
#define __CODE_H__

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned char Byte;
typedef unsigned char OpCode;

typedef struct OpCodes {
  Byte constant;
} OpCodes;

extern OpCodes OP;

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

IntBag int_bag(int len, ...);
IntBag i(int i1);
IntBag ii(int i1, int i2);
IntBag iii(int i1, int i2, int i3);

#endif  // __CODE_H__
