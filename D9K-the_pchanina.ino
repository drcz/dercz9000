//////////////////////////////////////////////////////////////////
///                     -= PCHANINA 1.0 =-
///        a silly game for the dercz9000 video console
///             Alicante 2016.03.25-30, drcz@tlen.pl
//////////////////////////////////////////////////////////////////
///     I state this only once: USE AT YOUR OWN RISK.
/// If it fries your TVset, your arduino, or both, that's
/// neither my responsibility, nor my business.

/// TODO "you won" surprise screen
/// TODO status bar or leds for keys and lives.
/// TODO shrink memory usage (pending)
/// TODO more/better levels
/// TODO disclaimer, game description, comments +schematics?


/// THE drcz9000 VIDEO CONSOLE ///////////////////////////////////

/// The console utilizes the divine tvout library by Myles Metzer.
/// -- cf http://playground.arduino.cc/Main/TVout
#include <TVout.h>
#include <fontALL.h> /// TODO moze tylko 8x8 i 4x6?
#include <avr/pgmspace.h>

TVout TV;

/// We use 120x96 resolution.
/// Turns out we could even use 128x96, hehe.
void setup_tv() {
  TV.begin(PAL,120,96);
  TV.clear_screen();  
}
/// note that now TVout utilizes 16*96=1536B,
/// no we're left with less than 0.5KB of SRAM (cf below).

/// "The gamepad", ie 4 switches, I connect to pins 3-6.
/// I've picked them because TVout utilizes only pins >6.
#define BTN_LEFT  3
#define BTN_DOWN  4
#define BTN_UP    5
#define BTN_RIGHT 6

void setup_gamepad() {
  pinMode(BTN_LEFT,INPUT);
  pinMode(BTN_UP,INPUT);
  pinMode(BTN_DOWN,INPUT);
  pinMode(BTN_RIGHT,INPUT);
}

/// In order to abstract from these choices, we introduce
/// a "joystick" datastructure, so that if you prefer to use
/// some other device, just modify the refresh_joystick():
struct {
  char dx,dy; /// they both range {-1,0,1}
} the_joystick;

/// this one we will call more than once in each game cycle...
void refresh_joystick() {
  if(digitalRead(BTN_LEFT)) the_joystick.dx=-1;
  else if(digitalRead(BTN_RIGHT)) the_joystick.dx=1;
  else if(digitalRead(BTN_UP)) the_joystick.dy=-1;
  else if(digitalRead(BTN_DOWN)) the_joystick.dy=1;
  /// NB we disallow "diagonal" moves (eg left+up).
}

/// ...so after it's finally used as input, we need to reset it:
void reset_joystick() { the_joystick.dx=the_joystick.dy=0; }

/// TODO sometimes it does "two moves instead of one"
/// -- try to fix it by waiting for button's release?


/// THE GAME'S WORLD /////////////////////////////////////////////

/// Types of things inhabiting the game's world:
enum {EMPTY,
      DIRT,
      WALL,
      KEY,
      LOCK,
      NIDERITE,
      ANT,
      ACTOR,
      N_O_TYPES};

/// the actor == player's avatar.
/// NB the position and facing are in things[], below.
struct {
  unsigned char current_level; /// the level we're at
  unsigned char lives; /// lives left
  unsigned char niderite_left; /// when 0 we move to next level
  unsigned char dead; /// TODO this one is not needed anymore
  unsigned char keys; /// number of carried keys
} actor; /// maybe it should rather be called "game_stats"?

#define MAX_LIVES 3 /// ha!

/// Now, the first thing I found out the hard way was that
/// UNO has only 2KB of SRAM, most of which is being consumed
/// by TVout... so there's no place fo keeping the whole level
/// map in SRAM.

/// NB "sram" in Polish means "I poop" -- so now you know.

/// So instead we keep the static parts of the maps in the flash
/// memory (where we have 32KB -- minus 512B bootloader, TVout,
/// sprites, and the game's code -- which still is A LOT).
#define N_O_LEVELS 5

#define MAP_W 23 /// HEIL
#define MAP_H 23 /// ERIS

extern const unsigned char levels[N_O_LEVELS][MAP_H][MAP_W+1];
/// cf the end of this file -- and notice levels contain also
/// the starting positoins for ants, niderite and actor.
/// novum: AND locks and keys.
/// these ones we will interpret as EMPTY (ie "floor").

/// ...and use SRAM only for dynamic things (ants/niderite/actor)
/// AND LOCKS AND DOORS

#define MAX_N_O_THINGS 33
unsigned char n_o_things;
struct {
  unsigned char x,y,type;
  char dx,dy;
} things[MAX_N_O_THINGS];
/// TODO the above can be shrunk to 2B per thing (notebook)


/// One thing we want for sure is to find the thing at x,y.
/// We pick the convention of returning MAX_N_O_THINGS if
/// no thing is preset in there.
unsigned char index_of_thing_at(unsigned char x,unsigned char y) {
  unsigned char i;
  for(i=0;i<n_o_things;i++) {
    if(things[i].type!=EMPTY /// dismiss empty slots
       && things[i].x==x
       && things[i].y==y) {
      return i;
     }
  }
  return MAX_N_O_THINGS;
}

/// Therefore to find out what's on the map, we first check the
/// things, and if no [interesting] thing is found, we revert to
/// static map from flash memory (note the pgm_read_byte, cf
/// https://www.arduino.cc/en/Reference/PROGMEM )
/// Notice the use of short (it could be signed char actually),
/// for convenience of displaying... you'll see it soon.
/// Also, we remove thing by switching it's type to EMPTY.
unsigned char read_map_at(short x,short y) {
  unsigned char i;
  /// outside the map area is just dirt
  if(x<0 || y<0 || x>=MAP_W || y>=MAP_H) return DIRT;
  /// a thing maybe?
  i=index_of_thing_at(x,y);
  if(i<n_o_things) return things[i].type;
  /// nope, therefore use the static map.
  switch(pgm_read_byte(&levels[actor.current_level][y][x])) {
  case '#': return WALL;
  case '.':
  case 's': /// ignore the "starting position" of the actor
  case 'O': /// and niderite crystals
  case 'k': /// and keys [!]
  case 'l': /// and locks ...
  case '^': /// and the ants...
  case 'v':
  case '>':
  case '<': return EMPTY;
  default: return DIRT;
  }
}

/// In order to initialize given level, we need to set things up:
void initialize_level(unsigned char level) {
  unsigned char i,j;
  actor.current_level=level;
  actor.dead=0;
  actor.niderite_left=0;
  actor.keys=0;
  for(i=0;i<MAX_N_O_THINGS;i++) things[i].type=EMPTY;
  n_o_things=0;
  /// we'll make the actor our first thing; facing down (shyness?)
  things[n_o_things].dx=0; things[n_o_things].dy=1;
  things[n_o_things++].type=ACTOR; 
  for(i=0;i<MAP_W;i++)
    for(j=0;j<MAP_H;j++)
      /// the following switch is wet. so what?
      switch(pgm_read_byte(&levels[level][j][i])) {
      case 's':
	things[0].x=i; things[0].y=j;
	break;
      case 'k':
	things[n_o_things].x=i; things[n_o_things].y=j;
        things[n_o_things].dx=things[n_o_things].dy=0;
        things[n_o_things++].type=KEY;
	break;
      case 'l':
	things[n_o_things].x=i; things[n_o_things].y=j;
        things[n_o_things].dx=things[n_o_things].dy=0;
        things[n_o_things++].type=LOCK;
	break;
      case 'O':
	things[n_o_things].x=i; things[n_o_things].y=j;
	things[n_o_things].dx=things[n_o_things].dy=0;
	things[n_o_things++].type=NIDERITE;
	actor.niderite_left++;
	break;
      case '^':
      case 'v':
      case '<':
      case '>':
	things[n_o_things].x=i;	things[n_o_things].y=j;
	things[n_o_things].type=ANT;
	things[n_o_things].dx=things[n_o_things].dy=0;
	switch(pgm_read_byte(&levels[level][j][i])) {
	case '^': things[n_o_things].dy=-1; break;
	case 'v': things[n_o_things].dy=-1; break;
	case '<': things[n_o_things].dx=-1; break;
	case '>': things[n_o_things].dx=1; break;
	}
	n_o_things++;
	break;
      }
}


/// THE GAME'S MECHANICS /////////////////////////////////////////

/// (...)

/// there used to be 2d array of procedure pointers, but turned
/// out it takes too much SRAM, and I didn't want to initialize
/// it as const, because it then stopped being THAT redable...

///void (*collision[N_O_TYPES][N_O_TYPES])(unsigned char,
///                                        unsigned char);

/// so I re-declared the following procedures as inline,
/// and there you go...

/// the two arguments are indexes of things in things[] array.
/// in some cases the second argument is ignored; eg no wall
/// has its entry in things[], but bumping at wall modifies
/// only the first (active) object, which _has_ to be dynamic
/// (how could something static bump into anything?).

inline
void cl_nothing(unsigned char active, unsigned char passive) {
  return;
}

inline
void cl_left_bump(unsigned char active, unsigned char passive) {
  things[active].dx*=-1; things[active].dy*=-1;
}

inline
void cl_both_bump(unsigned char active, unsigned char passive) {
  things[active].dx*=-1; things[active].dy*=-1;
  things[passive].dx*=-1; things[passive].dy*=-1;
}

inline
void cl_left_stop(unsigned char active, unsigned char passive) {
  things[active].dx=things[active].dy=0;
}

inline
void cl_right_push(unsigned char active, unsigned char passive) {
  things[passive].dx=things[active].dx;
  things[passive].dy=things[active].dy;
}

inline
void cl_anihilate(unsigned char active, unsigned char passive) {
  things[passive].type=things[active].type=EMPTY;
  actor.niderite_left-=2;
}

inline
void cl_kill(unsigned char active, unsigned char passive) {
  actor.dead=1;
}

inline
void cl_kill_ant(unsigned char active, unsigned char passive) {
  things[active].dx*=-1;
  things[active].dy*=-1;
  things[passive].type=EMPTY;
}

inline
void cl_open(unsigned char active, unsigned char passive) {
  if(actor.keys) {
    actor.keys--;
    things[passive].type=EMPTY;
  }
}

inline
void cl_pick(unsigned char active, unsigned char passive) {
  actor.keys++;
  things[passive].type=EMPTY;
}

void game_cycle() {
  unsigned char i,nx,ny,t;
  for(i=0;i<n_o_things;i++) {
    if(things[i].type==EMPTY) continue;
    if(things[i].dx==0 && things[i].dy==0) continue;
    if(i==0) /// the actor.
      if(the_joystick.dx!=0 || the_joystick.dy!=0) {
	things[i].dx=the_joystick.dx;
	things[i].dy=the_joystick.dy;
      } else continue; /// no move this time.
    nx=things[i].x+things[i].dx;
    ny=things[i].y+things[i].dy;
    t=read_map_at(nx,ny);
    if(t==EMPTY) { /// clear, move on.
      things[i].x=nx; things[i].y=ny;
    } else
      // ech, it used to be SOO PRETTY:
      // collision[things[i].type][t](i,index_of_thing_at(nx,ny));
      // but SRAM is SRAM...
      switch(things[i].type) {
      case ACTOR:
	switch(t) {
	case ANT:
	  cl_kill(i,index_of_thing_at(nx,ny));
	  break;
	case NIDERITE:
	  cl_right_push(i,index_of_thing_at(nx,ny));
	  break;
        case LOCK:
          cl_open(i,index_of_thing_at(nx,ny));
          break;
        case KEY:
          cl_pick(i,index_of_thing_at(nx,ny));
          break;
	}
	break;

      case NIDERITE:
	switch(t) {
	case ANT:
          cl_kill_ant(i,index_of_thing_at(nx,ny));
          break;
        /* it's better to be able to stop it
	case ACTOR:
	  cl_left_bump(i,index_of_thing_at(nx,ny));          
	  break;
        */
	case NIDERITE:
	  cl_anihilate(i,index_of_thing_at(nx,ny));
	  break;
	default:
	  cl_left_stop(i,index_of_thing_at(nx,ny));
	}
	break;

      case ANT:
	switch(t) {
	case ACTOR:
	  cl_kill(i,index_of_thing_at(nx,ny));
	  break;
        case NIDERITE:
	case ANT:
	  cl_both_bump(i,index_of_thing_at(nx,ny));
	  break;
	default:
	  cl_left_bump(i,index_of_thing_at(nx,ny));
	}
	break;
      }
  }
}

/// DISPLAY //////////////////////////////////////////////////////

/// Now the sprites...
enum {S_EMPTY,
      S_DIRT,
      S_WALL,
      S_KEY,
      S_LOCK,
      S_NIDERITE,S_NIDERITE2, /// NIDERITE2 has a "blink"...
      S_ANT_U,S_ANT_D,S_ANT_L,S_ANT_R,
      S_ANT_U2,S_ANT_D2,S_ANT_L2,S_ANT_R2, /// "animation frame"
      S_ACTOR_U,S_ACTOR_D,S_ACTOR_L,S_ACTOR_R,
      S_ACTOR_U2,S_ACTOR_D2,S_ACTOR_L2,S_ACTOR_R2, /// as above
      S_ACTOR_DEAD,S_ACTOR_DEAD2,
      N_O_SPRITES};

/// I have prepared awesome 16x16 sprites, but they turned out
/// too big -- the viewport 7x5 tiles was too small to play
/// comfortably... So yup 8x8, like C=64 characters.
#define SPRITE_H 8
#define SPRITE_W 8
extern const unsigned char sprite[N_O_SPRITES][SPRITE_H+2];

/// (...)
char anim_frame=1; /// TODO opis
/// TODO no need to pass courtain
void display_board(unsigned char courtain) {
  char i,j;
  unsigned char x=0,y=0,spr,ind;  

  TV.delay_frame(1);
  for(j=things[0].y-5;j<=things[0].y+5;j++) {
    for(i=things[0].x-7;i<=things[0].x+7;i++) {
      switch(read_map_at(i,j)) {
        
      case WALL: spr=S_WALL; break;
      case DIRT: spr=S_DIRT; break;
      case KEY: spr=S_KEY; break;
      case LOCK: spr=S_LOCK; break;

      case NIDERITE:
	spr=S_NIDERITE;
	if(random()%13==7) spr=S_NIDERITE2; /// blink!
	break;

      case ANT:
	ind=index_of_thing_at(i,j);
	if(things[ind].dx==1) spr=S_ANT_R;
	else if(things[ind].dx==-1) spr=S_ANT_L;
	else if(things[ind].dy==1) spr=S_ANT_D;
	else spr=S_ANT_U;

	if(anim_frame<0) spr+=4; /// "animate"
	break;

      case ACTOR:
	ind=0; /// we already know that...
	if(things[ind].dx==1) spr=S_ACTOR_R;
	else if(things[ind].dx==-1) spr=S_ACTOR_L;
	else if(things[ind].dy==-1) spr=S_ACTOR_U;
	else spr=S_ACTOR_D;
	
	if(anim_frame<0
	   && (the_joystick.dx!=0 || the_joystick.dy!=0))
	  spr+=4; /// "animate", but only when moving.

        /// WAIT! and what if the actor is dead?
        if(actor.dead) {
          spr=S_ACTOR_DEAD;
          if(random()%2) spr=S_ACTOR_DEAD2;
        }
	break;
	
      default: spr=S_EMPTY; /// defensively.
      }
      ///if(rand()%7>chance) spr=S_EMPTY; /// random fade in/out.
      if(y<courtain) spr=S_EMPTY; /// theatrical fade in/out.
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[spr]); /// TODO: offset?
      x++;
    }
    x=0; y++;
  }
  anim_frame*=-1; /// flip! :)
}

void animate_gameover() {
  unsigned char y;
  TV.delay_frame(1);
  TV.select_font(font8x8);
  TV.clear_screen();
  for(y=0;y<40;y++) {
    TV.print(24,y,"GAME OVER");
    TV.print(24,0,"         "); /// so cool ;)
    TV.delay(23);
  }
  TV.delay(997);
  TV.clear_screen();
}

void display_title_screen() {
  TV.delay_frame(1);
  TV.select_font(font8x8);
  TV.clear_screen();
  TV.print(40,30,"the");
  TV.print(0,39,"PCHANINA");
  TV.select_font(font4x6);
  TV.print(8,90,"MMXVI by derczansky studio");
}

/// THE MAIN LOOP ////////////////////////////////////////////////

/// the game can be in one of 6 states, where TITLE, GAME_OVER
/// and WON are static images, FADE_IN/OUT are when the board
/// appears/disappears, and PLAY is the game proper (ie when
/// game cycles are realised).
unsigned char game_state=0;
enum{TITLE,FADE_IN,PLAY,FADE_OUT,GAME_OVER,WON};

/// it's also handy to know whether after fade out-in sequence
/// we get into the same [death], or new level.
unsigned char fade_cause;
enum{DEATH,LEVELUP};
/// and finally there's 0-13 fade gradation...
unsigned char global_courtain=0;
#define MAX_COURTAIN 13 /// the height of viewport + 2 for timig

void display_d9k_logo(); /// a bit narcisstic, no?
void setup() {
  randomSeed(analogRead(0)); /// haha! magick.
  setup_tv();
  setup_gamepad();
  display_d9k_logo();  
  game_state=TITLE;
}

/// one more thing. we want the game to operate at ~7Hz,
/// (ie 7 game cycles per second), which means each cycle
/// should take ~142ms.
#define TIME_STEP 142
/// ...BUT the courtain should move faster than that:
#define COURTAIN_TIME_STEP 42 /// oh, The Answer.

void loop() {
  long then,now,interval;

  switch(game_state) {

  case WON: /// TODO!!!!!!
    break;

  case TITLE: 
    display_title_screen();
    if(the_joystick.dx!=0 || the_joystick.dy!=0) {
      initialize_level(0);
      actor.lives=MAX_LIVES;
      game_state=FADE_IN;
      global_courtain=MAX_COURTAIN;
    }
    break;

  case GAME_OVER:
    animate_gameover();
    game_state=TITLE;
    break;

  case FADE_OUT:
    display_board(global_courtain);
    if(++global_courtain>=MAX_COURTAIN) {
      if(fade_cause==DEATH && actor.lives==0)
	game_state=GAME_OVER;
      else {
	if(fade_cause==LEVELUP) actor.current_level++;
	initialize_level(actor.current_level);
	game_state=FADE_IN;
      }
    }
    break;

  case FADE_IN:
    display_board(global_courtain);
    if(--global_courtain<=0) game_state=PLAY;
    break;

  case PLAY:
    display_board(0);
    game_cycle();
    if(actor.niderite_left<=0) {
      game_state=FADE_OUT;
      fade_cause=LEVELUP;
    }
    if(actor.dead) {
      actor.lives--;
      game_state=FADE_OUT;
      fade_cause=DEATH;  
    }
    break;

  }
  reset_joystick();
  /// wait till the end of this cycle.
  then=now=TV.millis();
  if(game_state==PLAY) interval=TIME_STEP;
    else interval=COURTAIN_TIME_STEP;
  while(now-then<interval) {
    /// controls can be altered at any moment, right?
    refresh_joystick();
    now=TV.millis();
  }
}


/// THE DATA (SPRITES AND MAPS) //////////////////////////////////

PROGMEM const unsigned char levels[N_O_LEVELS][MAP_H][MAP_W+1] = {
  /// LEVEL 1
  {" #####           ##### ",
   "##.#.##         ##.#.##",
   "#.....#         #.....#",
   "##.#.## ####### ##.#.##",
   "#.....# #..O..# #.....#",
   "###########.###########",
   "#.......# #.# #.......#",
   "#......## #.# ##......#",
   "#......#  #.#  #......#",
   "#......#  #.#  #v.....#",
   "#......####.####.v....#",
   "#.................v...#",
   "#......###.O.###...v..#",
   "#.....^# #...# #....v.#",
   "#....^.###.O.###.....v#",
   "#...^.................#",
   "#..^...####s####......#",
   "#.^....#  #.#  #......#",
   "#^.....# ##.## #......#",
   "#......# #...# #......#",
   "######## #.O.# ########",
   " ##      #...#      ## ",
   "  #       ###       #  "},
  /// LEVEL 2
  {"        ###            ",
   "  ###   #v##           ",
   " ##.##  #.###          ",
   " #...#  #.########     ",
   " ##.##  #.......O#     ",
   "  ###   #.######O#     ",
   "        #.......O#     ",
   "        #.######.#     ",
   "     ####.######.#     ",
   "     #v..OO..###.#     ",
   "     #.......###.#     ",
   "######...s...###l######",
   " ##...<..........#     ",
   "  ####...........#     ",
   "     ####.O.O.O..#     ",
   "        #........#     ",
   "        #........####  ",
   "        #>..........#  ",
   "        #.###########  ",
   "        #k#            ",
   "        ###            ",
   "        #              "},
  /// LEVEL 3
  {"     #     #     #     ",
   "    ###############    ",
   "    #>............#    ",
   "  ###.###########l###  ",
   " ##v....O#  ##O....v## ",
   " #...##..O# #...##...# ",
   " #^..##...# #...##..^# ",
   " ##.O...O.####...O..## ",
   "  #.##.....#....#####  ",
   "  #.######O#O#### #v#  ",
   " ##.###.##O#O#..###.## ",
   " #...................# ",
   " #..###############..# ",
   " #..#    #####    #..# ",
   " #..#   ##...##   #.^# ",
   " #..#####..s..#####..# ",
   " #...O..O.....O..O...# ",
   " #.^.####..O..####...# ",
   " #########...######### ",
   "      #>.......k#      ",
   "      #####O#####      ",
   "   ####   ###   ####   ",
   "      #         #      "},
  /// LEVEL 4
  {"###             #######",
   "#k#     ######  #.....#",
   "#.##    #....#  #.###.#",
   "#.#######.##.####.# #.#",
   "#.##...l.s##....O.# #.#",
   "#....###..##.##.#####.#",
   "###### #..##O##.......#",
   "       #..#...####O####",
   " #######..########.#   ",
   " #.O...O...........#   ",
   " #.#####OO########.#   ",
   " #.#   #..#      #O####",
   " #.#####..###### #....#",
   " #.............# ###..#",
   " #.#####OO##.#.# #...O#",
   " #.#   #^^##.#.###.O###",
   " #.#########.#......#  ",
   " #^.........<########  ",
   " #.###########         ",
   " #.#                   ",
   " #.#############       ",
   " #.............#       ",
   " ###############       "},
  /// LEVEL 5
  { "#######################",
    "#.....<#>........<#.O.#",
    "#.##...#.........<#..##",
    "#.##.O.O.#..#######..# ",
    "#.##..##.#.....O.##..##",
    "#.s...#..#..#.#..#....#",
    "#.....##.##.#.#.##....#",
    "#####O##....#.###.O.OO#",
    "#......###..#.##.....##",
    "#..#...OO#^......##^###",
    "#.....##.####O#######.#",
    "#O....O...............#",
    "###################...#",
    " #.....<#####....##.###",
    " #.#..#.O......####.#  ",
    " #....v.#.###.......#  ",
    " #..##..#.#########O###",
    " #......#.............#",
    " #..##..###O#######.###",
    " #.^....###O##>..##.#  ",
    " #.#..#.....###..##O#  ",
    " #>.....##### #O.O..#  ",
    " ########     #######  "}
    /// TODO TODO TODO
};



PROGMEM const unsigned char sprite[N_O_SPRITES][SPRITE_H+2] = {
  /// S_EMPTY
  {SPRITE_W,SPRITE_H,
   0,0,0,0,0,0,0,0
  },

  /// S_DIRT
  {SPRITE_W,SPRITE_H,
   0b10101010,
   0b01010101,
   0b10101010,
   0b01010101,
   0b10101010,
   0b01010101,
   0b10101010,
   0b01010101
  },

  /// S_WALL
  {SPRITE_W,SPRITE_H,
   0b01111110,
   0b10000001,
   0b10000001,
   0b10000101,
   0b10000101,
   0b10001101,
   0b10000001,
   0b01111110
  },

  /// KEY
  {SPRITE_W,SPRITE_H,
  0b00000000,
  0b00000000,
  0b00000000,
  0b11100000,
  0b10111111,
  0b11100101,
  0b00000000,
  0b00000000},

  /// LOCK
  {SPRITE_W,SPRITE_H,
  0b00111100,
  0b01000010,
  0b01000010,
  0b01111110,
  0b01011110,
  0b01111110,
  0b01111110,
  0b00000000},

  /// S_NIDERITE
  {SPRITE_W,SPRITE_H,
   0b00000000,
   0b00011000,
   0b00101100,
   0b01001110,
   0b01111110,
   0b00111100,
   0b00011000,
   0b00000000
  },

  /// S_NIDERITE2
  {SPRITE_W,SPRITE_H,
   0b00000000,
   0b00011000,
   0b00101100,
   0b01001110,
   0b01110010,
   0b00110100,
   0b00011000,
   0b00000000
  },

  /// S_ANT_U
  {SPRITE_W,SPRITE_H,
   0b01000010,
   0b00111100,
   0b00011000,
   0b00011001,
   0b11111110,
   0b00111100,
   0b01011010,
   0b10000010
  },

  /// S_ANT_D
  {SPRITE_W,SPRITE_H,
   0b01000001,
   0b01011010,
   0b00111100,
   0b01111111,
   0b10011000,
   0b00011000,
   0b00111100,
   0b01000010
  },

  /// S_ANT_L
  {SPRITE_W,SPRITE_H,
   0b00010000,
   0b10001011,
   0b01001100,
   0b01111110,
   0b01111110,
   0b01001100,
   0b10001010,
   0b00001001
  },

  /// S_ANT_R
  {SPRITE_W,SPRITE_H,
   0b00001000,
   0b11010001,
   0b00110010,
   0b01111110,
   0b01111110,
   0b00110010,
   0b01010001,
   0b10010000
  },

  /// S_ANT_U2
  {SPRITE_W,SPRITE_H,
   0b01000010,
   0b00111100,
   0b00011000,
   0b10011000,
   0b01111111,
   0b00111100,
   0b01011010,
   0b01000001
  },

  /// S_ANT_D2
  {SPRITE_W,SPRITE_H,
   0b10000010,
   0b01011010,
   0b00111100,
   0b11111110,
   0b00011001,
   0b00011000,
   0b00111100,
   0b01000010
  },

  /// S_ANT_L2
  {SPRITE_W,SPRITE_H,
   0b00001001,
   0b10001010,
   0b01001100,
   0b01111110,
   0b01111110,
   0b01001100,
   0b10001011,
   0b00010000
  },

  /// S_ANT_R2
  {SPRITE_W,SPRITE_H,
   0b10010000,
   0b01010001,
   0b00110010,
   0b01111110,
   0b01111110,
   0b00110010,
   0b11010001,
   0b00001000
  },

  /// S_ACTOR_U
  {SPRITE_W,SPRITE_H,
   0b01111110,
   0b10000001,
   0b10000001,
   0b01011010,
   0b00111100,
   0b10111101,
   0b10111101,
   0b00000100

  },

  /// S_ACTOR_D
  {SPRITE_W,SPRITE_H,
   0b01111110,
   0b10000001,
   0b10100101,
   0b01000010,
   0b00111100,
   0b10111101,
   0b10111101,
   0b00100000
  },

  /// S_ACTOR_L
  {SPRITE_W,SPRITE_H,
   0b00111100,
   0b01000010,
   0b01010010,
   0b01000010,
   0b00111100,
   0b00011000,
   0b00111000,
   0b00011000
  },

  /// S_ACTOR_R
  {SPRITE_W,SPRITE_H,
   0b00111100,
   0b01000010,
   0b01001010,
   0b01000010,
   0b00111100,
   0b00011000,
   0b00011100,
   0b00011000
  },

  /// S_ACTOR_U2
  {SPRITE_W,SPRITE_H,
   0b01111110,
   0b10000001,
   0b10000001,
   0b01011010,
   0b00111100,
   0b10111101,
   0b10111101,
   0b00100000
  },

  /// S_ACTOR_D2
  {SPRITE_W,SPRITE_H,
   0b01111110,
   0b10000001,
   0b10100101,
   0b01000010,
   0b00111100,
   0b10111101,
   0b10111101,
   0b00000100
  },

  /// S_ACTOR_L2
  {SPRITE_W,SPRITE_H,
   0b00111100,
   0b01000010,
   0b01010010,
   0b01000010,
   0b00111100,
   0b00011000,
   0b00111100,
   0b01100100
  },

  /// S_ACTOR_R2
  {SPRITE_W,SPRITE_H,
   0b00111100,
   0b01000010,
   0b01001010,
   0b01000010,
   0b00111100,
   0b00011000,
   0b00111000,
   0b00110110
  },
  
  /// S_ACTOR_DEAD
  {SPRITE_W,SPRITE_H,
  0b00000000,
  0b00000010,
  0b00100000,
  0b00000100,
  0b00110000,
  0b01001110,
  0b01111110,
  0b00000000
  },

  /// S_ACTOR_DEAD2
  {SPRITE_W,SPRITE_H,
  0b00000000,
  0b01000000,
  0b00010000,
  0b00000000,
  0b00110010,
  0b01001110,
  0b01111110,
  0b00000000
  }
};

/// sorry, I had to:

PROGMEM const unsigned char dercz9000[] = {
  120,96,
0b11111111, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110011, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00011111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000100, 0b10111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001100, 0b01111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000100, 0b00111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b01101111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00101000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b00000000, 0b00001000, 0b00001111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b10000000, 0b00001000, 0b00011111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11000000, 0b00001000, 0b00111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11100000, 0b00001000, 0b00111111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11100000, 0b00000000, 0b00111100, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11110000, 0b00000001, 0b00001111, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11110000, 0b00000010, 0b00001011, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11101111, 0b11110000, 0b00000001, 0b00110011, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11100111, 0b11111000, 0b00000000, 0b00010011, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11000011, 0b11111000, 0b00000000, 0b00111011, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11000001, 0b11111000, 0b00000000, 0b00000100, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11000000, 0b01100000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11000000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11000000, 0b00000000, 0b00000000, 0b00000001, 0b01111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b00010000, 0b00000000, 0b00000000, 0b00110111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01010110, 0b00111000, 0b00000000, 0b00000000, 0b00101111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011100, 0b00101111, 0b01111000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110100, 0b00000011, 0b11101000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01110000, 0b00000000, 0b00010000, 0b00000000, 0b00000000, 0b00000101, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11100010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b01111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11100010, 0b00000000, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11100001, 0b00000100, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11100000, 0b00000001, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b00000001, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b00100001, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b01110111, 0b11111111, 
0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b01111111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00010011, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b01111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b01101111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b01001111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b00001111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b00001111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00001101, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b00001111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00101111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b00001111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111000, 0b00001111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00001111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 
0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00001111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01011111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00001111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00001111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01011111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00000111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00000111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111100, 0b00000111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111100, 0b00000111, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b01111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b01111111, 0b11111111, 
0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11111011, 0b11111111, 0b11111111, 0b00000001, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11111001, 0b11111111, 0b11111111, 0b10000001, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11110000, 0b11111111, 0b11111111, 0b11100001, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11110000, 0b00111111, 0b11111111, 0b11110111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11110000, 0b00011111, 0b11111101, 0b11111111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11110000, 0b00001111, 0b11111100, 0b00011101, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11100000, 0b00000111, 0b11111110, 0b00000111, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11100000, 0b00000011, 0b11111111, 0b00000001, 0b11111111, 0b11111111, 
0b11111111, 0b11111111, 0b11111010, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111110, 0b00000111, 0b11100000, 0b00000001, 0b11111111, 0b00000000, 0b11111110, 0b01111111, 
0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111111, 0b00000111, 0b11100000, 0b00000000, 0b11111101, 0b10000000, 0b00000000, 0b00011111, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111111, 0b00000011, 0b11100000, 0b00000000, 0b00111011, 0b10000000, 0b00000000, 0b00011111, 
0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111111, 0b00000011, 0b11100000, 0b00000000, 0b00011000, 0b00001000, 0b00000000, 0b00001011, 
0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00011111, 0b11111000, 0b01111000, 0b00000011, 0b11100000, 0b00000000, 0b01000000, 0b00000000, 0b00000000, 0b00000010, 
0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00001111, 0b11111100, 0b00110000, 0b00000111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00001111, 0b11111100, 0b00010000, 0b00000111, 0b11100000, 0b00000000, 0b00101000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00001111, 0b11111100, 0b00000000, 0b00000111, 0b11100000, 0b00000000, 0b01000010, 0b00100000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00001111, 0b11111110, 0b00000000, 0b00000111, 0b11100000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00001111, 0b11111110, 0b00000000, 0b00000001, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00011111, 0b11111110, 0b00000000, 0b00000000, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b01000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00011111, 0b11111110, 0b00000000, 0b00000001, 0b11110000, 0b10111000, 0b00000000, 0b00000000, 0b01000000, 0b11000000, 
0b11111111, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00011111, 0b11111100, 0b00000000, 0b00000010, 0b10000000, 0b00000000, 0b01000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00111111, 0b11111100, 0b00010000, 0b01000000, 0b00000000, 0b00000100, 0b01111000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00111111, 0b11111100, 0b01010111, 0b10111100, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00111111, 0b11111100, 0b01111110, 0b00111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b01111111, 0b11111110, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b01111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b01111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b01111111, 0b11111111, 0b11000000, 0b00001000, 0b00000000, 0b00000000, 0b00000011, 0b10000111, 0b00011100, 0b01110000, 
0b11111111, 0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b01111111, 0b11111111, 0b11000000, 0b00001000, 0b00000000, 0b00000000, 0b00000100, 0b01001000, 0b10100010, 0b10001000, 
0b11111111, 0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b01111111, 0b11111111, 0b11100000, 0b01111001, 0b11001110, 0b11100111, 0b10000100, 0b01001000, 0b10100010, 0b10001000, 
0b11111111, 0b11111111, 0b11111111, 0b11110000, 0b00000000, 0b01111111, 0b11111111, 0b11100000, 0b10001010, 0b00101001, 0b00000001, 0b00000100, 0b01001000, 0b10100010, 0b10001000, 
0b11111111, 0b11111111, 0b11111111, 0b11110100, 0b00000000, 0b01111111, 0b11111111, 0b11100000, 0b10001011, 0b11101001, 0b00000011, 0b00000011, 0b11001000, 0b10100010, 0b10001000, 
0b11111111, 0b11111111, 0b11111111, 0b11101000, 0b00000000, 0b11111111, 0b11111111, 0b11100000, 0b10001010, 0b00001001, 0b00000010, 0b00000000, 0b00001000, 0b10100010, 0b10001000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b11111111, 0b11111111, 0b11100000, 0b01111001, 0b11101000, 0b11100111, 0b10000011, 0b10000111, 0b00011100, 0b01110000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00100000, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b10000000, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111011, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000
  };

void display_d9k_logo() {  
  TV.clear_screen();  
  TV.bitmap(0,0,dercz9000);
  delay(6660);
}

/// the end.
