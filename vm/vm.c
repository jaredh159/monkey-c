#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include "../code/code.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048

static Instruct* instructions;
static ConstantPool* constant_pool;
static Object* stack[STACK_SIZE];
static int sp;
static VmErr err = NULL;

static VmErr push(Object* object);
Object* pop();
VmErr execute_binary_operation(OpCode op);
VmErr execute_binary_integer_operation(
  OpCode op, IntegerLiteral* left, IntegerLiteral* right);

void vm_init(Bytecode* bytecode) {
  free(err);
  err = malloc(500);
  instructions = bytecode->instructions;
  constant_pool = bytecode->constants;
  sp = 0;
}

VmErr vm_run(void) {
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
      case OP_ADD:  // fallthrough
      case OP_SUB:  // fallthrough
      case OP_MUL:  // fallthrough
      case OP_DIV:  // fallthrough
        err = execute_binary_operation(op);
        if (err)
          return err;
        break;
      case OP_POP:
        pop();
        break;
    }
  }
  return NULL;
}

VmErr execute_binary_operation(OpCode op) {
  Object* right = pop();
  Object* left = pop();
  if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ)
    return execute_binary_integer_operation(
      op, (IntegerLiteral*)left, (IntegerLiteral*)right);
  sprintf(err, "unsupported types for binary operation: %s %s",
    object_type(*left), object_type(*right));
  return err;
}

VmErr execute_binary_integer_operation(
  OpCode op, IntegerLiteral* left, IntegerLiteral* right) {
  int rightValue = right->value;
  int leftValue = left->value;
  Object* object = malloc(sizeof(Object));
  object->type = INTEGER_OBJ;
  switch ((int)op) {
    case OP_SUB:
      object->value.i = leftValue - rightValue;
      break;
    case OP_ADD:
      object->value.i = leftValue + rightValue;
      break;
    case OP_MUL:
      object->value.i = leftValue * rightValue;
      break;
    case OP_DIV:
      object->value.i = leftValue / rightValue;
      break;
    default:
      sprintf(err, "unknown integer operator: %d", op);
      return err;
  }
  push(object);
  return NULL;
}

VmErr push(Object* object) {
  if (sp > STACK_SIZE)
    return "stack overflow";
  stack[sp++] = object;
  return NULL;
}

Object* pop() {
  return stack[--sp];
}

Object* vm_stack_top(void) {
  if (sp == 0) {
    return NULL;
  } else {
    return stack[sp - 1];
  }
}

Object* vm_last_popped(void) {
  return stack[sp];
}
