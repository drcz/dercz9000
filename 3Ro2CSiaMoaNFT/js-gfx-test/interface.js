/// INPUT /////////////////////////////////////////////////////
var joystick={dx:0,dy:0,h:false,r:false}; /// a joystick, right ;)

reset_joystick = function() {
    joystick.dx = joystick.dy = 0;
    joystick.h = joystick.r = 0;
};

window.addEventListener('keydown',
			function(evt) {
			    switch(evt.keyCode) {
			    case 37: // left arr.
			    case 65: // A
				joystick.dx=-1; joystick.dy=0; break;
			    case 39: // right arr.
			    case 68: // D
				joystick.dx=1; joystick.dy=0; break;
			    case 38: /// up arr.
				evt.preventDefault(); /// no scrolling
			    case 87: /// W
				joystick.dx=0; joystick.dy=-1;
				break;
			    case 40: // down arr.
				evt.preventDefault(); /// no scrolling
			    case 83: // S
				joystick.dx=0; joystick.dy=1;
				break;
			    case 112: /// f1
				evt.preventDefault(); // no brower help
			    case 72: /// H
				joystick.h=true;
				break;
			    case 27: /// esc
			    case 82: /// R
				joystick.r=true;
				break;
			    }
			},
			false);


/// OUTPUT

var screen_w=480, screen_h=384;
//var screen_w=823, screen_h=600;
var kanwa=document.getElementById("ekraniszcze");
var kontekst=kanwa.getContext("2d");

var load_sound = function(url,max) {
    var smpls = [];
    for(var i=0;i<max;i++) {
	smpls.push(new Audio(url));
    }
    return {"samples":smpls,"index":0,"max":max};
};

var low_beep=load_sound('snd/beep2.ogg',4);
var high_beep=load_sound('snd/beep.ogg',4);
var Sounds = {
    'death' : low_beep,
    'pipe' : low_beep,
    'squeak-high' : low_beep,
    'squeak-low' : low_beep,
    's-2' : low_beep,
    's-1' : low_beep,
    's1' : high_beep,
    's2' : high_beep,
    's3' : high_beep,
    's4' : high_beep,
    'short-high-chord' :  high_beep
};

var PLAY=function(type,vol) {
    var s = Sounds[type];
    s.index = ++s.index%s.max;
    snd = s.samples[s.index];
    snd.volume = vol;
    snd.play();
};

var load_image = function(url){
    var i=new Image();
    i.src=url;
    return(i);
};  

//var Background = load_image('img/background.png');

var Sprites = {

    'HERO-SQUARE' : {'up' : [ load_image('img/hero-square-up.png'),
			      load_image('img/hero-square2-up.png') ],
		     'down' : [ load_image('img/hero-square-down.png'),
				load_image('img/hero-square2-down.png') ],
		     'up2' : [ load_image('img/hero-square-up2.png'),
			       load_image('img/hero-square2-up2.png') ],
		     'down2' : [ load_image('img/hero-square-down2.png'),
				 load_image('img/hero-square2-down2.png') ],
		     'left' : [ load_image('img/hero-square-left.png'),
				load_image('img/hero-square2-left.png') ],
		     'right' : [ load_image('img/hero-square-right.png'),
				 load_image('img/hero-square2-right.png') ]
		    },

    'HERO-TRIANGLE' : {'up' : [ load_image('img/hero-triangle-up.png'),
				load_image('img/hero-triangle2-up.png') ],
		       'down' : [ load_image('img/hero-triangle-down.png'),
				  load_image('img/hero-triangle2-down.png') ],
		       'up2' : [ load_image('img/hero-triangle-up2.png'),
				 load_image('img/hero-triangle2-up2.png') ],
		       'down2' : [ load_image('img/hero-triangle-down2.png'),
				   load_image('img/hero-triangle2-down2.png') ],
		       'left' : [ load_image('img/hero-triangle-left.png'),
				  load_image('img/hero-triangle2-left.png') ],
		       'right' : [ load_image('img/hero-triangle-right.png'),
				   load_image('img/hero-triangle2-right.png') ]
		      },

    'HERO-DISK' : {'up' : [ load_image('img/hero-disc-up.png'),
			    load_image('img/hero-disc2-up.png') ],
		   'down' : [ load_image('img/hero-disc-down.png'),
			      load_image('img/hero-disc2-down.png') ],
		   'up2' : [ load_image('img/hero-disc-up2.png'),
			     load_image('img/hero-disc2-up2.png') ],
		   'down2' : [ load_image('img/hero-disc-down2.png'),
			       load_image('img/hero-disc2-down2.png') ],
		   'left' : [ load_image('img/hero-disc-left.png'),
			      load_image('img/hero-disc2-left.png') ],
		   'right' : [ load_image('img/hero-disc-right.png'),
			       load_image('img/hero-disc2-right.png') ]
		  },

    'GUN' : {'up' : [ load_image('img/gun-up.png') ],
	     'down' : [ load_image('img/gun-down.png') ],
	     'left' : [ load_image('img/gun-left.png') ],
	     'right' : [ load_image('img/gun-right.png') ]
	    },

    'PARTICLE' : [ load_image('img/particle-h.png'),
		   load_image('img/particle-v.png') ],

    'HEART' : [load_image('img/heart.png')],

    'BLANK' : [ load_image('img/blank.png')],

    'SQUARE' : [ load_image('img/square.png'),
		 load_image('img/square2.png')],
    'TRIANGLE' : [ load_image('img/triangle.png'),
		   load_image('img/triangle2.png') ],
    'DISK' : [ load_image('img/disc.png'),
	       load_image('img/disc2.png')],

    'HOLE-SQUARE' : [ load_image('img/hole-square.png') ],
    'HOLE-TRIANGLE' : [ load_image('img/hole-triangle.png') ],
    'HOLE-DISK' : [ load_image('img/hole-disc.png') ],

    'WALL': [ load_image('img/wall.png'),
	      //load_image('img/wall2.png') ],
	      load_image('img/wall.png') ],

    'DOOR': [ load_image('img/door.png')],
    'KEY': [ load_image('img/key.png') ],

    'MACHINE': [ load_image('img/machine.png') ],

    'PIPE-H': [ load_image('img/pipe-h.png') ],
    'PIPE-V': [ load_image('img/pipe-v.png') ],

    'TURNCOCK-H': [ load_image('img/turncock-h.png')],
    'TURNCOCK-V': [ load_image('img/turncock-v.png')],

    'PIPE-DL': load_image('img/pipe-dl.png'),
    'PIPE-DR': load_image('img/pipe-dr.png'),
    'PIPE-UL': load_image('img/pipe-ul.png'),
    'PIPE-UR': load_image('img/pipe-ur.png'),

    'BOOM': load_image('img/boom.png')

};

var Letters = {

' ' : load_image('img/blank.png'),

 '0' : load_image('img/ltr/0.png'),
 '1' : load_image('img/ltr/1.png'),
 '2' : load_image('img/ltr/2.png'),
 '3' : load_image('img/ltr/3.png'),
 '4' : load_image('img/ltr/4.png'),
 '5' : load_image('img/ltr/5.png'),
 '6' : load_image('img/ltr/6.png'),
 '7' : load_image('img/ltr/7.png'),
 '8' : load_image('img/ltr/8.png'),
 '9' : load_image('img/ltr/9.png'),
 'a' : load_image('img/ltr/a.png'),
 'A' : load_image('img/ltr/A.png'),
 'b' : load_image('img/ltr/b.png'),
 'B' : load_image('img/ltr/B.png'),
 'c' : load_image('img/ltr/c.png'),
 'C' : load_image('img/ltr/C.png'),
 'd' : load_image('img/ltr/d.png'),
 'D' : load_image('img/ltr/D.png'),
 'e' : load_image('img/ltr/e.png'),
 'E' : load_image('img/ltr/E.png'),
 '!' : load_image('img/ltr/excl.png'),
 'f' : load_image('img/ltr/f.png'),
 'F' : load_image('img/ltr/F.png'),
 'g' : load_image('img/ltr/g.png'),
 'G' : load_image('img/ltr/G.png'),
 'h' : load_image('img/ltr/h.png'),
 'H' : load_image('img/ltr/H.png'),
 '-' : load_image('img/ltr/hyphen.png'),
 'i' : load_image('img/ltr/i.png'),
 'I' : load_image('img/ltr/I.png'),
 'j' : load_image('img/ltr/j.png'),
 'J' : load_image('img/ltr/J.png'),
 'k' : load_image('img/ltr/k.png'),
 'K' : load_image('img/ltr/K.png'),
 'l' : load_image('img/ltr/l.png'),
 'L' : load_image('img/ltr/L.png'),
 'm' : load_image('img/ltr/m.png'),
 'M' : load_image('img/ltr/M.png'),
 'n' : load_image('img/ltr/n.png'),
 'N' : load_image('img/ltr/N.png'),
 'o' : load_image('img/ltr/o.png'),
 'O' : load_image('img/ltr/O.png'),
 '.' : load_image('img/ltr/period.png'),
 'p' : load_image('img/ltr/p.png'),
 'P' : load_image('img/ltr/P.png'),
 'q' : load_image('img/ltr/q.png'),
 'Q' : load_image('img/ltr/Q.png'),
 '\'' : load_image('img/ltr/quot.png'),
 'r' : load_image('img/ltr/r.png'),
 'R' : load_image('img/ltr/R.png'),
 's' : load_image('img/ltr/s.png'),
 'S' : load_image('img/ltr/S.png'),
 't' : load_image('img/ltr/t.png'),
 'T' : load_image('img/ltr/T.png'),
 'u' : load_image('img/ltr/u.png'),
 'U' : load_image('img/ltr/U.png'),
 'v' : load_image('img/ltr/v.png'),
 'V' : load_image('img/ltr/V.png'),
 'w' : load_image('img/ltr/w.png'),
 'W' : load_image('img/ltr/W.png'),
 'x' : load_image('img/ltr/x.png'),
 'X' : load_image('img/ltr/X.png'),
 'y' : load_image('img/ltr/y.png'),
 'Y' : load_image('img/ltr/Y.png'),
 'z' : load_image('img/ltr/z.png'),
 'Z' : load_image('img/ltr/Z.png')
};

var vcx = 0;
var vcy = 0;
var anim_frame=0;
var anim_cnt=0;

var draw_board = function(world,status,msg,fade) {
    anim_cnt++;
    anim_cnt%=6;
    anim_frame=(anim_cnt<3?0:1);
    
    var tile_w=32, tile_h=32;
    var offset_x=0, offset_y=0;
    var viewport_dw=7, viewport_dh=5;

    var hero = find_hero(world);
    if(hero!=null) {
	vcx=hero.x;
	vcy=hero.y;
    }
    var viewport_center_x=vcx;
    var viewport_center_y=vcy;
    //kontekst.drawImage(Background,0,0);
    //kontekst.fillStyle="#FFFFFF";
    //kontekst.fillRect(0,0,screen_w,screen_h);

    var x=offset_x,y=offset_y;
    for(var j=viewport_center_y-viewport_dh;j<=viewport_center_y+viewport_dh;j++) {
	for(var i=viewport_center_x-viewport_dw;i<=viewport_center_x+viewport_dw;i++) {
	    var o=find_by_pos(world,i,j);
	    var sprite=Sprites['BLANK'][0];
	    if(o!=null && y<=fade*tile_h) {
		sprite=Sprites[o.type];
		switch(o.type) {
		case 'WALL':
		case 'SQUARE':
		case 'DISK':
		case 'TRIANGLE':
		case 'PARTICLE':
		    sprite=sprite[anim_frame];//anim_cnt%2];
		    break;
		case 'HOLE-SQUARE':
		case 'HOLE-DISK':
		case 'HOLE-TRIANGLE':
		case 'MACHINE':
		case 'DOOR':
		case 'KEY':
		case 'PIPE-H':
		case 'PIPE-V':
		case 'TURNCOCK-H':
		case 'TURNCOCK-V':
		    sprite=sprite[0];
		    break;
		case 'GUN':
		    sprite=sprite[o.facing][0];
		    break;
		case 'HERO-SQUARE':
		case 'HERO-TRIANGLE':
		case 'HERO-DISK':
		    sprite=sprite[o.facing][anim_frame]; // !!
		    break;
		}
		//kontekst.drawImage(sprite,x,y);		
	    }
	    kontekst.drawImage(sprite,x,y);
	    x+=tile_w;
	}
	x=offset_x;
	y+=tile_h;
    }

    if(status!=null) {
	/// statusbar        
	for(var i=0;i<status.lives;i++) {
    	    kontekst.drawImage(Sprites['HEART'][0],x,y);
	    x+=tile_w;
	}
	kontekst.drawImage(Sprites['BLANK'][0],x,y); x+=tile_w;
	/// !!!!!!
	for(var i=0;i<hero.inventory.length;i++) {
	    kontekst.drawImage(Sprites['KEY'][0],x,y);
	    x+=tile_w;
	} /// \!!!!!
	kontekst.drawImage(Sprites['BLANK'][0],x,y); x+=tile_w;
	for(var i=0;i<status.squares;i++) {
	    sprite = Sprites['SQUARE'][0];
    	    kontekst.drawImage(sprite,x,y);
	    x+=tile_w;
	}
	kontekst.drawImage(Sprites['BLANK'][0],x,y); x+=tile_w;
	for(var i=0;i<status.triangles;i++) {
	    sprite = Sprites['TRIANGLE'][0];
    	    kontekst.drawImage(sprite,x,y);
	    x+=tile_w;
	}
	kontekst.drawImage(Sprites['BLANK'][0],x,y); x+=tile_w;
	for(var i=0;i<status.disks;i++) {
	    sprite = Sprites['DISK'][0];
    	    kontekst.drawImage(sprite,x,y);
	    x+=tile_w;
	}    
    }
    while(x<screen_w) {
	kontekst.drawImage(Sprites['BLANK'][0],x,y);
	x+=tile_w;
    }

    /// aa, message!
    if(msg != '') {
	msg = msg.toLowerCase();
	write_centered_line(msg, offset_y+(viewport_dh-1)*tile_h);
    }			   
};


/// i takie takie... //////////////////////

/// assuming the background is already there...
var write_line = function(txt,x,y) {
    var tile_w=32; /// :)
    //txt=txt.toLowerCase();
    for(var i=0;i<txt.length;i++) {
	var ch = Letters[txt[i]];
	if(ch==undefined) ch=Letters[' ']; //?
	kontekst.drawImage(ch/*[rand(0,1)]*/, x,y);
	x+=tile_w;
    }
};

var write_centered_line = function(txt,y) {
    var viewport_dw = 7; /// @#$!
    var tile_w=32; /// :)
    var mx = Math.round(((2*viewport_dw+1)-txt.length)/2);
    write_line(txt,mx*tile_w,y);
};

var draw_title = function() {
    var tile_h = 32; // ;)
    var text = [
	"",
	"3Ro2CSiaMoNFT",
	"",
	"an adaptation",
	"of LudumDare 35",
	"game by",
	"derczansky",
	"studio",
	"for dercz9000",
	"console",
	"",
	"- any move -"
    ];
    var y=0;
    for(var i=0;i<text.length;i++) {
	write_centered_line(text[i],y);
	y+=tile_h;
    }
};

var draw_help = function() {
    var tile_h = 32; // ;)
    var text = [
	"jo jo jo"
//       123456789012345678901
    ];

    var y=0;
    for(var i=0;i<text.length;i++) {
	write_centered_line(text[i],y);
	y+=tile_h;
    }
};

/// MAIN //////////////////////////////////////////////////////

/// yeah, whatever.
var _max_lives_ = 5; /// ??
var lives = _max_lives_;
var level = 0;
var message = {'text':'', 'expires':0};
var the_world = mk_world([],2,3);
var GAME_STATE = 'TITLE';
var OLD_GAME_STATE = 'TITLE'; //...
var _fadin_ = 0;
var _max_fadin_ = 17;
var _min_fadin_ = -2;

var init_level = function(n) {
    level = n;
    message = {'text':Levels[n].name, 'expires':18};
    the_world = load_level(level);
    GAME_STATE = 'FADE IN';
    _fadin_ = _min_fadin_;
};



var do_play = function() {
    var game_stats = count_shapes(the_world);
    game_stats.lives = lives; // !!
    
    /// HAHA! MAYBE THIS LEVEL IS ALREADY SOLVED? <- a fine place do check that! [not really]
    if(game_stats.squares==0
       && game_stats.disks==0
       && game_stats.triangles==0) {
	GAME_STATE = 'FADE OUT levelup';
	PLAY('short-high-chord',0.99);
    }

    /// show the world
    draw_board(the_world,game_stats,((message.expires-->0)?message.text:''),_max_fadin_);
    /// get some feedback

    /// SPEAKING OF WHICH -- REQUEST FOR HELP?
    if(joystick.h) {
	OLD_GAME_STATE = 'PLAY';
	GAME_STATE = 'HELP';
	reset_joystick();
    }
    /// MAYBE A SUICIDE?
    if(joystick.r) {
	reset_joystick();
	GAME_STATE = 'FADE OUT death';
	message.text = 'SUICIDE...';
	message.expires = 26;
	PLAY('death',0.84);
    }


    var dx=joystick.dx;
    var dy=joystick.dy;    
    reset_joystick();
    /// apply to the only protagonist
    var hero = find_hero(the_world);
    hero.dx = dx; hero.dy = dy;
    if(dx!=0 || dy!=0) {
	PLAY('s-1',0.9-0.23*Math.random());
	hero.facing=new_facing(hero.facing,dx,dy);
    }
    the_world = update_thing(the_world, hero);
    /// and see what happens
    the_world = world_step(the_world);

    /// and do the sound FX / messages stuff!
    for(var i=0;i<the_world.facts.length;i++) {
	var fact = the_world.facts[i];
	var vol = 0.9-0.13*Math.random();
	switch(fact[0]) {
	case 'SHIFT TO':
	    if(message.expires<=0) {
		message.text='SHIFT!';
		message.expires=2;
	    }
	    vol=1.0;
	case 'PUSHED':
	    switch(fact[2]) {
	    case 'SQUARE': PLAY('s1',vol); break;
	    case 'DISK': PLAY('s2',vol); break;
	    case 'TRIANGLE': PLAY('s3',vol); break;
	    //default: PLAY('push',vol); break; // pff...
	    }
	    break;
	case 'ANNIHILATE':
	    switch(rand(1,3)) {
	    case 1: message.text='GOOD!'; break;
	    case 2: message.text='COOL!'; break;
	    case 3: message.text='well done.'; break;
	    }
	    message.expires=2;
	    PLAY('short-high-chord',vol);
	    break;

	case 'TELEPORT':
	    if(message.expires<=0) {
		switch(rand(1,3)) {
		case 1: message.text='WHOA!'; break;
		case 2: message.text='WOW!'; break;
		case 3: message.text='WEE!'; break;
		}
		message.expires=2;
	    }
	    PLAY('pipe',vol);
	    break;

	case 'TURNCOCK':
	    PLAY('squeak-high',1.0);
	    message.text='turncock turned';
	    message.expires=4;
	    break;
	case 'PICKUP':
	    message.text="got a key now";
	    message.expires=5;
	    PLAY('s4',vol);
	    break;
	case 'OPENED':
	    message.text='door unlocks';
	    message.expires=4;
	    PLAY('s4',vol);
	    break;
	case 'FAILED TO OPEN':
	    message.text="it's locked";
	    message.expires=4;
	    PLAY('s-2',vol); break;	    

	case 'FAILED TO PUSH':
	    message.text="can't do that";
	    message.expires=4;
	    PLAY('s-2',vol); break;	    

	case "NO UNSHIFTIN":
	    message.text="can't unshift";
	    message.expires=5;
	    PLAY('s-2',vol); break;	    

	case 'THIS PIPE IS CLOSED':
	    switch(rand(1,2)) {
	    case 1: message.text="can't do that";break;
	    case 2: message.text='find a turncock';break;
	    }
	    message.expires=4;
	    PLAY('s-2',vol); break;

	case 'DIES':
	    switch(rand(1,2)) {
	    case 1: message.text = 'YOU DIE!'; break;
	    case 2: message.text = 'OOPS... YOU DIE'; break;
	    }
	    message.expires = 16;
	    PLAY('death',1.0);
	    _faind_ = _max_fadin_-1;
	    GAME_STATE = 'FADE OUT death';
	    break;
	}
    }
};


var do_fadein = function() {
    var game_stats = count_shapes(the_world);
    game_stats.lives = lives; // !!
    draw_board(the_world,game_stats,((message.expires-->0)?message.text:''),_fadin_);
    if(++_fadin_ >= _max_fadin_) {
	GAME_STATE = 'PLAY';
	reset_joystick();
    }
};

var do_fadeout_death = function() {
    var game_stats = {'squares':0,'triangles':0,'discs':0, 'lives':lives-(_fadin_%2)};
    draw_board(the_world,game_stats,((message.expires-->0)?message.text:''),_fadin_);
    hero = find_hero(the_world);
    switch(mod(_fadin_,4)) {
    case 0: hero.facing = 'right'; break;
    case 1: hero.facing = 'down'; break;
    case 2: hero.facing = 'left'; break;
    case 3: hero.facing = 'up2'; break;
    }
    if(_fadin_-- < 0) {
	if(--lives<=0) {
	    GAME_STATE = 'GAME OVER';
	}
	else {
	    init_level(level);
	    _fadin_ = _min_fadin_;
	    GAME_STATE = 'FADE IN';
	}
    }
};

var do_fadeout_levelup = function() {
    draw_board(the_world,null,'LEVEL COMPLETED.',_fadin_);
    hero = find_hero(the_world);
    switch(mod(_fadin_,4)) {
    case 0: hero.facing = 'right'; break;
    case 1: hero.facing = 'down'; break;
    case 2: hero.facing = 'left'; break;
    case 3: hero.facing = 'up2'; break;
    }
    if(_fadin_-- <= 0) {
	if(++level>=Levels.length) {
	    GAME_STATE = 'VICTOLY';
	} else {
	    init_level(level);
	    _fadin_ = _min_fadin_;
	    GAME_STATE = 'FADE IN';
	}
    }
};

var do_gameover = function() {
    draw_board(the_world,null,'GAME OVER.',-1);
    if(++_fadin_>= 33) {
	_max_lives_ = 5; /// ??
	lives = _max_lives_;
	level = 0;
	message = {'text':'', 'expires':0};
	the_world = mk_world([],2,3);
	GAME_STATE = 'TITLE';
    }
};

var fin_world = mk_world([{'type':'HERO-SQUARE','x':4,'y':0,'facing':'left'},
			  {'type':'HERO-TRIANGLE','x':2,'y':0,'facing':'left'},
			  {'type':'HERO-DISK','x':6,'y':0,'facing':'right'}],
			 33,23);

var do_victoly = function() {
    if(_fadin_==3) PLAY('s-2',0.33);
    if(_fadin_==6) PLAY('s-1',0.33);
    if(_fadin_==8) _fadin_=0;
    draw_board(fin_world,null,'VICTOLY!!!',33);
};

var do_title = function() {
    draw_board(fin_world,null,'',0);
    draw_title();
    if(joystick.h) {
	OLD_GAME_STATE = 'TITLE';
	GAME_STATE = 'HELP'; /// ?!
	reset_joystick();
    }
    if(joystick.dx!=0 || joystick.dy!=0) {
	init_level(0);
	GAME_STATE = 'FADE IN';
	reset_joystick();
    }
};


var do_help = function () {
    draw_board(fin_world,null,'',0);
    draw_help();
    if(joystick.dx!=0 || joystick.dy!=0) {
	reset_joystick();
	GAME_STATE = OLD_GAME_STATE;
    }
};


///////////////////////////////////////

var Game_states_step = {
    'PLAY': do_play,
    'FADE IN': do_fadein,
    'FADE OUT death': do_fadeout_death,
    'FADE OUT levelup': do_fadeout_levelup,
    'GAME OVER': do_gameover,
    'HELP': do_help,    
    'VICTOLY': do_victoly,
    'TITLE': do_title
};

setInterval(function(){ Game_states_step[GAME_STATE](); },143); /// ~7Hz -- like brain in a dreamstate.
