MRuby::Gem::Specification.new('mruby-jit') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby-jit developers'
end

MRuby.each_target do |target|
  patch "include/mruby.h" do |f|
    line_after f, '#include "mruby/value.h"', <<-EOP

static inline mrb_value
mrb_cache_value(void *p)
{
  mrb_value v;

  MRB_SET_VALUE(v, MRB_TT_CACHE_VALUE, value.p, p);

  return v;
}
    EOP
    line_before f, '} mrb_callinfo;', "  void *jit_entry;\n"
    line_before f, 'typedef struct mrb_state {', <<-EOP
typedef void * mrbjit_code_area;
typedef struct mrbjit_comp_info {
  mrb_code *prev_pc;
  mrbjit_code_area code_base;
  int disable_jit;
  int nest_level;
} mrbjit_comp_info;

    EOP
    line_before f, '} mrb_state;',<<-EOP
  mrb_int is_method_cache_used;
  mrbjit_comp_info compile_info; /* JIT stuff */
    EOP
  end

  patch "include/mruby/irep.h" do |f|
    line_after f, "#ifndef MRUBY_IREP_H", <<-EOP

#include "jit.h"
#include <setjmp.h>
    EOP
    line_before f, '} mrb_irep;', <<-EOP

  mrb_int is_method_cache_used;

  /* JIT stuff */
  int *prof_info;
  mrbjit_codetab *jit_entry_tab;
    EOP
    line_after f, '} mrb_irep;', <<-EOP

typedef struct mrbjit_vmstatus {
  mrb_irep **irep;
  struct RProc **proc;
  mrb_code **pc;
  mrb_value **pool;
  mrb_sym **syms;
  mrb_value **regs;
  int *ai;
  void **optable;
  void **gototable;
  jmp_buf **prev_jmp;
} mrbjit_vmstatus;
    EOP
  end

  patch "include/mruby/value.h" do |f|
    2.times do
      line_before f, /(\s*)(MRB_TT_MAXDEFINE[^0-9]+)([0-9]+)/, nil, 1 do |d, m|
        c = m[3].to_i
        <<-EOP
  MRB_TT_CACHE_VALUE, /*  #{c} */
  MRB_TT_MAXDEFINE    /*  #{c+1} */
        EOP
      end
      f.gets
    end
  end

  patch "include/mruby/variable.h" do |f|
    line_before f, /^mrb_value mrb_iv_get\(/, "int mrbjit_iv_off(mrb_state *mrb, mrb_value obj, mrb_sym sym);\n"
  end

  patch "src/class.c" do |f|
    search f, /^mrb_define_method/
    line_after f, "}", <<-EOP

static void
clear_method_cache(mrb_state *mrb)
{
  int i;
  int j;
  int ilen;
  int plen;
  mrb_irep *irep;
  mrb_value *pool;
  
  ilen = mrb->irep_len;
  for (i = 0; i < ilen; i++) {
    irep = mrb->irep[i];
    if (irep->is_method_cache_used) {
      plen = irep->plen;
      pool = irep->pool;
      for (j = 0; j < plen; j++) {
	if (mrb_tt(pool[j]) == MRB_TT_CACHE_VALUE) {
	  pool[j].value.p = 0;
	}
      }
      irep->is_method_cache_used = 0;
    }
  }
}
    EOP
    search f, /^mrb_define_method_vm/
    line_after f, "", <<-EOP
  if (mrb->is_method_cache_used) {
    clear_method_cache(mrb);
    mrb->is_method_cache_used = 0;
  }

    EOP
  end
end
