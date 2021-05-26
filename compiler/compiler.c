#include "compiler.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../parser/parser.h"

static Instruct* instructions = NULL;
static ConstantPool* constant_pool = NULL;
static IntBag _ = {0};

CompilerErr compile_statements(List*);
int add_constant(Object*);
int emit(OpCode, IntBag);
int add_instruction(Instruct*);

void compiler_init(void) {
  if (instructions != NULL) {
    free(constant_pool->constants);
    free(instructions->bytes);
    free(instructions);
    free(constant_pool);
  }
  instructions = malloc(sizeof(Instruct));
  constant_pool = malloc(sizeof(ConstantPool));
  constant_pool->length = 0;
  constant_pool->constants = malloc(sizeof(Object) * MAX_CONSTANTS);
  instructions->length = 0;
  instructions->bytes = malloc(sizeof(Byte) * MAX_INSTRUCTIONS);
}

CompilerErr compile(void* node, NodeType type) {
  CompilerErr err = NULL;
  switch (type) {
    case PROGRAM_NODE:
      err = compile_statements(((Program*)node)->statements);
      if (err)
        return err;
      break;
    case EXPRESSION_STATEMENT_NODE:
      err = compile(((ExpressionStatement*)node)->expression, EXPRESSION_NODE);
      if (err)
        return err;
      emit(OP_POP, _);
      break;
    case INTEGER_LITERAL_NODE: {
      Object* int_lit = malloc(sizeof(Object));
      int_lit->type = INTEGER_OBJ;
      int_lit->value.i = ((IntegerLiteral*)node)->value;
      int constant_idx = add_constant(int_lit);
      emit(OP_CONSTANT, i(constant_idx));
    } break;
    case BOOLEAN_LITERAL_NODE:
      if (((BooleanLiteral*)node)->value) {
        emit(OP_TRUE, _);
      } else {
        emit(OP_FALSE, _);
      }
      break;
    case EXPRESSION_NODE: {
      Expression* exp = node;
      switch (exp->type) {
        case EXPRESSION_INFIX: {
          InfixExpression* infix = exp->node;
          err = compile(infix->left, EXPRESSION_NODE);
          if (err)
            return err;
          err = compile(infix->right, EXPRESSION_NODE);
          if (err)
            return err;
          switch ((int)infix->operator[0]) {
            case '+':
              emit(OP_ADD, _);
              break;
            case '-':
              emit(OP_SUB, _);
              break;
            case '/':
              emit(OP_DIV, _);
              break;
            case '*':
              emit(OP_MUL, _);
              break;
            default:
              err = malloc(100);
              sprintf(err, "ERROR: unknown operator %s\n", infix->operator);
              return err;
          }
        } break;
        case EXPRESSION_BOOLEAN_LITERAL:
          err = compile(exp->node, BOOLEAN_LITERAL_NODE);
          if (err)
            return err;
          break;
        case EXPRESSION_INTEGER_LITERAL:
          err = compile(exp->node, INTEGER_LITERAL_NODE);
          if (err)
            return err;
          break;
      }
      break;
    }
  }
  return NULL;
}

int emit(OpCode op, IntBag operands) {
  Instruct* instruction = code_make_nv(op, operands);
  int pos = add_instruction(instruction);
  return pos;
}

CompilerErr compile_statements(List* statements) {
  CompilerErr err = NULL;
  List* current = statements;
  for (; current != NULL; current = current->next) {
    if (current->item != NULL) {
      Statement* stmt = (Statement*)current->item;
      err = compile(stmt->node, ast_statement_node_type(stmt));
      if (err)
        return err;
    }
  }
  return NULL;
}

Bytecode* compiler_bytecode(void) {
  Bytecode* bytecode = malloc(sizeof(Bytecode));
  bytecode->constants = constant_pool;
  bytecode->instructions = instructions;
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

int add_constant(Object* obj) {
  constant_pool->constants[constant_pool->length] = *obj;
  constant_pool->length += 1;
  return constant_pool->length - 1;
}

int add_instruction(Instruct* instruction) {
  int insert_idx = instructions->length;
  instructions->length += instruction->length;
  for (int i = 0; i < instruction->length; i++) {
    instructions->bytes[insert_idx + i] = instruction->bytes[i];
  }
  free(instruction);
  return insert_idx;
}
