#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>

void compiler_init(void) {
  //
}

CompilerError compile(void* node, NodeType type) {
  if (node && type) {
    return NULL;
  }
  return NULL;
}

Bytecode* compiler_bytecode(void) {
  Bytecode* bytecode = malloc(sizeof(Bytecode));
  bytecode->constants = make_constant_pool(0);
  bytecode->instructions = malloc(sizeof(Instruct));
  bytecode->instructions->length = 0;
  bytecode->instructions->bytes = NULL;
  return bytecode;
}

ConstantPool* make_constant_pool(int len, ...) {
  ConstantPool* pool = malloc(sizeof(ConstantPool));
  pool->length = len;

  if (len == 0) {
    pool->constants = NULL;
    return pool;
  }

  va_list ap;
  va_start(ap, len);

  Object* constants = malloc(sizeof(Object) * len);
  for (int i = 0; i < len; i++) {
    Object constant = va_arg(ap, Object);
    constants[i] = constant;
  }

  va_end(ap);

  pool->constants = constants;
  return pool;
}
