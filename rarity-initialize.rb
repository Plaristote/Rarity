##################################################################
## File Name:		rarity-initialize.rb			##
## Author:		Michael Martin Moro			##
## Comment:							##
##                                                              ##
## Creation date:	Mon May 13 11:45:45 2013		##
## Last modified:	Mon May 13 11:51:13 2013		##
##################################################################

require 'yaml'
require 'erb'
require 'optparse'

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

@classes  = classes
@includes = includes
@options  = options

