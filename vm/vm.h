#ifndef __VM_H__
#define __VM_H__

#include "../compiler/compiler.h"
#include "../object/object.h"

#define GLOBALS_SIZE 65536

typedef char* VmErr;

// incomplete declaration for encapsulation
typedef struct Vm_t* Vm;

Vm vm_new(Bytecode* bytecode);
Vm vm_new_with_globals(Bytecode* bytecode, Object** globals);
VmErr vm_run(Vm vm);
Object* vm_stack_top(Vm vm);
Object* vm_last_popped(Vm vm);

#endif  // __VM_H__
