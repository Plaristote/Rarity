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
    map = {
      'std::string'   => "(CppToRuby::String(#{name}))",
      'string'        => "(CppToRuby::String(#{name}))",
      'int'           => "(CppToRuby::Int(#{name}))",
      'unsigned int'  => "(CppToRuby::UnsignedInt(#{name}))",
      'bool'          => "(CppToRuby::Bool(#{name}))",
      'float'         => "(CppToRuby::Float(#{name}))",
      'std::string*'  => "(CppToRuby::String(*#{name}))",
      'string*'       => "(CppToRuby::String(*#{name}))",
      'int*'          => "(CppToRuby::Int(*#{name}))",
      'unsigned int*' => "(CppToRuby::UnsignedInt(*#{name}))",
      'bool*'         => "(CppToRuby::Bool(*#{name}))",
      'float*'        => "(CppToRuby::Float(*#{name}))",
    }
    if map[type].nil?
      if type.include? '*'
        '*' + name
      else
        name
      end
    else
      map[type]
    end
  end
end
end

options = {
    output: 'rarity-bindings.cpp',
    input:  '.'
  }

OptionParser.new do |opts|
  opts.banner = "usage: #{ARGV[0]} [options]"
  opts.on '-o', '--output PATH', 'Set an output file'     do |v| options[:output] = v end
  opts.on '-i', '--input  PATH',  'Set input directory' do |v| options[:input]  = v end
end.parse!

bindings      = Dir.glob "#{options[:input]}/**/bindings-*.yml"

yaml_blob     = String.new
classes       = nil

bindings.each do |path|
  yaml_source = File.read path
  yaml_blob  += "\n#{yaml_source}"
end
classes       = YAML.load yaml_blob
includes      = []

supported_types = [ 'bool', 'float', 'int', 'unsigned int', 'long', 'double' ]

classes.each do |classname, klass|
  includes << klass['include']
  klass['methods'].each do |name, method|
    method['binding_params']   = String.new
    method['params_apply']     = ''
    unless method['params'].nil?
      method['params'].count.times do |it|
        method['binding_params'] += ", VALUE param_#{it}"
      end

      method['params'].each_with_index do |param, index|
        method['params_apply'] += ', ' if method['params_apply'] != ''
        if param == 'std::string' or param == 'string'
          method['params_apply'] += "(RubyToCpp::String(param_#{index}))"
        elsif param == 'unsigned int'
          method['params_apply'] += "(RubyToCpp::UnsignedInt(param_#{index}))"
        elsif supported_types.include? param
          method['params_apply'] += "(RubyToCpp::#{param.capitalize}(param_#{index}))"
        else
          method['params_apply'] += "(api_param<#{param}>(param_#{index}, \"#{param}\"))"
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
