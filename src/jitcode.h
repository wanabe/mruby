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

#ifdef ENABLE_JIT

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

  void 
    gen_type_guard(enum mrb_vtype tt, mrb_code *mruby_pc)
  {
    /* Input eax for type tag
    if (tt == MRB_TT_FLOAT) {
      cmp(eax, 0xfff00000);
      jb("@f");
    } 
    else {
      cmp(eax, 0xfff00000 | tt);
      jz("@f");
    }*/
    /* Input r2 for type tag */
    adds(r2, r2, 0x100000);
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

  void
    gen_bool_guard(int b, mrb_code *mruby_pc)
  {
    /* Input eax for tested boolean */
    /*cmp(eax, 0xfff00001);*/
    add(r2, r2, 0x100000);
    cmp(r2, 1);
    if (b) {
      bne(3);
    }
    else {
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
    mov(dword [ecx + off], eax);
    jno("@f");
    sub(esp, 8);
    movsd(qword [esp], xmm1);
    mov(eax, dword [ecx + off]);
    cvtsi2sd(xmm0, eax);
    mov(eax, y);
    cvtsi2sd(xmm1, eax);
    addsd(xmm0, xmm1);
    movsd(dword [ecx + off], xmm0);
    movsd(xmm1, ptr [esp]);
    add(esp, 8);
    L("@@");*/
    movw(r3, off);
    add(r3, r3, r1);
    ldr(r2, r3 + 4); /* Get type tag */
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc);
    ldr(r4, r3);
    adds(r2, r4, y);
    str(r2, r3);
    bvc(6);
    fmsr(s0, r4);
    fsitod(d0, s0);
    movw(r2, y);
    fmsr(s2, r2);
    fsitod(d1, s2);
    faddd(d0, d0, d1);
    fstd(d0, r3);

    return code;
  }

#define COMP_GEN_II(CMPINST)                                         \
do {                                                                 \
    /*mov(eax, dword [ecx + off0]);*/                                \
    /*cmp(eax, dword [ecx + off1]);*/                                \
    /*CMPINST(al);*/                                                 \
    /*cwde();*/                                                      \
    /*add(eax, eax);*/                                               \
    /*add(eax, 0xfff00001);*/                                        \
    /*mov(dword [ecx + off0 + 4], eax);*/                            \
    /*mov(dword [ecx + off0], 1);*/                                  \
    ldr(r2, r4);                                                     \
    ldr(r3, r5);                                                     \
    cmp(r2, r3);                                                     \
    mov32(r3, mrb_mktt(MRB_TT_FALSE));                               \
    setCond(CMPINST);                                                \
    add(r3, r3, MRB_TT_TRUE - MRB_TT_FALSE);                         \
    setCond(AL);                                                     \
    movw(r2, 1);                                                     \
    stm(r4, r2, r3);                                                 \
} while(0)

#define COMP_GEN_IF(CMPINST)                                         \
do {                                                                 \
    /*cvtsi2sd(xmm0, ptr [ecx + off0]);*/                            \
    /*comisd(xmm0, ptr [ecx + off1]);*/                              \
    /*CMPINST(al);*/                                                 \
    /*cwde();*/                                                      \
    /*add(eax, eax);*/                                               \
    /*add(eax, 0xfff00001);*/                                        \
    /*mov(dword [ecx + off0 + 4], eax);*/                            \
    /*mov(dword [ecx + off0], 1);*/                                  \
    flds(s0, r4);                                                    \
    fsitod(d0, s0);                                                  \
    fldd(d1, r5);                                                    \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
    mov32(r3, mrb_mktt(MRB_TT_FALSE));                               \
    setCond(CMPINST);                                                \
    add(r3, r3, MRB_TT_TRUE - MRB_TT_FALSE);                         \
    setCond(AL);                                                     \
    movw(r2, 1);                                                     \
    stm(r4, r2, r3);                                                 \
} while(0)

#define COMP_GEN_FI(CMPINST)                                         \
do {                                                                 \
    /*sub(esp, 8);*/                                                 \
    /*movsd(qword [esp], xmm1);*/                                    \
    /*movsd(xmm0, ptr [ecx + off0]);*/                               \
    /*cvtsi2sd(xmm1, ptr [ecx + off1]);*/                            \
    /*comisd(xmm0, xmm1);*/                                          \
    /*CMPINST(al);*/                                                 \
    /*cwde();*/                                                      \
    /*add(eax, eax);*/                                               \
    /*add(eax, 0xfff00001);*/                                        \
    /*mov(dword [ecx + off0 + 4], eax);*/                            \
    /*mov(dword [ecx + off0], 1);*/                                  \
    /*movsd(xmm1, ptr [esp]);*/                                      \
    /*add(esp, 8);*/                                                 \
    fldd(d0, r4);                                                    \
    flds(s2, r5);                                                    \
    fsitod(d1, s2);                                                  \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
    mov32(r3, mrb_mktt(MRB_TT_FALSE));                               \
    setCond(CMPINST);                                                \
    add(r3, r3, MRB_TT_TRUE - MRB_TT_FALSE);                         \
    setCond(AL);                                                     \
    movw(r2, 1);                                                     \
    stm(r4, r2, r3);                                                 \
} while(0)

#define COMP_GEN_FF(CMPINST)                                         \
do {                                                                 \
    /*movsd(xmm0, dword [ecx + off0]);*/                             \
    /*comisd(xmm0, ptr [ecx + off1]);*/                              \
    /*CMPINST(al);*/                                                 \
    /*cwde();*/                                                      \
    /*add(eax, eax);*/                                               \
    /*add(eax, 0xfff00001);*/                                        \
    /*mov(dword [ecx + off0 + 4], eax);*/                            \
    /*mov(dword [ecx + off0], 1);*/                                  \
    fldd(d0, r4);                                                    \
    fldd(d1, r5);                                                    \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
    mov32(r3, mrb_mktt(MRB_TT_FALSE));                               \
    setCond(CMPINST);                                                \
    add(r3, r3, MRB_TT_TRUE - MRB_TT_FALSE);                         \
    setCond(AL);                                                     \
    movw(r2, 1);                                                     \
    stm(r4, r2, r3);                                                 \
} while(0)
    
#define COMP_GEN(CMPINST)                                            \
do {                                                                 \
    int regno = GETARG_A(**ppc);                                     \
    const Xtaak::uint32 off0 = regno * sizeof(mrb_value);            \
    const Xtaak::uint32 off1 = off0 + sizeof(mrb_value);             \
    /*mov(eax, dword [ecx + off0 + 4]); /* Get type tag */           \
    movw(r4, off0);                                                  \
    add(r4, r4, r1);                                                 \
    ldr(r2, r4 + 4);                                                 \
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc);     \
    /*mov(eax, dword [ecx + off1 + 4]); /* Get type tag */           \
    movw(r5, off1);                                                  \
    add(r5, r5, r1);                                                 \
    ldr(r2, r5 + 4);                                                 \
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno + 1]), *ppc); \
                                                                     \
    if (mrb_type(regs[regno]) == MRB_TT_FLOAT &&                     \
             mrb_type(regs[regno + 1]) == MRB_TT_FIXNUM) {           \
          COMP_GEN_FI(CMPINST);                                      \
    }                                                                \
    else if (mrb_type(regs[regno]) == MRB_TT_FIXNUM &&               \
             mrb_type(regs[regno + 1]) == MRB_TT_FLOAT) {            \
          COMP_GEN_IF(CMPINST);                                      \
    }                                                                \
    else if (mrb_type(regs[regno]) == MRB_TT_FLOAT &&                \
             mrb_type(regs[regno + 1]) == MRB_TT_FLOAT) {            \
          COMP_GEN_FF(CMPINST);                                      \
    }                                                                \
    else {                                                           \
          COMP_GEN_II(CMPINST);                                      \
    }                                                                \
} while(0)

  const void *
    emit_eq(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    COMP_GEN(EQ);

    return code;
  }

  const void *
    emit_lt(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    COMP_GEN(LT);

    return code;
  }

  const void *
    emit_le(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    COMP_GEN(LE);

    return code;
  }

  const void *
    emit_gt(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    COMP_GEN(GT);

    return code;
  }

  const void *
    emit_ge(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    COMP_GEN(GE);

    return code;
  }

  const void *
    emit_jmpif(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs)
  {
    const void *code = getCurr();
    const int cond = GETARG_A(**ppc);
    const Xtaak::uint32 coff =  cond * sizeof(mrb_value);
    
    /*mov(eax, ptr [ecx + coff + 4]);*/
    movw(r3, coff + 4);
    add(r3, r3, r1);
    ldr(r2, r3);
    if (mrb_test(regs[cond])) {
      gen_bool_guard(1, *ppc + 1);
    }
    else {
      gen_bool_guard(0, *ppc + GETARG_sBx(**ppc));
    }

    return code;
  }

  const void *
    emit_jmpnot(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs)
  {
    const void *code = getCurr();
    const int cond = GETARG_A(**ppc);
    const Xtaak::uint32 coff =  cond * sizeof(mrb_value);
    
    /*mov(eax, ptr [ecx + coff + 4]);*/
    movw(r3, coff + 4);
    add(r3, r3, r1);
    ldr(r2, r3);
    if (!mrb_test(regs[cond])) {
      gen_bool_guard(0, *ppc + 1);
    }
    else {
      gen_bool_guard(1, *ppc + GETARG_sBx(**ppc));
    }

    return code;
  }
};

#endif  /* ENABLE_JIT */

#endif  /* MRUBY_JITCODE_H */
