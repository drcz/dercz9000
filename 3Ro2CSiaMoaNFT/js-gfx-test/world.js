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
