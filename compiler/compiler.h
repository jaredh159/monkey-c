#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "../ast/ast.h"
#include "../code/code.h"
#include "../object/object.h"
#include "symbol_table.h"

#define MAX_CONSTANTS 64
#define MAX_INSTRUCTIONS 1024

typedef char* CompilerErr;

typedef struct ConstantPool {
  UInt8 length;
  Object* constants;
} ConstantPool;

typedef struct Bytecode {
  Instruct* instructions;
  ConstantPool* constants;
} Bytecode;

void compiler_init(void);
CompilerErr compile(void* node, NodeType type);
Bytecode* compiler_bytecode(void);
ConstantPool* make_constant_pool(int len, ...);

#endif  // __COMPILER_H__
