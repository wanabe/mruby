#include "mruby.h"
#include "mruby/irep.h"

#ifdef ENABLE_JIT
void
mrbjit_dispatch(mrb_state *mrb, mrb_irep *irep, mrb_code **ppc, mrb_value *regs)
{
}
#endif
