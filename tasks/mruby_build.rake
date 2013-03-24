load "#{MRUBY_ROOT}/tasks/mruby_build_gem.rake"
load "#{MRUBY_ROOT}/tasks/mruby_build_commands.rake"

module MRuby
  class << self
    def targets
      @targets ||= {}
    end

    def each_target(&block)
      @targets.each do |key, target|
        target.instance_eval(&block)
      end
    end
  end

  class Toolchain
    class << self
      attr_accessor :toolchains
    end

    def initialize(name, &block)
      @name, @initializer = name.to_s, block
      MRuby::Toolchain.toolchains ||= {}
      MRuby::Toolchain.toolchains[@name] = self
    end

    def setup(conf)
      conf.instance_eval(&@initializer)
    end

    def self.load
      Dir.glob("#{MRUBY_ROOT}/tasks/toolchains/*.rake").each do |file|
        Kernel.load file
      end
    end
  end
  Toolchain.load

  class Patch
    @@table = {}
    def self.[](src, dst)
      if !@@table[dst]
        FileUtils.mkdir_p File.dirname(dst)
        FileUtils.cp src, dst
        @@table[dst] = new(dst)
      end
      @@table[dst]
    end
    def initialize(fname)
      @fname = fname
      @content = open(fname, "r") {|f| f.readlines}
      @content.each {|l| l.chomp!}
    end
    def apply(patch)
      @line = 0
      case patch
      when String
        instance_eval(open(patch, "r") {|f| f.read}, patch)
      when Proc
        instance_eval(&patch)
      end
      open(@fname, "w") {|f| f.puts @content}
    end

    def search(line)
      until line === @content[@line]
        @line += 1
        raise "can't find #{line}" unless @content[@line]
      end
      $~
    end

    def line_after(pattern, patch, delete = 0)
      search pattern
      patch = patch.split("\n")
      @content[@line + 1, delete] = patch
      @line += patch.length + 1
    end

    def line_before(pattern, patch, delete = 0)
      m = search pattern
      patch = yield(patch, m) if block_given?
      patch = patch.split("\n")
      @content[@line, delete] = patch
      @line += patch.length
      m
    end

    def each_line(pattern, &b)
      @content[@line..-1].each do |l|
        l.gsub!(pattern) {b.call($~)}
      end
    end

    def next_line
      @line += 1
    end
  end # Patch

  class Build
    class << self
      attr_accessor :current
    end
    include Rake::DSL
    include LoadGems
    attr_accessor :name, :bins, :exts, :file_separator
    attr_reader :libmruby, :libmruby_core, :gems, :patchs

    COMPILERS = %w(cc cxx objc asm)
    COMMANDS = COMPILERS + %w(linker archiver yacc gperf git exts mrbc)
    attr_block MRuby::Build::COMMANDS

    Exts = Struct.new(:object, :executable, :library)

    def initialize(name='host', &block)
      @name = name.to_s

      unless MRuby.targets[@name]
        if ENV['OS'] == 'Windows_NT'
          @exts = Exts.new('.o', '.exe', '.a')
        else
          @exts = Exts.new('.o', '', '.a')
        end

        @file_separator = '/'
        @cc = Command::Compiler.new(self, %w(.c))
        @cxx = Command::Compiler.new(self, %w(.cc .cxx .cpp))
        @objc = Command::Compiler.new(self, %w(.m))
        @asm = Command::Compiler.new(self, %w(.S .asm))
        @linker = Command::Linker.new(self)
        @archiver = Command::Archiver.new(self)
        @yacc = Command::Yacc.new(self)
        @gperf = Command::Gperf.new(self)
        @git = Command::Git.new(self)
        @mrbc = Command::Mrbc.new(self)

        @bins = %w(mruby mrbc mirb)
        @gems, @libmruby, @libmruby_core, @patchs = [], [], [], []
        @build_mrbtest_lib_only = false

        MRuby.targets[@name] = self
      end

      MRuby::Build.current = MRuby.targets[@name]
      MRuby.targets[@name].instance_eval(&block)
    end

    def toolchain(name)
      tc = Toolchain.toolchains[name.to_s]
      fail "Unknown #{name} toolchain" unless tc
      tc.setup(self)
    end

    def root
      MRUBY_ROOT
    end

    def build_dir
      "#{MRUBY_ROOT}/build/#{self.name}"
    end

    def mrbcfile
      MRuby.targets['host'].exefile("#{MRuby.targets['host'].build_dir}/bin/mrbc")
    end

    def compilers
      COMPILERS.map do |c|
        instance_variable_get("@#{c}")
      end
    end

    def define_rules
      compilers.each do |compiler|
        if respond_to?(:enable_gems?) && enable_gems?
          compiler.defines -= %w(DISABLE_GEMS) 
        else
          compiler.defines += %w(DISABLE_GEMS) 
        end
        compiler.define_rules build_dir, File.expand_path(File.join(File.dirname(__FILE__), '..'))
      end
    end

    def filename(name)
      if name.is_a?(Array)
        name.flatten.map { |n| filename(n) }
      else
        '"%s"' % name.gsub('/', file_separator)
      end
    end

    def cygwin_filename(name)
      if name.is_a?(Array)
        name.flatten.map { |n| cygwin_filename(n) }
      else
        '"%s"' % `cygpath -w "#{filename(name)}"`.strip
      end
    end

    def exefile(name)
      if name.is_a?(Array)
        name.flatten.map { |n| exefile(n) }
      else
        "#{name}#{exts.executable}"
      end
    end

    def objfile(name)
      if name.is_a?(Array)
        name.flatten.map { |n| objfile(n) }
      else
        "#{name}#{exts.object}"
      end
    end

    def libfile(name)
      if name.is_a?(Array)
        name.flatten.map { |n| libfile(n) }
      else
        "#{name}#{exts.library}"
      end
    end

    def build_mrbtest_lib_only
      @build_mrbtest_lib_only = true
    end

    def build_mrbtest_lib_only?
      @build_mrbtest_lib_only
    end

    def run_test
      puts ">>> Test #{name} <<<"
      mrbtest = exefile("#{build_dir}/test/mrbtest")
      sh "#{filename mrbtest.relative_path}#{$verbose ? ' -v' : ''}"
      puts 
    end

    def print_build_summary
      puts "================================================"
      puts "      Config Name: #{@name}"
      puts " Output Directory: #{self.build_dir.relative_path}"
      puts "         Binaries: #{@bins.join(', ')}" unless @bins.empty?
      unless @gems.empty?
        puts "    Included Gems:"
        @gems.map do |gem|
          puts "             #{gem.name}"
          puts "               - Binaries: #{gem.bins.join(', ')}" unless gem.bins.empty?
        end
      end
      puts "================================================"
      puts
    end

    def patch(file, patch = nil, &b)
      src = "#{root}/#{file}"
      dst = "#{build_dir}/#{file}"
      obj = objfile(dst.sub(/\.cc?$/, ""))
      dir = File.dirname(src)
      src = [src, patch] if patch
      patch = b if b
      task :patch => dst
      patchs << dst
      file dst => src do |t|
        Patch[t.prerequisites.first, t.name].apply(patch)
      end
      return unless obj
      file obj => dst do |t|
        cc.run t.name, t.prerequisites.first, [], [dir]
      end
    end
  end # Build

  class CrossBuild < Build
    def run_test
      mrbtest = exefile("#{build_dir}/test/mrbtest")
      puts "You should run #{mrbtest} on target device."
      puts 
    end
  end # CrossBuild
end # MRuby
