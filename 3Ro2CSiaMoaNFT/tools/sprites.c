PROGMEM const byte sprite[N_O_SPRITES][SPRITE_H+2] = {
/// S_BLANK
{ 8,8, 
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111
}
,
/// S_BOOM
{ 8,8, 
 0b11011101,
 0b11101111,
 0b10111011,
 0b11111111,
 0b11101101,
 0b11111011,
 0b11011110,
 0b11111111
}
,
/// S_DISC2
{ 8,8, 
 0b11010011,
 0b10101001,
 0b01111110,
 0b10101010,
 0b01111110,
 0b00101010,
 0b10111101,
 0b11000011
}
,
/// S_DISC
{ 8,8, 
 0b11000011,
 0b10111101,
 0b01010100,
 0b01111110,
 0b01010101,
 0b01111110,
 0b10010101,
 0b11001011
}
,
/// S_DOOR
{ 8,8, 
 0b11111111,
 0b11000011,
 0b10111101,
 0b10101101,
 0b10101101,
 0b10111101,
 0b10000001,
 0b11111111
}
,
/// S_GUN_DOWN
{ 8,8, 
 0b10000001,
 0b11011011,
 0b11000011,
 0b11100111,
 0b11100111,
 0b11111111,
 0b11100111,
 0b11111111
}
,
/// S_GUN_LEFT
{ 8,8, 
 0b11111111,
 0b11111110,
 0b11111000,
 0b10100010,
 0b10100010,
 0b11111000,
 0b11111110,
 0b11111111
}
,
/// S_GUN_RIGHT
{ 8,8, 
 0b11111111,
 0b01111111,
 0b00011111,
 0b01000101,
 0b01000101,
 0b00011111,
 0b01111111,
 0b11111111
}
,
/// S_GUN_UP
{ 8,8, 
 0b11111111,
 0b11100111,
 0b11111111,
 0b11100111,
 0b11100111,
 0b11000011,
 0b11011011,
 0b10000001
}
,
/// S_HEART
{ 8,8, 
 0b11111111,
 0b11011101,
 0b10101010,
 0b10000000,
 0b10000000,
 0b11000001,
 0b11101011,
 0b11110111
}
,
/// S_HERO_DISC2_DOWN2
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00001100,
 0b00011110,
 0b00010010,
 0b00010010,
 0b10101101,
 0b11000011
}
,
/// S_HERO_DISC2_DOWN
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00110000,
 0b01111000,
 0b01001000,
 0b01001000,
 0b10110101,
 0b11000011
}
,
/// S_HERO_DISC2_LEFT
{ 8,8, 
 0b11000011,
 0b10000001,
 0b01000000,
 0b00111000,
 0b01001100,
 0b01001100,
 0b10111001,
 0b11000011
}
,
/// S_HERO_DISC2_RIGHT
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00000010,
 0b00011100,
 0b00110010,
 0b00110010,
 0b10011101,
 0b11000011
}
,
/// S_HERO_DISC2_UP2
{ 8,8, 
 0b11000011,
 0b10110101,
 0b01001000,
 0b01001000,
 0b01111000,
 0b00110000,
 0b10000001,
 0b11000011
}
,
/// S_HERO_DISC2_UP
{ 8,8, 
 0b11000011,
 0b10101101,
 0b00010010,
 0b00010010,
 0b00011110,
 0b00001100,
 0b10000001,
 0b11000011
}
,
/// S_HERO_DISC_DOWN2
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00000100,
 0b00000110,
 0b00000010,
 0b00000010,
 0b10000101,
 0b11000011
}
,
/// S_HERO_DISC_DOWN
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00100000,
 0b01100000,
 0b01000000,
 0b01000000,
 0b10100001,
 0b11000011
}
,
/// S_HERO_DISC_LEFT
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00000000,
 0b00000000,
 0b00000000,
 0b01001100,
 0b10111001,
 0b11000011
}
,
/// S_HERO_DISC_RIGHT
{ 8,8, 
 0b11000011,
 0b10000001,
 0b00000000,
 0b00000000,
 0b00000000,
 0b00110010,
 0b10011101,
 0b11000011
}
,
/// S_HERO_DISC_UP2
{ 8,8, 
 0b11000011,
 0b10100001,
 0b01000000,
 0b01000000,
 0b01100000,
 0b00100000,
 0b10000001,
 0b11000011
}
,
/// S_HERO_DISC_UP
{ 8,8, 
 0b11000011,
 0b10000101,
 0b00000010,
 0b00000010,
 0b00000110,
 0b00000100,
 0b10000001,
 0b11000011
}
,
/// S_HERO_SQUARE2_DOWN2
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00001100,
 0b00011110,
 0b00010010,
 0b00010010,
 0b00001101,
 0b00000000
}
,
/// S_HERO_SQUARE2_DOWN
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00110000,
 0b01111000,
 0b01001000,
 0b01001000,
 0b10110000,
 0b00000000
}
,
/// S_HERO_SQUARE2_LEFT
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00000000,
 0b00111000,
 0b01001100,
 0b01001100,
 0b00111000,
 0b01000000
}
,
/// S_HERO_SQUARE2_RIGHT
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00000000,
 0b00011100,
 0b00110010,
 0b00110010,
 0b00011100,
 0b00000010
}
,
/// S_HERO_SQUARE2_UP2
{ 8,8, 
 0b00000000,
 0b10110000,
 0b01001000,
 0b01001000,
 0b01111000,
 0b00110000,
 0b00000000,
 0b00000000
}
,
/// S_HERO_SQUARE2_UP
{ 8,8, 
 0b00000000,
 0b00001101,
 0b00010010,
 0b00010010,
 0b00011110,
 0b00001100,
 0b00000000,
 0b00000000
}
,
/// S_HERO_SQUARE_DOWN2
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00000100,
 0b00000110,
 0b00000010,
 0b00000010,
 0b00000100,
 0b00000000
}
,
/// S_HERO_SQUARE_DOWN
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00100000,
 0b01100000,
 0b01000000,
 0b01000000,
 0b00100000,
 0b00000000
}
,
/// S_HERO_SQUARE_LEFT
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00000000,
 0b00000000,
 0b00000000,
 0b01001100,
 0b00111000,
 0b00000000
}
,
/// S_HERO_SQUARE_RIGHT
{ 8,8, 
 0b00000000,
 0b00000000,
 0b00000000,
 0b00000000,
 0b00000000,
 0b00110010,
 0b00011100,
 0b00000000
}
,
/// S_HERO_SQUARE_UP2
{ 8,8, 
 0b00000000,
 0b00100000,
 0b01000000,
 0b01000000,
 0b01100000,
 0b00100000,
 0b00000000,
 0b00000000
}
,
/// S_HERO_SQUARE_UP
{ 8,8, 
 0b00000000,
 0b00000100,
 0b00000010,
 0b00000010,
 0b00000110,
 0b00000100,
 0b00000000,
 0b00000000
}
,
/// S_HERO_TRIANGLE2_DOWN2
{ 8,8, 
 0b11111100,
 0b11111000,
 0b11110000,
 0b11101101,
 0b11010010,
 0b10010010,
 0b00001100,
 0b00000000
}
,
/// S_HERO_TRIANGLE2_DOWN
{ 8,8, 
 0b00111111,
 0b00011111,
 0b00001111,
 0b10110111,
 0b01001011,
 0b01001001,
 0b00110000,
 0b00000000
}
,
/// S_HERO_TRIANGLE2_LEFT
{ 8,8, 
 0b00111111,
 0b00011111,
 0b00001111,
 0b00110111,
 0b01001011,
 0b01001001,
 0b00110000,
 0b00001000
}
,
/// S_HERO_TRIANGLE2_RIGHT
{ 8,8, 
 0b11111100,
 0b11111000,
 0b11110000,
 0b11101100,
 0b11010010,
 0b10010010,
 0b00001100,
 0b00010000
}
,
/// S_HERO_TRIANGLE2_UP2
{ 8,8, 
 0b00000000,
 0b00110000,
 0b01001001,
 0b01001011,
 0b10110111,
 0b00001111,
 0b00011111,
 0b00111111
}
,
/// S_HERO_TRIANGLE2_UP
{ 8,8, 
 0b00000000,
 0b00001100,
 0b10010010,
 0b11010010,
 0b11101101,
 0b11110000,
 0b11111000,
 0b11111100
}
,
/// S_HERO_TRIANGLE_DOWN2
{ 8,8, 
 0b11111100,
 0b11111000,
 0b11110000,
 0b11100100,
 0b11000010,
 0b10000010,
 0b00000101,
 0b00000000
}
,
/// S_HERO_TRIANGLE_DOWN
{ 8,8, 
 0b00111111,
 0b00011111,
 0b00001111,
 0b00100111,
 0b01000011,
 0b01000001,
 0b10100000,
 0b00000000
}
,
/// S_HERO_TRIANGLE_LEFT
{ 8,8, 
 0b00111111,
 0b00011111,
 0b00001111,
 0b00000111,
 0b00000011,
 0b01001001,
 0b00110000,
 0b01000000
}
,
/// S_HERO_TRIANGLE_RIGHT
{ 8,8, 
 0b11111100,
 0b11111000,
 0b11110000,
 0b11100000,
 0b11000000,
 0b10010010,
 0b00001100,
 0b00000010
}
,
/// S_HERO_TRIANGLE_UP2
{ 8,8, 
 0b00000000,
 0b10100000,
 0b01000001,
 0b01000011,
 0b00100111,
 0b00001111,
 0b00011111,
 0b00111111
}
,
/// S_HERO_TRIANGLE_UP
{ 8,8, 
 0b00000000,
 0b00000101,
 0b10000010,
 0b11000010,
 0b11100100,
 0b11110000,
 0b11111000,
 0b11111100
}
,
/// S_HOLE_DISC
{ 8,8, 
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11000011,
 0b10111101,
 0b11000011,
 0b11111111
}
,
/// S_HOLE_SQUARE
{ 8,8, 
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b10000001,
 0b10111101,
 0b10000001,
 0b11111111
}
,
/// S_HOLE_TRIANGLE
{ 8,8, 
 0b11111111,
 0b11111111,
 0b11111111,
 0b11111111,
 0b11100111,
 0b11011011,
 0b10000001,
 0b11111111
}
,
/// S_KEY
{ 8,8, 
 0b11111111,
 0b11111111,
 0b10000001,
 0b10101001,
 0b10001111,
 0b11111111,
 0b11111111,
 0b11111111
}
,
/// S_MACHINE
{ 8,8, 
 0b11111111,
 0b10111101,
 0b11011011,
 0b11100111,
 0b10010001,
 0b10101101,
 0b10000001,
 0b11111111
}
,
/// S_PARTICLE_H
{ 8,8, 
 0b11111111,
 0b10111111,
 0b11110111,
 0b11100111,
 0b11101111,
 0b11111101,
 0b11111111,
 0b11111111
}
,
/// S_PARTICLE_V
{ 8,8, 
 0b11111111,
 0b11111101,
 0b11111111,
 0b11100111,
 0b11110011,
 0b11111111,
 0b11011111,
 0b11111111
}
,
/// S_PIPE_DL
{ 8,8, 
 0b11111111,
 0b01111111,
 0b10001111,
 0b11110111,
 0b10111011,
 0b10011011,
 0b01001011,
 0b10000001
}
,
/// S_PIPE_DR
{ 8,8, 
 0b11111111,
 0b11111110,
 0b11110000,
 0b11101110,
 0b11011100,
 0b11011000,
 0b11010010,
 0b10111101
}
,
/// S_PIPE_H
{ 8,8, 
 0b11111111,
 0b01111110,
 0b10000000,
 0b11111110,
 0b10101010,
 0b10000000,
 0b01111110,
 0b11111111
}
,
/// S_PIPE_UL
{ 8,8, 
 0b10111101,
 0b01001011,
 0b00011011,
 0b00111011,
 0b01110111,
 0b00001111,
 0b01111111,
 0b11111111
}
,
/// S_PIPE_UR
{ 8,8, 
 0b10000001,
 0b11010010,
 0b11011001,
 0b11011101,
 0b11101111,
 0b11110001,
 0b11111110,
 0b11111111
}
,
/// S_PIPE_V
{ 8,8, 
 0b10111101,
 0b11001011,
 0b11011011,
 0b11001011,
 0b11011011,
 0b11001011,
 0b11011011,
 0b10000001
}
,
/// S_SHADE
{ 8,8, 
 0b01010101,
 0b10101010,
 0b01010101,
 0b10101010,
 0b01010101,
 0b10101010,
 0b01010101,
 0b10101010
}
,
/// S_SQUARE2
{ 8,8, 
 0b10010101,
 0b00101010,
 0b01111110,
 0b00101010,
 0b01111110,
 0b10101010,
 0b01111110,
 0b10000001
}
,
/// S_SQUARE
{ 8,8, 
 0b10000001,
 0b01111110,
 0b01010101,
 0b01111110,
 0b01010101,
 0b01111110,
 0b01010100,
 0b10101001
}
,
/// S_TRIANGLE2
{ 8,8, 
 0b11111001,
 0b11110010,
 0b11101110,
 0b11001010,
 0b10111110,
 0b00101010,
 0b01111110,
 0b10000001
}
,
/// S_TRIANGLE
{ 8,8, 
 0b11111001,
 0b11110110,
 0b11100100,
 0b11011110,
 0b10010100,
 0b01111110,
 0b01010100,
 0b10010001
}
,
/// S_TURNCOCK_H
{ 8,8, 
 0b11111111,
 0b01100110,
 0b10001001,
 0b11010010,
 0b10101010,
 0b10010001,
 0b01100110,
 0b11111111
}
,
/// S_TURNCOCK_V
{ 8,8, 
 0b10111101,
 0b11001011,
 0b11010011,
 0b10101001,
 0b10010101,
 0b11000011,
 0b11011011,
 0b10100101
}
,
/// S_WALL
{ 8,8, 
 0b01110111,
 0b10111011,
 0b11011101,
 0b11101110,
 0b01110111,
 0b10111011,
 0b11011101,
 0b11101110
}
};
/// names:
enum {
  S_BLANK,
  S_BOOM,
  S_DISC2,
  S_DISC,
  S_DOOR,
  S_GUN_DOWN,
  S_GUN_LEFT,
  S_GUN_RIGHT,
  S_GUN_UP,
  S_HEART,
  S_HERO_DISC2_DOWN2,
  S_HERO_DISC2_DOWN,
  S_HERO_DISC2_LEFT,
  S_HERO_DISC2_RIGHT,
  S_HERO_DISC2_UP2,
  S_HERO_DISC2_UP,
  S_HERO_DISC_DOWN2,
  S_HERO_DISC_DOWN,
  S_HERO_DISC_LEFT,
  S_HERO_DISC_RIGHT,
  S_HERO_DISC_UP2,
  S_HERO_DISC_UP,
  S_HERO_SQUARE2_DOWN2,
  S_HERO_SQUARE2_DOWN,
  S_HERO_SQUARE2_LEFT,
  S_HERO_SQUARE2_RIGHT,
  S_HERO_SQUARE2_UP2,
  S_HERO_SQUARE2_UP,
  S_HERO_SQUARE_DOWN2,
  S_HERO_SQUARE_DOWN,
  S_HERO_SQUARE_LEFT,
  S_HERO_SQUARE_RIGHT,
  S_HERO_SQUARE_UP2,
  S_HERO_SQUARE_UP,
  S_HERO_TRIANGLE2_DOWN2,
  S_HERO_TRIANGLE2_DOWN,
  S_HERO_TRIANGLE2_LEFT,
  S_HERO_TRIANGLE2_RIGHT,
  S_HERO_TRIANGLE2_UP2,
  S_HERO_TRIANGLE2_UP,
  S_HERO_TRIANGLE_DOWN2,
  S_HERO_TRIANGLE_DOWN,
  S_HERO_TRIANGLE_LEFT,
  S_HERO_TRIANGLE_RIGHT,
  S_HERO_TRIANGLE_UP2,
  S_HERO_TRIANGLE_UP,
  S_HOLE_DISC,
  S_HOLE_SQUARE,
  S_HOLE_TRIANGLE,
  S_KEY,
  S_MACHINE,
  S_PARTICLE_H,
  S_PARTICLE_V,
  S_PIPE_DL,
  S_PIPE_DR,
  S_PIPE_H,
  S_PIPE_UL,
  S_PIPE_UR,
  S_PIPE_V,
  S_SHADE,
  S_SQUARE2,
  S_SQUARE,
  S_TRIANGLE2,
  S_TRIANGLE,
  S_TURNCOCK_H,
  S_TURNCOCK_V,
  S_WALL,
  N_O_SPRITES
};