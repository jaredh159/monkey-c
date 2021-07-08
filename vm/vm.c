#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../code/code.h"
#include "../compiler/compiler.h"

#define STACK_SIZE 2048
#define MAX_FRAMES 1024

#define SET_ERR(fmt, ...)             \
  do {                                \
    err = malloc(500 * sizeof(char)); \
    sprintf(err, fmt, __VA_ARGS__);   \
  } while (0)

typedef struct Frame {
  Closure* cl;
  int ip;
  int base_pointer;
} Frame;

struct Vm_t {
  ConstantPool* constant_pool;
  Object* stack[STACK_SIZE];
  Object** globals;
  Frame* frames[MAX_FRAMES];
  int frames_index;
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
static Object* new_compiled_fn(Instruct* instructions, int num_locals);
static Frame* new_frame(Closure* cl, int base_pointer);
static Frame* current_frame(Vm vm);
static void push_frame(Vm vm, Frame* frame);
static Frame* pop_frame(Vm vm);
static Instruct* current_instructions(Vm vm);
static VmErr call_closure(Vm vm, Object* fn, int num_args);
static VmErr call_builtin(Vm vm, Object* fn, int num_args);
static VmErr execute_call(Vm vm, int num_args);
static VmErr push_closure(Vm vm, int const_index);

static VmErr err = NULL;

Vm vm_new(Bytecode* bytecode) {
  Object** globals = calloc(GLOBALS_SIZE, sizeof(Object*));
  return vm_new_with_globals(bytecode, globals);
}

Vm vm_new_with_globals(Bytecode* bytecode, Object** globals) {
  struct Vm_t* vm = malloc(sizeof(struct Vm_t));
  vm->globals = globals;
  vm->constant_pool = bytecode->constants;
  vm->sp = 0;
  Object* main_fn = new_compiled_fn(bytecode->instructions, 0);
  Closure* main_closure = malloc(sizeof(Closure));
  main_closure->fn = main_fn->value.compiled_fn;
  vm->frames[0] = new_frame(main_closure, 0);
  vm->frames_index = 1;
  return vm;
}

VmErr vm_run(Vm vm) {
  int global_index, local_index, ip;
  Instruct* ins;
  while (current_frame(vm)->ip < current_instructions(vm)->length - 1) {
    current_frame(vm)->ip++;
    ip = current_frame(vm)->ip;
    ins = current_instructions(vm);
    OpCode op = ins->bytes[ip];
    switch (op) {
      case OP_CONSTANT: {
        int const_idx = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
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
        int pos = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip = pos - 1;
      } break;

      case OP_JUMP_NOT_TRUTHY: {
        int pos = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
        Object* condition = pop(vm);
        if (!is_truthy(*condition)) {
          current_frame(vm)->ip = pos - 1;
        }
      } break;

      case OP_NULL:
        err = push(vm, &M_NULL);
        if (err)
          return err;
        break;

      case OP_GET_GLOBAL:
        global_index = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
        err = push(vm, vm->globals[global_index]);
        if (err)
          return err;
        break;

      case OP_SET_GLOBAL:
        global_index = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
        vm->globals[global_index] = pop(vm);
        break;

      case OP_SET_LOCAL: {
        local_index = (int)ins->bytes[ip + 1];
        Frame* frame = current_frame(vm);
        frame->ip += 1;
        vm->stack[frame->base_pointer + local_index] = pop(vm);
      } break;

      case OP_GET_LOCAL: {
        local_index = (int)ins->bytes[ip + 1];
        Frame* frame = current_frame(vm);
        frame->ip += 1;
        err = push(vm, vm->stack[frame->base_pointer + local_index]);
        if (err)
          return err;
      } break;

      case OP_ARRAY: {
        int num_elements = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
        Object* array = build_array(vm, vm->sp - num_elements, vm->sp);
        vm->sp = vm->sp - num_elements;
        err = push(vm, array);
        if (err)
          return err;
      } break;

      case OP_HASH: {
        int num_elements = read_uint16(&ins->bytes[ip + 1]);
        current_frame(vm)->ip += 2;
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

      case OP_CALL: {
        int num_args = (int)ins->bytes[ip + 1];
        current_frame(vm)->ip += 1;
        err = execute_call(vm, num_args);
        if (err)
          return err;
      } break;

      case OP_RETURN_VALUE: {
        Object* return_value = pop(vm);
        Frame* frame = pop_frame(vm);
        vm->sp = frame->base_pointer - 1;
        err = push(vm, return_value);
        if (err)
          return err;
      } break;

      case OP_RETURN: {
        Frame* frame = pop_frame(vm);
        vm->sp = frame->base_pointer - 1;
        err = push(vm, &M_NULL);
        if (err)
          return err;
      } break;

      case OP_GET_BUILTIN: {
        int builtin_index = (int)ins->bytes[ip + 1];
        current_frame(vm)->ip += 1;
        Object* builtin = get_builtin_by_index(builtin_index);
        err = push(vm, builtin);
        if (err)
          return err;
      } break;

      case OP_CLOSURE: {
        int const_index = read_uint16(&ins->bytes[ip + 1]);
        int __todo__ = (int)ins->bytes[ip + 3];
        current_frame(vm)->ip += 3;
        err = push_closure(vm, const_index);
        if (err)
          return err;
      } break;
    }
  }
  return NULL;
}

static VmErr push_closure(Vm vm, int const_index) {
  Object* constant = &vm->constant_pool->constants[const_index];
  if (constant->type != COMPILED_FUNCTION_OBJ) {
    SET_ERR("not a function: %s", object_type(*constant));
    return err;
  }
  Closure* closure = malloc(sizeof(Closure));
  closure->fn = constant->value.compiled_fn;
  Object* object = malloc(sizeof(Object));
  object->type = CLOSURE_OBJ;
  object->value.closure = closure;
  return push(vm, object);
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

static Object* new_compiled_fn(Instruct* instructions, int num_locals) {
  CompiledFunction* compiled_fn = malloc(sizeof(CompiledFunction));
  compiled_fn->num_locals = num_locals;
  compiled_fn->instructions = instructions;
  Object* obj = malloc(sizeof(Object));
  obj->type = COMPILED_FUNCTION_OBJ;
  obj->value.compiled_fn = compiled_fn;
  return obj;
}

static Frame* new_frame(Closure* closure, int base_pointer) {
  Frame* frame = malloc(sizeof(Frame));
  frame->cl = closure;
  frame->ip = -1;
  frame->base_pointer = base_pointer;
  return frame;
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

static Frame* current_frame(Vm vm) {
  return vm->frames[vm->frames_index - 1];
}

static void push_frame(Vm vm, Frame* frame) {
  vm->frames[vm->frames_index++] = frame;
}

static Frame* pop_frame(Vm vm) {
  return vm->frames[--vm->frames_index];
}

static Instruct* current_instructions(Vm vm) {
  return current_frame(vm)->cl->fn->instructions;
}

static VmErr execute_call(Vm vm, int num_args) {
  Object* fn = vm->stack[vm->sp - 1 - num_args];
  switch (fn->type) {
    case CLOSURE_OBJ:
      return call_closure(vm, fn, num_args);
    case BUILT_IN_OBJ:
      return call_builtin(vm, fn, num_args);
    case 0:
      return "null pointer for execute_call()";
    default:
      return "calling non-function and non-built-in";
  }
}

static VmErr call_closure(Vm vm, Object* fn, int num_args) {
  if (num_args != fn->value.closure->fn->num_params) {
    SET_ERR("wrong number of arguments: want=%d, got=%d",
      fn->value.closure->fn->num_params, num_args);
    return err;
  }
  Frame* frame = new_frame(fn->value.closure, vm->sp - num_args);
  push_frame(vm, frame);
  vm->sp = frame->base_pointer + fn->value.closure->fn->num_locals;
  return NULL;
}

static VmErr call_builtin(Vm vm, Object* fn, int num_args) {
  List* args = NULL;
  for (int i = vm->sp - num_args; i < vm->sp; i++) {
    args = list_append(args, vm->stack[i]);
  }
  Object result = (fn->value.builtin_fn)(args);
  if (result.type == ERROR_OBJ)
    return result.value.str;
  return push(vm, memcpy(malloc(sizeof(Object)), &result, sizeof(Object)));
}
