/*
** mruby/jit.h - JIT structure
**
** See Copyright Notice in mruby.h
*/

#ifndef MRUBY_JIT_H
#define MRUBY_JIT_H

#define COMPILE_THRESHOLD 1000

typedef struct mrbjit_varinfo {
  /* SRC */
  int reg_no;
  int up;

  /* DIST */
  enum {
    REG,
    MEMORY,
    STACK_FRMAME
  } where;
  union {
    void *ptr;
    int no;
    int offset;
  } addr;
} mrbjit_varinfo;

typedef struct mrbjit_branchinfo {
  /* You can judge */
  /* prev_base == current not branch */
  /* prev_base != current branched   */
  mrbjit_code_area *current_base;	/* code area of cureent */
  mrbjit_code_area *branch_base;	/* code area of branched */
} mrbjit_branchinfo;


typedef union mrbjit_inst_spec_info {
  mrbjit_branchinfo brainfo;	/* For Conditional Branch */
} mrbjit_inst_spec_info;

typedef struct mrbjit_code_info {
  mrbjit_code_area code_base;
  mrb_code *prev_pc;
  void *(*entry)();
  mrbjit_varinfo dstinfo;	/* For Local assignment */
  mrbjit_inst_spec_info inst_spec;
  int used;
} mrbjit_code_info;

typedef struct mrbjit_codetab {
  int size;
  mrbjit_code_info *body;
} mrbjit_codetab;

mrb_value mrb_uvget(mrb_state *, int, int);
void mrb_uvset(mrb_state *, int, int, mrb_value);
mrb_callinfo* mrbjit_cipush(mrb_state *);
void mrbjit_cipop(mrb_state *);
void mrbjit_stack_extend(mrb_state *, int, int);
void mrbjit_argnum_error(mrb_state *, int);

#endif  /* MRUBY_JIT_H */
