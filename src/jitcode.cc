#include "jitcode.h"

extern "C" {
#include "mruby.h"
#include "mruby/jit.h"

#ifdef ENABLE_JIT

mrbjit_code_area
mrbjit_alloc_code()
{
  return (mrbjit_code_area)(new MRBJitCode());
}

const void *
mrbjit_emit_entry(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  return code->emit_entry(irep);
}

const void *
mrbjit_emit_code(mrbjit_code_area coderaw, mrb_state *mrb, mrb_irep *irep, mrb_code **ppc)
{
  MRBJitCode *code = (MRBJitCode *) coderaw;
  return code->emit_entry(irep);
}

#endif // ENABLE_JIT

} /* extern "C" */
