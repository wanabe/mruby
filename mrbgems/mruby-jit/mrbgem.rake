MRuby::Gem::Specification.new('mruby-jit') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby-jit developers'
  patch "include/mruby/irep.h"
  patch "src/vm.c"
end
