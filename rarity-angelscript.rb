$: << (File.expand_path '..', __FILE__)
require "rarity-initialize"

def solve_type classes, type
  striped_type = type.gsub /const\s+/, ''
  striped_type = striped_type.gsub /&/, ''
  striped_type = striped_type.gsub '*', ''

  supported_types = [ 'int', 'float', 'double', 'bool', 'string', 'void' ]
  if supported_types.include? striped_type
    striped_type
  elsif striped_type == 'std::string'
    'string'
  else
    res = nil
    classes.each do |name, klass|
      res = klass['alias'] + '@' if name == striped_type
    end
    res
  end
end

options         = @options
includes        = @includes
classes         = @classes

classes.each do |classname, klass|
  includes << klass['include']
  klass['alias']          = classname if klass['alias'].nil?
  parts                   = klass['alias'].split '::'
  klass['belongs_to']     = parts[0...parts.size - 1].join '::' unless parts.size == 1
  klass['alias']          = parts.join '_'
  klass['binding-symbol'] = (classname.gsub '::', '__').gsub /[<>,]/n, '___'
end

classes.each do |classname, klass|
  klass['methods'].delete_if do |name, method|
    next true if name == 'initialize'
    can_define          = true
    method['alias']   ||= name
    method['alias'].scan /^operator(<<|>>|--|\+\+|==|!=|<=|>=|\[\]|[+\-%*<>\/])/ do
      offset                   = $~.offset 1
      op_name                  = name[offset[0]...offset[1]]
      method['alias']          = case op_name
        when '[]'
          'opIndex'
        when '='
          'opAssign'
        when '=='
          'opEquals'
        when '<<'
          'opShl'
        when '>>'
          'opShr'
        when '++'
          'opPostInc'
        when '--'
          'opPostDec'
        else
          can_define = false
      end
    end
    return_type         = solve_type classes, method['return']
    can_define          = can_define && (not return_type.nil?)
    method['decl']      = unless method['static'] == true
      "#{return_type} #{method['alias']}("
    else
      "#{return_type} #{klass['alias']}_#{method['alias']}("
    end
    unless method['params'].nil?
      first_param       = true
      method['params'].each do |param|
        method['decl'] += ', ' unless first_param
        param_type      = solve_type classes, param
        can_define      = can_define && (not param_type.nil?)
        method['decl'] += param_type unless param_type.nil?
        first_param     = false
      end
    end
    method['decl']     += ')'
    method['decl']     += ' const' if method['const'] == true
    puts "Cannot port method #{classname}::#{name} to AngelScript" unless can_define
    not can_define
  end
end

includes     = includes.uniq
template     = ((File.expand_path __FILE__ + '/..') + '/angelscript/scriptengine.cpp')
cpp_template = ERB.new (File.read template)

File.open options[:output], 'w' do |f|
  cpp_source = cpp_template.result binding
  f.write cpp_source
end
