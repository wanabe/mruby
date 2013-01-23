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
#include "mruby/irep.h"
#include "mruby/jit.h"
} /* extern "C" */

class MRBJitCode: public Xtaak::CodeGenerator {

 public:

 MRBJitCode():
  CodeGenerator(1024)
  {
  }

  const void *emit_entry(mrb_state *mrb, mrb_irep *irep) {
    const void* func_ptr = getCurr();
    /*push(ebp);
    mov(ebp, (Xbyak::uint32)mrb->stbase);
    add(ebp, (Xbyak::uint32)mrb->ci->stackidx);
    push(ecx);*/

    return func_ptr;
  }

  void emit_exit() {
    mov(pc, lr);
    /*pop(ecx);
    pop(ebp);
    ret();*/
  }
  
  const void *emit_mov() {
    const void *code = getCurr();

    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
