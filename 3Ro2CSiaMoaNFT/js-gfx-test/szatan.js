////////////////////////////////////////////////////////////////////////////////
/// some utilities.

/// not sure whether I want anything to be "random", but...
var rand = function(lo,hi) { return(lo+Math.floor(Math.random()*(hi-lo+1))); };

/// hehe.
var signum = function(n) {
    if(n>0) return 1;
    if(n<0) return -1;
    return 0;
};

/// to rotate things, display, something...? no?
var new_facing = function(old_facing, dx,dy) {
    if(dx>0) return 'right';
    else if(dy<0) {
	if(old_facing=='left' || old_facing=='down' || old_facing=='up2') return 'up2';
	else return 'up';
    } else if(dy>0) {
	if(old_facing=='left' || old_facing=='up' || old_facing=='down2') return 'down2';
	return 'down';
    }
    else return 'left';    
};

/// the real modulo, not the signed % shit.
var mod = function(m,n) {
    var r=m%n;
    if(r<0) r=n+r;
    return r;
};

/// filter -- just in case your js implementation doesn't have it or sth
var filter = function(list,f) {
    var new_list = [];
    for(var i=0;i<list.length;i++)
	if(f(list[i])) new_list.push(list[i]);
    return new_list;
};

////////////////////////////////////////////////////////////////////////////////
/// the World.

/// Call the press: Wittgenstein in his Tractatus was only partly right!
/// ** THE WORLD IS THE TOTALITY OF FACTS, _AND_ THINGS. ** (wat?)

/// Each thins has its unique index, non-unique type, position (x,y),
/// and sometimes some more attributes (like "expiry date" or sth).
/// The facts... well actually it's just the log of recent Important Changes,
/// recorded as tupes of strings [?], probably most in form of SPO triplets
/// (did I really say that?!), like ['OPENED','HERO','DOOR'].
///  -- this enables to separate the sound FXs or sth from the mechanics,
///  and might be useful when debugging (or may be not).

/// A twist: the world is the surface of torus; therefore we add 2 perimeters
/// this mod thing spoils some dx/dy computations, but WHO CARES?

/// Therefore:
var mk_world = function(things,vp,hp) { /// TODO sure?
    return {'things':things, 'facts':[],'vp':vp,'hp':hp};
};

/// world's "selectors". ///////////////////////////////////////////////////////
var find_all_by_label = function(world, label) {
    var found=[];
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing!=null && thing.label==label) found.push(thing);
    }
    return found;
};

//// NB these two can use some weird topolgy... 
var find_all_by_pos = function(world, x,y) {
    x=mod(x,world['hp']); y=mod(y,world['vp']); /// Torus Topology ;)
    var found=[];
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing!=null && thing.x==x && thing.y==y) found.push(thing);
    }
    return found;
};
/// most of the time all you need is this:
var find_by_pos = function(world, x,y) {
    x=mod(x,world['hp']); y=mod(y,world['vp']); /// Torus Topology ;)
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing!=null && thing.x==x && thing.y==y) return(thing);
    }
    return null;
};

var find_hero = function(world) {
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing!=null
	   && (thing.type=='HERO-SQUARE'
	      || thing.type=='HERO-TRIANGLE'
	      || thing.type=='HERO-DISK')) return(thing);
    }
    return null; /// wtf?
};

/// todo var find_pipe_ends ??

/// world's "modifiers". ///////////////////////////////////////////////////////
var insert_thing = function(world, thing) {
    thing.index=world.things.length;
    world.things[thing.index]=thing;
//    world=notice(world,['INS',thing]); //dbg
    return(world);
};

var update_thing = function(world, thing) {
    if(world.things[thing.index]!=null)
	world.things[thing.index]=thing;
//    world=notice(world,['UPD',thing.type,thing]); //dbg
    return(world);
};

var delete_thing = function(world, thing) {    
    world.things[thing.index]=null;
//    world=notice(world,['DEL',thing]); //dbg
    return(world);
};

/// after all the world-operations, remove the empty "slots",
/// and update the indexes.
/// (this is because the game cycle uses "for" loop, during which
///  some things can get deleted; but the indexes should remain
///  the same till the end of the loop, because it's a "for" y'know.)
var new_world_order = function(world) {
    var new_things=[];
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing!=null) {
	    thing.index=new_things.length;
	    //x=mod(x,world_rx); y=mod(y,world_ry); /// also here?
	    new_things[thing.index]=thing;
	}
    }
    world.things=new_things;
    return(world);
};

/// log certain triplets, or whatever...
var notice = function(world, fact) {
    //console.log(fact); // dbg !!
    world.facts.push(fact);
    return(world);
};

/// do this when the new cycle starts (i guess).
var forget_facts = function(world) {
    world.facts = [];
    return(world);
};

/// plus some meta-actions? [hunoz?]
var mk_delete = function(thing) {
    return function(world) {
	return delete_thing(world,thing);
    };
};

var mk_insert = function(thing) {
    return function(world) {
	return insert_thingt(world,thing);
    };
};


/// the most important thing in the world: explosions. /////////////////////////
var mk_boom = function(x,y, expires) {
    return({'type':'BOOM', 'x':x, 'y':y, 'expires':expires});
};

var e_none = function(world, thing) { return(world); };

var e_trivial = function(world, thing) {
    world = delete_thing(world, thing);
    world = notice(world, ['DISAPPEAR',thing.x,thing.y]);
    return(world); 
};

var e_small = function(world, thing) {
    world = delete_thing(world, thing);
    world = insert_thing(world, mk_boom(thing.x,thing.y,1));
    //world = notice(world, ['BOOM',thing.x,thing.y]); // ??
    return(world); 
};

/// BOOKMARK:EXPLOSIONS
var explosion_for = function(thing) {
    if(thing==undefined) return e_none;

    switch(thing.type) {
    case 'WALL':
	/// (...)
	return e_none;
    default:
	//return e_trivial;
	return e_small;
    }
};


/// now the collisions; including the explosions, of course. ///////////////////

var c_nothing = function(world) { return(world); }; /// "the identity", hehe.

var c_delete_passive = function(world, active,passive) {
    world = delete_thing(world, passive);
    return(world);
};

var c_delete_active = function(world, active,passive) {
    world = delete_thing(world, active);
    return(world);
};

var c_explode_passive = function(world, active,passive) {
    world = (explosion_for(passive))(world, passive);
    return(world);
};

var c_explode_active = function(world, active,passive) {
    world = (explosion_for(active))(world, active);
    return(world);
};

var c_bump_passive = function(world, active,passive) {
    passive.dx*=-1;
    passive.dy*=-1;
    world = update_thing(world, passive);
    world = notice(world, ['BUMPS',passive.type]); // ?
    return(world);
};

var c_bump_active = function(world, active,passive) {
    active.dx*=-1;
    active.dy*=-1;
    world = update_thing(world, active);
    world = notice(world, ['BUMPS',active.type]); // ?
    return(world);
};

var c_stop_passive = function(world, active,passive) {
    passive.dx=0;
    passive.dy=0;
    world = update_thing(world, passive);
    world = notice(world, ['STOPS',passive.type]); // ?
    return(world);
};

var c_stop_active = function(world, active,passive) {
    active.dx=0;
    active.dy=0;
    world = update_thing(world, active);
    world = notice(world, ['STOPS',active.type]); // ?
    return(world);
};


var mk_particle = function(x,y, dx,dy) {
    return({'type':'PARTICLE', 'x':x,'y':y, 'dx':dx,'dy':dy});
};

/*
var c_fission_active = function(world, active,passive) {
    var dx = signum(passive.x - active.x);
    var dy = signum(passive.y - active.y);
    var x = passive.x;
    var y = passive.y;
    world = delete_thing(world, active);
    /// notice the swap of dx/dy:
    world = insert_thing(world, mk_particle(x,y,dy,dx));
    world = insert_thing(world, mk_particle(x,y,-dy,-dx));
    world = notice(world, ['FISSION']); // ?!
    return(world);
};

var c_fission_passive = function(world, active,passive) {
    var dx = signum(active.x - passive.x);
    var dy = signum(active.y - passive.y);
    var x = active.x;
    var y = active.y;
    world = delete_thing(world, passive);
    /// notice the swap of dx/dy:
    world = insert_thing(world, mk_particle(x+dy,y+dx,dy,dx));
    world = insert_thing(world, mk_particle(x-dy,y-dx,-dy,-dx));
    world = notice(world, ['FISSION']); // ?!
    return(world);
};
*/

var c_death = function(world, active,passive) {
    /// does death come alone, or with eager reinforcements?
    active.x=passive.x;
    active.y=passive.y;
    //active.dead = true; 
    world = update_thing(world, active);
    world = notice(world, ['DIES',active.type]);
    return(world);
};

var c_push = function(world, active,passive) {
    var dx = signum(passive.x - active.x); // !!!
    var dy = signum(passive.y - active.y); // !!
    if(active.dx != undefined) {
	dx = active.dx; dy = active.dy; // way better.
    }
    var pt = passive.type;
    var x = passive.x;
    var y = passive.y;
    world = move_thing(world, passive, x+dx,y+dy);
    var is_it_still_there = find_by_pos(world,x,y);
    if(is_it_still_there==null || is_it_still_there.type!=pt) {
	world = move_thing(world, active, x,y);
	world = notice(world, ['PUSHED',active.type,passive.type]);
    } else {
	world = notice(world, ['FAILED TO PUSH',active.type,passive.type]);
    }
    return(world);
};

var c_fail = function(world, active,passive) {
    /// only for sfx/msg:
    world = notice(world, ['FAILED TO PUSH',active.type,passive.type,"from c_fail"]);
    return(world);
};

/*
var c_set_in_motion = function(world, active,passive) {
    var dx = signum(passive.x - active.x);
    var dy = signum(passive.y - active.y);
    passive.dx = dx;
    passive.dy = dy;
    world = update_thing(world, passive);
    world = notice(world, ['SETS IN MOTION',active.type,passive.type]);
    return(world);
};
*/

var c_pickup = function(world, active,passive) {
    if(active.inventory==undefined) active.inventory=[];
    active.inventory.push(passive); /// TODO sure?
    world = delete_thing(world, passive);
    world = update_thing(world, active);
    world = notice(world,['PICKUP',active.type,passive.type]);
    return(world);
};

/// BOOKMARK:KEYLOCK
/// here we can construct more sophisticated key-lock pairs,
/// eg keys with colors, or more sick pairs as in S.Ahmad's "CHIMERA"
/// (the "bread opens toaster" thing; a great game anyway).
var key_lock_match = function(key_candidate,lock) {
    return(key_candidate.type=='KEY' && lock.type=='DOOR');
};

var c_open = function(world, active,passive) {
    var inventory = active.inventory;
    if(inventory!=undefined) {
	for(var i=0;i<inventory.length;i++) {
	    var thing = inventory[i];
	    if(key_lock_match(thing,passive)) {
		inventory.splice(i,1);
		active.inventory = inventory; // ;)
		world = update_thing(world, active);
		world = delete_thing(world, passive);
		world = notice(world,['OPENED', active.type, passive.type]);
		return(world);
	    }
	}
    }
    world = notice(world,['FAILED TO OPEN', active.type, passive.type]);
    return(world);
};

/// for hero only!
var c_active2square = function(world, active,passive) {
    active.type = 'HERO-SQUARE';
    world = update_thing(world, active);
    world = notice(world, ["SHIFT TO", active.type, "SQUARE"]);
    return(world);
};
var c_active2triangle = function(world, active,passive) {
    active.type = 'HERO-TRIANGLE';
    world = update_thing(world, active);
    world = notice(world, ["SHIFT TO", active.type, "TRIANGLE"]);
    return(world);
};
var c_active2disk = function(world, active,passive) {
    active.type = 'HERO-DISK';
    world = update_thing(world, active);
    world = notice(world, ["SHIFT TO", active.type, "DISK"]);
    return(world);
};

var c_annihilate = function(world, active,passive) {
    world = c_explode_passive(world, active,passive);
    world = c_delete_active(world, active,passive);
    world = notice(world, ["ANNIHILATE",active.type]);
    return(world);
};

var c_active_stepover = function(world, active,passive) {
    active.x = passive.x;
    active.y = passive.y;
    //active.dx = active.dy = 0;
    world = delete_thing(world, passive);
    world = insert_thing(world, passive);
    return(world);
};

var c_active_teleport = function(world, active, passive) {
    var dx = signum(passive.x - active.x);
    var dy = signum(passive.y - active.y);
    if(active.dx != undefined) {
	/// don't subtract like this on Torus...
	dx = active.dx; dy = active.dy;
    }
    /// is it the end, and do we come from proper direction?
    if(passive.dx != undefined
       && passive.dx ==-dx
       && passive.dy ==-dy) {
	var pipe_ends = filter(find_all_by_label(world, passive.label),
			       function(e) {
				   return (e.x != passive.x
					   && e.y != passive.y
					   && e.type != 'TURNCOCK-H'
					   && e.type != 'TURNCOCK-V');
			       });

	/// "assert(pipe_ends.length==2)"
	var end = pipe_ends[0];
	/// oh, and check if it's open
	if(!end.open) {
	    world = notice(world, ["THIS PIPE IS CLOSED"]);
	    return world;
	}
	/// this is tricky, and I've got fever right now :)
	var ox = active.x, oy = active.y;
	var odx = dx, ody = dy;
	var nx = end.x + end.dx;
	var ny = end.y + end.dy;
	var ai = active.index;
	active.x = end.x; active.y = end.y;
	active.dx = end.dx; active.dy = end.dy;
	world = update_thing(world, active);
	world = move_thing(world, active, nx,ny);
	var check = world.things[ai];
	if(check.x==nx && check.y==ny) {
	    world = notice(world, ["TELEPORT",active.type,passive.type]); 
	} else {
	    active.x = ox; active.y = oy;
	    active.dx = odx; active.dx = ody;
	    world = update_thing(world, active);
	    world = notice(world, ["FAILED TO TELEPORT",active.type,passive.type]);
	}
    } else {
	/// no notice there I guess...
    }
    return(world);
};

var c_turncock = function(world, active,passive) {
    var pipe_ends = filter(find_all_by_label(world, passive.label),
			   function(e) {
			       return (e.type != 'TURNCOCK-H'
				       && e.type != 'TURNCOCK-V');
			   });
    for(var i=0;i<pipe_ends.length;i++) {
	var end = pipe_ends[i];
	end.open = true; ///!end.open; ?
	world = update_thing(world, end);
    }    
    world = notice(world,["TURNCOCK"]);
    return(world);
};

var c_just_tell = function(world, active,passive) {
    /// blee.
    world = notice(world,["NO UNSHIFTIN"]);
    return(world);
};


/// plus the action...
/// notice the non-categorical order, ie "compose(f,g)=gf"
var compose = function(f,g) {
    return(function(world, active, passive) {
	///return(g(f(world))); // why not?
	world = f(world,active,passive);
	world = g(world,active,passive);
	return(world);
    });
};

/// BOOKMARK:COLLISIONS
/// mind the order of the following rules, as ONLY the first
/// one that fits is being fired.
var collisions = {
    'HERO-SQUARE|SQUARE': c_push,
    'HERO-SQUARE|HOLE-SQUARE': c_death,
    'HERO-SQUARE|TRIANGLE': compose(c_push, c_active2triangle),
    'HERO-SQUARE|DISK':  compose(c_push, c_active2disk),
    'HERO-TRIANGLE|TRIANGLE': c_push,
    'HERO-TRIANGLE|DISK':  compose(c_push, c_active2disk),
    'HERO-TRIANGLE|HOLE-TRIANGLE': c_death,
    'HERO-TRIANGLE|MACHINE': compose(c_active2square,c_explode_passive),
    'HERO-DISK|DISK': c_push,
    'HERO-DISK|HOLE-DISK': c_death,
    'HERO-TRIANGLE|MACHINE':  compose(c_active2square,c_explode_passive),
    'HERO-DISK|MACHINE':  compose(c_active2triangle,c_explode_passive),

    'HERO-SQUARE|MACHINE': c_just_tell,

    'HERO-TRIANGLE|SQUARE': c_fail,
    'HERO-DISK|SQUARE': c_fail,
    'HERO-DISK|TRIANGLE': c_fail,

    'SQUARE|HOLE-SQUARE' : c_annihilate,
    'TRIANGLE|HOLE-TRIANGLE' : c_annihilate,
    'DISK|HOLE-DISK' : c_annihilate,

    'HERO-SQUARE|HOLE-DISK': c_active_stepover,
    'HERO-TRIANGLE|HOLE-DISK': c_active_stepover,
    'HERO-SQUARE|HOLE-TRIANGLE': c_active_stepover,
    'HERO-DISK|HOLE-TRIANGLE': c_active_stepover,
    'HERO-DISK|HOLE-SQUARE': c_active_stepover,
    'HERO-TRIANGLE|HOLE-SQUARE': c_active_stepover,
    'SQUARE|HOLE-DISK': c_active_stepover,
    'TRIANGLE|HOLE-DISK': c_active_stepover,
    'SQUARE|HOLE-TRIANGLE': c_active_stepover,
    'DISK|HOLE-TRIANGLE': c_active_stepover,
    'DISK|HOLE-SQUARE': c_active_stepover,
    'TRIANGLE|HOLE-SQUARE': c_active_stepover,

    'HERO-SQUARE|KEY': c_pickup,
    'HERO-TRIANGLE|KEY': c_pickup,
    'HERO-DISK|KEY': c_pickup,

    'HERO-SQUARE|DOOR': c_open,
    'HERO-TRIANGLE|DOOR': c_open,
    'HERO-DISK|DOOR': c_open,

    '*|TURNCOCK-H': c_turncock,
    '*|TURNCOCK-V': c_turncock,

    '*|PIPE-H': c_active_teleport,
    '*|PIPE-V': c_active_teleport,

    'PARTICLE|HERO-SQUARE' : c_death,
    'PARTICLE|HERO-TRIANGLE' : c_death,
    'PARTICLE|HERO-DISK' : c_death,
    'HERO-SQUARE|PARTICLE' : c_death,
    'HERO-TRIANGLE|PARTICLE' : c_death,
    'HERO-DISK|PARTICLE' : c_death,
    'PARTICLE|*' : c_explode_active,
    '*|PARTICLE' : c_explode_passive
};

var collision_for = function(active, passive) {
    /// NB: if in need of some unique collison, add it here!
    var collision = collisions[active.type+'|'+passive.type];    
    if(collision!=undefined) return(collision);
    /// ok, try less specific wrt passive.
    collision = collisions[active.type+'|*'];
    if(collision!=undefined) return(collision);
    /// mhm, try less specific wrt active.
    collision = collisions['*|'+passive.type];
    if(collision!=undefined) return(collision);
    /// no idea then.
    return(c_nothing);
};

/// movement (including collisions). ///////////////////////////////////////////

var move_thing = function(world, thing,x,y) {
    x=mod(x,world['hp']); y=mod(y,world['vp']); /// Torus Topology
    if(thing==null) return(world); /// defensive...
    var obstacle = find_by_pos(world,x,y);
    if(obstacle == null) {
	thing.x = x;
	thing.y = y;
	world = update_thing(world, thing);
    } else {
	//console.log(['col',thing.type,obstacle.type]);//dbg
	world = (collision_for(thing, obstacle))(world, thing,obstacle);
    }
    return(world);
};


/// the world cycle (including movements, and some other stuff as well). ///////

var step_trivial = function(world, thing) {
    return(world);
};

var step_movable = function(world, thing) {
    if(thing.dx==undefined
       || thing.dy==undefined
       || (thing.dx==0 && thing.dy==0))
	return(world); /// being defensive?
    var nx = thing.x+thing.dx;
    var ny = thing.y+thing.dy;
    world = move_thing(world, thing,nx,ny);
    return(world);
};


var step_gun = function(world, thing) {
    if(true || thing.on) { /// TMP
	if(--thing.count<=0) {
	    thing.count=thing.maxcount;
	    var dx=thing.dx;
	    var dy=thing.dy;
	    var px=thing.x;//+dx;
	    var py=thing.y;//+dy;
	    world = insert_thing(world, mk_particle(px,py,dx,dy));
	    //world = notice(world, ['GUN SHOOTS',px,py]); //?
	}
	world = update_thing(world, thing);
    }
    return(world);
};


var step_expires = function(world,thing) {
    if(thing.expires--<=0) {
	world = delete_thing(world, thing);
    } else {
	world = update_thing(world, thing);	
    }
    return(world);
};

/// BOOKMARK:STEPS
var step_for = function(thing) {
    switch(thing.type) {
    case 'BOOM':
    case 'FIRE':
	return(step_expires);

    case 'HERO-SQUARE':
    case 'HERO-TRIANGLE': 
    case 'HERO-DISK': 
    case 'PARTICLE':
	return(step_movable);

    case 'GUN':
	return(step_gun);

    default:
	return(step_trivial);
    }
};


var world_step=function(world) {   
    world = forget_facts(world);
    for(var i=0;i<world.things.length;i++) {
	var thing=world.things[i];
	if(thing==null) continue;
	/// TODO: maybe check whether this thing has any step at all?
	world = (step_for(thing))(world, thing);
    }
    world=new_world_order(world);
    return(world);
};


/// this is the end.
/// dear js, you are weird :)
var clone_obj = function(obj) {
    var keys = Object.keys(obj);
    var new_obj = {};
    for(var k=0;k<keys.length;k++) {
	new_obj[keys[k]] = obj[keys[k]];
    }
    return(new_obj);
};

var stdLegend = {
    '&':{'type':'HERO-SQUARE','dx':0,'dy':0,'facing':'left','inventory':[]},
    '#':{'type':'WALL'},
    'M':{'type':'MACHINE'},
    '(':{'type':'HOLE-DISK'},
    '<':{'type':'HOLE-TRIANGLE'},
    '[':{'type':'HOLE-SQUARE'},
    'O':{'type':'DISK'},
    'V':{'type':'TRIANGLE'},
    'H':{'type':'SQUARE'},
    '|':{'type':'PIPE-V'},
    '-':{'type':'PIPE-H'},
    'L':{'type':'PIPE-UR'},
    'J':{'type':'PIPE-UL'},
    '7':{'type':'PIPE-DL'},
    'T':{'type':'PIPE-DR'},
    ';':{'type':'KEY'},
    'I':{'type':'DOOR'}
};


var Levels = [
    /// level 0 -- nauka o regułach
    {'map' : [
	"......................",
	".....##############...",
	"....#M....I........#..",
	".####...####..&....#..",
	"#(.O....#..#....###...",
	"#<.V....#..#.;..#.....",
	"#[.H....#...####......",
	".#######.............." 
    ],
     'legend' : {},
     'name' : 'enter the torus'
    },
    /// level 1 -- nauka o wpadaniu w dziury.
    {'map' : [
	"..#..[..........#..",
	".#M..[.....&.....#.",
	"#....[.........V..#",
	".H...[...........<[",
	"###################"
    ],
     'legend' : {},
     'name' : 'holes'
    },
    /// level 2 -- nauka o dzialkach.
    {'map' : [
	".......M###..(..#.|.###.",
	".......##A#######.L-#.#B",
	"......#.............###.",
	"...####.................",
	"###(.O....#......#######",
	"...<......#..&..#....V..",
	"--7.....#.#.....#.T-----",
	"..|.....###..O..#.|.....",
	"..L------7#.....#.|....." 
    ],
     'legend' : {'A': {'type':'GUN','dx':0,'dy':1,'facing':'down','count':0,'maxcount':3},
		 'B': {'type':'GUN','dx':1,'dy':0,'facing':'right','count':0,'maxcount':6}},
     'name' : 'guns of torxton'
    },
    /// level 3 -- nauka o blokowaniu dzialek figurami
    {'map' : [
	"...##....###[###...",
	"........#.......#..",
	"#########.[...H.###",
	".............A..[..",
	"###..####.&.....###",
	"..#..#..#.......#..",
	"..#H.#...###[###...",
	"..#..#.....#.#....."
    ],
     'legend' : {'A': {'type':'GUN','dx':-1,'dy':0,'facing':'left','count':0,'maxcount':5}},
     'name' : 'panic'
    },

    /// level 4 -- nauka o ruroportacji
    {'map' : [
	".....TC7.....",
	"#####|#a#####",
	"#....|#.&....",
	"#....|#...B7.",
	"#...AJ#....|.",
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
	"D#...b......#",
	"..<......V..."
    ],
     'legend': {
	 "A":{'dx':-1,'dy':0,'type':'PIPE-H','label':'A','open':false},
	 "a":{'dx':0,'dy':1,'type':'PIPE-V','label':'A','open':false},
	 "B":{'dx':-1,'dy':0,'type':'PIPE-H','label':'B','open':true},
	 "b":{'dx':0,'dy':1,'type':'PIPE-V','label':'B','open':true},
	 "C":{'type':'TURNCOCK-H', 'label':'A'},
	 "D":{'type':'GUN','dx':0,'dy':-1,'facing':'up','count':0,'maxcount':5}},
     'name' : 'pipes'
    },
    /// level 5 ...
     {
      'map' : [
	  ".....|...######...........",
	  ".....L---b....##########..",
	  ".###.#<.V....;#.<#...(..#.",
	  ".#&#.#H####.###.V#......#.",
	  ".#H#D#......O.(..#...a..#.",
	  "AO.###[###.#######...L----",
	  "#.[....V................#.",
	  "---------7.T--------------",
	  ".........|<|..............",
	  "---7.....|.L--------------",
	  "...|.....|.....#..........",
	  ".#.L-----J.....#.###.###.#",
	  "M#.......#.....#.#M#.#.#.#",
	  "##...T-----B...#.#.#.###.#",
	  ".....|...#.....I..........",
	  ".#...C...#######.#.#.#.#.#"
      ],
	 'legend' : {
	     "A":{'dx':1,'dy':0,'type':'PIPE-H','label':'A','open':true},
	     "a":{'dx':0,'dy':-1,'type':'PIPE-V','label':'A','open':true},
	     "B":{'dx':1,'dy':0,'type':'PIPE-H','label':'B','open':false},
	     "b":{'dx':1,'dy':0,'type':'PIPE-H','label':'B','open':false},
	     "C":{'type':'TURNCOCK-V', 'label':'B'},
	     "D":{'type':'GUN','dx':0,'dy':-1,'facing':'up','count':0,'maxcount':7}
	 },
	 'name' : 'first dark ride'
     },
    /// level 6 oczoplas
    {
	'map' : [
	    "........#########MM|........|.#.....#.#..........",
	    "................#HHL--------JO#..V...............",
	    "##########...&..#...................#.#..########",
	    ".[[[......#######MMT--------7(#..V.......<<<.....",
	    "#######............|........|(#.....#.#..########",
	    ".......#...........|........|(#..V..............."
	],
	'legend' : {},
	'name' : 'nightclubing'
    },
    /// level 7
    {
	'map' : [
	    "....[....M####",
	    "....[T---a.[;#",
	    "....[C..###.<#",
	    "##I##|..#&.H.#",
	    "....AJ..#.V.<.",
	    "#####...###..#",
	    "..........####"
 	],
	'legend' : {
	    "A":{'dx':-1,'dy':0,'type':'PIPE-H','label':'A','open':false},
	    "a":{'dx':1,'dy':0,'type':'PIPE-H','label':'A','open':false},
	    "C":{'type':'TURNCOCK-V', 'label':'A'},
	},
	'name' : 'sokobong'
    },

    /// level 8
    {
	'map' : [
	    ".........||............",
	    "...T-----J|............",
	    "...|..####L--7......H..",
	    "...|..#..I...|####.....",
	    "...|###..###.|.<;#..H..",
	    "T--J#&....;#.A..<#.....",
	    "|;..###..###..V..##.###",
	    "|..V<.I..#[[[[[[[#.....",
	    "|.<V<.####.......I.....",
	    "|.<V<............(.....",
	    "L----------------------",
	    "###............I.V..<..",
	    "#.#...a--7T-------C----",
	    "###......||M...........",
	    ".D.......||........O...",
	    ".........||............",
	],
	'legend' : {
	    "A":{'dx':0,'dy':1,'type':'PIPE-V','label':'A','open':false},
	    "a":{'dx':-1,'dy':0,'type':'PIPE-H','label':'A','open':false},
	    "C":{'type':'TURNCOCK-H', 'label':'A'},
	    "D":{'type':'GUN','dx':0,'dy':1,'facing':'down','count':0,'maxcount':2}
	},
	'name' : 'coffee or tea'
    },

    /// level 9
    {
	'map' : [
	    "...[#........................#......|.#.......",
	    "...[#......T------------------------J.#.......",
	    "######I#<##|....#............#....##..########",
	    "....#.H....|H...###I###......#..T-B...........",
	    "###.#......|....<<<<<<<#.....#..|<#########.##",
	    "....#.##.H.|....<<<<<<<####.##..|<<#D.........",
	    "#####......|....<<<V....a7<.<#..|<<<#######.##",
	    "-----------J....<<<V.&...|<.<#..L----------C--",
	    ".........#[[.T-----------J<.<#................",
	    "##########[[.|............<.<#################",
	    ".........###[|#O#######O#.<M<#................",
	    "...........#[|#.#.....#.#..####...............",
	    "...........#;|#(#.V...#(###....#..............",
	    "-------------J###.#.#####.#...A---------------",
	    "...[#.....................#...........I.......",
	    "...[I.....................#..##T----7.#...O.(.",
	    "...[#...................H...V#.b....|.#.......",
	    "...[#.....................#..#....;.|.#.......",
	],
	'legend' : {
	    "A":{'dx':-1,'dy':0,'type':'PIPE-H','label':'A','open':true},
	    "a":{'dx':-1,'dy':0,'type':'PIPE-H','label':'A','open':true},
	    "B":{'dx':1,'dy':0,'type':'PIPE-H','label':'B','open':false},
	    "b":{'dx':0,'dy':1,'type':'PIPE-V','label':'B','open':false},
	    "C":{'type':'TURNCOCK-H', 'label':'B'},
	    "D":{'type':'GUN','dx':1,'dy':0,'facing':'right','count':0,'maxcount':8}
	},
	'name' : 'fire of mind'
    },

    /// level 10
    {
	'map' : [
	    "....d.D..",
	    ".........",
	    "&.......<",
	    ".........",
	    "..[[.H...",
	    "........V",
	    ".........",
	    "...H....."
 	],
	'legend' : {
	    "D":{'type':'GUN','dx':1,'dy':0,'facing':'right','count':0,'maxcount':5},
	    "d":{'type':'GUN','dx':0,'dy':-1,'facing':'up','count':2,'maxcount':5}
	},
	'name' : 'outer space'
    }

];

var load_level = function(num) {
    var things = [];
    var the_map = Levels[num].map;
    var the_legend = Levels[num].legend;
    var perim_v = the_map.length;
    var perim_h = the_map[0].length;
    for(var j=0;j<perim_v;j++)
	for(var i=0;i<perim_h;i++) {
	    var t = the_map[j][i];
	    var thing = stdLegend[t];
	    if(thing==undefined) thing = the_legend[t];
	    if(thing==undefined) continue;
	    var new_thing = clone_obj(thing);
	    new_thing.x = i;
	    new_thing.y = j;
	    things.push(new_thing);
	}
    world = mk_world(things,perim_v,perim_h); // muy importante!
    return new_world_order(world);
};

/// check how many of these little shits are on the level y'know...
/// YAEEEY, SHAPES AND NUMBERS! :D
var count_shapes = function(world) {
    var shapes={'squares':0,'triangles':0,'disks':0};
    for(var i=0;i<world.things.length;i++) {
	switch(world.things[i].type) {
	case 'SQUARE': shapes.squares++; break;
	case 'TRIANGLE': shapes.triangles++; break;
	case 'DISK': shapes.disks++; break;
	}
    }
    return shapes;
};/// INPUT /////////////////////////////////////////////////////
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
