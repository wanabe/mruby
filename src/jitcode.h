/*
** mruby/jitcode.h - Class for XTAAK
**
** See Copyright Notice in mruby.h
*/

#ifndef MRUBY_JITCOD_H
#define MRUBY_JITCODE_H

#include <xtaak/xtaak.h>

class MRBJitCode: public Xtaak::CodeGenerator {

 public:
  const void *emit_mov() {
    const void *code = getCurr();

    return code;
  }
};

#endif  /* MRUBY_JITCODE_H */
