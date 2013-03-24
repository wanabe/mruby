MRuby::Gem::Specification.new('mruby-jit') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby-jit developers'
end

MRuby.each_target do |target|
  patch "include/mruby.h" do
    line_after '#include "mruby/value.h"', <<-EOP

static inline mrb_value
mrb_cache_value(void *p)
{
  mrb_value v;

  MRB_SET_VALUE(v, MRB_TT_CACHE_VALUE, value.p, p);

  return v;
}
    EOP
    line_before '} mrb_callinfo;', "  void *jit_entry;\n"
    line_before 'typedef struct mrb_state {', <<-EOP
typedef void * mrbjit_code_area;
typedef struct mrbjit_comp_info {
  mrb_code *prev_pc;
  mrbjit_code_area code_base;
  int disable_jit;
  int nest_level;
} mrbjit_comp_info;

    EOP
    line_before '} mrb_state;',<<-EOP
  mrb_int is_method_cache_used;
  mrbjit_comp_info compile_info; /* JIT stuff */
    EOP
  end

  patch "include/mruby/irep.h" do
    line_after "#ifndef MRUBY_IREP_H", <<-EOP

#include "jit.h"
#include <setjmp.h>
    EOP
    line_before '} mrb_irep;', <<-EOP

  mrb_int is_method_cache_used;

  /* JIT stuff */
  int *prof_info;
  mrbjit_codetab *jit_entry_tab;
    EOP
    line_after '} mrb_irep;', <<-EOP

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

  patch "include/mruby/value.h" do
    2.times do
      line_before /(\s*)(MRB_TT_MAXDEFINE[^0-9]+)([0-9]+)/, nil, 1 do |d, m|
        c = m[3].to_i
        <<-EOP
  MRB_TT_CACHE_VALUE, /*  #{c} */
  MRB_TT_MAXDEFINE    /*  #{c+1} */
        EOP
      end
      next_line
    end
  end

  patch "include/mruby/variable.h" do
    line_before /^mrb_value mrb_iv_get\(/, "int mrbjit_iv_off(mrb_state *mrb, mrb_value obj, mrb_sym sym);\n"
  end

  patch "src/class.c" do
    search /^mrb_define_method/
    line_after "}", <<-EOP

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
    search /^mrb_define_method_vm/
    line_after "", <<-EOP
  if (mrb->is_method_cache_used) {
    clear_method_cache(mrb);
    mrb->is_method_cache_used = 0;
  }

    EOP
  end

  patch "src/codegen.c" do
    search /^new_lit\(/
    line_after '}', <<-EOP

static inline int
new_lit2(codegen_scope *s, mrb_value val)
{
  if (s->irep->plen == s->pcapa) {
    s->pcapa *= 2;
    s->irep->pool = (mrb_value *)codegen_realloc(s, s->irep->pool, sizeof(mrb_value)*s->pcapa);
  }
  s->irep->pool[s->irep->plen] = val;
  return s->irep->plen++;
}
    EOP
    search /^new_sym\(/
    line_after '}', <<-EOP

static void
genop_send(codegen_scope *s, mrb_code i)
{
  int off;

  genop(s, i);
  off = new_lit2(s, mrb_cache_value(0));
  new_lit2(s, mrb_fixnum_value(1));
  genop(s, MKOP_Bx(OP_NOP, off));
}

static void
genop_send_peep(codegen_scope *s, mrb_code i, int val)
{
  int off;

  genop_peep(s, i, val);
  off = new_lit2(s, mrb_fixnum_value(1));
  new_lit2(s, mrb_fixnum_value(1));
  genop(s, MKOP_Bx(OP_NOP, off));
}
    EOP
    ops = "SEND|SENDB|ADD|SUB|MUL|DIV|LT|LE|GT|GE|EQ|AREF|APOST|RANGE|ARRAY"
    ops << "|HASH"
    each_line /genop(_peep)?\(s, MKOP_ABC\(OP_(#{ops}),/ do |m|
      "genop_send#{m[1]}(s, MKOP_ABC(OP_#{m[2]},"
    end
    search /^scope_finish/
    line_after '{', "int i;\n"
    line_before /^\s*irep->pool = /, <<-EOP
  irep->jit_entry_tab = (mrbjit_codetab *)mrb_malloc(mrb, sizeof(mrbjit_codetab)*s->pc);
  for (i = 0; i < s->pc; i++) {
    irep->jit_entry_tab[i].size = 2;
    irep->jit_entry_tab[i].body = 
      (mrbjit_code_info *)mrb_calloc(mrb, 1, sizeof(mrbjit_code_info)*2);
  }
  irep->prof_info = (int *)mrb_calloc(mrb, 1, sizeof(int)*s->pc);
    EOP
  end

  patch "src/dump.c" do
    line_after /size \+= get_irep_header_size/, "  mrb_gc_arena_restore(mrb, 0);\n"
    line_after /size \+= get_iseq_block_size/, "  mrb_gc_arena_restore(mrb, 0);\n"
    line_after /size \+= get_pool_block_size/, "  mrb_gc_arena_restore(mrb, 0);\n"
    line_after /size \+= get_syms_block_size/, "  mrb_gc_arena_restore(mrb, 0);\n"
    line_after /buf \+= uint32_dump\(\(uint32_t\)irep->iseq/, "    mrb_gc_arena_restore(mrb, 0);\n"
    search /^write_irep_record/
    search /^\s*case DUMP_IREP_HEADER:/
    line_after /^\s*}$/, "    mrb_gc_arena_restore(mrb, 0);\n"
    [/^mrb_write_irep/, /^mrb_dump_irep/].each do |reg|
      search reg
      m = search /^(\s*)for \(irep_no=top;/
      line_before /^#{m[1]}}$/, "#{m[1]}  mrb_gc_arena_restore(mrb, 0);\n"
    end
  end

  patch "src/gc.c" do
    search /^obj_free/
    line_after "  case MRB_TT_FLOAT:", "  case MRB_TT_CACHE_VALUE:\n"
  end

  patch "src/init.c" do
    line_before "void mrb_init_mrblib(mrb_state*);", "void mrb_init_irep(mrb_state*);\n"
    search /^mrb_init_core/
    line_before "}", <<-EOP
#ifdef ENABLE_IREP
  mrb_init_irep(mrb);
#endif
    EOP
  end

  patch "src/load.c" do
    search /^read_rite_irep_record/
    line_before "  *len = src - recordStart;", <<-EOP
  // JIT Block
  irep->jit_entry_tab = (mrbjit_codetab *)mrb_calloc(mrb, 1, sizeof(mrbjit_codetab)*irep->ilen);
  for (i = 0; i < irep->ilen; i++) {
    irep->jit_entry_tab[i].size = 2;
    irep->jit_entry_tab[i].body = (mrbjit_code_info *)mrb_calloc(mrb, 2, sizeof(mrbjit_code_info));
  }
  irep->prof_info = (int *)mrb_calloc(mrb, 1, sizeof(int)*irep->ilen);

    EOP
  end
end
