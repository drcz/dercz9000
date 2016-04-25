#!/usr/bin/ruby
files = `ls *.png`.split("\n");
names = [];
puts "PROGMEM const byte sprite[N_O_SPRITES][SPRITE_H+2] = {";
(0...files.count).each{|i|
  f = files[i]
  name = "S_"+f.split(".png")[0].upcase.gsub("-","_")
  names << name
  puts "/// #{name}"
  puts `./digitalize.py #{f}`
  puts "," if(i<(files.count-1))
}
puts "};";
puts "/// names:";
puts "enum {";
names.each{|name|
  puts "  #{name},"
}
puts "  N_O_SPRITES"
puts "};"
