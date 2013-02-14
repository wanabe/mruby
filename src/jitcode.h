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
#include "mruby/variable.h"
#include "mruby/jit.h"
} /* extern "C" */

#ifdef ENABLE_JIT

/* Regs Map                               *
 * r10   -- pointer to regs               *
 * r9    -- pointer to pc                 */
class MRBJitCode: public Xtaak::CodeGenerator {

 public:

 MRBJitCode():
  CodeGenerator(1024 * 1024)
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
    ldr(r0, "@f");
    str(r0, r9);
    mov(pc, lr);
    L("@@");
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
    adds(r1, r1, 0x100000);
    if (tt == MRB_TT_FLOAT) {
      bcc("@f");
    }
    else {
      cmp(r1, tt);
      /*jz("@f");*/
      beq("@f");
    }

    /* Guard fail exit code */
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    ret();*/
    mov32(r0, (Xtaak::uint32)mruby_pc);
    str(r0, r9);
    mov(pc, lr);
    L("@@");
  }

  void
    gen_bool_guard(int b, mrb_code *mruby_pc)
  {
    /* Input eax for tested boolean */
    /*cmp(eax, 0xfff00001);*/
    add(r0, r0, 0x100000);
    cmp(r0, 1);
    if (b) {
      bne("@f");
    }
    else {
      beq("@f");
    }

    /* Guard fail exit code */
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    ret();*/
    mov32(r0, (Xtaak::uint32)mruby_pc);
    str(r0, r9);
    mov(pc, lr);
    L("@@");
  }

  const void *
    emit_nop(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    return code;
  }

  const void *
    emit_move(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const Xtaak::uint32 srcoff = GETARG_B(**ppc) * sizeof(mrb_value);
    /*movsd(xmm0, ptr [ecx + srcoff]);
    movsd(ptr [ecx + dstoff], xmm0);*/
    movw(r2, srcoff);
    add(r2, r2, r10);
    ldm(r2, r0, r1);
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);
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
    mov32(r2, (Xtaak::uint32)irep->pool + srcoff);
    ldm(r2, r0, r1);
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);

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
    movw(r0, src);
    mov32(r1, mrb_mktt(MRB_TT_FIXNUM));
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);

    return code;
  }

  const void *
    emit_loadself(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);

    /*movsd(xmm0, ptr [ecx]);
    movsd(ptr [ecx + dstoff], xmm0);*/
    ldm(r10, r0, r1);
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);
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
    movw(r0, 1);
    mov32(r1, mrb_mktt(MRB_TT_TRUE));
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);

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
    movw(r0, 1);
    mov32(r1, mrb_mktt(MRB_TT_FALSE));
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);

    return code;
  }

  const void *
    emit_getiv(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 idpos = GETARG_Bx(**ppc);
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*const int argsize = 2 * sizeof(void *);*/

    /*push(ecx);
    push(ebx);
    push((Xbyak::uint32)irep->syms[idpos]);
    push((Xbyak::uint32)mrb);
    call((void *)mrb_vm_iv_get);
    add(sp, argsize);
    pop(ebx);
    pop(ecx);
    mov(dword [ecx + dstoff], eax);
    mov(dword [ecx + dstoff + 4], edx);*/
    push(r9, r10, fp, lr);
    movw(r0, dstoff);
    add(r0, r0, r10);
    mov32(r1, (Xtaak::uint32)mrb);
    mov32(r2, (Xtaak::uint32)irep->syms[idpos]);
    bl((void *)mrb_vm_iv_get);
    pop(r9, r10, fp, lr);

    return code;
  }

  const void *
    emit_setiv(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 idpos = GETARG_Bx(**ppc);
    const Xtaak::uint32 srcoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*const int argsize = 4 * sizeof(void *);*/

    /*push(ecx);
    push(ebx);
    mov(eax, dword [ecx + srcoff + 4]);
    push(eax);
    mov(eax, dword [ecx + srcoff]);
    push(eax);
    push((Xbyak::uint32)irep->syms[idpos]);
    push((Xbyak::uint32)mrb);
    call((void *)mrb_vm_iv_set);
    add(sp, argsize);
    pop(ebx);
    pop(ecx);*/
    push(r9, r10, fp, lr);
    movw(r0, srcoff);
    add(r0, r0, r10);
    ldm(r0, r2, r3);
    mov32(r0, (Xtaak::uint32)mrb);
    mov32(r1, (Xtaak::uint32)irep->syms[idpos]);
    bl((void *)mrb_vm_iv_set);
    pop(r9, r10, fp, lr);

    return code;
  }

  const void *
    emit_getconst(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const int sympos = GETARG_Bx(**ppc);
    const mrb_value v = mrb_vm_const_get(mrb, irep->syms[sympos]);

    /*mov(dword [ecx + dstoff], v.value.i);
    mov(dword [ecx + dstoff + 4], v.ttt);*/
    ldrd(r0, "@f");
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);
    b(1);
    L("@@");
    dd((Xtaak::uint32)v.value.i);
    dd((Xtaak::uint32)v.ttt);

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
    movw(r0, 0);
    mov32(r1, mrb_mktt(MRB_TT_FALSE));
    movw(r2, dstoff);
    add(r2, r2, r10);
    stm(r2, r0, r1);

    return code;
  }

#define OVERFLOW_CHECK_GEN(AINSTF)                                      \
    /*jno("@f");*/                                                      \
    /*sub(esp, 8);*/                                                    \
    /*movsd(qword [esp], xmm1);*/                                       \
    /*mov(eax, dword [ecx + reg0off]);*/                                \
    /*cvtsi2sd(xmm0, eax);*/                                            \
    /*mov(eax, dword [ecx + reg1off]);*/                                \
    /*cvtsi2sd(xmm1, eax);*/                                            \
    /*AINSTF(xmm0, xmm1);*/                                             \
    /*movsd(dword [ecx + reg0off], xmm0);*/                             \
    /*movsd(xmm1, ptr [esp]);*/                                         \
    /*add(esp, 8);*/                                                    \
    /*L("@@");*/                                                        \
    bvc("@f");                                                          \
    fmsr(s0, r0);                                                       \
    fsitod(d0, s0);                                                     \
    fmsr(s2, r1);                                                       \
    fsitod(d1, s2);                                                     \
    AINSTF(d0, d0, d1);                                                 \
    fstd(d0, r2);                                                       \
    L("@@");                                                            \

#define ARTH_GEN(AINSTI, AINSTF)                                        \
  do {                                                                  \
    int reg0pos = GETARG_A(**ppc);                                      \
    int reg1pos = reg0pos + 1;                                          \
    const Xtaak::uint32 reg0off = reg0pos * sizeof(mrb_value);          \
    const Xtaak::uint32 reg1off = reg1pos * sizeof(mrb_value);          \
    enum mrb_vtype r0type = (enum mrb_vtype) mrb_type(regs[reg0pos]);   \
    enum mrb_vtype r1type = (enum mrb_vtype) mrb_type(regs[reg1pos]);   \
\
    if (r0type != r1type) {                                             \
      return NULL;                                                      \
    }                                                                   \
    /*mov(eax, dword [ecx + reg0off + 4]); /* Get type tag */           \
    movw(r2, reg0off);                                                  \
    add(r2, r2, r10);                                                   \
    ldr(r1, r2 + 4);                                                    \
    gen_type_guard(r0type, *ppc);                                       \
    /*mov(eax, dword [ecx + reg1off + 4]); /* Get type tag */           \
    movw(r3, reg1off);                                                  \
    add(r3, r3, r10);                                                   \
    ldr(r1, r3 + 4);                                                    \
    gen_type_guard(r1type, *ppc);                                       \
\
    if (r0type == MRB_TT_FIXNUM && r1type == MRB_TT_FIXNUM) {           \
      /*mov(eax, dword [ecx + reg0off]);*/                              \
      /*AINSTI(eax, dword [ecx + reg1off]);*/                           \
      /*mov(dword [ecx + reg0off], eax);*/                              \
      ldr(r0, r2);                                                      \
      ldr(r1, r3);                                                      \
      AINSTI(r3, r0, r1);                                               \
      str(r2, r3);                                                      \
      OVERFLOW_CHECK_GEN(AINSTF);                                       \
    }                                                                   \
    else if (r0type == MRB_TT_FLOAT && r1type == MRB_TT_FLOAT) {        \
      /*movsd(xmm0, ptr [ecx + reg0off]);*/                             \
      /*AINSTF(xmm0, ptr [ecx + reg1off]);*/                            \
      /*movsd(ptr [ecx + reg0off], xmm0);*/                             \
      fldd(d0, r2);                                                     \
      fldd(d1, r3);                                                     \
      AINSTF(d0, d0, d1);                                               \
      fstd(d0, r2);                                                     \
    }                                                                   \
    else {                                                              \
      /*mov(dword [ebx], (Xbyak::uint32)*ppc);*/                        \
      /*ret();*/                                                        \
      ldr(r0, "@f");                                                    \
      str(r0, r9);                                                      \
      mov(pc, lr);                                                      \
      L("@@");                                                          \
      dd((Xtaak::uint32)*ppc);                                          \
    }                                                                   \
} while(0)

  const void *
    emit_add(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    ARTH_GEN(adds, faddd);
    return code;
  }

  const void *
    emit_sub(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    ARTH_GEN(subs, fsubd);
    return code;
  }

  const void *
    emit_mul(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    //ARTH_GEN(imul, mulsd);
    return code;
  }

  const void *
    emit_div(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    return code;
  }

#define OVERFLOW_CHECK_I_GEN(AINSTF)                                    \
    /*jno("@f");*/                                                      \
    /*sub(esp, 8);*/                                                    \
    /*movsd(qword [esp], xmm1);*/                                       \
    /*mov(eax, dword [ecx + off]);*/                                    \
    /*cvtsi2sd(xmm0, eax);*/                                            \
    /*mov(eax, y);*/                                                    \
    /*cvtsi2sd(xmm1, eax);*/                                            \
    /*AINSTF(xmm0, xmm1);*/                                             \
    /*movsd(dword [ecx + off], xmm0);*/                                 \
    /*movsd(xmm1, ptr [esp]);*/                                         \
    /*add(esp, 8);*/                                                    \
    /*L("@@");*/                                                        \
    bvc("@f");                                                          \
    fmsr(s0, r0);                                                       \
    fsitod(d0, s0);                                                     \
    movw(r0, y);                                                        \
    fmsr(s2, r0);                                                       \
    fsitod(d1, s2);                                                     \
    AINSTF(d0, d0, d1);                                                 \
    fstd(d0, r2);                                                       \
    L("@@");                                                            \

#define ARTH_I_GEN(AINSTI, AINSTF)                                      \
  do {                                                                  \
    const Xtaak::uint32 y = GETARG_C(**ppc);                            \
    const Xtaak::uint32 off = GETARG_A(**ppc) * sizeof(mrb_value);      \
    int regno = GETARG_A(**ppc);                                        \
    enum mrb_vtype atype = (enum mrb_vtype) mrb_type(regs[regno]);      \
    /*mov(eax, dword [ecx + off + 4]); /* Get type tag */               \
    movw(r2, off);                                                      \
    add(r2, r2, r10);                                                   \
    ldr(r1, r2 + 4); /* Get type tag */                                 \
    gen_type_guard(atype, *ppc);                                        \
\
    if (atype == MRB_TT_FIXNUM) {                                       \
      /*mov(eax, dword [ecx + off]);*/                                  \
      /*AINSTI(eax, y);*/                                               \
      /*mov(dword [ecx + off], eax);*/                                  \
      ldr(r0, r2);                                                      \
      AINSTI(r1, r0, y);                                                \
      str(r1, r2);                                                      \
      OVERFLOW_CHECK_I_GEN(AINSTF);                                     \
    }                                                                   \
    else if (atype == MRB_TT_FLOAT) {                                   \
      /*sub(esp, 8);*/                                                  \
      /*movsd(qword [esp], xmm1);*/                                     \
      /*movsd(xmm0, ptr [ecx + off]);*/                                 \
      /*mov(eax, y);*/                                                  \
      /*cvtsi2sd(xmm1, eax);*/                                          \
      /*AINSTF(xmm0, xmm1);*/                                           \
      /*movsd(ptr [ecx + off], xmm0);*/                                 \
      /*movsd(xmm1, ptr [esp]);*/                                       \
      /*add(esp, 8);*/                                                  \
      fldd(d0, r2);                                                     \
      movw(r0, y);                                                      \
      fmsr(s2, r0);                                                     \
      fsitod(d1, s2);                                                   \
      AINSTF(d0, d0, d1);                                               \
      fstd(d0, r2);                                                     \
    }                                                                   \
    else {                                                              \
      /*mov(dword [ebx], (Xbyak::uint32)*ppc);*/                        \
      /*ret();*/                                                        \
      ldr(r0, "@f");                                                    \
      str(r0, r9);                                                      \
      mov(pc, lr);                                                      \
      L("@@");                                                          \
      dd((Xtaak::uint32)*ppc);                                          \
    }                                                                   \
} while(0)
    
  const void *
    emit_addi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    ARTH_I_GEN(adds, faddd);
    return code;
  }

  const void *
    emit_subi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    ARTH_I_GEN(subs, fsubd);
    return code;
  }

#define COMP_GEN_II(CMPINST)                                         \
do {                                                                 \
    /*mov(eax, dword [ecx + off0]);*/                                \
    /*cmp(eax, dword [ecx + off1]);*/                                \
    /*CMPINST(al);*/                                                 \
    /*mov(ah, 0);*/                                                  \
    ldr(r2, r4);                                                     \
    ldr(r3, r5);                                                     \
    cmp(r2, r3);                                                     \
} while(0)

#define COMP_GEN_IF(CMPINST)                                         \
do {                                                                 \
    /*cvtsi2sd(xmm0, ptr [ecx + off0]);*/                            \
    /*xor(eax, eax);*/                                               \
    /*comisd(xmm0, ptr [ecx + off1]);*/                              \
    /*CMPINST(al);*/                                                 \
    flds(s0, r4);                                                    \
    fsitod(d0, s0);                                                  \
    fldd(d1, r5);                                                    \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
} while(0)

#define COMP_GEN_FI(CMPINST)                                         \
do {                                                                 \
    /*sub(esp, 8);*/                                                 \
    /*movsd(qword [esp], xmm1);*/                                    \
    /*movsd(xmm0, ptr [ecx + off0]);*/                               \
    /*cvtsi2sd(xmm1, ptr [ecx + off1]);*/                            \
    /*xor(eax, eax);*/                                               \
    /*comisd(xmm0, xmm1);*/                                          \
    /*CMPINST(al);*/                                                 \
    /*movsd(xmm1, ptr [esp]);*/                                      \
    /*add(esp, 8);*/                                                 \
    fldd(d0, r4);                                                    \
    flds(s2, r5);                                                    \
    fsitod(d1, s2);                                                  \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
} while(0)

#define COMP_GEN_FF(CMPINST)                                         \
do {                                                                 \
    /*movsd(xmm0, dword [ecx + off0]);*/                             \
    /*xor(eax, eax);*/                                               \
    /*comisd(xmm0, ptr [ecx + off1]);*/                              \
    /*CMPINST(al);*/                                                 \
    fldd(d0, r4);                                                    \
    fldd(d1, r5);                                                    \
    fcmpd(d0, d1);                                                   \
    fmstat();                                                        \
} while(0)
    
#define COMP_GEN(CMPINST)                                            \
do {                                                                 \
    int regno = GETARG_A(**ppc);                                     \
    const Xtaak::uint32 off0 = regno * sizeof(mrb_value);            \
    const Xtaak::uint32 off1 = off0 + sizeof(mrb_value);             \
    /*mov(eax, dword [ecx + off0 + 4]); /* Get type tag */           \
    movw(r4, off0);                                                  \
    add(r4, r4, r10);                                                \
    ldr(r1, r4 + 4);                                                 \
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc);     \
    /*mov(eax, dword [ecx + off1 + 4]); /* Get type tag */           \
    movw(r5, off1);                                                  \
    add(r5, r5, r10);                                                \
    ldr(r1, r5 + 4);                                                 \
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
    /*cwde();*/                                                      \
    /*add(eax, eax);*/                                               \
    /*add(eax, 0xfff00001);*/                                        \
    /*mov(dword [ecx + off0 + 4], eax);*/                            \
    /*mov(dword [ecx + off0], 1);*/                                  \
    mov32(r1, mrb_mktt(MRB_TT_FALSE));                               \
    setCond(CMPINST);                                                \
    add(r1, r1, MRB_TT_TRUE - MRB_TT_FALSE);                         \
    setCond(AL);                                                     \
    movw(r0, 1);                                                     \
    stm(r4, r0, r1);                                                 \
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
    emit_getupvar(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 uppos = GETARG_C(**ppc);
    const Xtaak::uint32 idxpos = GETARG_B(**ppc);
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);
    /*const int argsize = 3 * sizeof(void *);*/

    /*push(ecx);
    push(ebx);
    push(idxpos);
    push(uppos);
    push((Xbyak::uint32)mrb);
    call((void *)mrb_uvget);
    add(sp, argsize);
    pop(ebx);
    pop(ecx);
    mov(dword [ecx + dstoff], eax);
    mov(dword [ecx + dstoff + 4], edx);*/
    push(r9, r10, fp, lr);
    movw(r0, dstoff);
    add(r0, r0, r10);
    mov32(r1, (Xtaak::uint32)mrb);
    mov32(r2, uppos);
    mov32(r3, idxpos);
    bl((void *)mrb_uvget);
    pop(r9, r10, fp, lr);

    return code;
  }

  const void *
    emit_setupvar(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
  {
    const void *code = getCurr();
    const Xtaak::uint32 uppos = GETARG_C(**ppc);
    const Xtaak::uint32 idxpos = GETARG_B(**ppc);
    const Xtaak::uint32 valoff = GETARG_A(**ppc) * sizeof(mrb_value);
    const int argsize = (5 - 3) * sizeof(void *);

    /*push(ecx);
    push(ebx);
    mov(eax, dword [ecx + valoff + 4]);
    push(eax);
    mov(eax, dword [ecx + valoff]);
    push(eax);
    push(idxpos);
    push(uppos);
    push((Xbyak::uint32)mrb);
    call((void *)mrb_uvset);
    add(sp, argsize);
    pop(ebx);
    pop(ecx);*/
    push(r9, r10, fp, lr);
    movw(r0, valoff + 4);
    add(r0, r0, r10);
    ldmda(r0, r2, r3);
    push(r2, r3);
    mov32(r0, (Xtaak::uint32)mrb);
    mov32(r1, uppos);
    mov32(r2, idxpos);
    bl((void *)mrb_uvset);
    add(sp, sp, argsize);
    pop(r9, r10, fp, lr);

    return code;
  }

  const void *
    emit_jmpif(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs)
  {
    const void *code = getCurr();
    const int cond = GETARG_A(**ppc);
    const Xtaak::uint32 coff =  cond * sizeof(mrb_value);
    
    /*mov(eax, ptr [ecx + coff + 4]);*/
    movw(r1, coff + 4);
    add(r1, r1, r10);
    ldr(r0, r1);
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
    movw(r1, coff + 4);
    add(r1, r1, r10);
    ldr(r0, r1);
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
