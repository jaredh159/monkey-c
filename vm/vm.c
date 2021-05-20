#include "vm.h"
#include <stdlib.h>
#include "../compiler/compiler.h"

void vm_init(Bytecode* bytecode) {
  if (!bytecode) {
    exit(EXIT_FAILURE);  // TEMP, silence unused param compiler warning
  }
}

VmErr vm_run(void) {
  return NULL;
}

Object* vm_stack_top(void) {
  return NULL;
}
