#!/usr/bin/ruby

`ls *.png`.split("\n").each{|i|  
  `convert #{i} -scale 400% 32x32/#{i}`
}
puts "done"
