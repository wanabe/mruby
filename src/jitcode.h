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

  void emit_exit(mrb_code *mruby_pc) {
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    ret();*/
    ldr(r3, pc + 4);
    str(r3, r1);
    mov(pc, lr);
    dd((Xtaak::uint32)mruby_pc);
  }
  
  const void *emit_mov(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 srcoff = GETARG_B(**ppc) * sizeof(mrb_value);
    /*movsd(xmm0, ptr [ebp + srcoff]);
    movsd(ptr [ebp + dstoff], xmm0);*/
    movw(r3, srcoff);
    add(r3, r3, r0);
    ldm(r3, r4, r5, r6, r7);
    movw(r3, dstoff);
    add(r3, r3, r0);
    stm(r3, r4, r5, r6, r7);
    return code;
  }

  const void *emit_loadl(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 srcoff = GETARG_B(**ppc) * sizeof(mrb_value);
    /*movsd(xmm0, ptr [ecx + srcoff]);
    movsd(ptr [ebp + dstoff], xmm0);*/
    movw(r3, srcoff);
    add(r3, r3, r2);
    ldm(r3, r4, r5, r6, r7);
    movw(r3, dstoff);
    add(r3, r3, r0);
    stm(r3, r4, r5, r6, r7);
    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
