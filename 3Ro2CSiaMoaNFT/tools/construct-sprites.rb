#!/usr/bin/ruby

img = 'nowsze-kafle8x8.png';
names = [
         "hero-triangle", "hero-square", "hero-disc",
         "hero-triangle2", "hero-square2", "hero-disc2",
         "triangle", "square", "disc",
         "triangle2", "square2", "disc2",
         "hole-triangle", "hole-square", "hole-disc",
         "wall", "gun", "particle-h",
         "pipe-h", "pipe-dl", "turncock-h",
         "key", "door", "machine",
         "boom", "shade", "heart"
        ];
puts "slicing #{img} into separate images..."
size=8;
ind=0;
(0..8).each{|row|
  (0..2).each{|col|
    `convert -extract #{size}x#{size}+#{size*col}+#{size*row} #{img} #{names[ind]}.png`;
    ind=ind+1
  }
}
puts "done."
puts "generating lacking ones";
`./gen-lacking.sh`;
puts "done."
