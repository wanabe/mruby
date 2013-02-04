#include "jitcode.h"

extern "C" {

#ifdef ENABLE_JIT

const void *
mrbjit_gen_entry(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  return code->gen_entry(mrb, irep);
}

void
mrbjit_gen_exit(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  code->gen_exit(*ppc);
}

void
mrbjit_gen_jump_block(mrbjit_code_area coderaw, void *entry)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  code->gen_jump_block(entry);
}

const void *
mrbjit_emit_code(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs)
{
  MRBJitCode *code = (MRBJitCode *) irep->compile_info->code_base;
  const void *entry;

  if (code == NULL) {
    code = new MRBJitCode();
    irep->compile_info->code_base = code;
    entry = code->gen_entry(mrb, irep);
  }

  switch(GET_OPCODE(**ppc)) {
  case OP_MOVE:
    return code->emit_move(mrb, irep, ppc);

  case OP_LOADL:
    return code->emit_loadl(mrb, irep, ppc);

  case OP_LOADI:
    return code->emit_loadi(mrb, irep, ppc);

    // OP_LOADSYM

  case OP_LOADNIL:
    return code->emit_loadnil(mrb, irep, ppc);

  case OP_LOADT:
    return code->emit_loadt(mrb, irep, ppc);

  case OP_LOADF:
    return code->emit_loadf(mrb, irep, ppc);


  case OP_ADDI:
    return code->emit_addi(mrb, irep, ppc, regs);

  case OP_EQ:
    return code->emit_eq(mrb, irep, ppc, regs);

  case OP_LT:
    return code->emit_lt(mrb, irep, ppc, regs);

  case OP_LE:
    return code->emit_le(mrb, irep, ppc, regs);

  case OP_GT:
    return code->emit_gt(mrb, irep, ppc, regs);

  case OP_GE:
    return code->emit_ge(mrb, irep, ppc, regs);

  case OP_JMPIF:
    return code->emit_jmpif(mrb, irep, ppc, regs);

  case OP_JMPNOT:
    return code->emit_jmpnot(mrb, irep, ppc, regs);

  default:
    return NULL;
  }
}

#endif // ENABLE_JIT

} /* extern "C" */
