#include "compiler.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/parser.h"
#include "symbol_table.h"

#define BACKPATCH_LATER i(255)

typedef struct EmittedInstruction {
  OpCode op_code;
  int position;
} EmittedInstruction;

struct Compiler_t {
  Instruct* instructions;
  ConstantPool* constant_pool;
  EmittedInstruction last_instruction;
  EmittedInstruction previous_instruction;
  SymbolTable symbol_table;
};

static const IntBag _ = {0};

static CompilerErr compile_statements(Compiler c, List* statements);
static CompilerErr compile_expressions(Compiler c, List* expressions);
static int add_constant(Compiler c, Object* object);
static int emit(Compiler c, OpCode op_code, IntBag operands);
static int add_instruction(Compiler c, Instruct* instructions);
static void set_last_instruction(Compiler c, OpCode op_code, int position);
static void remove_last_pop(Compiler c);
static void replace_instruction(Compiler c, int pos, Instruct* new_instruction);
static void change_operand(Compiler c, int op_code_pos, int operand);

Compiler compiler_new() {
  Compiler compiler = malloc(sizeof(struct Compiler_t));
  compiler->instructions = malloc(sizeof(Instruct));
  compiler->constant_pool = malloc(sizeof(ConstantPool));
  compiler->constant_pool->length = 0;
  compiler->constant_pool->constants = malloc(sizeof(Object) * MAX_CONSTANTS);
  compiler->instructions->length = 0;
  compiler->instructions->bytes = malloc(sizeof(Byte) * MAX_INSTRUCTIONS);
  compiler->symbol_table = symbol_table_new();
  return compiler;
}

Compiler compiler_new_with_state(
  SymbolTable symbol_table, ConstantPool* constant_pool) {
  Compiler compiler = compiler_new();
  compiler->symbol_table = symbol_table;
  compiler->constant_pool = constant_pool;
  return compiler;
}

CompilerErr compile(Compiler c, void* node, NodeType type) {
  CompilerErr err = malloc(100);
  switch (type) {
    case PROGRAM_NODE:           // fallthrough
    case BLOCK_STATEMENTS_NODE:  // fallthrough
      err = compile_statements(c, ((BlockStatement*)node)->statements);
      if (err)
        return err;
      break;

    case LET_STATEMENT_NODE: {
      LetStatement* let_stmt = node;
      err = compile(c, let_stmt->value, EXPRESSION_NODE);
      if (err)
        return err;
      Symbol* symbol =
        symbol_table_define(c->symbol_table, let_stmt->name->value);
      emit(c, OP_SET_GLOBAL, i(symbol->index));
    } break;

    case EXPRESSION_STATEMENT_NODE:
      err =
        compile(c, ((ExpressionStatement*)node)->expression, EXPRESSION_NODE);
      if (err)
        return err;
      emit(c, OP_POP, _);
      break;

    case INTEGER_LITERAL_NODE: {
      Object* int_lit = malloc(sizeof(Object));
      int_lit->type = INTEGER_OBJ;
      int_lit->value.i = ((IntegerLiteral*)node)->value;
      int constant_idx = add_constant(c, int_lit);
      emit(c, OP_CONSTANT, i(constant_idx));
    } break;

    case BOOLEAN_LITERAL_NODE:
      if (((BooleanLiteral*)node)->value) {
        emit(c, OP_TRUE, _);
      } else {
        emit(c, OP_FALSE, _);
      }
      break;

    case EXPRESSION_NODE: {
      Expression* exp = node;
      switch (exp->type) {
        case EXPRESSION_INFIX: {
          InfixExpression* infix = exp->node;
          if (infix->operator[0] == '<') {
            err = compile(c, infix->right, EXPRESSION_NODE);
            if (err)
              return err;
            err = compile(c, infix->left, EXPRESSION_NODE);
            if (err)
              return err;
            emit(c, OP_GREATER_THAN, _);
            return NULL;
          }
          err = compile(c, infix->left, EXPRESSION_NODE);
          if (err)
            return err;
          err = compile(c, infix->right, EXPRESSION_NODE);
          if (err)
            return err;
          switch ((int)infix->operator[0]) {
            case '+':
              emit(c, OP_ADD, _);
              break;
            case '-':
              emit(c, OP_SUB, _);
              break;
            case '/':
              emit(c, OP_DIV, _);
              break;
            case '>':
              emit(c, OP_GREATER_THAN, _);
              break;
            case '*':
              emit(c, OP_MUL, _);
              break;
            case '=':
              emit(c, OP_EQUAL, _);
              break;
            case '!':
              emit(c, OP_NOT_EQUAL, _);
              break;
            default:
              sprintf(err, "unknown operator %s", infix->operator);
              return err;
          }
        } break;

        case EXPRESSION_PREFIX: {
          PrefixExpression* prefix = exp->node;
          err = compile(c, prefix->right, EXPRESSION_NODE);
          if (err)
            return err;
          switch ((int)prefix->operator[0]) {
            case '!':
              emit(c, OP_BANG, _);
              break;
            case '-':
              emit(c, OP_MINUS, _);
              break;
            default:
              sprintf(err, "unknown operator %s", prefix->operator);
              return err;
          }
        } break;

        case EXPRESSION_BOOLEAN_LITERAL:
          err = compile(c, exp->node, BOOLEAN_LITERAL_NODE);
          if (err)
            return err;
          break;

        case EXPRESSION_INTEGER_LITERAL:
          err = compile(c, exp->node, INTEGER_LITERAL_NODE);
          if (err)
            return err;
          break;

        case EXPRESSION_STRING_LITERAL: {
          StringLiteral* str = exp->node;
          Object* str_lit = malloc(sizeof(Object));
          str_lit->type = STRING_OBJ;
          str_lit->value.str = str->value;
          int constant_idx = add_constant(c, str_lit);
          emit(c, OP_CONSTANT, i(constant_idx));
        } break;

        case EXPRESSION_IDENTIFIER: {
          Identifier* ident = exp->node;
          Symbol* symbol = symbol_table_resolve(c->symbol_table, ident->value);
          if (symbol == NULL) {
            sprintf(err, "undefined variable %s", ident->value);
            return err;
          }
          emit(c, OP_GET_GLOBAL, i(symbol->index));
        } break;

        case EXPRESSION_ARRAY_LITERAL: {
          ArrayLiteral* array = exp->node;
          err = compile_expressions(c, array->elements);
          if (err)
            return err;
          emit(c, OP_ARRAY, i(list_count(array->elements)));
        } break;

        case EXPRESSION_HASH_LITERAL: {
          HashLiteralExpression* hash = exp->node;
          int num_pairs = list_count(hash->pairs);
          List* current = hash->pairs;
          for (int i = 0; i < num_pairs; i++, current = current->next) {
            HashLiteralPair* pair = current->item;
            err = compile(c, pair->key, EXPRESSION_NODE);
            if (err)
              return err;
            err = compile(c, pair->value, EXPRESSION_NODE);
            if (err)
              return err;
          }
          emit(c, OP_HASH, i(num_pairs * 2));
        } break;

        case EXPRESSION_IF: {
          IfExpression* if_exp = exp->node;
          err = compile(c, if_exp->condition, EXPRESSION_NODE);
          if (err)
            return err;

          int jump_not_truthy_pos =
            emit(c, OP_JUMP_NOT_TRUTHY, BACKPATCH_LATER);
          err = compile(c, if_exp->consequence, BLOCK_STATEMENTS_NODE);
          if (err)
            return err;

          if (c->last_instruction.op_code == OP_POP) {
            remove_last_pop(c);
          }

          int jump_pos = emit(c, OP_JUMP, BACKPATCH_LATER);
          int after_conseq_pos = c->instructions->length;
          change_operand(c, jump_not_truthy_pos, after_conseq_pos);

          if (if_exp->alternative == NULL) {
            emit(c, OP_NULL, _);
          } else {
            err = compile(c, if_exp->alternative, BLOCK_STATEMENTS_NODE);
            if (err)
              return err;

            if (c->last_instruction.op_code == OP_POP) {
              remove_last_pop(c);
            }
          }
          int after_alt_pos = c->instructions->length;
          change_operand(c, jump_pos, after_alt_pos);
        } break;
      }
      break;
    }
  }
  return NULL;
}

int emit(Compiler c, OpCode op, IntBag operands) {
  Instruct* instruction = code_make_nv(op, operands);
  int pos = add_instruction(c, instruction);
  set_last_instruction(c, op, pos);
  return pos;
}

static void replace_instruction(
  Compiler c, int pos, Instruct* new_instruction) {
  for (int i = 0; i < new_instruction->length; i++) {
    c->instructions->bytes[pos + i] = new_instruction->bytes[i];
  }
}

static void change_operand(Compiler c, int op_code_pos, int operand) {
  OpCode op = c->instructions->bytes[op_code_pos];
  Instruct* new_instruction = code_make(op, operand);
  replace_instruction(c, op_code_pos, new_instruction);
}

static void remove_last_pop(Compiler c) {
  c->instructions->length--;
  c->last_instruction = c->previous_instruction;
}

void set_last_instruction(Compiler c, OpCode op_code, int position) {
  c->previous_instruction = c->last_instruction;
  c->last_instruction.op_code = op_code;
  c->last_instruction.position = position;
}

CompilerErr compile_expressions(Compiler c, List* expressions) {
  CompilerErr err = NULL;
  for (List* current = expressions; current != NULL; current = current->next) {
    if (current->item != NULL) {
      err = compile(c, (Expression*)current->item, EXPRESSION_NODE);
      if (err)
        return err;
    }
  }
  return NULL;
}

CompilerErr compile_statements(Compiler c, List* statements) {
  CompilerErr err = NULL;
  for (List* current = statements; current != NULL; current = current->next) {
    if (current->item != NULL) {
      Statement* stmt = current->item;
      err = compile(c, stmt->node, ast_statement_node_type(stmt));
      if (err)
        return err;
    }
  }
  return NULL;
}

Bytecode* compiler_bytecode(Compiler c) {
  Bytecode* bytecode = malloc(sizeof(Bytecode));
  bytecode->constants = c->constant_pool;
  bytecode->instructions = c->instructions;
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

int add_constant(Compiler c, Object* obj) {
  c->constant_pool->constants[c->constant_pool->length] = *obj;
  c->constant_pool->length += 1;
  return c->constant_pool->length - 1;
}

int add_instruction(Compiler c, Instruct* instruction) {
  int insert_idx = c->instructions->length;
  c->instructions->length += instruction->length;
  for (int i = 0; i < instruction->length; i++) {
    c->instructions->bytes[insert_idx + i] = instruction->bytes[i];
  }
  free(instruction);
  return insert_idx;
}
