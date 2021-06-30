#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../test/test.h"

void assert_symbol_is(Symbol* symbol, char* name, SymbolScope scope, int index,
  const char* test_name);

void test_define(void) {
  SymbolTable global = symbol_table_new();
  Symbol* abc = symbol_table_define(global, "abc");
  assert_symbol_is(abc, "abc", SCOPE_GLOBAL, 0, "test_define");
  Symbol* a = symbol_table_define(global, "a");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 1, "test_define");
}

void test_scoped_define(void) {
  SymbolTable global = symbol_table_new();
  Symbol* a = symbol_table_define(global, "a");
  Symbol* b = symbol_table_define(global, "b");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, __func__);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, __func__);

  SymbolTable local_1 = symbol_table_new_enclosed(global);
  Symbol* c = symbol_table_define(local_1, "c");
  Symbol* d = symbol_table_define(local_1, "d");
  assert_symbol_is(c, "c", SCOPE_LOCAL, 0, __func__);
  assert_symbol_is(d, "d", SCOPE_LOCAL, 1, __func__);

  SymbolTable local_2 = symbol_table_new_enclosed(local_1);
  Symbol* e = symbol_table_define(local_2, "e");
  Symbol* f = symbol_table_define(local_2, "f");
  assert_symbol_is(e, "e", SCOPE_LOCAL, 0, __func__);
  assert_symbol_is(f, "f", SCOPE_LOCAL, 1, __func__);
}

void test_resolve(void) {
  char* t = "test_resolve";
  SymbolTable global = symbol_table_new();
  symbol_table_define(global, "a");
  symbol_table_define(global, "b");
  symbol_table_define(global, "bb");
  symbol_table_define(global, "bbb");
  symbol_table_define(global, "abc");
  Symbol* a = symbol_table_resolve(global, "a");
  Symbol* b = symbol_table_resolve(global, "b");
  Symbol* bb = symbol_table_resolve(global, "bb");
  Symbol* bbb = symbol_table_resolve(global, "bbb");
  Symbol* abc = symbol_table_resolve(global, "abc");
  Symbol* unknown = symbol_table_resolve(global, "unknown");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, t);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, t);
  assert_symbol_is(bb, "bb", SCOPE_GLOBAL, 2, t);
  assert_symbol_is(bbb, "bbb", SCOPE_GLOBAL, 3, t);
  assert_symbol_is(abc, "abc", SCOPE_GLOBAL, 4, t);
  assert(unknown == NULL, "unknown ident is NULL", t);
}

void test_resolve_local(void) {
  SymbolTable global = symbol_table_new();
  symbol_table_define(global, "a");
  symbol_table_define(global, "b");
  SymbolTable local = symbol_table_new_enclosed(global);
  symbol_table_define(local, "c");
  symbol_table_define(local, "d");
  Symbol* a = symbol_table_resolve(local, "a");
  Symbol* b = symbol_table_resolve(local, "b");
  Symbol* c = symbol_table_resolve(local, "c");
  Symbol* d = symbol_table_resolve(local, "d");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, __func__);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, __func__);
  assert_symbol_is(c, "c", SCOPE_LOCAL, 0, __func__);
  assert_symbol_is(d, "d", SCOPE_LOCAL, 1, __func__);
}

void test_resolve_nested_local(void) {
  SymbolTable global = symbol_table_new();
  symbol_table_define(global, "a");
  symbol_table_define(global, "b");
  SymbolTable local_1 = symbol_table_new_enclosed(global);
  symbol_table_define(local_1, "c");
  symbol_table_define(local_1, "d");
  SymbolTable local_2 = symbol_table_new_enclosed(local_1);
  symbol_table_define(local_2, "e");
  symbol_table_define(local_2, "f");

  Symbol* a = symbol_table_resolve(local_1, "a");
  Symbol* b = symbol_table_resolve(local_1, "b");
  Symbol* c = symbol_table_resolve(local_1, "c");
  Symbol* d = symbol_table_resolve(local_1, "d");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, __func__);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, __func__);
  assert_symbol_is(c, "c", SCOPE_LOCAL, 0, __func__);
  assert_symbol_is(d, "d", SCOPE_LOCAL, 1, __func__);

  a = symbol_table_resolve(local_2, "a");
  b = symbol_table_resolve(local_2, "b");
  Symbol* e = symbol_table_resolve(local_2, "e");
  Symbol* f = symbol_table_resolve(local_2, "f");
  assert_symbol_is(a, "a", SCOPE_GLOBAL, 0, __func__);
  assert_symbol_is(b, "b", SCOPE_GLOBAL, 1, __func__);
  assert_symbol_is(e, "e", SCOPE_LOCAL, 0, __func__);
  assert_symbol_is(f, "f", SCOPE_LOCAL, 1, __func__);
}

void test_define_resolve_builtins(void) {
  SymbolTable global = symbol_table_new();
  SymbolTable local_1 = symbol_table_new_enclosed(global);
  SymbolTable local_2 = symbol_table_new_enclosed(local_1);
  SymbolTable tables[] = {global, local_1, local_2};
  symbol_table_define_builtin(global, 0, "a");
  symbol_table_define_builtin(global, 1, "b");
  symbol_table_define_builtin(global, 2, "c");

  for (int i = 0; i < LEN(tables); i++) {
    Symbol* a = symbol_table_resolve(tables[i], "a");
    Symbol* b = symbol_table_resolve(tables[i], "b");
    Symbol* c = symbol_table_resolve(tables[i], "c");
    assert_symbol_is(a, "a", SCOPE_BUILTIN, 0, __func__);
    assert_symbol_is(b, "b", SCOPE_BUILTIN, 1, __func__);
    assert_symbol_is(c, "c", SCOPE_BUILTIN, 2, __func__);
  }
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
  test_define_resolve_builtins();
  test_scoped_define();
  test_resolve_local();
  test_resolve_nested_local();
  test_char_hash();
  test_resolve();
  test_define();
  printf("\n");
  return 0;
}

void assert_symbol_is(Symbol* symbol, char* name, SymbolScope scope, int index,
  const char* test_name) {
  if (!symbol) {
    fail(ss("name `%s` not resolvable", name), test_name);
    return;
  }

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
