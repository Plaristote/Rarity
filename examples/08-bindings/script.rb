class MyRubyClass
  include Cpp

  def initialize
    @personA = Person.new "Roger"
    @personB = Person.new "Patrice"
    @personC = Person.new "Pierre"
    @personD = Manager.new "Paul"
    puts "MyRubyClass initialized"
  end

  def run on_done = nil
    puts "Running MyRubyClass"
    display_roles
    assign_persons
    display_persons
    if on_done.nil?
      puts "no on done callback"
    else
      on_done.call "Coucou petite perruche !"
    end
  end

  def display_roles
    puts "Roles:"
    [@personA, @personB, @personC, @personD].each do |person|
      puts "#{person.get_name} has role #{person.get_role}"
    end
    puts ""
  end

  def assign_persons
    @personD.add_person @personA
    @personD.add_person @personC
  end

  def display_persons
    puts "PersonD persons:"
    @personD.get_persons.each do |person|
      puts "- #{person.get_name}"
    end
  end
end

puts "Loaded script.rb"
