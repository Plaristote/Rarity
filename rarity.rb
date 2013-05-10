require 'yaml'
require 'erb'
require 'optparse'

class String
  def underscore
    self.gsub(/::/, '/').
    gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
    gsub(/([a-z\d])([A-Z])/,'\1_\2').
    tr("-", "_").
    downcase
  end
end

class CppHelpers
class << self
  def _return name, type
    "(CppToRuby(#{name}))"
  end
end
end

options = {
    output: 'rarity-bindings.cpp',
    input:  '.',
    mod:    nil
  }

OptionParser.new do |opts|
  opts.banner = "usage: #{ARGV[0]} [options]"
  opts.on '-o', '--output PATH', 'Set an output file'            do |v| options[:output] = v end
  opts.on '-i', '--input  PATH', 'Set input directory'           do |v| options[:input]  = v end
  opts.on '-m', '--module NAME', 'Wrap the bindings in a module' do |v| options[:mod]    = v end
end.parse!

bindings      = Dir.glob "#{options[:input]}/**/bindings-*.yml"

yaml_blob     = String.new
classes       = nil

bindings.each do |path|
  yaml_source = File.read path
  yaml_blob  += "\n#{yaml_source}"
end
classes       = YAML.load yaml_blob
classes     ||= []
includes      = []

supported_types = [ 'bool', 'float', 'int', 'unsigned int', 'long', 'double' ]

##
## TODO Implement Template support
##
classes.each do |classname, klass|
  next if klass['template'].nil?
end

classes.each do |classname, klass|
  includes << klass['include']
  operator_overload_count      = 0
  klass['alias'] = classname if klass['alias'].nil?
  parts = klass['alias'].split '::'
  klass['belongs_to'] = parts[0...parts.size - 1].join '::' unless parts.size == 1
  klass['alias']      = parts.last
  klass['binding-symbol']      = (classname.gsub '::', '__').gsub /[<>,]/n, '___'
  klass['methods'].each do |name, method|
    method['name']             = method['alias']
    method['name']           ||= name
    name.scan /^operator(<<|>>|--|\+\+|==|!=|<=|>=|\[\]|[+\-%*<>\/])/ do
      offset                   = $~.offset 1
      op_name                  = name[offset[0]...offset[1]]
      method['name']           = "operator_overload_#{operator_overload_count}" if method['alias'].nil?
      method['ruby_name']    ||= op_name
      operator_overload_count += 1
    end
    method['ruby_name']      ||= name.underscore
    puts "Generating bindings for #{classname}::#{name}. C-Function will be #{method['name']}. Ruby name will be #{method['ruby_name']}"
    method['binding_params']   = String.new
    method['params_apply']     = ''
    method['params_check']     = ''
    unless method['params'].nil?
      method['params'].count.times do |it|
        method['binding_params'] += ", VALUE param_#{it}"
      end

      method['params'].each_with_index do |param, index|
        method['params_apply'] += ', ' if method['params_apply'] != ''
        if param == 'const char*'
          method['params_apply'] += "(Ruby::ToCppType<std::string>(param_#{index})).c_str()"
        else
          method['params_apply'] += "(Ruby::ToCppType<#{param}>(param_#{index}))"
        end
        expected_ruby_type = param
        expected_ruby_type = param[0...param.size - 1] if param =~ /\*$/
        expected_ruby_type = 'String' if param == 'const char*' or param == 'std::string' or param == 'string'
        expected_ruby_type = 'Fixnum' if [ 'int', 'unsigned int', 'short', 'unsigned short' ].include? param
        expected_ruby_type = 'Float'  if [ 'double', 'float' ].include? param
        expected_ruby_type = 'Bignum' if param == 'long'
        expected_ruby_type = 'Array'  if param =~ /vector/
        expected_ruby_type = 'Proc'   if param =~ /function/
        if param == 'bool'
          method['params_check'] = "{ if (param_#{index} != Qtrue && param_#{index} != Qfalse) { rb_raise((Ruby::Constant(\"ArgumentError\").GetRubyInstance()), \"Mismatched type in #{classname}::#{name}, argument ##{index + 1}. Expecting Boolean.\"); } }"
        else
          method['params_check'] += "{ Ruby::Object tmp(param_#{index}); const std::string tmp_typename = (Ruby::ToCppType<std::string>(tmp.Apply(\"class\").Apply(\"name\")));
  const std::string exc_message = \"Mismatched type in #{classname}::#{name}, argument ##{index + 1}. Excepting #{expected_ruby_type}, got \" + tmp_typename;

  if (tmp_typename != \"#{expected_ruby_type}\")
    rb_raise((Ruby::Constant(\"ArgumentError\").GetRubyInstance()), exc_message.c_str());
}\n"
        end
      end
    end
  end unless klass['methods'].nil?
end

includes     = includes.uniq
template     = ((File.expand_path __FILE__ + "/..") + '/cpp-api-template.cpp')
cpp_template = ERB.new (File.read template)

File.open options[:output], 'w' do |f|
  cpp_source  = cpp_template.result binding
  f.write cpp_source
end
