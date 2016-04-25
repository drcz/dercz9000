//////////////////////////////////////////////////////////////////
///                           it's...
/// 
///    the 3 Rules of 2-Color Shapeshiftin' in a Microworld
///                 on a Non-Flickering Torus
///
/// an adaptation of my Ludum Dare 35 game for dercz9000 console.
/// -- check out the original (my first LD, made in js in 48h):
/// ludumdare.com/compo/ludum-dare-35/?action=preview&uid=88774
///
///             Alicante 2016.04.23-25, drcz@tlen.pl
//////////////////////////////////////////////////////////////////

/// The following code is a collage of D9K-the_pchanina.ino
/// excerpts, with some ruby generated code, with a grain of
/// cheap tricks. At the moment it's great mess.

/// todo get rid of ~1K of memory usage... [is it possible?]

/// todo cleanup


/// THE drcz9000 VIDEO CONSOLE ///////////////////////////////////
#include<string.h>
/// flash memory access
#define FLASH(X) pgm_read_byte(&X)
#define FLASHW(X) pgm_read_word(&X)

/// The "console" utilizes the divine Myles Metzer's TVout library
/// http://playground.arduino.cc/Main/TVout
#include <TVout.h>
#include <fontALL.h> /// TODO raczej swoye i w inwersyji
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
/// direction/facing/sth (fits 3b anyway)
enum {dUP,dLEFT,dDOWN,dRIGHT,dUP2,dDOWN2,dCENTER};

#define BITS_FOR_X 6
#define BITS_FOR_Y 5
#define BITS_FOR_TYPE 5
#define BITS_FOR_TELEPIPES 3
#define MAX_TELEPIPES 4
#define BITS_FOR_GUNS 2
#define MAX_GUNS 4
#define MAX_THINGS 80 // ? <=57 + all the projectiles and booms.
#define MAX_MESSAGE 15

#define MAX_LEVELS 5 // !!

struct {
  byte dir : 3;
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
  long push_time;
} joystick = {dCENTER,0,0,dCENTER};

/// minimum number of ms for a push to be approved:
#define MIN_PUSH_TIME 13 // does it even make any sense?

/// this one we will call as often as possible:
void refresh_joystick() {
  joystick.reads.bits=0; /// clear them all...
  /// modify this to use your controller (a real joystick?)
  if(digitalRead(BTN_UP)) joystick.reads.up=1;
  else if(digitalRead(BTN_LEFT)) joystick.reads.left=1;
  else if(digitalRead(BTN_DOWN)) joystick.reads.down=1;
  else if(digitalRead(BTN_RIGHT)) joystick.reads.right=1;
  /// \modify
  
  if(joystick.last_reads.bits==0) {
    /// waiting for push...
    if(joystick.reads.bits>0) { /// button pushed!
      joystick.push_time=TV.millis();
      joystick.last_reads.bits=joystick.reads.bits;
    }
  } else {
    /// waiting for release
    if(joystick.reads.bits==0) { /// there it goes!
      /// did the push last long enough?
      if((TV.millis()-joystick.push_time)>=MIN_PUSH_TIME) {
	if(joystick.last_reads.up) joystick.dir=dUP;
	else if(joystick.last_reads.left) joystick.dir=dLEFT;
	else if(joystick.last_reads.down) joystick.dir=dDOWN;
	else if(joystick.last_reads.right) joystick.dir=dRIGHT;
      }
      joystick.last_reads.bits=0;
    }
  } /// NB we disallow "diagonal" moves (eg left+up).
}

/// ...so after it's finally used as input, we need to reset it:
void reset_joystick() {
  joystick.dir=dCENTER;
  //joystick.last_reads.bits=0; joystick.push_time=0;
}


/// THE GAME'S WORLD /////////////////////////////////////////////

/// static level data (flash)
extern const struct Level_static {

  char *name;

  //byte n_o_guns : BITS_FOR_GUNS;
  struct {
    byte x ;//: BITS_FOR_X;
    byte y ;//: BITS_FOR_Y;
    byte dir ;//: 2; // not necessary, but for alingment, sth sth
    byte init_count ;// : 2; // !
    byte max_count ;//: 3;  // !
  } gun[MAX_GUNS];

  //byte n_o_telepipes : BITS_FOR_TELEPIPES;
  struct {
    byte x ;//: BITS_FOR_X;
    byte y ;//: BITS_FOR_Y;
    byte dest_x ;//: BITS_FOR_X;
    byte dest_y ;//: BITS_FOR_Y;
    byte dir ;//: 2;
  } telepipe[MAX_TELEPIPES];

  //byte n_o_turncocks : 2; // =< n_o_telepipes/2
  struct {
    byte x ;//: BITS_FOR_X;
    byte y ;//: BITS_FOR_Y;
    byte end1 ;//: BITS_FOR_TELEPIPES;
    byte end2 ;//: BITS_FOR_TELEPIPES;
  } turncock[MAX_TELEPIPES/2];

  byte w ;//: BITS_FOR_X;
  byte h ;//: BITS_FOR_Y;
  char *map[18];

} levels[];

/// dynamic levels data (sram) 
struct Level_dynamic {

  byte index : 4;
  byte w : BITS_FOR_X;
  byte h : BITS_FOR_Y;

  struct {
    byte count : 3;
  } gun[MAX_GUNS];

  struct {
    byte is_open : 1;
  } telepipe[MAX_TELEPIPES];

  struct {
    byte x : BITS_FOR_X;
    byte y : BITS_FOR_Y;
    byte type : BITS_FOR_TYPE;
  } thing[MAX_THINGS];

  byte lives : 3;
  byte keys : 2;

  byte squares_left : 4;
  byte triangles_left : 4;
  byte discs_left : 4;

  byte game_state : 3;

  byte disp_courtain : 4;
  byte anim_frame : 1;
  byte actor_frame : 1; /// as it blinks randomly...
  byte actor_facing : 3;  /// because of dUP2 an dDOWN2...
  byte cx : BITS_FOR_X;
  byte cy : BITS_FOR_Y;

  struct {
    char text[MAX_MESSAGE];
    byte length;
    byte expires;
  } message;

} cur_level;

/// game states:
enum{TITLE,FADE_IN,PLAY,FADE_OUT_DEATH,FADE_OUT_LEVELUP,GAME_OVER,VICTOLY};


/// Types of things inhabiting the game's world:
enum {
  NOTHING,
  BOOM,
  ACTOR_SQUARE,
  ACTOR_TRIANGLE,
  ACTOR_DISC,
  SQUARE,
  TRIANGLE,
  DISC,
  HOLE_SQUARE,
  HOLE_TRIANGLE,
  HOLE_DISC,
  PROJECTILE_U,
  PROJECTILE_D,
  PROJECTILE_L,
  PROJECTILE_R,
  KEY,
  DOOR,
  MACHINE,
  PIPE_H,
  PIPE_V,
  TURNCOCK_H,
  TURNCOCK_V,
  GUN_U,
  GUN_D,
  GUN_L,
  GUN_R,
  PIPE_DL,
  PIPE_DR,
  PIPE_UL,
  PIPE_UR,
  WALL,
  MAX_TYPES};

/// simpified types, for collision lookups:
enum {
  ST_NOT_ACTIVE,
  ST_ACTOR_SQUARE,
  ST_ACTOR_TRIANGLE,
  ST_ACTOR_DISC,
  ST_SQUARE,
  ST_TRIANGLE,
  ST_DISC,
  ST_HOLE_SQUARE,
  ST_HOLE_TRIANGLE,
  ST_HOLE_DISC,
  ST_PROJECTILE,
  ST_KEY,
  ST_DOOR,
  ST_PIPE,
  ST_TURNCOCK,
  ST_MACHINE,
  ST_MAX
};

inline byte simple_type_for(byte type) {
  switch(type) {
  case ACTOR_SQUARE : return(ST_ACTOR_SQUARE);
  case ACTOR_TRIANGLE : return(ST_ACTOR_TRIANGLE);
  case ACTOR_DISC : return(ST_ACTOR_DISC);
  case SQUARE : return(ST_SQUARE);
  case TRIANGLE : return(ST_TRIANGLE);
  case DISC : return(ST_DISC);
  case HOLE_SQUARE : return(ST_HOLE_SQUARE);
  case HOLE_TRIANGLE : return(ST_HOLE_TRIANGLE);
  case HOLE_DISC : return(ST_HOLE_DISC);
  case PROJECTILE_U : return(ST_PROJECTILE);
  case PROJECTILE_D : return(ST_PROJECTILE);
  case PROJECTILE_L : return(ST_PROJECTILE);
  case PROJECTILE_R : return(ST_PROJECTILE);
  case KEY : return(ST_KEY);
  case DOOR : return(ST_DOOR);
  case PIPE_H : return(ST_PIPE);
  case PIPE_V : return(ST_PIPE);
  case TURNCOCK_H : return(ST_TURNCOCK);
  case TURNCOCK_V : return(ST_TURNCOCK);
  case MACHINE: return(ST_MACHINE);
  default: return(ST_NOT_ACTIVE);
  }
}

//////////////////////////////////////////////////////////
/// THE MAP
//////////////////////////////////////////////////////////

byte index_of_thing_at(byte x,byte y) {
  byte i;  
  for(i=0;i<MAX_THINGS;i++) {
    if(cur_level.thing[i].type==NOTHING) continue;
    if(cur_level.thing[i].x==x
       && cur_level.thing[i].y==y) break;
  }
  return(i);
}

/// this time read_map does not include the actor.
/// notice x and y are signed here.
byte read_map(char x,char y) {
  byte i;
  // normalize the position (we are on torus, y'know)
  while(x<0) x+=cur_level.w;
  while(x>=cur_level.w) x-=cur_level.w;
  while(y<0) y+=cur_level.h;
  while(y>=cur_level.h) y-=cur_level.h;
  // check dynamic things
  i=index_of_thing_at(x,y);
  if(i<MAX_THINGS) return(cur_level.thing[i].type);
  // if none found, check the static map
  switch(FLASH(levels[cur_level.index].map[y][x])) {
  case '#': return(WALL);
  case '-': return(PIPE_H);
  case '|': return(PIPE_V);
  case 'L': return(PIPE_UR);
  case 'J': return(PIPE_UL);
  case '7': return(PIPE_DL);
  case 'T': return(PIPE_DR);
  case 'h': return(TURNCOCK_H);
  case 'v': return(TURNCOCK_V);
  case 'd': return(GUN_D);
  case 'u': return(GUN_U);
  case 'l': return(GUN_L);
  case 'r': return(GUN_R);
  default: return(NOTHING);
  }
}
////////////////////////////////////////////////////////////
/// messages stuff:
////////////////////////////////////////////////////////////
void set_message(char *msg, byte expires) {
  byte i,l;
  l=(byte)strlen(msg);
  for(i=0;i<l;i++) cur_level.message.text[i]=msg[i];
  for(i=l;i<MAX_MESSAGE;i++) cur_level.message.text[i]=' ';
  cur_level.message.text[l]=0; //?
  cur_level.message.length=l;
  cur_level.message.expires=expires;
}

inline void msg_cool() {
  switch(random()%3) {
  case 0: set_message("GOOD!",2);break;
  case 1: set_message("COOL!",2);break;
  case 2: set_message("well done.",2);break;
  }
  /// todo a tone depending on actor's type?
}

inline void msg_telepipe() {
  switch(random()%3) {
  case 0: set_message("WHOA!",2);break;
  case 1: set_message("WOW!",2);break;
  case 2: set_message("WEE!",2);break;
  }
  /// todo a STRANGE tone!
}

inline void sound_for(byte shape) {
  switch(shape) {
  case ACTOR_SQUARE:
  case HOLE_SQUARE:
  case SQUARE:
    /// TODO
    break;
  case ACTOR_TRIANGLE:
  case HOLE_TRIANGLE:
  case TRIANGLE:
    /// TODO
    break;
  case ACTOR_DISC:
  case HOLE_DISC:
  case DISC:
    /// TODO
    break;
  }
}

////////////////////////////////////////////////////////////
/// things!
////////////////////////////////////////////////////////////

byte first_free_slot() {
  byte i;
  for(i=0;i<MAX_THINGS;i++)
    if(cur_level.thing[i].type==NOTHING) break;
  return(i);
}

void create_thing(byte type,byte x,byte y) {
  byte ind=first_free_slot();
  if(ind<MAX_THINGS) {
    cur_level.thing[ind].type=type;
    cur_level.thing[ind].x=x;
    cur_level.thing[ind].y=y;
  }  /// else, if it's not possible, it's not possible.
}

////////////////////////////////////////////////////////////

/// movement computations on a torus:
inline byte new_x(byte x,byte dir) {
  if(dir==dRIGHT) {
    if(++x>=cur_level.w) x=0;
  } else if(dir==dLEFT) {
    if(x==0) x=cur_level.w;
    x--;
  }
  return(x);
}

inline byte new_y(byte y,byte dir) {
  if(dir==dDOWN) {
    if(++y>=cur_level.h) y=0;
  } else if(dir==dUP) {
    if(y==0) y=cur_level.h;
    y--;
  }
  return(y);
}

inline byte dxdy2dir(char dx,char dy) {
  if(dx<0) return dLEFT;
  if(dx>0) return dRIGHT;
  if(dy<0) return dUP;
  if(dy>0) return dDOWN;
  return dCENTER; /// ?!
}

void move_thing(byte,byte);

inline char d_on_circle(byte a,byte b,byte perim) {
  char d=a-b;
  if(a==0 && b==perim-1) d=1;
  if(b==0 && a==perim-1) d=-1;
  return d;
}

/// possible collisions ///////////////////////////////////

/// nothing happens
void ___n(byte active, byte passive) { /* pfff. */ }

/// actor dies
void _die(byte active, byte passive) {
  if(random()%2) set_message("YOU DIE!",16);
  else  set_message("OOPS... YOU DIE",16);
  /// todo dzwiek
  cur_level.game_state=FADE_OUT_DEATH;
}

/// trying to push (actor only)
void _psh(byte active, byte passive) {
  byte ox,oy;
  char dx,dy;
  dx=d_on_circle(cur_level.thing[passive].x,
		 cur_level.thing[active].x,		 
		 cur_level.w);
  dy=d_on_circle(cur_level.thing[passive].y,
		 cur_level.thing[active].y,
		 cur_level.h);
  ox=cur_level.thing[passive].x;
  oy=cur_level.thing[passive].y;
  move_thing(passive,dxdy2dir(dx,dy));
  if(cur_level.thing[passive].x==ox
     && cur_level.thing[passive].y==oy
     && cur_level.thing[passive].type!=NOTHING) {
    //set_message("can't do that",4); <- bez sensu.
    /// todo czy dzwiek??
  } else {
    cur_level.thing[active].x=ox;
    cur_level.thing[active].y=oy;
    /// todo dzwiek puszu!
  }
}

/// trying to push + shapeshift to triangle (actor only)
void _pst(byte active, byte passive) {
  cur_level.thing[active].type=ACTOR_TRIANGLE;
  set_message("shift!",2);
  _psh(active,passive);
}

/// trying to push + shapeshift to disc (actor only)
void _psd(byte active, byte passive) {
  cur_level.thing[active].type=ACTOR_DISC;
  set_message("shift!",2);
  _psh(active,passive);
}


/// can't unshift (actor only)
void _nus(byte active, byte passive) {
  set_message("can't unshift",5);
  /// todo dzwiek niski
}

/// ushift to square (actor only)
void _ssq(byte active, byte passive) {
  cur_level.thing[passive].type=NOTHING;
  cur_level.thing[active].type=ACTOR_SQUARE;
  set_message("unshift!",2);
  /// todo dzwiek?
}

/// ushift to triangle (actor only)
void _str(byte active, byte passive) {
  cur_level.thing[passive].type=NOTHING;
  cur_level.thing[active].type=ACTOR_TRIANGLE;
  set_message("unshift!",2);
  /// todo dzwiek?
}

/// steping over
void _sto(byte active, byte passive) {
  byte t;
  cur_level.thing[active].x=cur_level.thing[passive].x;
  cur_level.thing[active].y=cur_level.thing[passive].y;
  /// make the active one earlier on 'the things list'
  if(active>passive) {
    t=cur_level.thing[active].type;
    cur_level.thing[active].type=cur_level.thing[passive].type;
    cur_level.thing[passive].type=t;
  }
}

/// picking up (actor only)
void _pck(byte active, byte passive) {
  cur_level.thing[passive].type=NOTHING;
  if(cur_level.keys++==0)
    set_message("got a key now",5);
  else
    set_message("picked a key",5);
    /// todo dzwiek wysoki
}

/// trying to open (actor only)
void _opn(byte active, byte passive) {
  if(cur_level.keys>0) {
    cur_level.keys--;
    cur_level.thing[passive].type=BOOM; // ??
    set_message("door unlocks",4);
    /// todo dzwiek wysoki
  } else {
    set_message("it's locked",4);
    /// todo dzwiek niski
  }
}

/// telepiping maybe?
void _tlp(byte active, byte i) {
  byte x,y,ox,oy;
  if(i>=MAX_TELEPIPES) return;
  /// ok, this is it. but, is it open? [check by i index]
  if(cur_level.telepipe[i].is_open) {
    ox=cur_level.thing[active].x;
    oy=cur_level.thing[active].y;
    x=FLASH(levels[cur_level.index].telepipe[i].x);
    y=FLASH(levels[cur_level.index].telepipe[i].y);
    /// is it from the correct direction???
    if(read_map(x,y)==PIPE_V && y==oy) return;
    if(read_map(x,y)==PIPE_H && x==ox) return;
    x=FLASH(levels[cur_level.index].telepipe[i].dest_x);
    y=FLASH(levels[cur_level.index].telepipe[i].dest_y);
    cur_level.thing[active].x=x;
    cur_level.thing[active].y=y;
    move_thing(active,
	       FLASH(levels[cur_level.index].telepipe[i].dir));
    if(cur_level.thing[active].x==x
       && cur_level.thing[active].y==y) {
      /// get back, turns out you can't go there...
      cur_level.thing[active].x=ox;
      cur_level.thing[active].y=oy;
      /// no sfx/msg in the original for this case!
    } else {
      /// boom done.
      msg_telepipe(); /// sfx in this msg_*
    }
    return;
  } else {
    set_message("find a turncock",5);
    /// todo niski dzwiek
    return;
  }
}

/// turning the turncock
void _trn(byte active, byte i) {
  if(i>=MAX_TELEPIPES/2) return; // not possible?
  cur_level.telepipe[FLASH(levels[cur_level.index].turncock[i].end1)].is_open=1;
  cur_level.telepipe[FLASH(levels[cur_level.index].turncock[i].end2)].is_open=1;
  set_message("turncock turned",4);
}

/// boom (active explodes)
void _bma(byte active, byte passive) {
  cur_level.thing[active].type=BOOM;
}

/// boom (passive explodes)
void _bmp(byte active, byte passive) {
  cur_level.thing[passive].type=BOOM;
}

/// boom (both explode)
void _bmb(byte active, byte passive) {
  cur_level.thing[active].type=BOOM;
  cur_level.thing[passive].type=NOTHING; //?
}

/// annihilate square
void _ans(byte active, byte passive) {
  cur_level.thing[active].type=BOOM;
  cur_level.thing[passive].type=BOOM;
  cur_level.squares_left--;
  msg_cool();
}
/// annihilate triangle
void _ant(byte active, byte passive) {
  cur_level.thing[active].type=BOOM;
  cur_level.thing[passive].type=BOOM;
  cur_level.triangles_left--;
  msg_cool();
}
/// annihilate disk
void _and(byte active, byte passive) {
  cur_level.thing[active].type=BOOM;
  cur_level.thing[passive].type=BOOM;
  cur_level.discs_left--;
  msg_cool();
}



typedef void (*collision_proc)(byte,byte);
PROGMEM const collision_proc collision[ST_MAX][ST_MAX] = {
  // pass. >  N-A  ASQ  ATR  ADI  SQ   TR   DI   HSQ  HTR  HDI  PRJ  KEY  DOR  PIP  TRN MCH
  // V act.
  /*N-A*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,___n },
  /*ASQ*/ { ___n,___n,___n,___n,_psh,_pst,_psd,_die,_sto,_sto,_die,_pck,_opn,_tlp,_trn,_nus },
  /*ATR*/ { ___n,___n,___n,___n,___n,_psh,_psd,_sto,_die,_sto,_die,_pck,_opn,_tlp,_trn,_ssq },
  /*ADI*/ { ___n,___n,___n,___n,___n,___n,_psh,_sto,_sto,_die,_die,_pck,_opn,_tlp,_trn,_str },
  /*SQ */ { ___n,___n,___n,___n,___n,___n,___n,_ans,_sto,_sto,_bmp,___n,___n,_tlp,___n,___n },
  /*TR */ { ___n,___n,___n,___n,___n,___n,___n,_sto,_ant,_sto,_bmp,___n,___n,_tlp,___n,___n },
  /*DI */ { ___n,___n,___n,___n,___n,___n,___n,_sto,_sto,_and,_bmp,___n,___n,_tlp,___n,___n },
  /*HSQ*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*HTR*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*HDI*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*PRJ*/ { _bma,_die,_die,_die,_bma,_bma,_bma,_bma,_bma,_bma,_bmb,_bma,_bma,_bma,_bma,_bma },
  /*KEY*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*DOR*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*PIP*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*TRN*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
  /*MCH*/ { ___n,___n,___n,___n,___n,___n,___n,___n,___n,___n,_bmp,___n,___n,___n,___n,___n },
};


void move_thing(byte ind, byte dir) {
  byte x,y,t,i,ind2;
  x=new_x(cur_level.thing[ind].x,dir);
  y=new_y(cur_level.thing[ind].y,dir);
  if((t=read_map(x,y))==NOTHING) {
    /// nothing in there, go on!
    cur_level.thing[ind].x=x;
    cur_level.thing[ind].y=y;
  } else {
    ind2=index_of_thing_at(x,y);
    if(ind2==MAX_THINGS) {
    // the whole elegance is fucked because of telepipes and turncocks.
      /// IF it's a telepipe or a turncock, then set ind2 to its index.
      switch(t) {
      case PIPE_H:
      case PIPE_V:
	for(i=0;i<MAX_TELEPIPES;i++) {
	  if(x==FLASH(levels[cur_level.index].telepipe[i].x)
	     && y==FLASH(levels[cur_level.index].telepipe[i].y)) {
	    ind2=i;
	    break;
	  }
	}
	break;
      case TURNCOCK_H:
      case TURNCOCK_V:
	for(i=0;i<MAX_TELEPIPES/2;i++) {
	  if(x==FLASH(levels[cur_level.index].turncock[i].x)
	     && y==FLASH(levels[cur_level.index].turncock[i].y)) {
	    ind2=i;
	    break;
	  }
	}
	break;
      }
      y=simple_type_for(t);
    } else {
      y=simple_type_for(cur_level.thing[ind2].type);
    }
    x=simple_type_for(cur_level.thing[ind].type);
    ((collision_proc) FLASHW(collision[x][y]))(ind,ind2);
  }
}

/// fancy rotations, who cares...
byte new_facing(byte old_facing, byte dir) {
  switch(dir) {
  case dRIGHT:
  case dLEFT:
    return dir;
  case dUP:
    switch(old_facing) {
    case dLEFT:
    case dDOWN:
    case dUP2:
      return dUP2;
    default:
      return dUP;
    }
  case dDOWN:
    switch(old_facing) {
    case dLEFT:
    case dDOWN2:
    case dUP:
      return dDOWN2;
    default:
      return dDOWN;
    }
  }
}

/// the heart of it all:
void world_step() {
  byte i,j;
  for(i=0;i<MAX_THINGS;i++) {
    switch(cur_level.thing[i].type) {

    case ACTOR_SQUARE:
    case ACTOR_TRIANGLE:
    case ACTOR_DISC:
      if(joystick.dir!=dCENTER) {
	cur_level.actor_facing=new_facing(cur_level.actor_facing,joystick.dir);
	move_thing(i,joystick.dir);
	/// center the viewport around the actor..
	cur_level.cx=cur_level.thing[i].x;
	cur_level.cy=cur_level.thing[i].y;
	/// todo: dzwiek?
      }
      break;

    case PROJECTILE_R:
      move_thing(i,dRIGHT);
      break;
    case PROJECTILE_L:
      move_thing(i,dLEFT);
      break;
    case PROJECTILE_U:
      move_thing(i,dUP);
      break;
    case PROJECTILE_D:
      move_thing(i,dDOWN);
      break;

    case BOOM:
      /// just disappear.
      cur_level.thing[i].type=NOTHING;
      break;   
    }
  }

  // and now the guns...
  for(i=0;i<MAX_GUNS;i++) {
    if(FLASH(levels[cur_level.index].gun[i].max_count==0)) continue;
    ///
    if(++cur_level.gun[i].count>=FLASH(levels[cur_level.index].gun[i].max_count)) {
      //set_message("POW!",2);
      cur_level.gun[i].count=0;
      j=first_free_slot(); /// this will be the index of new projectile.
      if(j<MAX_THINGS) { /// don't bother if there's no place for one...
	switch(FLASH(levels[cur_level.index].gun[i].dir)) {
	case dUP:
	  create_thing(PROJECTILE_U,
		       FLASH(levels[cur_level.index].gun[i].x),
		       FLASH(levels[cur_level.index].gun[i].y));
	  break;
	case dDOWN:
	  create_thing(PROJECTILE_D,
		       FLASH(levels[cur_level.index].gun[i].x),
		       FLASH(levels[cur_level.index].gun[i].y));
	  break;
	case dRIGHT:
	  create_thing(PROJECTILE_R,
		       FLASH(levels[cur_level.index].gun[i].x),
		       FLASH(levels[cur_level.index].gun[i].y));
	  break;
	case dLEFT:
	  create_thing(PROJECTILE_L,
		       FLASH(levels[cur_level.index].gun[i].x),
		       FLASH(levels[cur_level.index].gun[i].y));
	  break;
	}
	/// make it's first step NOW.
	move_thing(j,FLASH(levels[cur_level.index].gun[i].dir));
      }
    }
  }
}

///////////////////////////////////
/// acha! jeszcze level!
void initialize_level(byte n) {
  byte i,j;
  cur_level.index=n;
  cur_level.w=FLASH(levels[n].w);
  cur_level.h=FLASH(levels[n].h);

  cur_level.keys=0;
  cur_level.squares_left=0;
  cur_level.triangles_left=0;
  cur_level.discs_left=0;

  for(i=0;i<MAX_THINGS;i++) cur_level.thing[i].type=NOTHING;

  /// set guns' timers
  for(i=0;i<MAX_GUNS;i++)
    cur_level.gun[i].count=FLASH(levels[n].gun[i].init_count);
  /// set all telepipes as open....
  for(i=0;i<MAX_TELEPIPES;i++) {
    cur_level.telepipe[i].is_open=1;
  }
  /// ...and close the ones having turncocks
  for(i=0;i<MAX_TELEPIPES/2;i++) {
    cur_level.telepipe[FLASH(levels[n].turncock[i].end1)].is_open=0;
    cur_level.telepipe[FLASH(levels[n].turncock[i].end2)].is_open=0;
  }
  /// now set up the things
  for(j=0;j<cur_level.h;j++)
    for(i=0;i<cur_level.w;i++)
      switch(FLASH(levels[n].map[j][i])) {
      case ';': create_thing(KEY,i,j); break;
      case 'I': create_thing(DOOR,i,j); break;
      case 'M': create_thing(MACHINE,i,j); break;
      case '[': create_thing(HOLE_SQUARE,i,j); break;
      case '<': create_thing(HOLE_TRIANGLE,i,j); break;
      case '(': create_thing(HOLE_DISC,i,j); break;
      case 'H':
	cur_level.squares_left++;
	create_thing(SQUARE,i,j);
	break;
      case 'V':
	cur_level.triangles_left++;
	create_thing(TRIANGLE,i,j);
	break;
      case 'O':
	cur_level.discs_left++;
	create_thing(DISC,i,j);
	break;
      case '&':
	create_thing(ACTOR_SQUARE,i,j);
	cur_level.cx=i;
	cur_level.cy=j;
	break;
      default:
	break;
      }
  /// something something.
  cur_level.actor_facing=dLEFT;
  set_message((char *)FLASH(levels[n].name),13);
  /// boom done?
}


/// DISPLAY //////////////////////////////////////////////////////

/// Now the sprites...
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

#define SPRITE_H 8
#define SPRITE_W 8
extern const byte sprite[N_O_SPRITES][SPRITE_H+2];

#define MAX_COURTAIN 12


void draw_board(byte cx,byte cy,byte courtain) {
  byte x=0,y=0,spr;
  char i,j;
  cur_level.actor_frame=(random()%5==3);
  for(j=cy-5;j<=cy+5;j++) {
    for(i=cx-7;i<=cx+7;i++) {
      if(y>=courtain) spr=S_BLANK;
      else
      switch(read_map(i,j)) {
      case NOTHING: spr=S_BLANK; break;

      case WALL: spr=S_WALL; break;

      case HOLE_SQUARE: spr=S_HOLE_SQUARE; break;
      case HOLE_TRIANGLE: spr=S_HOLE_TRIANGLE; break;
      case HOLE_DISC: spr=S_HOLE_DISC; break;
      case PIPE_V: spr=S_PIPE_V; break;
      case PIPE_H: spr=S_PIPE_H; break;
      case PIPE_DL: spr=S_PIPE_DL; break;
      case PIPE_UR: spr=S_PIPE_UR; break;
      case PIPE_DR: spr=S_PIPE_DR; break;
      case PIPE_UL: spr=S_PIPE_UL; break;

      case KEY: spr=S_KEY; break;
      case DOOR: spr=S_DOOR; break;
      case MACHINE: spr=S_MACHINE; break;

      case TURNCOCK_V: spr=S_TURNCOCK_V; break;
      case TURNCOCK_H: spr=S_TURNCOCK_H; break;

      case PROJECTILE_L:
      case PROJECTILE_U:
      case PROJECTILE_D:
      case PROJECTILE_R:
	spr=(cur_level.anim_frame?S_PARTICLE_V:S_PARTICLE_H);
	break;

      case GUN_D: spr=S_GUN_DOWN; break;
      case GUN_U: spr=S_GUN_UP; break;
      case GUN_L: spr=S_GUN_LEFT; break;
      case GUN_R: spr=S_GUN_RIGHT; break;

      case BOOM: spr=S_BOOM; break;

      case SQUARE:
	spr=(cur_level.anim_frame?S_SQUARE:S_SQUARE2);
	break;
      case TRIANGLE:
	spr=(cur_level.anim_frame?S_TRIANGLE:S_TRIANGLE2);
	break;
      case DISC:
	spr=(cur_level.anim_frame?S_DISC:S_DISC2);
	break;

      case ACTOR_SQUARE:
	switch(cur_level.actor_facing) {
	case dUP:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_UP:S_HERO_SQUARE2_UP);
	  break;
	case dUP2:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_UP2:S_HERO_SQUARE2_UP2);
	  break;
	case dDOWN:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_DOWN:S_HERO_SQUARE2_DOWN);
	  break;
	case dDOWN2:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_DOWN2:S_HERO_SQUARE2_DOWN2);
	  break;
	case dRIGHT:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_RIGHT:S_HERO_SQUARE2_RIGHT);
	  break;
	case dLEFT:
	  spr=(cur_level.actor_frame?S_HERO_SQUARE_LEFT:S_HERO_SQUARE2_LEFT);
	  break;
	}
	break;

      case ACTOR_TRIANGLE:
	switch(cur_level.actor_facing) {
	case dUP:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_UP:S_HERO_TRIANGLE2_UP);
	  break;
	case dUP2:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_UP2:S_HERO_TRIANGLE2_UP2);
	  break;
	case dDOWN:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_DOWN:S_HERO_TRIANGLE2_DOWN);
	  break;
	case dDOWN2:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_DOWN2:S_HERO_TRIANGLE2_DOWN2);
	  break;
	case dRIGHT:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_RIGHT:S_HERO_TRIANGLE2_RIGHT);
	  break;
	case dLEFT:
	  spr=(cur_level.actor_frame?S_HERO_TRIANGLE_LEFT:S_HERO_TRIANGLE2_LEFT);
	  break;
	}
	break;

      case ACTOR_DISC:
	switch(cur_level.actor_facing) {
	case dUP:
	  spr=(cur_level.actor_frame?S_HERO_DISC_UP:S_HERO_DISC2_UP);
	  break;
	case dUP2:
	  spr=(cur_level.actor_frame?S_HERO_DISC_UP2:S_HERO_DISC2_UP2);
	  break;
	case dDOWN:
	  spr=(cur_level.actor_frame?S_HERO_DISC_DOWN:S_HERO_DISC2_DOWN);
	  break;
	case dDOWN2:
	  spr=(cur_level.actor_frame?S_HERO_DISC_DOWN2:S_HERO_DISC2_DOWN2);
	  break;
	case dRIGHT:
	  spr=(cur_level.actor_frame?S_HERO_DISC_RIGHT:S_HERO_DISC2_RIGHT);
	  break;
	case dLEFT:
	  spr=(cur_level.actor_frame?S_HERO_DISC_LEFT:S_HERO_DISC2_LEFT);
	  break;
	}
	break;
      default: spr=S_BLANK; break; ///?
      }
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[spr]);
      x++;
    }
    y++; x=0;
  }
  /// statusbar
  if(cur_level.game_state==PLAY) {
    for(i=0;i<cur_level.lives;i++) {
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_HEART]); x++;
    }
    TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_BLANK]); x++;
    for(i=0;i<cur_level.keys;i++) {
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_KEY]); x++;
    }
    TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_BLANK]); x++;
    for(i=0;i<cur_level.squares_left;i++) {
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_SQUARE]); x++;
    }
    TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_BLANK]); x++;
    for(i=0;i<cur_level.triangles_left;i++) {
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_TRIANGLE]); x++;
    }
    TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_BLANK]); x++;
    for(i=0;i<cur_level.discs_left;i++) {
      TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_DISC]); x++;
    }
  }
  while(x<15) { /// 15=2*7+1 elo
    TV.bitmap(x*SPRITE_W,y*SPRITE_H,sprite[S_BLANK]); x++;
  }
  /// a message?
  if(cur_level.message.expires>0) {    
    cur_level.message.expires--;
    TV.print(SPRITE_W*((15-cur_level.message.length)/2),
	     SPRITE_H*5,
	     cur_level.message.text);
  }
  /// flip!
  cur_level.anim_frame=!cur_level.anim_frame;
}


void display_title_screen() { //// TODO
  TV.delay_frame(1);
//  TV.select_font(font8x8);
  TV.clear_screen();
  /*
  TV.print(40,30,"the");
  TV.print(0,39,"PCHANINA");
  TV.select_font(font4x6);
  TV.print(8,70,"MMXVI by derczansky studio");
  if(display_state.frame) {
    TV.print(16,90,"-- push any button --");
  }
  display_state.frame=1-display_state.frame;
  */
}

void display_d9k_logo(); /// a bit narcisstic, no?


/// MAIN /////////////////////////////////////////////////////

void setup() {
//  randomSeed(analogRead(0)); /// haha! magick.
  setup_tv();
  setup_gamepad();
//  display_d9k_logo();  
  cur_level.game_state=TITLE;
  cur_level.disp_courtain=0;
  cur_level.actor_frame=0;
  cur_level.anim_frame=0;
  //// cos jeszczo?
}

#define TIME_STEP 142 // ~7Hz

void loop() {
  long then,now;

  //TV.clear_screen(); ? KU PAMIECI

  switch(cur_level.game_state) {

    case PLAY:
      draw_board(cur_level.cx,cur_level.cy,MAX_COURTAIN);
      world_step();
      if(cur_level.squares_left==0
	 && cur_level.triangles_left==0
	 && cur_level.discs_left==0) {
	set_message("LEVEL COMPLETED",11);
	cur_level.game_state=FADE_OUT_LEVELUP;
      }
      joystick.dir=dCENTER;
      break;

    case FADE_IN:
      draw_board(cur_level.cx,cur_level.cy,cur_level.disp_courtain);
      if(++cur_level.disp_courtain>=MAX_COURTAIN)
	cur_level.game_state=PLAY;
      break;

    case FADE_OUT_DEATH:
      draw_board(cur_level.cx,cur_level.cy,cur_level.disp_courtain);
      if(--cur_level.disp_courtain==0) {
	if(--cur_level.lives<=0) {
	  cur_level.disp_courtain=MAX_COURTAIN;
	  cur_level.game_state=GAME_OVER;	  
	} else {
	  initialize_level(cur_level.index);
	  cur_level.game_state=FADE_IN;
	}
      }
      break;

    case FADE_OUT_LEVELUP:
      draw_board(cur_level.cx,cur_level.cy,cur_level.disp_courtain);
      if(--cur_level.disp_courtain==0) {
	if(++cur_level.index>=MAX_LEVELS) {
	  cur_level.game_state=VICTOLY;	  
	} else {
	  initialize_level(++cur_level.index);
	  cur_level.game_state=FADE_IN;
	}
      }
      break;

    case GAME_OVER:
      set_message("GAME OVER",1); // ;)
      draw_board(cur_level.cx,cur_level.cy,0);
      if(--cur_level.disp_courtain==0) {      
	cur_level.game_state=TITLE;	
      }
      break;

    case TITLE:
      /// TODO
      initialize_level(0);
      cur_level.lives=5;
      cur_level.disp_courtain=0;
      cur_level.game_state=FADE_IN;
      break;

    case VICTOLY:
      set_message("VICTOLY!",1); // ;)      
      draw_board(cur_level.cx,cur_level.cy,0);
      break;

  }

  reset_joystick();
  /// wait till the end of this cycle.
  then=now=TV.millis();
  while(now-then < TIME_STEP) {
    refresh_joystick();
    now=TV.millis();
  }
}


/// THE DATA (SPRITES AND MAPS) //////////////////////////////////

#define EMPTY {0,0,0,0,0} /// :D
#define EMPTY_TRN {0,0,0,0}  /// ;)

PROGMEM const struct Level_static levels[] = {

  /// level 0 ///////////////////////////////
  {"enter the torus",
   /// guns
   {EMPTY,EMPTY},
   /// telepipes
   {EMPTY,EMPTY,EMPTY,EMPTY},
   /// turncocks
   {EMPTY_TRN,EMPTY_TRN},

   /// map:
   22,8, /// w,h
   {"......................",
    ".....##############...",
    "....#M....I........#..", 	
    ".####...####..&....#..",
    "#(.O....#..#....###...", 
    "#<.V....#..#.;..#.....", 
    "#[.H....#...####......",
    ".#######..............",
    "", "", "", "", "",
    "", "", "", "", ""
   }
  },

  /// level 1 /////////////////////////////
  {
    "holes",
    /// guns
    {EMPTY,EMPTY},
    /// telepipes
    {EMPTY,EMPTY,EMPTY,EMPTY},
    /// turncocks
    {EMPTY_TRN,EMPTY_TRN},

    /// map
    19,5,
    {"..#..[..........#..",
     ".#M..[.....&.....#.",
     "#....[.........V..#",
     ".H...[...........<[",
     "###################",
     "", "", "", "", "",
     "", "", "", "", "",
     "", "", ""
    }
  },

  /// level 2 ////////////////////////////
  {
    "guns of torxton",
    /// guns
    //x,y, dir,  ic,mc
    {{9,1, dDOWN, 0,3},
     {23,1, dRIGHT, 0,6}},

    /// telepipes
    {EMPTY,EMPTY,EMPTY,EMPTY},
    /// turncocks
    {EMPTY_TRN,EMPTY_TRN},

    /// map
    24,9,    
    {".......M###..(..#.|.###.",
     ".......##d#######.L-#.#r",
     "......#.............###.",
     "...####.................",
     "###(.O....#......#######",
     "...<......#..&..#....V..",
     "--7.....#.#.....#.T-----",
     "..|.....###..O..#.|.....",
     "..L------7#.....#.|.....",
     "", "", "", "", "",
     "", "", "", ""      
    }
  },

  /// level 3 ///////////////////////////////
  {"panic",
   /// guns
   {{13,3, dLEFT, 0,5}, EMPTY},
   /// telepipes
   {EMPTY,EMPTY,EMPTY,EMPTY},
   /// turncocks
   {EMPTY_TRN,EMPTY_TRN},

   /// map:
   19,8, /// w,h
   {
	"...##....###[###...",
	"........#.......#..",
	"#########.[...H.###",
	".............l..[..",
	"###..####.&.....###",
	"..#..#..#.......#..",
	"..#H.#...###[###...",
	"..#..#.....#.#.....",
	"", "", "", "", "",
	"", "", "", "", ""
   }
  },

  /// LEVEL 4 //////////////////////////////////////
  {
    "pipes",
    
    /// guns
    //x,y, dir,  ic,mc
    {{0,16, dUP, 0,5},EMPTY},

    /// telepipes
    {{7,1, 4,4, dLEFT},   // 0
     {4,4, 7,1, dDOWN},   // 1

     {10,3, 5,16, dDOWN}, // 2
     {5,16, 10,3, dLEFT}},  // 3

    /// turncocks
    {{6,0, 0,1}, EMPTY_TRN},

    /// map
    13,18,
    {".....Th7.....",
     "#####|#|#####",
     "#....|#.&....",
     "#....|#...-7.",
     "#...-J#....|.",
     "#.;...#....|.",
     "#.....#....|.",
     "###########|#",
     "...........|.",
     ".VHH.......|.",
     "...........|.",
     ".<[[.T-----J.",
     ".....|.<..V..",
     ".....|.<..V..",
     ".....|.<..V..",
     ".#I##|#######",
     "u#...|......#",
     "..<......V..."
    }
  }
};


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
/*
  TV.bitmap(0,0,dercz9000);
  delay(2012);
  TV.tone(128,1000);
  delay(1000);
  TV.tone(256,1000);  
  delay(1000);  
  TV.tone(384,1000);
  delay(1000);  
  TV.tone(512,900);
  */
}

/// the end.
