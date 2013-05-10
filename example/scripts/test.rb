class MyRubyClass
  def initialize
    puts "Initalizing ruby class"
    @my_class = ::Boots::MyClass.new "Name set from ruby"
    Boots::MyClass.print_array [ "Value1", "Value2", "Value3" ]

    puts "Operator overload test: #{@my_class.[](5)} (#{@my_class.get_name.size})"

    Boots::MyClass::Some.thing
  end

  def run my_class = nil
    my_class ||= @my_class
    puts "[Ruby] #{my_class.get_name}"
  end
end
