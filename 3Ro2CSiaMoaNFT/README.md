I'm trying to port my Ludum Dare 35 entry to dercz9000 console.
(http://ludumdare.com/compo/ludum-dare-35/?action=preview&uid=88774).

I have reconstructed the game's mechanics in C, it compiles to curses version

gcc curses-test.c -o test -lncurses

./test

I have created sprites in gimp, tested their look in js version (cf js-gfx-test/),
and converted to C table with some python/ruby scripts (cf tools).

I have many problems with the program on arduino (this PROGMEM stuff is pretty tricky!),
but the basics now seem to work, look here: https://vine.co/v/iUl0FlKJLL2

(There's still a lot to be done, unfortunately I won't have access to my arduino for the next month...)

TBC
