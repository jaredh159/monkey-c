#ifndef __VM_H__
#define __VM_H__

#include "../compiler/compiler.h"
#include "../object/object.h"

#define GLOBALS_SIZE 65536

typedef char* VmErr;

void vm_init(Bytecode* bytecode);
void vm_init_with_globals(Bytecode* bytecode, Object** globals);
VmErr vm_run(void);
Object* vm_stack_top(void);
Object* vm_last_popped(void);

#endif  // __VM_H__
