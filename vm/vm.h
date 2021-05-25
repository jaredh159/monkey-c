#include "../compiler/compiler.h"
#include "../object/object.h"

typedef char* VmErr;

void vm_init(Bytecode*);
VmErr vm_run(void);
Object* vm_stack_top(void);
Object* vm_last_popped(void);
