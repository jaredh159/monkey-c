#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../code/code.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048

#define SET_ERR(fmt, ...)             \
  do {                                \
    err = malloc(500 * sizeof(char)); \
    sprintf(err, fmt, __VA_ARGS__);   \
  } while (0)

struct Vm_t {
  Instruct* instructions;
  ConstantPool* constant_pool;
  Object* stack[STACK_SIZE];
  Object** globals;
  int sp;
};

static VmErr push(Vm vm, Object* object);
static Object* pop(Vm vm);
static Object* bool_obj(bool boolean);
static Object* build_array(Vm vm, int start_index, int end_index);
static Object* build_hash(Vm vm, int start_index, int end_index);
static VmErr exec_index_expr(Vm vm, Object* left, Object* index);
static VmErr exec_array_index(Vm vm, Object* array, Object* index);
static VmErr exec_hash_index(Vm vm, Object* hash, Object* index);
static VmErr exec_binary_operation(Vm vm, OpCode op);
static VmErr exec_binary_int_operation(Vm vm, OpCode op, int left, int right);
static VmErr exec_binary_str_operation(
  Vm vm, OpCode op, char* left, char* right);
static VmErr exec_comparison(Vm vm, OpCode op);
static VmErr exec_int_comparison(Vm vm, OpCode op, int left, int right);
static VmErr exec_bang_operator(Vm vm);
static VmErr exec_minus_operator(Vm vm);
void inspect_stack(Vm vm, const char* fn);

static VmErr err = NULL;

Vm vm_new(Bytecode* bytecode) {
  struct Vm_t* vm = malloc(sizeof(struct Vm_t));
  vm->globals = calloc(GLOBALS_SIZE, sizeof(Object*));
  vm->instructions = bytecode->instructions;
  vm->constant_pool = bytecode->constants;
  vm->sp = 0;
  return vm;
}

Vm vm_new_with_globals(Bytecode* bytecode, Object** globals) {
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

      case OP_ARRAY: {
        int num_elements = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        Object* array = build_array(vm, vm->sp - num_elements, vm->sp);
        vm->sp = vm->sp - num_elements;
        err = push(vm, array);
        if (err)
          return err;
      } break;

      case OP_HASH: {
        int num_elements = read_uint16(&vm->instructions->bytes[ip + 1]);
        ip += 2;
        Object* hash = build_hash(vm, vm->sp - num_elements, vm->sp);
        vm->sp = vm->sp - num_elements;
        err = push(vm, hash);
        if (err)
          return err;
      } break;

      case OP_INDEX: {
        Object* index = pop(vm);
        Object* left = pop(vm);
        err = exec_index_expr(vm, left, index);
        if (err)
          return err;
      } break;
    }
  }
  return NULL;
}

static VmErr exec_index_expr(Vm vm, Object* left, Object* index) {
  if (left->type == ARRAY_OBJ && index->type == INTEGER_OBJ) {
    return exec_array_index(vm, left, index);
  } else if (left->type == HASH_OBJ) {
    return exec_hash_index(vm, left, index);
  } else {
    SET_ERR("index operator not supported: %s", object_type(*left));
    return err;
  }
}

static VmErr exec_array_index(Vm vm, Object* array, Object* index) {
  List* elements = array->value.list;
  int i = index->value.i;
  int max = list_count(elements) - 1;
  if (i < 0 || i > max) {
    return push(vm, &M_NULL);
  }

  List* current = elements;
  for (int j = 0; j <= max; j++, current = current->next) {
    if (j == i) {
      return push(vm, current->item);
    }
  }

  puts("unexpected error indexing into array");
  exit(EXIT_FAILURE);
}

static VmErr exec_hash_index(Vm vm, Object* hash, Object* index) {
  char* index_hash = object_hash(*index);
  if (index_hash == NULL) {
    SET_ERR("unusable as hash key: %s", object_type(*index));
    return err;
  }

  for (List* current = hash->value.list; current; current = current->next) {
    HashPair* pair = current->item;
    char* key_hash = object_hash(*pair->key);
    if (strcmp(key_hash, index_hash) == 0)
      return push(vm, pair->value);
  }

  return push(vm, &M_NULL);
}

static Object* build_array(Vm vm, int start_index, int end_index) {
  Object* array = malloc(sizeof(Object));
  array->type = ARRAY_OBJ;
  List* elements = NULL;
  for (int i = start_index; i < end_index; i++)
    elements = list_append(elements, vm->stack[i]);
  array->value.list = elements;
  return array;
}

static Object* build_hash(Vm vm, int start_index, int end_index) {
  Object* hash = malloc(sizeof(Object));
  hash->type = HASH_OBJ;
  List* pairs = NULL;
  for (int i = start_index; i < end_index; i += 2) {
    Object* key = vm->stack[i];
    Object* value = vm->stack[i + 1];
    HashPair* pair = malloc(sizeof(HashPair));
    pair->key = key;
    pair->value = value;
    pairs = list_append(pairs, pair);
  }
  hash->value.list = pairs;
  return hash;
}

VmErr exec_minus_operator(Vm vm) {
  Object* operand = pop(vm);
  if (operand->type != INTEGER_OBJ) {
    SET_ERR("unsupported type for negation: %s", object_type(*operand));
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
    SET_ERR("unsupported types for comparison operation: %s %s",
      object_type(*left), object_type(*right));
    return err;
  }
  switch (op) {
    case OP_EQUAL:
      return push(vm, bool_obj(right->value.b == left->value.b));
    case OP_NOT_EQUAL:
      return push(vm, bool_obj(right->value.b != left->value.b));
    default:
      SET_ERR("unknown operator: %d, (%s %s)", op, object_type(*left),
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
      SET_ERR("unknown operator: %d", op);
      return err;
  }
}

VmErr exec_binary_operation(Vm vm, OpCode op) {
  Object* right = pop(vm);
  Object* left = pop(vm);

  if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ)
    return exec_binary_int_operation(vm, op, left->value.i, right->value.i);

  if (left->type == STRING_OBJ && right->type == STRING_OBJ)
    return exec_binary_str_operation(vm, op, left->value.str, right->value.str);

  SET_ERR("unsupported types for binary operation: %s %s", object_type(*left),
    object_type(*right));
  return err;
}

static VmErr exec_binary_int_operation(Vm vm, OpCode op, int left, int right) {
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
      SET_ERR("unknown integer operator: %d", op);
      return err;
  }
  return push(vm, object);
}

static VmErr exec_binary_str_operation(
  Vm vm, OpCode op, char* left, char* right) {
  if (op != OP_ADD) {
    SET_ERR("unknown string operator: %d", op);
    return err;
  }
  char* combined = malloc(strlen(left) + strlen(right) + 1);
  sprintf(combined, "%s%s", left, right);
  Object* object = malloc(sizeof(Object));
  object->type = STRING_OBJ;
  object->value.str = combined;
  return push(vm, object);
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

void inspect_stack(Vm vm, const char* fn) {
  printf("\n-- inspected from (%s) sp=%d --\n", fn, vm->sp);
  for (int i = 0; i < vm->sp; i++) {
    printf("vm->stack[%d]=%p\n", i, (void*)vm->stack[i]);
    object_print(*vm->stack[i]);
  }
}
