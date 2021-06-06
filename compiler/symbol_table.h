#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

enum Scopes {
  SCOPE_GLOBAL,
};

typedef int SymbolScope;

// incomplete declaration for encapsulation
typedef struct SymbolTable_t* SymbolTable;

typedef struct Symbol {
  char* name;
  SymbolScope scope;
  int index;
} Symbol;

SymbolTable symbol_table_new();
Symbol* symbol_table_define(SymbolTable table, char* name);
Symbol* symbol_table_resolve(SymbolTable table, char* name);
char* symbol_scope_name(SymbolScope scope);

#endif  // __SYMBOL_TABLE_H__
