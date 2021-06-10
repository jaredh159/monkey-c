#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../code/code.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048

struct Vm_t {
  Instruct* instructions;
  ConstantPool* constant_pool;
  Object* stack[STACK_SIZE];
  Object** globals;
  int sp;
};

static VmErr err = NULL;

static VmErr push(Vm vm, Object* object);
static Object* pop(Vm vm);
static Object* bool_obj(bool boolean);
static VmErr exec_binary_operation(Vm vm, OpCode op);
static VmErr exec_binary_int_operation(Vm vm, OpCode op, int left, int right);
static VmErr exec_comparison(Vm vm, OpCode op);
static VmErr exec_int_comparison(Vm vm, OpCode op, int left, int right);
static VmErr exec_bang_operator(Vm vm);
static VmErr exec_minus_operator(Vm vm);
static void err_init(void);
void inspect_stack(Vm vm, const char* fn);

Vm vm_new(Bytecode* bytecode) {
  err_init();
  struct Vm_t* vm = malloc(sizeof(struct Vm_t));
  vm->globals = calloc(GLOBALS_SIZE, sizeof(Object*));
  vm->instructions = bytecode->instructions;
  vm->constant_pool = bytecode->constants;
  vm->sp = 0;
  return vm;
}

Vm vm_new_with_globals(Bytecode* bytecode, Object** globals) {
  err_init();
  struct Vm_t* vm = malloc(sizeof(struct Vm_t));
  vm->globals = globals;
  vm->instructions = bytecode->instructions;
  vm->constant_pool = bytecode->constants;
  vm->sp = 0;
  return vm;
}

VmErr vm_run(Vm vm) {
  int global_index;
  for (int ip = 0; ip < vm->instructions->length; ip++) {
    OpCode op = vm->instructions->bytes[ip];
    switch (op) {
      case OP_CONSTANT: {
        int const_idx = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        err = push(vm, &vm->constant_pool->constants[const_idx]);
        if (err)
          return err;
      } break;
      case OP_TRUE:
        err = push(vm, &TRUE);
        if (err)
          return err;
        break;
      case OP_FALSE:
        err = push(vm, &FALSE);
        if (err)
          return err;
        break;
      case OP_ADD:  // fallthrough
      case OP_SUB:  // fallthrough
      case OP_MUL:  // fallthrough
      case OP_DIV:  // fallthrough
        err = exec_binary_operation(vm, op);
        if (err)
          return err;
        break;
      case OP_EQUAL:         // fallthrough
      case OP_NOT_EQUAL:     // fallthrough
      case OP_GREATER_THAN:  // fallthrough
        err = exec_comparison(vm, op);
        if (err)
          return err;
        break;
      case OP_POP:
        pop(vm);
        break;
      case OP_MINUS:
        err = exec_minus_operator(vm);
        if (err)
          return err;
        break;
      case OP_BANG:
        err = exec_bang_operator(vm);
        if (err)
          return err;
        break;
      case OP_JUMP: {
        int pos = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip = pos - 1;
      } break;
      case OP_JUMP_NOT_TRUTHY: {
        int pos = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        Object* condition = pop(vm);
        if (!is_truthy(*condition)) {
          ip = pos - 1;
        }
      } break;
      case OP_NULL:
        err = push(vm, &M_NULL);
        if (err)
          return err;
        break;
      case OP_GET_GLOBAL:
        global_index = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        err = push(vm, vm->globals[global_index]);
        if (err)
          return err;
        break;
      case OP_SET_GLOBAL:
        global_index = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        vm->globals[global_index] = pop(vm);
        break;
    }
  }
  return NULL;
}

VmErr exec_minus_operator(Vm vm) {
  Object* operand = pop(vm);
  if (operand->type != INTEGER_OBJ) {
    sprintf(err, "unsupported type for negation: %s", object_type(*operand));
    return err;
  }
  Object* inverse = malloc(sizeof(Object));
  inverse->type = INTEGER_OBJ;
  inverse->value.i = -(operand->value.i);
  return push(vm, inverse);
}

VmErr exec_bang_operator(Vm vm) {
  Object* operand = pop(vm);
  if (operand->type == BOOLEAN_OBJ) {
    return push(vm, bool_obj(!operand->value.b));
  } else if (operand->type == NULL_OBJ) {
    return push(vm, &TRUE);
  } else {
    return push(vm, &FALSE);
  }
}

VmErr exec_comparison(Vm vm, OpCode op) {
  Object* right = pop(vm);
  Object* left = pop(vm);
  if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
    return exec_int_comparison(vm, op, left->value.i, right->value.i);
  }
  if (left->type != BOOLEAN_OBJ && right->type != BOOLEAN_OBJ) {
    sprintf(err, "unsupported types for comparison operation: %s %s",
      object_type(*left), object_type(*right));
    return err;
  }
  switch (op) {
    case OP_EQUAL:
      return push(vm, bool_obj(right->value.b == left->value.b));
    case OP_NOT_EQUAL:
      return push(vm, bool_obj(right->value.b != left->value.b));
    default:
      sprintf(err, "unknown operator: %d, (%s %s)", op, object_type(*left),
        object_type(*right));
      return err;
  }
}

VmErr exec_int_comparison(Vm vm, OpCode op, int leftValue, int rightValue) {
  switch (op) {
    case OP_EQUAL:
      return push(vm, bool_obj(leftValue == rightValue));
    case OP_NOT_EQUAL:
      return push(vm, bool_obj(leftValue != rightValue));
    case OP_GREATER_THAN:
      return push(vm, bool_obj(leftValue > rightValue));
    default:
      sprintf(err, "unknown operator: %d", op);
      return err;
  }
}

VmErr exec_binary_operation(Vm vm, OpCode op) {
  Object* right = pop(vm);
  Object* left = pop(vm);
  if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ)
    return exec_binary_int_operation(vm, op, left->value.i, right->value.i);
  sprintf(err, "unsupported types for binary operation: %s %s",
    object_type(*left), object_type(*right));
  return err;
}

VmErr exec_binary_int_operation(Vm vm, OpCode op, int left, int right) {
  Object* object = malloc(sizeof object);
  object->type = INTEGER_OBJ;
  switch ((int)op) {
    case OP_SUB:
      object->value.i = left - right;
      break;
    case OP_ADD:
      object->value.i = left + right;
      break;
    case OP_MUL:
      object->value.i = left * right;
      break;
    case OP_DIV:
      object->value.i = left / right;
      break;
    default:
      sprintf(err, "unknown integer operator: %d", op);
      return err;
  }
  push(vm, object);
  return NULL;
}

VmErr push(Vm vm, Object* object) {
  if (vm->sp > STACK_SIZE)
    return "stack overflow";
  vm->stack[vm->sp] = object;
  vm->sp++;
  return NULL;
}

Object* pop(Vm vm) {
  Object* object = vm->stack[--vm->sp];
  return object;
}

Object* vm_stack_top(Vm vm) {
  if (vm->sp == 0) {
    return NULL;
  } else {
    return vm->stack[vm->sp - 1];
  }
}

Object* vm_last_popped(Vm vm) {
  return vm->stack[vm->sp];
}

Object* bool_obj(bool boolean) {
  return boolean ? &TRUE : &FALSE;
}

static void err_init(void) {
  if (err == NULL)
    err = malloc(500);
  *err = '\0';
}

void inspect_stack(Vm vm, const char* fn) {
  printf("\n-- inspected from (%s) sp=%d --\n", fn, vm->sp);
  for (int i = 0; i < vm->sp; i++) {
    printf("vm->stack[%d]=%p\n", i, (void*)vm->stack[i]);
    object_print(*vm->stack[i]);
  }
}
