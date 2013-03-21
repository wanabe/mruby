MRuby::Gem::Specification.new('mruby-jit') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby-jit developers'

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

  patch "include/mruby/irep.h"
  patch "src/vm.c"
end
