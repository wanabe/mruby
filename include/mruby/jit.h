/*
** mruby/jit.h - JIT structure
**
** See Copyright Notice in mruby.h
*/

#ifndef MRUBY_JIT_H
#define MRUBY_JIT_H

typedef struct mrbjit_codeele {
  void *(*entry)();
  mrb_code *prev_pc;
} mrbjit_codeele;

typedef struct mrbjit_codetab {
  int size;
  mrbjit_codeele *element;
} mrbjit_codetab;

typedef struct mrbjit_varinfo {
  int reg_no;
  enum {
    REGSTOR,
    MEMORY,
    STACK_FRMAME
  } where;
  union {
    void *ptr;
    int no;
  } addr;
} mrbjit_varinfo;

typedef struct mrbjit_comp_info {
  mrb_code *prev_pc;
  mrbjit_varinfo varinfo;
} mrbjit_comp_info;

#endif  /* MRUBY_JIT_H */
