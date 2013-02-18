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
#include "mruby/proc.h"
#include "mruby/class.h"
#include "mruby/jit.h"

void mrbjit_exec_send(mrb_state *, mrbjit_vmstatus *);
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

  const Xtaak::Reg offset(const Xtaak::Reg &regSrc, Xtaak::uint32 off,
                          const Xtaak::Reg &regDst) {
    if (off < 0x100) {
      return regSrc + off;
    }
    if (Xtaak::inner::isShifterImm(off)) {
      add(regDst, regSrc, off);
    } else if (off < 0x10000) {
      movw(regDst, off);
      add(regDst, regDst, regSrc);
    } else {
      mov32(regDst, off);
      add(regDst, regDst, regSrc);
    }
    return regDst;
  }

  void
    call(void *func, const Xtaak::Reg &regDst)
  {
    int off = ((long)func - (long)getCurr() - 8) >> 2;
    if (off < -0x800000 || off > 0x7fffff) {
      mov32(regDst, (Xtaak::uint32)func);
      blx(regDst);
    }
    else {
      bl(func);
    }
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
    xor(eax, eax);
    ret();*/
    ldr(r0, "@f");
    str(r0, r9);
    movw(r0, 0);
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
    gen_type_guard(enum mrb_vtype tt, mrb_code *mruby_pc, const Xtaak::Reg &reg)
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
    adds(reg, reg, 0x100000);
    if (tt == MRB_TT_FLOAT) {
      bcc("@f");
    }
    else {
      cmp(reg, tt);
      /*jz("@f");*/
      beq("@f");
    }

    /* Guard fail exit code */
    /*mov(dword [ebx], (Xbyak::uint32)pc);
    xor(eax, eax);
    ret();*/
    mov32(r0, (Xtaak::uint32)mruby_pc);
    str(r0, r9);
    movw(r0, 0);
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
    xor(eax, eax);
    ret();*/
    mov32(r0, (Xtaak::uint32)mruby_pc);
    str(r0, r9);
    movw(r0, 0);
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
    ldrd(r0, offset(r10, srcoff, r2));
    strd(r0, offset(r10, dstoff, r2));
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
    ldrd(r0, r2);
    strd(r0, offset(r10, dstoff, r2));

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
    strd(r0, offset(r10, dstoff, r2));

    return code;
  }

  const void *
    emit_loadself(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc) 
  {
    const void *code = getCurr();
    const Xtaak::uint32 dstoff = GETARG_A(**ppc) * sizeof(mrb_value);

    /*movsd(xmm0, ptr [ecx]);
    movsd(ptr [ecx + dstoff], xmm0);*/
    ldrd(r0, r10);
    strd(r0, offset(r10, dstoff, r2));
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
    strd(r0, offset(r10, dstoff, r2));

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
    strd(r0, offset(r10, dstoff, r2));

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
    call((void*)mrb_vm_iv_get, r4);
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
    ldrd(r2, offset(r10, srcoff, r0));
    mov32(r0, (Xtaak::uint32)mrb);
    mov32(r1, (Xtaak::uint32)irep->syms[idpos]);
    call((void*)mrb_vm_iv_set, r4);
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
    strd(r0, offset(r10, dstoff, r2));
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
    strd(r0, offset(r10, dstoff, r2));

    return code;
  }

#define OffsetOf(s_type, field) ((size_t) &((s_type *)0)->field) 
#define CALL_MAXARGS 127

  const void *
    emit_send(mrb_state *mrb, mrbjit_vmstatus *status)
  {
    const void *code = getCurr();
    
    /*push(ecx);
    push(ebx);
    mov(eax, ptr[esp + 12]);
    push(eax);*/
    ldr(r1, sp);
    push(r9, r10, fp, lr);
    /* Update pc */
    /*mov(eax, dword [eax + OffsetOf(mrbjit_vmstatus, pc)]);
    mov(dword [eax], (Xbyak::uint32)(*status->pc));*/
    ldr(r0, offset(r1, OffsetOf(mrbjit_vmstatus, pc), r2));
    mov32(r2, (Xtaak::uint32)(*status->pc));
    str(r2, r0);

    /*push((Xbyak::uint32)mrb);
    call((void *)mrbjit_exec_send);
    add(esp, 8);
    pop(ebx);
    pop(ecx);*/
    mov32(r0, (Xtaak::uint32)mrb);
    call((void*)mrbjit_exec_send, r4);
    pop(r9, r10, fp, lr);

    return code;
  }

  void
    gen_overflow_check(const char op)
  {
    /*jno("@f");
    sub(esp, 8);
    movsd(qword [esp], xmm1);
    mov(eax, dword [ecx + reg0off]);
    cvtsi2sd(xmm0, eax);
    mov(eax, dword [ecx + reg1off]);
    cvtsi2sd(xmm1, eax);
    AINSTF(xmm0, xmm1);
    movsd(dword [ecx + reg0off], xmm0);
    movsd(xmm1, ptr [esp]);
    add(esp, 8);
    L("@@");*/
    bvc("@f");
    vmov(s0, r0);
    vcvt.f64.s32(d0, s0);
    vmov(s2, r1);
    vcvt.f64.s32(d1, s2);
    switch(op) {
    case '+':
      vadd.f64(d0, d0, d1);
      break;
    case '-':
      vsub.f64(d0, d0, d1);
      break;
    }
    vstr(d0, r2);
    L("@@");
  }

  bool
    gen_arth(const char op, mrb_code **ppc, mrb_value *regs)
  {
    int reg0pos = GETARG_A(**ppc);
    int reg1pos = reg0pos + 1;
    const Xtaak::uint32 reg0off = reg0pos * sizeof(mrb_value);
    const Xtaak::uint32 reg1off = reg1pos * sizeof(mrb_value);
    enum mrb_vtype r0type = (enum mrb_vtype) mrb_type(regs[reg0pos]);
    enum mrb_vtype r1type = (enum mrb_vtype) mrb_type(regs[reg1pos]);

    if (r0type != r1type) {
      return false;
    }
    /*mov(eax, dword [ecx + reg0off + 4]); / * Get type tag */
    movw(r2, reg0off);
    add(r2, r2, r10);
    ldr(r1, r2 + 4);
    gen_type_guard(r0type, *ppc, r1);
    /*mov(eax, dword [ecx + reg1off + 4]); / * Get type tag */
    movw(r3, reg1off);
    add(r3, r3, r10);
    ldr(r1, r3 + 4);
    gen_type_guard(r1type, *ppc, r1);

    if (r0type == MRB_TT_FIXNUM && r1type == MRB_TT_FIXNUM) {
      /*mov(eax, dword [ecx + reg0off]);*/
      /*AINSTI(eax, dword [ecx + reg1off]);*/
      /*mov(dword [ecx + reg0off], eax);*/
      ldr(r0, r2);
      ldr(r1, r3);
      switch(op) {
      case '+':
        adds(r3, r0, r1);
        break;
      case '-':
        subs(r3, r0, r1);
        break;
      }
      str(r3, r2);
      gen_overflow_check(op);
    }
    else if (r0type == MRB_TT_FLOAT && r1type == MRB_TT_FLOAT) {
      /*movsd(xmm0, ptr [ecx + reg0off]);*/
      /*AINSTF(xmm0, ptr [ecx + reg1off]);*/
      /*movsd(ptr [ecx + reg0off], xmm0);*/
      vldr(d0, r2);
      vldr(d1, r3);
      switch(op) {
      case '+':
        vadd.f64(d0, d0, d1);
        break;
      case '-':
        vsub.f64(d0, d0, d1);
        break;
      }
      vstr(d0, r2);
    }
    else {
      /*mov(dword [ebx], (Xbyak::uint32)*ppc);
      xor(eax, eax);
      ret();*/
      ldr(r0, "@f");
      str(r0, r9);
      movw(r0, 0);
      mov(pc, lr);
      L("@@");
      dd((Xtaak::uint32)*ppc);
    }
    return true;
  }

  const void *
    emit_add(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    if (!gen_arth('+', ppc, regs)) { return NULL; }
    return code;
  }

  const void *
    emit_sub(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    if (!gen_arth('-', ppc, regs)) { return NULL; }
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

  void
    gen_overflow_check_i(const char op, const Xtaak::uint32 y)
  {
    /*jno("@f");
    sub(esp, 8);
    movsd(qword [esp], xmm1);
    mov(eax, dword [ecx + off]);
    cvtsi2sd(xmm0, eax);
    mov(eax, y);
    cvtsi2sd(xmm1, eax);
    AINSTF(xmm0, xmm1);
    movsd(dword [ecx + off], xmm0);
    movsd(xmm1, ptr [esp]);
    add(esp, 8);
    L("@@");*/
    bvc("@f");
    vmov(s0, r0);
    vcvt.f64.s32(d0, s0);
    movw(r0, y);
    vmov(s2, r0);
    vcvt.f64.s32(d1, s2);
    switch(op) {
    case '+':
      vadd.f64(d0, d0, d1);
      break;
    case '-':
      vsub.f64(d0, d0, d1);
      break;
    }
    vstr(d0, r2);
    L("@@");
  }

  void
    gen_arth_i(const char op, mrb_code **ppc, mrb_value *regs)
  {
    const Xtaak::uint32 y = GETARG_C(**ppc);
    const Xtaak::uint32 off = GETARG_A(**ppc) * sizeof(mrb_value);
    int regno = GETARG_A(**ppc);
    enum mrb_vtype atype = (enum mrb_vtype) mrb_type(regs[regno]);
    /*mov(eax, dword [ecx + off + 4]); / * Get type tag */
    movw(r2, off);
    add(r2, r2, r10);
    ldr(r1, r2 + 4); /* Get type tag */
    gen_type_guard(atype, *ppc, r1);

    if (atype == MRB_TT_FIXNUM) {
      /*mov(eax, dword [ecx + off]);
      AINSTI(eax, y);
      mov(dword [ecx + off], eax);*/
      ldr(r0, r2);
      switch(op) {
      case '+':
        adds(r1, r0, y);
        break;
      case '-':
        subs(r1, r0, y);
        break;
      }
      str(r1, r2);
      gen_overflow_check_i(op, y);
    }
    else if (atype == MRB_TT_FLOAT) {
      /*sub(esp, 8);
      movsd(qword [esp], xmm1);
      movsd(xmm0, ptr [ecx + off]);
      mov(eax, y);
      cvtsi2sd(xmm1, eax);
      AINSTF(xmm0, xmm1);
      movsd(ptr [ecx + off], xmm0);
      movsd(xmm1, ptr [esp]);
      add(esp, 8);*/
      vldr(d0, r2);
      movw(r0, y);
      vmov(s2, r0);
      vcvt.f64.s32(d1, s2);
      switch(op) {
      case '+':
        vadd.f64(d0, d0, d1);
        break;
      case '-':
        vsub.f64(d0, d0, d1);
        break;
      }
      vstr(d0, r2);
    }
    else {
      /*mov(dword [ebx], (Xbyak::uint32)*ppc);
      xor(eax, eax);
      ret();*/
      ldr(r0, "@f");
      str(r0, r9);
      movw(r0, 0);
      mov(pc, lr);
      L("@@");
      dd((Xtaak::uint32)*ppc);
    }
  }

  const void *
    emit_addi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_arth_i('+', ppc, regs);
    return code;
  }

  const void *
    emit_subi(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_arth_i('-', ppc, regs);
    return code;
  }

  void
    gen_comp(Cond cond, mrb_code **ppc, mrb_value *regs)
  {
    int regno = GETARG_A(**ppc);
    const Xtaak::uint32 off0 = regno * sizeof(mrb_value);
    /*const Xtaak::uint32 off1 = off0 + sizeof(mrb_value); */
    /*mov(eax, dword [ecx + off0 + 4]); / * Get type tag */
    movw(r4, off0);
    add(r4, r4, r10);
    ldm(r4, r0, r1, r2, r3);
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno]), *ppc, r1);
    /*mov(eax, dword [ecx + off1 + 4]); / * Get type tag */
    gen_type_guard((enum mrb_vtype)mrb_type(regs[regno + 1]), *ppc, r3);

    if (mrb_type(regs[regno]) == MRB_TT_FLOAT &&
        mrb_type(regs[regno + 1]) == MRB_TT_FIXNUM) {
      /*sub(esp, 8);
      movsd(qword [esp], xmm1);
      movsd(xmm0, ptr [ecx + off0]);
      cvtsi2sd(xmm1, ptr [ecx + off1]);
      xor(eax, eax);
      comisd(xmm0, xmm1);
      CMPINST(al);
      movsd(xmm1, ptr [esp]);
      add(esp, 8);*/
      vmov(d0, r0, r1);
      vmov(s2, r2);
      vcvt.f64.s32(d1, s2);
      vcmp.f64(d0, d1);
      vmrs(APSR_nzcv, fpscr);
    }
    else if (mrb_type(regs[regno]) == MRB_TT_FIXNUM &&
             mrb_type(regs[regno + 1]) == MRB_TT_FLOAT) {
      /*cvtsi2sd(xmm0, ptr [ecx + off0]);
      xor(eax, eax);
      comisd(xmm0, ptr [ecx + off1]);
      CMPINST(al);*/
      vmov(s0, r0);
      vcvt.f64.s32(d0, s0);
      vmov(d1, r2, r3);
      vcmp.f64(d0, d1);
      vmrs(APSR_nzcv, fpscr);
    }
    else if (mrb_type(regs[regno]) == MRB_TT_FLOAT &&
             mrb_type(regs[regno + 1]) == MRB_TT_FLOAT) {
      /*movsd(xmm0, dword [ecx + off0]);
      xor(eax, eax);
      comisd(xmm0, ptr [ecx + off1]);
      CMPINST(al);*/
      vmov(d0, r0, r1);
      vmov(d1, r2, r3);
      vcmp.f64(d0, d1);
      vmrs(APSR_nzcv, fpscr);
    }
    else {
      /*mov(eax, dword [ecx + off0]);
      cmp(eax, dword [ecx + off1]);
      CMPINST(al);
      mov(ah, 0);*/
      cmp(r0, r2);
    }
    /*cwde();
    add(eax, eax);
    add(eax, 0xfff00001);
    mov(dword [ecx + off0 + 4], eax);
    mov(dword [ecx + off0], 1);*/
    mov32(r1, mrb_mktt(MRB_TT_FALSE));
    setCond(cond);
    add(r1, r1, MRB_TT_TRUE - MRB_TT_FALSE);
    setCond(AL);
    movw(r0, 1);
    stm(r4, r0, r1);
 }
  
  const void *
    emit_eq(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_comp(EQ, ppc, regs);

    return code;
  }

  const void *
    emit_lt(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_comp(LT, ppc, regs);

    return code;
  }

  const void *
    emit_le(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_comp(LE, ppc, regs);

    return code;
  }

  const void *
    emit_gt(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_comp(GT, ppc, regs);

    return code;
  }

  const void *
    emit_ge(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs) 
  {
    const void *code = getCurr();
    gen_comp(GE, ppc, regs);

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
    call((void*)mrb_uvget, r4);
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
    call((void*)mrb_uvset, r4);
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
    ldr(r0, offset(r10, coff + 4, r1));
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
    ldr(r0, offset(r10, coff + 4, r1));
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
