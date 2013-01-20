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
#include "mruby/jit.h"
} /* extern "C" */

class MRBJitCode: public Xtaak::CodeGenerator {

 public:

 MRBJitCode():
  CodeGenerator(1024)
  {
  }

  const void *emit_entry(mrb_irep *irep) {
    const void* func_ptr = getCurr();
    try {
      /* prologue */
    }
    catch (Xtaak::Error err) {
      puts(ConvertErrorToString(err));
      printf("Foo");
      exit(0);
    }

    return func_ptr;
  }
  
  const void *emit_mov() {
    const void *code = getCurr();

    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
