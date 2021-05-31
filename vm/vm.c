#include "vm.h"
#include <stdbool.h>
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
Object* bool_obj(bool boolean);
VmErr execute_binary_operation(OpCode op);
VmErr execute_binary_integer_operation(
  OpCode op, IntegerLiteral* left, IntegerLiteral* right);
VmErr execute_comparison(OpCode op);
VmErr execute_integer_comparison(
  OpCode op, IntegerLiteral* left, IntegerLiteral* right);
VmErr execute_bang_operator(void);
VmErr execute_minus_operator(void);

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
      case OP_TRUE:
        err = push(&TRUE);
        if (err)
          return err;
        break;
      case OP_FALSE:
        err = push(&FALSE);
        if (err)
          return err;
        break;
      case OP_ADD:  // fallthrough
      case OP_SUB:  // fallthrough
      case OP_MUL:  // fallthrough
      case OP_DIV:  // fallthrough
        err = execute_binary_operation(op);
        if (err)
          return err;
        break;
      case OP_EQUAL:         // fallthrough
      case OP_NOT_EQUAL:     // fallthrough
      case OP_GREATER_THAN:  // fallthrough
        err = execute_comparison(op);
        if (err)
          return err;
        break;
      case OP_POP:
        pop();
        break;
      case OP_MINUS:
        err = execute_minus_operator();
        if (err)
          return err;
        break;
      case OP_BANG:
        err = execute_bang_operator();
        if (err)
          return err;
        break;
      case OP_JUMP: {
        int pos = read_uint16(&instructions->bytes[ip + 1]);
        ip = pos - 1;
      } break;
      case OP_JUMP_NOT_TRUTHY: {
        int pos = read_uint16(&instructions->bytes[ip + 1]);
        ip += 2;
        Object* condition = pop();
        if (!is_truthy(*condition)) {
          ip = pos - 1;
        }
      } break;
      case OP_NULL:
        err = push(&M_NULL);
        if (err)
          return err;
        break;
    }
  }
  return NULL;
}

VmErr execute_minus_operator(void) {
  Object* operand = pop();
  if (operand->type != INTEGER_OBJ) {
    sprintf(err, "unsupported type for negation: %s", object_type(*operand));
    return err;
  }
  Object* inverse = malloc(sizeof(Object));
  inverse->type = INTEGER_OBJ;
  inverse->value.i = -(operand->value.i);
  return push(inverse);
}

VmErr execute_bang_operator(void) {
  Object* operand = pop();
  if (operand->type == BOOLEAN_OBJ) {
    return push(bool_obj(!operand->value.b));
  } else if (operand->type == NULL_OBJ) {
    return push(&TRUE);
  } else {
    return push(&FALSE);
  }
}

VmErr execute_comparison(OpCode op) {
  Object* right = pop();
  Object* left = pop();
  if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
    return execute_integer_comparison(
      op, (IntegerLiteral*)left, (IntegerLiteral*)right);
  }
  if (left->type != BOOLEAN_OBJ && right->type != BOOLEAN_OBJ) {
    sprintf(err, "unsupported types for comparison operation: %s %s",
      object_type(*left), object_type(*right));
    return err;
  }
  BooleanLiteral* rightBool = (BooleanLiteral*)right;
  BooleanLiteral* leftBool = (BooleanLiteral*)left;
  switch (op) {
    case OP_EQUAL:
      return push(bool_obj(rightBool->value == leftBool->value));
    case OP_NOT_EQUAL:
      return push(bool_obj(rightBool->value != leftBool->value));
    default:
      sprintf(err, "unknown operator: %d, (%s %s)", op, object_type(*left),
        object_type(*right));
      return err;
  }
}

VmErr execute_integer_comparison(
  OpCode op, IntegerLiteral* left, IntegerLiteral* right) {
  switch (op) {
    case OP_EQUAL:
      return push(bool_obj(left->value == right->value));
    case OP_NOT_EQUAL:
      return push(bool_obj(left->value != right->value));
    case OP_GREATER_THAN:
      return push(bool_obj(left->value > right->value));
    default:
      sprintf(err, "unknown operator: %d", op);
      return err;
  }
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

Object* bool_obj(bool boolean) {
  return boolean ? &TRUE : &FALSE;
}
