# dercz9000
My first Arduino project, a DIY Arduino UNO video console with a game which is neither space invaders nor asteroids!

![a screenshot](/screenshots/s0.jpg?raw=true "screenshot 1")
![another screenshot](/screenshots/s1.jpg?raw=true "screenshot 2")

The whole game is in single file, requires 2KB of RAM, and ~18.5KB of flash.
Some things still need to be done, in particular more/better levels.
More details in the source code.

Special thanks to Myles Metzer for his impressive TVout library (http://playground.arduino.cc/Main/TVout) and to Panicz Godek for introducing me to ANSI C bitfields.

![dercz9000 console proud prototype](/screenshots/d9k-prototype.jpg?raw=true "The Prototype")

just plug 4 switches (with 10K resistors) to pins 3,4,5 and 6,
plug tvout (http://playground.arduino.cc/Main/TVout), and it should run.

See the thing in action here https://youtu.be/UpDY4lFhlhA

# USE AT YOUR OWN RISK!
If it fries your TVset, your arduino, or both, that's neither my responsibility, nor my business.
