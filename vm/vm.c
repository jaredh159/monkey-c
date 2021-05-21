#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include "../code/code.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048

static Instruct* instructions;
static ConstantPool* constant_pool;
static Object* stack[STACK_SIZE];
int sp;

static VmErr push(Object* object);

void vm_init(Bytecode* bytecode) {
  instructions = bytecode->instructions;
  constant_pool = bytecode->constants;
  sp = 0;
}

VmErr vm_run(void) {
  VmErr err = NULL;
  for (int ip = 0; ip < instructions->length; ip++) {
    OpCode op = instructions->bytes[ip];
    switch (op) {
      case OP_CONSTANT: {
        int const_idx = read_uint16(&instructions->bytes[ip + 1]);
        ip += 2;
        err = push(&constant_pool->constants[const_idx]);
        if (err)
          return err;
      } break;
    }
  }
  return NULL;
}

VmErr push(Object* object) {
  if (sp > STACK_SIZE)
    return "stack overflow";
  stack[sp++] = object;
  return NULL;
}

Object* vm_stack_top(void) {
  if (sp == 0) {
    return NULL;
  } else {
    return stack[sp - 1];
  }
}
