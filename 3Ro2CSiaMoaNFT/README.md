I'm trying to port my Ludum Dare 35 entry to dercz9000 console.
(http://ludumdare.com/compo/ludum-dare-35/?action=preview&uid=88774).

I have reconstructed the game's mechanics in C, it compiles to timer+curses version

gcc curses-test.c -o test -lncurses
./test

I have created sprites in gimp, tested their look in js version (cf js-gfx-test/),
and converted to C table with some python/ruby scripts (cf tools).

However the current C version uses way too much memory:

$ avr-size torus.cpp.elf 
   text	  data  bss  dec    hex	  filename
  16314	  1276  238  17828  45a4  torus.cpp.elf

-- if I'm computing the shit correctly we shoud have data+bss < 512B
(actually it should be <450B or so as we need some place for a call stack?).

TBC
