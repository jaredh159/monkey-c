#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../test/test.h"

void assert_symbol_is(
  Symbol* symbol, char* name, SymbolScope scope, int index, char* test_name);

void test_define(void) {
  SymbolTable global = symbol_table_new();
  Symbol* abc = symbol_table_define(global, "abc");
  assert_symbol_is(abc, "abc", SCOPE_GLOBAL, 0, "test_define");
  Symbol* a = symbol_table_define(global, "a");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 1, "test_define");
}

void test_resolve(void) {
  char* t = "test_resolve";
  SymbolTable global = symbol_table_new();
  symbol_table_define(global, "a");
  symbol_table_define(global, "b");
  symbol_table_define(global, "abc");
  Symbol* a = symbol_table_resolve(global, "a");
  Symbol* b = symbol_table_resolve(global, "b");
  Symbol* abc = symbol_table_resolve(global, "abc");
  Symbol* unknown = symbol_table_resolve(global, "unknown");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, t);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, t);
  assert_symbol_is(abc, "abc", SCOPE_GLOBAL, 2, t);
  assert(unknown == NULL, "unknown ident is NULL", t);
}

void test_char_hash(void) {
  char* t = "char_hash";
  int symbol_char_hash(char ch);
  assert_int_is(0, symbol_char_hash('a'), "a should be 0", t);
  assert_int_is(1, symbol_char_hash('b'), "b should be 1", t);
  assert_int_is(26, symbol_char_hash('A'), "A should be 26", t);
  assert_int_is(52, symbol_char_hash('0'), "0 should be 52", t);
  assert_int_is(53, symbol_char_hash('1'), "1 should be 53", t);
  assert_int_is(62, symbol_char_hash('_'), "_ should be 62", t);
}

int main(int argc, char** argv) {
  pass_argv(argc, argv);
  test_char_hash();
  test_resolve();
  test_define();
  printf("\n");
  return 0;
}

void assert_symbol_is(
  Symbol* symbol, char* name, SymbolScope scope, int index, char* test_name) {
  if (strcmp(symbol->name, name) != 0) {
    fail(ss("symbol name incorrect, want=%s, got=%s", name, symbol->name),
      test_name);
    return;
  }

  if (symbol->scope != scope) {
    fail(ss("symbol scope incorrect, want=%s, got=%s", symbol_scope_name(scope),
           symbol_scope_name(symbol->scope)),
      test_name);
    return;
  }

  if (symbol->index != index) {
    fail(si("symbol scope index, want=%d, got=%d", index, symbol->index),
      test_name);
  }

  assert(true, "symbol equality correct", test_name);
}