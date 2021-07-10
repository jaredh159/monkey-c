#include "symbol_table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../object/object.h"

// 26 (uppercase) + 26 (lowercase) + 10 (digits) + 1 (underscore)
#define NUM_IDENT_CHARS 63

typedef struct HashNode {
  struct HashNode* chars[NUM_IDENT_CHARS];
  Symbol* symbol;
} HashNode;

typedef HashNode Store;

struct SymbolTable_t {
  struct SymbolTable_t* outer;
  Store* store;
  int num_definitions;
  Symbol* free_symbols[MAX_FREE_VARIABLES];
};

int symbol_char_hash(char ch);
static void symbol_put(HashNode* node, Symbol* symbol, int char_idx);
static Symbol* symbol_get(HashNode* node, char* name);
static HashNode* new_node(Symbol*);
static Symbol* define_free(SymbolTable t, Symbol* original);

static Symbol* new_symbol(char* name, int index, SymbolScope scope) {
  Symbol* symbol = malloc(sizeof(Symbol));
  symbol->name = strdup(name);
  symbol->index = index;
  symbol->scope = scope;
  return symbol;
}

SymbolTable symbol_table_new() {
  SymbolTable table = calloc(sizeof(struct SymbolTable_t), 1);
  table->num_definitions = 0;
  table->store = new_node(NULL);
  return table;
}

SymbolTable symbol_table_new_enclosed(SymbolTable outer) {
  SymbolTable table = symbol_table_new();
  table->outer = outer;
  return table;
}

Symbol* symbol_table_define(SymbolTable t, char* name) {
  SymbolScope scope = t->outer == NULL ? SCOPE_GLOBAL : SCOPE_LOCAL;
  Symbol* symbol = new_symbol(name, t->num_definitions, scope);
  symbol_put(t->store, symbol, 0);
  t->num_definitions++;
  return symbol;
}

static Symbol* define_free(SymbolTable t, Symbol* original) {
  int index = 0;
  while (t->free_symbols[index] != NULL) index++;
  t->free_symbols[index] = original;
  Symbol* symbol = new_symbol(original->name, index, SCOPE_FREE);
  symbol_put(t->store, symbol, 0);
  t->num_definitions++;
  return symbol;
}

Symbol* symbol_table_define_builtin(SymbolTable t, int index, char* name) {
  Symbol* symbol = new_symbol(name, index, SCOPE_BUILTIN);
  symbol_put(t->store, symbol, 0);
  return symbol;
}

Symbol* symbol_table_resolve(SymbolTable t, char* name) {
  Symbol* resolved = symbol_get(t->store, name);
  if (!resolved && t->outer) {
    resolved = symbol_table_resolve(t->outer, name);
    if (!resolved) {
      return NULL;
    }
    if (resolved->scope == SCOPE_GLOBAL || resolved->scope == SCOPE_BUILTIN)
      return resolved;
    return define_free(t, resolved);
  }
  return resolved;
}

char* symbol_scope_name(SymbolScope scope) {
  switch (scope) {
    case SCOPE_GLOBAL:
      return "GLOBAL";
    case SCOPE_LOCAL:
      return "LOCAL";
    case SCOPE_FREE:
      return "FREE";
    case SCOPE_BUILTIN:
      return "BUILTIN";
    default:
      printf("Unhandled scope %d in `symbol_scope_name()`\n", scope);
      exit(EXIT_FAILURE);
  }
}

static void symbol_put(HashNode* node, Symbol* symbol, int char_idx) {
  char ch = symbol->name[char_idx];
  int hash = symbol_char_hash(ch);
  bool is_last_char = char_idx == (int)strlen(symbol->name) - 1;
  if (is_last_char) {
    node->chars[hash] = new_node(symbol);
    return;
  } else if (node->chars[hash] == NULL) {
    node->chars[hash] = new_node(NULL);
  }
  symbol_put(node->chars[hash], symbol, ++char_idx);
}

static Symbol* symbol_get(HashNode* node, char* name) {
  char ch = *name;
  int hash = symbol_char_hash(ch);
  HashNode* char_node = node->chars[hash];
  if (char_node == NULL)
    return NULL;
  if (strlen(name) != 1)
    return symbol_get(char_node, ++name);
  return char_node->symbol;
}

static HashNode* new_node(Symbol* symbol) {
  HashNode* node = calloc(1, sizeof(HashNode));
  node->symbol = symbol;
  return node;
}

int symbol_char_hash(char ch) {
  if (islower(ch))
    return ch - 'a';
  if (isupper(ch))
    return ch - 'A' + 26;
  if (isdigit(ch))
    return ch - '0' + 52;
  if (ch == '_')
    return 62;
  printf("Unexpected char for ident hashing: %c\n", ch);
  exit(EXIT_FAILURE);
}

SymbolTable symbol_table_outer(SymbolTable table) {
  return table->outer;
}

int symbol_table_num_definitions(SymbolTable table) {
  return table->num_definitions;
}

void symbol_table_define_builtins(SymbolTable table) {
  symbol_table_define_builtin(table, BUILTIN_LEN, "len");
  symbol_table_define_builtin(table, BUILTIN_FIRST, "first");
  symbol_table_define_builtin(table, BUILTIN_REST, "rest");
  symbol_table_define_builtin(table, BUILTIN_PUSH, "push");
  symbol_table_define_builtin(table, BUILTIN_PUTS, "puts");
  symbol_table_define_builtin(table, BUILTIN_LAST, "last");
}

int symbol_table_num_free(SymbolTable table) {
  int index = 0;
  while (table->free_symbols[index++])
    ;
  return index - 1;
}

Symbol** symbol_table_get_free(SymbolTable table) {
  return table->free_symbols;
}
