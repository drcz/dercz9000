#include<time.h>
#include<curses.h>
#include<stdio.h>
#include<string.h>

/// for "arduino compatibility"
typedef unsigned char byte;
//#define FLASH(X) pgm_read_byte(&X)
//#define FLASHW(X) pgm_read_word(&X)
#define FLASH(X) X
#define FLASHW(X) X
////////////////////////////////////////

#define BITS_FOR_X 6
#define BITS_FOR_Y 5
#define BITS_FOR_TYPE 5
#define BITS_FOR_TELEPIPES 3
#define MAX_TELEPIPES 4
#define BITS_FOR_GUNS 2
#define MAX_GUNS 4
#define MAX_THINGS 100 // ? <=57 + all the projectiles and booms.
#define MAX_MESSAGE 15

/// direction/facing/sth (fits 3b anyway)
enum {dUP,dLEFT,dDOWN,dRIGHT,dUP2,dDOWN2,dCENTER};

/// shapes (2b)
enum {shSQUARE, shTRIANGLE, shDISC};

/////////////////////////////////////////////////////////////////////
struct {
  byte dir; // :D
} joystick;

//////////////////////////////////////////////////////////////////////

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
  case shSQUARE:
  case SQUARE:
    /// TODO
    break;
  case shTRIANGLE:
  case TRIANGLE:
    /// TODO
    break;
  case shDISC:
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

/*
inline char dist_on_circle(byte a, byte b, byte perim) {
  char d1,d2;
  d1=a-b;
  while(d1<0) d1+=perim;
  while(d1>=perim) d1-=perim;
  d2=b-a;
  while(d2<0) d2+=perim;
  while(d2>=perim) d2-=perim;
  if(d1<d2) return d1;
  return d2;
}
*/

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
/*PROGMEM*/ const collision_proc collision[ST_MAX][ST_MAX] = {
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


//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////

#define MAX_LEVELS 5

#define EMPTY {0,0,0,0,0} /// :D
#define EMPTY_TRN {0,0,0,0}  /// ;)

/*PROGMEM*/ const struct Level_static levels[] = {

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
  set_message(FLASH(levels[n].name),13);
  /// boom done?
}

//////////////////////////////////////////////////////////////////////
void draw_board(byte cx,byte cy,byte courtain) {
  byte x,y;
  char i,j,ch;
  x=0;y=1;
  move(1,1);
  printw("(%d,%d) %d %d",cx,cy, read_map(cx,cy),index_of_thing_at(cx,cy));
  for(j=cy-5;j<=cy+5;j++) {
    for(i=cx-7;i<=cx+7;i++) {
      if(y>=courtain) ch=' ';
      else
      switch(read_map(i,j)) {
      case NOTHING: ch=' '; break;
      case WALL: ch='#'; break;
      case ACTOR_SQUARE: ch='S'; break;
      case ACTOR_TRIANGLE: ch='T'; break;
      case ACTOR_DISC: ch='D'; break;
      case SQUARE: ch=']'; break;
      case TRIANGLE: ch='>'; break;
      case DISC: ch=')'; break;
      case HOLE_SQUARE: ch='['; break;
      case HOLE_TRIANGLE: ch='<'; break;
      case HOLE_DISC: ch='('; break;
      case PIPE_V: ch='I'; break;
      case PIPE_H: ch='='; break;
      case PIPE_DL: ch='\\'; break;
      case PIPE_UR: ch='\\'; break;
      case PIPE_DR: ch='/'; break;
      case PIPE_UL: ch='/'; break;
      case KEY: ch='%'; break;
      case DOOR: ch='0'; break;
      case MACHINE: ch='M'; break;
      case TURNCOCK_V:
      case TURNCOCK_H:ch='Q';break;
      case PROJECTILE_L:
      case PROJECTILE_U:
      case PROJECTILE_D:
      case PROJECTILE_R: ch='.'; break;
      case GUN_D:
      case GUN_U: ch='|'; break;
      case GUN_L:
      case GUN_R: ch='-'; break;
      case BOOM: ch='*'; break;
      default: ch='?'; /// ?!
      }
      move(y+1,x+1);
      printw("%c",ch);
      x++;
    }
    y++; x=0;
  }
  /// statusbar
  if(cur_level.game_state==PLAY) {
    for(i=0;i<cur_level.lives;i++) {
      move(y+1,x+1); printw("Y");
      x++;
    }
    move(y+1,x+1); printw(" "); x++;
    for(i=0;i<cur_level.keys;i++) {
      move(y+1,x+1);  printw("%%");
      x++;
    }
    move(y+1,x+1); printw(" "); x++;
    for(i=0;i<cur_level.squares_left;i++) {
      move(y+1,x+1); printw("]");
      x++;
    }
    move(y+1,x+1); printw(" "); x++;
    for(i=0;i<cur_level.triangles_left;i++) {
      move(y+1,x+1); printw(">");
      x++;
    }
    move(y+1,x+1); printw(" "); x++;
    for(i=0;i<cur_level.discs_left;i++) {
      move(y+1,x+1); printw(")");
      x++;
    }
  }
  while(x<15) {
    move(y+1,x+1); printw(" ");
    x++; 
  }
  /// a message?
  if(cur_level.message.expires>0) {    
    cur_level.message.expires--;
    for(i=0;i<cur_level.message.length;i++) {
      move(1+5,1+(15-cur_level.message.length)/2 +i);
      printw("%c",cur_level.message.text[i]);
    }
  }
}

//////////////////////////////////////////////////////////////////////
#define MAX_COURTAIN 12

int main() {
  clock_t then,now,time_step;
  time_step=CLOCKS_PER_SEC/5; /// 7[Hz]
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);
  clear();
  joystick.dir=dCENTER;
  for(;;) {
    then=now=clock();
    while(now-then<time_step) {
      switch(getch()) {
      case 'q': endwin(); return 0;
      case 'h': joystick.dir=dLEFT; break;
      case 'l': joystick.dir=dRIGHT; break;
      case 'k': joystick.dir=dUP; break;
      case 'j': joystick.dir=dDOWN; break;
      }
      now=clock();
    }
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
  }
}
