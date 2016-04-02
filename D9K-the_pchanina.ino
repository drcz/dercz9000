//////////////////////////////////////////////////////////////////
///                     -= PCHANINA 1.0 =-
///        a silly game for the dercz9000 video console
///           Alicante 2016.03.25-04.02, drcz@tlen.pl
//////////////////////////////////////////////////////////////////

///     I state this only once: USE AT YOUR OWN RISK.

/// If it fries your TVset, your arduino, or both, that's
/// neither my responsibility, nor my business.

/// TODO more/better levels
/// TODO perhaps change strings to bitmaps?
/// TODO disclaimer, game description, comments + schematics
/// ...?

/// THE drcz9000 VIDEO CONSOLE ///////////////////////////////////

/// The "console" utilizes the divine Myles Metzer's TVout library
/// -- cf http://playground.arduino.cc/Main/TVout
#include <TVout.h>
#include <fontALL.h> /// TODO convert strings to bitmaps perhaps?
#include <avr/pgmspace.h>

TVout TV;

/// We use 120x96 resolution. Note: TVout utilizes 16*96=1536B!
void setup_tv() {
  TV.begin(PAL,120,96);
  TV.clear_screen();  
}

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

/// had to add "d" prefix because of name clash or sth...
enum {dUP,dLEFT,dDOWN,dRIGHT,dCENTER};

struct {
  /// the only "public" field, with "approved" input value:
  byte dir : 3;
  /// pure magic to simplify (and abstract) reads from
  /// switches (or some other input device):
  struct {
    union {
      struct {
	byte up : 1;
	byte left : 1;
	byte down : 1;
	byte right : 1;
      };
      byte bits;
    };
  } reads,last_reads;
  /// and to know how long the last push was:
  long push_time;
} the_joystick = {dCENTER,0,0,dCENTER};

/// minimum number of ms for a push to be approved:
#define MIN_PUSH_TIME 13

/// this one we will call as often as possible:
void refresh_joystick() {
  the_joystick.reads.bits=0; /// clear them all.
  /// modify this to use your controller (a real joystick?)
  if(digitalRead(BTN_UP)) the_joystick.reads.up=1;
  else if(digitalRead(BTN_LEFT)) the_joystick.reads.left=1;
  else if(digitalRead(BTN_DOWN)) the_joystick.reads.down=1;
  else if(digitalRead(BTN_RIGHT)) the_joystick.reads.right=1;
  /// \modify
  
  if(the_joystick.last_reads.bits==0) {
    /// waiting for push...
    if(the_joystick.reads.bits>0) { /// button pushed!
      the_joystick.push_time=TV.millis();
      the_joystick.last_reads.bits=the_joystick.reads.bits;
    }
  } else {
    /// waiting for release
    if(the_joystick.reads.bits==0) { /// there it goes!
      /// did the push last long enough?
      if((TV.millis()-the_joystick.push_time)>=MIN_PUSH_TIME) {
	if(the_joystick.last_reads.up) the_joystick.dir=dUP;
	else if(the_joystick.last_reads.left) the_joystick.dir=dLEFT;
	else if(the_joystick.last_reads.down) the_joystick.dir=dDOWN;
	else if(the_joystick.last_reads.right) the_joystick.dir=dRIGHT;
      }
      the_joystick.last_reads.bits=0;
    }
  }
  /// NB we disallow "diagonal" moves (eg left+up).
}

/// ...so after it's finally used as input, we need to reset it:
void reset_joystick() {
  the_joystick.dir=dCENTER;
//  the_joystick.last_reads.bits=0;
//  the_joystick.push_time=0;
}


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

extern const byte levels[N_O_LEVELS][MAP_H][MAP_W+1];
/// cf the end of this file -- and notice levels contain also
/// the starting positoins for ants, niderite, actor, locks&keys;
/// these ones we will interpret as EMPTY (ie "floor").

/// We'll use SRAM only for "dynamic" things (ants, niderite etc):
#define MAX_N_O_THINGS 33 // ?!
/// I initially wanted to do some oldschool prickwaving like:
/// #define THING_X(I) (((things[I])&0xf800)>>11)
/// #d. THING_SET_X(I,X) (things[I]=((thing[I])&0x07ff)+((X)<<11))
/// BUT then Panicz Godek had threatened me with spanking (!)
/// so I cowardly retreat. (Thank Satan I didn't show him the rest
/// of this crap). Anyway: no bitshifts. Godekstyle -- bitfields!
struct {
  unsigned x : 5;
  unsigned y : 5;
  unsigned dir : 3;
  unsigned type : 3;
} things[MAX_N_O_THINGS];
/// Actually it's quite handy. And no spanking!

inline byte bounce_dir(byte dir) {
  switch(dir) {
  case dUP: return dDOWN;
  case dDOWN: return dUP;
  case dLEFT: return dRIGHT;
  case dRIGHT: return dLEFT;
  default: return dCENTER;
  }
}

/// the actor, ie player's avatar.
#define MAX_LIVES 3 /// come on, it's not a cat.
#define MAX_KEYS 4  /// come on, it's not a... doorman?

struct {
  byte current_level : 5; /// the level we're at (32 levels?!)
  byte niderite_left : 6; /// when 0, move to next level.
  byte lives : 2; /// lives left. not much :)
  byte dead : 1; /// we could avoid using it, but... we don't.
  byte keys : 2; /// number of carried keys.
} actor;

/// One thing we want for sure is to find the thing at x,y.
/// We pick the convention of returning MAX_N_O_THINGS if
/// no thing is preset in there.
byte index_of_thing_at(byte x,byte y) {
  byte i;
  for(i=0;i<MAX_N_O_THINGS;i++) {
    if(things[i].type!=EMPTY /// dismiss empty slots
       && things[i].x==x
       && things[i].y==y) {
      return i;
     }
  }
  return MAX_N_O_THINGS;
}

/// To find out what's on a given location, first check things[],
/// and if no [interesting] thing is found, we revert to the
/// static map from flash memory (note the pgm_read_byte, cf
/// https://www.arduino.cc/en/Reference/PROGMEM).
/// Notice we use signed coordinates --  for convenience of
/// displaying... you'll see it soon.
/// Also, we remove thing by switching it's type to EMPTY.
byte read_map_at(char x,char y) {
  byte i;
  /// outside the map area is just dirt
  if(x<0 || y<0 || x>=MAP_W || y>=MAP_H) return DIRT;
  /// a thing maybe?
  i=index_of_thing_at((byte)x,(byte)y);
  if(i<MAX_N_O_THINGS) return things[i].type;
  /// nope, therefore use the static map.
  switch(pgm_read_byte(&levels[actor.current_level][y][x])) {
  case '#': return WALL;
  case ' ': return DIRT;
  default: return EMPTY;
  }
}

/// In order to initialize given level, we need to set things up:
void initialize_level() {
  byte i,j,next_index=0;
  actor.dead=0;
  actor.niderite_left=0;
  actor.keys=0;
  /// actually we don't need to clean this...
  for(i=0;i<MAX_N_O_THINGS;i++) things[i].type=EMPTY;
  /// we'll make the actor our first thing; facing down:
  things[next_index].type=ACTOR; things[next_index++].dir=dDOWN;
  for(i=0;i<MAP_W;i++)
    for(j=0;j<MAP_H;j++) {
      things[next_index].x=i; things[next_index].y=j;
      things[next_index].dir=dCENTER;
      things[next_index].type=ANT; // !!
      switch(pgm_read_byte(&levels[actor.current_level][j][i])) {
      case 'O':
	things[next_index++].type=NIDERITE;
	actor.niderite_left++;
	break;
      case 's':	things[0].x=i; things[0].y=j; break;
      case 'k': things[next_index++].type=KEY; break;
      case 'l': things[next_index++].type=LOCK; break;
      case '^': things[next_index++].dir=dUP; break;
      case 'v': things[next_index++].dir=dDOWN; break;
      case '<': things[next_index++].dir=dLEFT; break;
      case '>': things[next_index++].dir=dRIGHT; break;
      }
    }
  things[next_index].type=EMPTY; /// clean up.
}


/// THE GAME'S MECHANICS /////////////////////////////////////////

/// Here are the possible interactions between objects.
/// Their names as 5char so that they fit nicely into the table;
/// you'll see. The two arguments are indexes in things[] array.
/// Sometimes the second argument is ignored; eg no wall has its
/// entry in things[]; bumping at wall modifies the thing which
/// hit the wall (ie the active thing).

/// do nothing.
void _nthg(byte active, byte passive) { /* pfff. */ }
/// revert the active one's direction.
void _bump(byte active, byte passive) {
  things[active].dir=bounce_dir(things[active].dir);
}
/// bump both.
void _bbum(byte active, byte passive) {
  things[active].dir=bounce_dir(things[active].dir);
  things[passive].dir=bounce_dir(things[passive].dir);
}
/// make the active stop.
void _stop(byte active, byte passive) {
  things[active].dir=dCENTER;
}
/// make the passive move.
void _push(byte active, byte passive) {
  things[passive].dir=things[active].dir;
  TV.tone(440,200); /// :D
}
/// anihillate both.
void _anhl(byte active, byte passive) {
  things[passive].type=things[active].type=EMPTY;
  actor.niderite_left-=2;
  TV.tone(880,300);
}
/// kill the actor.
void _die(byte active, byte passive) {
  actor.dead=1;
  TV.tone(220,666);
}
/// kill the passive.
void _kill(byte active, byte passive) {
  things[active].dir=bounce_dir(things[active].dir);
  things[passive].type=EMPTY;
  TV.tone(440,300);
}
/// try to open the door.
void _open(byte active, byte passive) {
  if(actor.keys) {
    actor.keys--; things[passive].type=EMPTY;
    TV.tone(440,200);
  } else TV.tone(220,200);
}
/// pick the key. [make sure it's not 5th one... nvm]
void _pick(byte active, byte passive) {
  actor.keys++; things[passive].type=EMPTY; TV.tone(440,200);
}

/// Now this thing is way more readable when initialized like:
///  collision[ACTOR][DOOR] = _open; (...)
/// BUT it then consumes our precious SRAM, so instead we
/// construct this funny table. rows are "active", cols "passive":
typedef void (*collision_proc)(byte,byte);

PROGMEM const collision_proc collision[N_O_TYPES][N_O_TYPES] = {
  // pass. >  EMP.  DIRT  WALL  KEY   LOCK  NID.  ANT   ACTOR
  // v act. 
  /*EMPTY*/ {_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg },
  /*DIRT */ {_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg },
  /*WALL */ {_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg },
  /*KEY  */ {_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg },
  /*LOCK */ {_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg,_nthg },
  /*NID. */ {_stop,_stop,_stop,_stop,_stop,_anhl,_kill,_stop },
  /*ANT  */ {_bump,_bump,_bump,_bump,_bump,_bump,_bbum,_die  },
  /*ACTOR*/ {_nthg,_nthg,_nthg,_pick,_open,_push,_die ,_nthg }
};
/// Actually we could ask for collision[things[i].type-4] and
/// drop the first 5 rows. But flash is cheap, and maybe we'd
/// like the keys to be "pushable", rather than "pickable", so
/// that one would have to push the key into the door?

void game_cycle() {
  byte i,nx,ny,t;
  for(i=0;i<MAX_N_O_THINGS;i++) {
    if(things[i].type==EMPTY) continue;
    if(things[i].dir==dCENTER) continue;
    if(i==0) /// the actor.
       if(the_joystick.dir!=dCENTER)
	things[0].dir=the_joystick.dir;
        else continue; /// no move this time.
    nx=things[i].x; ny=things[i].y;
    switch(things[i].dir) {
    case dUP: ny--; break;
    case dDOWN: ny++; break;
    case dLEFT: nx--; break;
    case dRIGHT: nx++; break;
    }
    t=read_map_at((char)nx,(char)ny);
    if(t==EMPTY) { /// clear, move on.
      things[i].x=nx; things[i].y=ny;      
    } else
      ((collision_proc)
        pgm_read_word(&collision[things[i].type][t]))
	  (i,index_of_thing_at(nx,ny));
  }
}

/// DISPLAY //////////////////////////////////////////////////////

/// Now the sprites...
enum {S_EMPTY,
      S_DIRT,
      S_WALL,
      S_KEY,
      S_LOCK,
      S_NIDERITE,S_NIDERITE2, /// NIDERITE has a "blink"...
      S_ANT_U,S_ANT_D,S_ANT_L,S_ANT_R,
      S_ANT_U2,S_ANT_D2,S_ANT_L2,S_ANT_R2, /// "animation frame"
      S_ACTOR_U,S_ACTOR_D,S_ACTOR_L,S_ACTOR_R,
      S_ACTOR_U2,S_ACTOR_D2,S_ACTOR_L2,S_ACTOR_R2, /// as above
      S_ACTOR_DEAD,S_ACTOR_DEAD2,
      S_HEART, /// for statusbar.
      N_O_SPRITES};

/// I have prepared awesome 16x16 sprites, but they turned out
/// too big -- the viewport 7x5 tiles was too small to play
/// comfortably... So yup 8x8...
#define SPRITE_H 8
#define SPRITE_W 8
extern const byte sprite[N_O_SPRITES][SPRITE_H+2];

struct {
  byte frame : 1; /// to animate ant's/actor's legs etc.
  byte courtain : 4; /// cf FADE_IN/OUTs...
} display_state = {0,0};

#define MAX_COURTAIN 13 /// the height of viewport +2 for cooler
                        /// FADE_IN/OUTs timing.

void display_board() {
  char i,j;
  byte x=0,y=0,spr,ind;  
  TV.delay_frame(1); /// less flickering...
  for(j=things[0].y-5;j<=things[0].y+5;j++) { /// "5" hehe,
    for(i=things[0].x-7;i<=things[0].x+7;i++) { /// "7" hehehe.
      switch(read_map_at(i,j)) {
      case WALL: spr=S_WALL; break;
      case DIRT: spr=S_DIRT; break;
      case KEY: spr=S_KEY; break;
      case LOCK: spr=S_LOCK; break;
      
      case NIDERITE:
        spr=(random()%13?S_NIDERITE:S_NIDERITE2); break;

      case ANT:
	ind=index_of_thing_at((byte)i,(byte)j);
	switch(things[ind].dir) {
	case dRIGHT: spr=S_ANT_R; break;
	case dLEFT: spr=S_ANT_L; break;
	case dDOWN: spr=S_ANT_D; break;
	default: spr=S_ANT_U;
	}
	if(display_state.frame) spr+=4; /// "animate"
	break;

      case ACTOR:
	ind=0; /// we already know that...
	switch(things[ind].dir) {
	case dRIGHT: spr=S_ACTOR_R; break;
	case dLEFT: spr=S_ACTOR_L; break;
	case dDOWN: spr=S_ACTOR_D; break;
	default: spr=S_ACTOR_U;
	}
	if(display_state.frame && the_joystick.dir!=dCENTER)
	  spr+=4; /// "animate", but only when moving...
        if(actor.dead) { /// WAIT! what if the actor is dead?
          spr=(random()%2?S_ACTOR_DEAD:S_ACTOR_DEAD2);
        }
	break;

        default:spr=S_EMPTY; /// nothing in here.
      }
      if(y<display_state.courtain) spr=S_EMPTY; /// theatrical fade in/out.
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[spr]);
      x++;
    }
    x=0; y++;
  }
  /// -- statusbar --
  for(i=0;i<=MAX_LIVES;i++) {
    spr=S_EMPTY;
    if(actor.lives>i) spr=S_HEART;
    if(actor.dead && actor.lives==i && display_state.frame)
      spr=S_HEART; /// blinking heart...
    TV.bitmap(i*(SPRITE_W+1),88,sprite[spr]);
  }
  for(i=0;i<MAX_KEYS;i++) {
    spr=((actor.keys>i)?S_KEY:S_EMPTY);
    TV.bitmap(111-(i*(SPRITE_W+1)),88,sprite[spr]);
  }
  /// flip the frame...
  display_state.frame=1-display_state.frame;
  /// boom done.
}

void animate_gameover() {
  byte y;
  TV.delay_frame(1);
  TV.select_font(font8x8);
  TV.clear_screen();
  for(y=0;y<40;y++) {
    TV.print(24,y,"GAME OVER");
    TV.print(24,0,"         "); /// looks cooler.
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

void animate_victoly() {
  byte t,y=0;
  TV.delay_frame(1);
  TV.select_font(font8x8);
  TV.clear_screen();
  TV.tone(220); /// ?!
  for(t=0;t<200;t++) {
    TV.print(24,y,"VICTOLY!");
    TV.delay(23);
    if(t<80) y++;
    else if(t<130) y--; /// make it leave a juicy trace!
    else if(t<150) y++;
    else if(t<160) y--;
    else if(t<180) y++;
      else y++;
  }
  TV.delay(2323); /// ok, enough. we're done...
  TV.noTone();
  TV.clear_screen(); //  go home or play again.
}

void display_d9k_logo(); /// a bit narcisstic, no?

/// THE MAIN LOOP ////////////////////////////////////////////////

byte game_state=0;
enum{PLAY, /// the game proper.
     TITLE, /// a cool title screen, wait for button...
     GAME_OVER, /// ran out of lives
     WON, /// passed all levels
     FADE_IN, /// animation showing new level,
     FADE_OUT_DEATH, /// level disappears, to restart
     FADE_OUT_LEVELUP /// level disappeas, to load new one
};


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
#define FASTER_TIME_STEP 42 /// oh, The Answer.

void loop() {
  long then,now;

  switch(game_state) {

  case WON:
    animate_victoly();
    game_state=TITLE;
    break;

    break;

  case TITLE: 
    display_title_screen();
    if(the_joystick.dir!=dCENTER) {
      actor.current_level=0;
      actor.lives=MAX_LIVES;
      initialize_level();
      game_state=FADE_IN;
      display_state.courtain=MAX_COURTAIN;
      TV.clear_screen(); /// !!!!
    }
    break;

  case GAME_OVER:
    animate_gameover();
    game_state=TITLE;
    break;

  case FADE_OUT_DEATH:
    display_board();
    if(++display_state.courtain>=MAX_COURTAIN) {
      if(actor.lives==0) game_state=GAME_OVER;
      else {
	initialize_level();
	game_state=FADE_IN;
      }
    }
    break;

  case FADE_OUT_LEVELUP:
    display_board();
    if(++display_state.courtain>=MAX_COURTAIN) {
        if(actor.current_level++>=N_O_LEVELS) game_state=WON;
          else {
  	    initialize_level();
	    game_state=FADE_IN;
          }
    }
    break;

  case FADE_IN:
    display_board();
    if(--display_state.courtain<=0) game_state=PLAY;
    break;

  case PLAY:
    display_board();
    game_cycle();
    if(actor.niderite_left<=0)  game_state=FADE_OUT_LEVELUP;
    if(actor.dead) {actor.lives--; game_state=FADE_OUT_DEATH;}
    break;

  }
  reset_joystick();
  /// wait till the end of this cycle.
  then=now=TV.millis();
  while(now-then<(game_state==PLAY?TIME_STEP:FASTER_TIME_STEP)) {
    refresh_joystick(); /// controls can be altered at any moment
    now=TV.millis();
  }
}


/// THE DATA (SPRITES AND MAPS) //////////////////////////////////

PROGMEM const byte levels[N_O_LEVELS][MAP_H][MAP_W+1] = {
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
   "  ###l###########l###  ",
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
   " #^..####..O..####...# ",
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
   " #..........v..# ###..#",
   " #.#####OO##.#l# #...O#",
   " #.#   #^^##.#.###.O###",
   " #.#########.#......#  ",
   " #^..........########  ",
   " #.###########         ",
   " #.#                   ",
   " #.#############       ",
   " #............k#       ",
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
    /// TODO TODO TODO?
};



PROGMEM const byte sprite[N_O_SPRITES][SPRITE_H+2] = {
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
  },

  /// S_HEART
  {SPRITE_W,SPRITE_H,
   0b00000000,
   0b00100010,
   0b01110111,
   0b01111111,
   0b01111111,
   0b00111110,
   0b00011100,
   0b00001000
  }
};

/// sorry, I had to:
PROGMEM const byte dercz9000[] = {
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
  delay(2012);
  TV.tone(128,1000);
  delay(1000);
  TV.tone(256,1000);  
  delay(1000);  
  TV.tone(384,1000);
  delay(1000);  
  TV.tone(512,900);
}

/// the end.
