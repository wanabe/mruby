/*
** mruby/jitcode.h - Class for XTAAK
**
** See Copyright Notice in mruby.h
*/

#ifndef MRUBY_JITCOD_H
#define MRUBY_JITCODE_H

#include <xtaak/xtaak.h>
extern "C" {
#include "mruby.h"
#include "opcode.h"

#include "mruby/irep.h"
#include "mruby/jit.h"
} /* extern "C" */

/* Regs Map                               *
 * ebp   -- pointer to regs               *
 * ecx   -- pointer to pool               *
 * ebx   -- pointer to pc                 */
class MRBJitCode: public Xtaak::CodeGenerator {

 public:

 MRBJitCode():
  CodeGenerator(1024)
  {
  }

  const void *emit_entry(mrb_state *mrb, mrb_irep *irep) {
    const void* func_ptr = getCurr();
    return func_ptr;
  }

  void emit_exit() {
    mov(pc, lr);
    /*ret();*/
  }
  
  const void *emit_mov(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) {
    const void *code = getCurr();

    return code;
  }

  const void *emit_loadl(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) {
    const void *code = getCurr();

    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
