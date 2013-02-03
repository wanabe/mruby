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
#include "mruby/value.h"
#include "mruby/jit.h"
} /* extern "C" */

/* Regs Map                               *
 * r1    -- pointer to regs               *
 * r0    -- pointer to pc                 */
class MRBJitCode: public Xtaak::CodeGenerator {

 public:

 MRBJitCode():
  CodeGenerator(1024)
  {
  }

  const void *
    gen_entry(mrb_state *mrb, mrb_irep *irep) 
  {
    const void* func_ptr = getCurr();
    return func_ptr;
  }

  void 
    gen_exit(mrb_code *mruby_pc) 
  {
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    ret();*/
    ldr(r3, pc + 4);
    str(r3, r0);
    mov(pc, lr);
    dd((Xtaak::uint32)mruby_pc);
  }
  
  void 
    gen_jump_block(void *entry) 
  {
    b(entry);
  }

  void gen_type_guard(enum mrb_vtype tt, mrb_code *mruby_pc)
  {
    /* Input eax for type tag
    if (tt == MRB_TT_FLOAT) {
      cmp(eax, 0xfff00000);
      ja("@f");
    } 
    else {
      cmp(eax, 0xfff00000 | tt);
      jz("@f");
    }*/
    /* Input r2 for type tag */
    add(r2, r2, 0x100000);
    if (tt == MRB_TT_FLOAT) {
      bcc(3);
    }
    else {
      cmp(r2, tt);
      /*jz("@f");*/
      beq(3);
    }

    /* Guard fail exit code */
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    ret();*/
    ldr(r3, pc + 4);
    str(r3, r0);
    mov(pc, lr);
    dd((Xtaak::uint32)mruby_pc);
  }

  const void *
    emit_move(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 srcoff = GETARG_B(**ppc) * sizeof(mrb_value);
    /*movsd(xmm0, ptr [ecx + srcoff]);
    movsd(ptr [ecx + dstoff], xmm0);*/
    movw(r3, srcoff);
    add(r3, r3, r1);
    ldm(r3, r4, r5);
    movw(r3, dstoff);
    add(r3, r3, r1);
    stm(r3, r4, r5);
    return code;
  }

  const void *
    emit_loadl(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 srcoff = GETARG_Bx(**ppc) * sizeof(mrb_value);
    /*mov(eax, (Xbyak::uint32)irep->pool + srcoff);
    movsd(xmm0, ptr [eax]);
    movsd(ptr [ecx + dstoff], xmm0);*/
    mov32(r3, (Xtaak::uint32)irep->pool + srcoff);
    ldm(r3, r4, r5, r6, r7);
    movw(r3, dstoff);
    add(r3, r3, r1);
    stm(r3, r4, r5, r6, r7);

    return code;
  }

  const void *
    emit_loadi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 src = GETARG_sBx(**ppc);
    /*mov(eax, src);
    mov(dword [ecx + dstoff], eax);
    mov(eax, 0xfff00000 | MRB_TT_FIXNUM);
    mov(dword [ecx + dstoff + 4], eax);*/
    movw(r3, src);
    mov32(r4, mrb_mktt(MRB_TT_FIXNUM));
    movw(r2, dstoff);
    add(r2, r2, r1);
    stm(r2, r3, r4);

    return code;
  }

  const void *
    emit_loadt(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*mov(eax, 1);
    mov(dword [ecx + dstoff], eax);
    mov(eax, 0xfff00000 | MRB_TT_TRUE);
    mov(dword [ecx + dstoff + 4], eax);*/
    movw(r3, 1);
    mov32(r4, mrb_mktt(MRB_TT_TRUE));
    movw(r2, dstoff);
    add(r2, r2, r1);
    stm(r2, r3, r4);

    return code;
  }

  const void *
    emit_loadf(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*mov(eax, 1);
    mov(dword [ecx + dstoff], eax);
    mov(eax, 0xfff00000 | MRB_TT_FALSE);
    mov(dword [ecx + dstoff + 4], eax);*/
    movw(r3, 1);
    mov32(r4, mrb_mktt(MRB_TT_FALSE));
    movw(r2, dstoff);
    add(r2, r2, r1);
    stm(r2, r3, r4);

    return code;
  }

  const void *
    emit_loadnil(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*xor(eax, eax);
    mov(dword [ecx + dstoff], eax);
    mov(eax, 0xfff00000 | MRB_TT_FALSE);
    mov(dword [ecx + dstoff + 4], eax);*/
    movw(r3, 0);
    mov32(r4, mrb_mktt(MRB_TT_FALSE));
    movw(r2, dstoff);
    add(r2, r2, r1);
    stm(r2, r3, r4);

    return code;
  }

  const void *
    emit_addi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 y = GETARG_C(**ppc);
    const Xtaak::uint32 off = GETARG_A(**ppc) * sizeof(mrb_value);
    int regno = GETARG_A(**ppc);
    /*mov(eax, dword [ecx + off + 4]); // Get type tag
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc);
    mov(eax, dword [ecx + off]);
    add(eax, y);
    mov(dword [ecx + off], eax);*/
    movw(r3, off);
    add(r3, r3, r1);
    ldr(r2, r3 + 4); /* Get type tag */
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc);
    ldr(r2, r3);
    add(r2, r2, y);
    str(r2, r3);

    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
