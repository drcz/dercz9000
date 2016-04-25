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
};