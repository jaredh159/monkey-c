#include "../object/object.h"
#include "../compiler/compiler.h"

typedef char* VmErr;

void vm_init(Bytecode*);
VmErr vm_run(void);
Object* vm_stack_top(void);
