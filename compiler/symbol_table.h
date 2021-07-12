#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

enum Scopes {
  SCOPE_GLOBAL,
  SCOPE_LOCAL,
  SCOPE_BUILTIN,
  SCOPE_FREE,
  SCOPE_FUNCTION,
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
SymbolTable symbol_table_new_enclosed(SymbolTable outer);
Symbol* symbol_table_define(SymbolTable table, char* name);
Symbol* symbol_table_define_fn_name(SymbolTable table, char* name);
Symbol* symbol_table_define_builtin(SymbolTable table, int index, char* name);
void symbol_table_define_builtins(SymbolTable table);
Symbol* symbol_table_resolve(SymbolTable table, char* name);
int symbol_table_num_definitions(SymbolTable table);
int symbol_table_num_free(SymbolTable table);
Symbol** symbol_table_get_free(SymbolTable table);
char* symbol_scope_name(SymbolScope scope);
SymbolTable symbol_table_outer(SymbolTable table);

#endif  // __SYMBOL_TABLE_H__
