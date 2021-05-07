#ifndef __CODE_H__
#define __CODE_H__

typedef unsigned char UInt8;
typedef unsigned char Byte;
typedef unsigned char Opcode;

typedef struct OpCodes {
  Byte constant;
} OpCodes;

typedef struct BytecodeFragment {
  UInt8 length;
  Byte* bytes;
} BytecodeFragment;

extern OpCodes OP;

typedef struct Definition {
  char* name;
  int operand_widths[3];
  int num_operands;
} Definition;

Definition* code_opcode_lookup(Opcode);
BytecodeFragment* code_make(Opcode, int*);

#endif  // __CODE_H__
