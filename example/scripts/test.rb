class MyClass
  attr_accessor :rarity_cpp_pointer
end

class MyRubyClass
  def initialize
    puts "Initalizing ruby class"
    @my_class = MyClass.new "Name set from ruby"
  end

  def run my_class = nil
    my_class ||= @my_class
    puts "[Ruby] #{my_class.get_name}"
  end
end
