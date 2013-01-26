#include "jitcode.h"

extern "C" {

#ifdef ENABLE_JIT

const void *
mrbjit_emit_entry(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  return code->emit_entry(mrb, irep);
}

void
mrbjit_emit_exit(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  code->emit_exit(*ppc);
}

const void *
mrbjit_emit_code(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
{
  MRBJitCode *code = (MRBJitCode *) irep->compile_info->code_base;
  const void *entry;

  if (code == NULL) {
    code = new MRBJitCode();
    irep->compile_info->code_base = code;
    entry = code->emit_entry(mrb, irep);
  }

  switch(GET_OPCODE(**ppc)) {
  case OP_MOVE:
    return code->emit_mov(mrb, irep, ppc);

  case OP_LOADL:
    return code->emit_loadl(mrb, irep, ppc);

  default:
    return NULL;
  }
}

#endif // ENABLE_JIT

} /* extern "C" */
