// parttype.h

#ifndef _INC_PARTTYPE
#define _INC_PARTTYPE

/////////////////////////////////////////////////////////////////////////////
// Particle types

#define	PARTICLE_TYPE_NONE								0
#define	PARTICLE_TYPE_ARROW								1
#define	PARTICLE_TYPE_BULLET								2
#define	PARTICLE_TYPE_SHOTGUN							3
#define	PARTICLE_TYPE_GRENADE							4
#define	PARTICLE_TYPE_PLASMA1							5
#define	PARTICLE_TYPE_PLASMA2							6
#define	PARTICLE_TYPE_BULLET_SHELL						7
#define	PARTICLE_TYPE_SHOTGUN_SHELL					8
#define	PARTICLE_TYPE_BLOOD								9
#define	PARTICLE_TYPE_WOODCHIP							10
#define	PARTICLE_TYPE_FOLIAGE							11
#define	PARTICLE_TYPE_EXPLOSION							12
#define	PARTICLE_TYPE_SPARK								13
#define	PARTICLE_TYPE_WATERSPLASH						14
#define	PARTICLE_TYPE_TEST								15
#define	PARTICLE_TYPE_ACIDSPIT							16
#define	PARTICLE_TYPE_BREATH								17
#define	PARTICLE_TYPE_DEATHRAIN							18
#define	PARTICLE_TYPE_ENERGYBLAST						19
#define	PARTICLE_TYPE_EYEFIRE							20
#define	PARTICLE_TYPE_FIREBALL							21
#define	PARTICLE_TYPE_SHOCKWAVE							22
#define	PARTICLE_TYPE_SPELLCAST							23
#define	PARTICLE_TYPE_SARROWFLASH						24
#define	PARTICLE_TYPE_SARROWFIZZLE						25
#define	PARTICLE_TYPE_ENEMY_BULLET						26
#define	PARTICLE_TYPE_ENEMY_PLASMA1					27
#define	PARTICLE_TYPE_DUSTCLOUD_FOOTFALL				28
#define	PARTICLE_TYPE_DUSTCLOUD_BODYFALL				29
#define	PARTICLE_TYPE_WEFFECT_ASSAULTRIFLE			30
#define	PARTICLE_TYPE_WEFFECT_AUTOMATICSHOTGUN		31
#define	PARTICLE_TYPE_WEFFECT_GRENADELAUNCHER		32
#define	PARTICLE_TYPE_WEFFECT_KNIFEFORWARD			33
#define	PARTICLE_TYPE_WEFFECT_KNIFELEFT				34
#define	PARTICLE_TYPE_WEFFECT_KNIFERIGHT				35
#define	PARTICLE_TYPE_WEFFECT_MACHINEGUN				36
#define	PARTICLE_TYPE_WEFFECT_MINIGUN					37
#define	PARTICLE_TYPE_WEFFECT_PISTOL					38
#define	PARTICLE_TYPE_WEFFECT_RIOTSHOTGUN			39
#define	PARTICLE_TYPE_WEFFECT_TECHWEAPON1			40
#define	PARTICLE_TYPE_WEFFECT_TECHWEAPON2			41
#define	PARTICLE_TYPE_ENEMY_MUZZLEFLASH				42
#define	PARTICLE_TYPE_HULK_BLASTER						43
#define	PARTICLE_TYPE_RAPTOR_PROJECTILE				44
#define	PARTICLE_TYPE_PLASMA_EXPLOSION				45
#define	PARTICLE_TYPE_FIREBALL_EXPLOSION				46
#define	PARTICLE_TYPE_TECH_EXPLOSION					47
#define	PARTICLE_TYPE_GREENBLOOD						48
#define	PARTICLE_TYPE_ROBOT_BULLET						49
#define	PARTICLE_TYPE_ROBOT_ROCKET						50
#define	PARTICLE_TYPE_SPURT_BLOOD						51
#define	PARTICLE_TYPE_GUSH_BLOOD						52
#define	PARTICLE_TYPE_FOUNTAIN_BLOOD					53
#define	PARTICLE_TYPE_DROOL								54
#define	PARTICLE_TYPE_ARTERIAL_BLOOD					55
#define	PARTICLE_TYPE_SPURT_GREENBLOOD				56
#define	PARTICLE_TYPE_GUSH_GREENBLOOD					57
#define	PARTICLE_TYPE_FOUNTAIN_GREENBLOOD			58
#define	PARTICLE_TYPE_ARTERIAL_GREENBLOOD			59
#define	PARTICLE_TYPE_KNIFELEFT_BLOOD					60
#define	PARTICLE_TYPE_KNIFELEFT_GBLOOD				61
#define	PARTICLE_TYPE_KNIFELEFT_SPARK					62
#define	PARTICLE_TYPE_KNIFERIGHT_BLOOD				63
#define	PARTICLE_TYPE_KNIFERIGHT_GBLOOD				64
#define	PARTICLE_TYPE_KNIFERIGHT_SPARK				65
#define	PARTICLE_TYPE_KNIFEFORWARD_BLOOD				66
#define	PARTICLE_TYPE_KNIFEFORWARD_GBLOOD			67
#define	PARTICLE_TYPE_KNIFEFORWARD_SPARK				68
#define	PARTICLE_TYPE_TOMAHAWKLEFT_BLOOD				69
#define	PARTICLE_TYPE_TOMAHAWKLEFT_GBLOOD			70
#define	PARTICLE_TYPE_TOMAHAWKLEFT_SPARK				71
#define	PARTICLE_TYPE_TOMAHAWKRIGHT_BLOOD			72
#define	PARTICLE_TYPE_TOMAHAWKRIGHT_GBLOOD			73
#define	PARTICLE_TYPE_TOMAHAWKRIGHT_SPARK			74
#define	PARTICLE_TYPE_TOMAHAWKFORWARD_BLOOD			75
#define	PARTICLE_TYPE_TOMAHAWKFORWARD_GBLOOD		76
#define	PARTICLE_TYPE_TOMAHAWKFORWARD_SPARK			77
#define	PARTICLE_TYPE_KTFATAL_BLOOD					78
#define	PARTICLE_TYPE_KTFATAL_GBLOOD					79
#define	PARTICLE_TYPE_KTFATAL_SPARK					80
#define	PARTICLE_TYPE_ENEMY_PLASMA2					81
#define	PARTICLE_TYPE_WATER_DROP						82
#define	PARTICLE_TYPE_WATER_FOAM						83
#define	PARTICLE_TYPE_WATER_STEAM						84
#define	PARTICLE_TYPE_AUTOSHOTGUN						85
#define	PARTICLE_TYPE_GENERIC1							86
#define	PARTICLE_TYPE_GENERIC2							87
#define	PARTICLE_TYPE_GENERIC3							88
#define	PARTICLE_TYPE_GENERIC4							89
#define	PARTICLE_TYPE_GENERIC5							90
#define	PARTICLE_TYPE_GENERIC6							91
#define	PARTICLE_TYPE_GENERIC7							92
#define	PARTICLE_TYPE_GENERIC8							93
#define	PARTICLE_TYPE_GENERIC9							94
#define	PARTICLE_TYPE_GENERIC10							95
#define	PARTICLE_TYPE_GENERIC11							96
#define	PARTICLE_TYPE_GENERIC12							97
#define	PARTICLE_TYPE_GENERIC13							98
#define	PARTICLE_TYPE_GENERIC14							99
#define	PARTICLE_TYPE_GENERIC15							100
#define	PARTICLE_TYPE_GENERIC16							101
#define	PARTICLE_TYPE_GENERIC17							102
#define	PARTICLE_TYPE_GENERIC18							103
#define	PARTICLE_TYPE_GENERIC19							104
#define	PARTICLE_TYPE_GENERIC20							105
#define	PARTICLE_TYPE_GENERIC21							106
#define	PARTICLE_TYPE_GENERIC22							107
#define	PARTICLE_TYPE_GENERIC23							108
#define	PARTICLE_TYPE_GENERIC24							109
#define	PARTICLE_TYPE_GENERIC25							110
#define	PARTICLE_TYPE_GENERIC26							111
#define	PARTICLE_TYPE_GENERIC27							112
#define	PARTICLE_TYPE_GENERIC28							113
#define	PARTICLE_TYPE_GENERIC29							114
#define	PARTICLE_TYPE_GENERIC30							115
#define	PARTICLE_TYPE_ROCKET								116
#define	PARTICLE_TYPE_SHOCKWAVEPULSE1					117
#define	PARTICLE_TYPE_WEFFECT_ROCKET					118
#define	PARTICLE_TYPE_WEFFECT_SHOCKWAVEPULSE		119
#define	PARTICLE_TYPE_LHGUN_PULSE						120
#define	PARTICLE_TYPE_WATER_BUBBLE						121
#define	PARTICLE_TYPE_WATER_RIPPLE						122
#define	PARTICLE_TYPE_WATER_SPLASH						123
#define	PARTICLE_TYPE_GENERIC31							124
#define	PARTICLE_TYPE_GENERIC32							125
#define	PARTICLE_TYPE_GENERIC33							126
#define	PARTICLE_TYPE_GENERIC34							127
#define	PARTICLE_TYPE_GENERIC35							128
#define	PARTICLE_TYPE_GENERIC36							129
#define	PARTICLE_TYPE_GENERIC37							130
#define	PARTICLE_TYPE_GENERIC38							131
#define	PARTICLE_TYPE_GENERIC39							132
#define	PARTICLE_TYPE_GENERIC40							133
#define	PARTICLE_TYPE_GENERIC41							134
#define	PARTICLE_TYPE_GENERIC42							135
#define	PARTICLE_TYPE_GENERIC43							136
#define	PARTICLE_TYPE_GENERIC44							137
#define	PARTICLE_TYPE_GENERIC45							138
#define	PARTICLE_TYPE_GENERIC46							139
#define	PARTICLE_TYPE_GENERIC47							140
#define	PARTICLE_TYPE_GENERIC48							141
#define	PARTICLE_TYPE_GENERIC49							142
#define	PARTICLE_TYPE_GENERIC50							143
#define	PARTICLE_TYPE_GENERIC51							144
#define	PARTICLE_TYPE_GENERIC52							145
#define	PARTICLE_TYPE_GENERIC53							146
#define	PARTICLE_TYPE_GENERIC54							147
#define	PARTICLE_TYPE_GENERIC55							148
#define	PARTICLE_TYPE_GENERIC56							149
#define	PARTICLE_TYPE_GENERIC57							150
#define	PARTICLE_TYPE_GENERIC58							151
#define	PARTICLE_TYPE_GENERIC59							152
#define	PARTICLE_TYPE_GENERIC60							153
#define	PARTICLE_TYPE_GENERIC61							154
#define	PARTICLE_TYPE_GENERIC62							155
#define	PARTICLE_TYPE_GENERIC63							156
#define	PARTICLE_TYPE_GENERIC64							157
#define	PARTICLE_TYPE_GENERIC65							158
#define	PARTICLE_TYPE_GENERIC66							159
#define	PARTICLE_TYPE_GENERIC67							160
#define	PARTICLE_TYPE_GENERIC68							161
#define	PARTICLE_TYPE_GENERIC69							162
#define	PARTICLE_TYPE_GENERIC70							163
#define	PARTICLE_TYPE_GENERIC71							164
#define	PARTICLE_TYPE_GENERIC72							165
#define	PARTICLE_TYPE_GENERIC73							166
#define	PARTICLE_TYPE_GENERIC74							167
#define	PARTICLE_TYPE_GENERIC75							168
#define	PARTICLE_TYPE_GENERIC76							169
#define	PARTICLE_TYPE_GENERIC77							170
#define	PARTICLE_TYPE_GENERIC78							171
#define	PARTICLE_TYPE_GENERIC79							172
#define	PARTICLE_TYPE_GENERIC80							173
#define	PARTICLE_TYPE_GENERIC81							174
#define	PARTICLE_TYPE_GENERIC82							175
#define	PARTICLE_TYPE_GENERIC83							176
#define	PARTICLE_TYPE_GENERIC84							177
#define	PARTICLE_TYPE_GENERIC85							178
#define	PARTICLE_TYPE_GENERIC86							179
#define	PARTICLE_TYPE_GENERIC87							180
#define	PARTICLE_TYPE_GENERIC88							181
#define	PARTICLE_TYPE_GENERIC89							182
#define	PARTICLE_TYPE_GENERIC90							183
#define	PARTICLE_TYPE_GENERIC91							184
#define	PARTICLE_TYPE_GENERIC92							185
#define	PARTICLE_TYPE_GENERIC93							186
#define	PARTICLE_TYPE_GENERIC94							187
#define	PARTICLE_TYPE_GENERIC95							188
#define	PARTICLE_TYPE_GENERIC96							189
#define	PARTICLE_TYPE_GENERIC97							190
#define	PARTICLE_TYPE_GENERIC98							191
#define	PARTICLE_TYPE_GENERIC99							192
#define	PARTICLE_TYPE_GENERIC100						193
#define	PARTICLE_TYPE_ARROWEXPLOSIVE					194
#define	PARTICLE_TYPE_SHOTGUNEXPLOSIVE				195
#define	PARTICLE_TYPE_SHOTGUN_EXPLOSIVE_SHELL		196
#define	PARTICLE_TYPE_SHOCKWAVE_CHARGE				197
#define	PARTICLE_TYPE_SHOCKWAVEPULSE2					198
#define	PARTICLE_TYPE_SHOCKWAVEPULSE3					199
#define	PARTICLE_TYPE_SHOCKWAVEPULSE4					200
#define	PARTICLE_TYPE_SHOCKWAVEPULSE5					201
#define	PARTICLE_TYPE_GENERIC101						202	
#define	PARTICLE_TYPE_GENERIC102						203	
#define	PARTICLE_TYPE_GENERIC103						204	
#define	PARTICLE_TYPE_GENERIC104						205	
#define	PARTICLE_TYPE_GENERIC105						206	
#define	PARTICLE_TYPE_GENERIC106						207	
#define	PARTICLE_TYPE_GENERIC107						208	
#define	PARTICLE_TYPE_GENERIC108						209	
#define	PARTICLE_TYPE_GENERIC109						210	
#define	PARTICLE_TYPE_GENERIC110						211	
#define	PARTICLE_TYPE_GENERIC111						212	
#define	PARTICLE_TYPE_GENERIC112						213	
#define	PARTICLE_TYPE_GENERIC113						214	
#define	PARTICLE_TYPE_GENERIC114						215	
#define	PARTICLE_TYPE_GENERIC115						216	
#define	PARTICLE_TYPE_GENERIC116						217	
#define	PARTICLE_TYPE_GENERIC117						218	
#define	PARTICLE_TYPE_GENERIC118						219	
#define	PARTICLE_TYPE_GENERIC119						220	
#define	PARTICLE_TYPE_GENERIC120						221	
#define	PARTICLE_TYPE_GENERIC121						222	
#define	PARTICLE_TYPE_GENERIC122						223	
#define	PARTICLE_TYPE_GENERIC123						224	
#define	PARTICLE_TYPE_GENERIC124						225	
#define	PARTICLE_TYPE_GENERIC125						226	
#define	PARTICLE_TYPE_GENERIC126						227	
#define	PARTICLE_TYPE_GENERIC127						228	
#define	PARTICLE_TYPE_GENERIC128						229	
#define	PARTICLE_TYPE_GENERIC129						230	
#define	PARTICLE_TYPE_GENERIC130						231	
#define	PARTICLE_TYPE_GENERIC131						232	
#define	PARTICLE_TYPE_GENERIC132						233	
#define	PARTICLE_TYPE_GENERIC133						234	
#define	PARTICLE_TYPE_GENERIC134						235	
#define	PARTICLE_TYPE_GENERIC135						236	
#define	PARTICLE_TYPE_GENERIC136						237	
#define	PARTICLE_TYPE_GENERIC137						238	
#define	PARTICLE_TYPE_GENERIC138						239	
#define	PARTICLE_TYPE_GENERIC139						240	
#define	PARTICLE_TYPE_GENERIC140						241	
#define	PARTICLE_TYPE_GENERIC141						242	
#define	PARTICLE_TYPE_GENERIC142						243	
#define	PARTICLE_TYPE_GENERIC143						244	
#define	PARTICLE_TYPE_GENERIC144						245	
#define	PARTICLE_TYPE_GENERIC145						246	
#define	PARTICLE_TYPE_GENERIC146						247	
#define	PARTICLE_TYPE_GENERIC147						248	
#define	PARTICLE_TYPE_GENERIC148						249	
#define	PARTICLE_TYPE_GENERIC149						250	
#define	PARTICLE_TYPE_GENERIC150						251	
#define	PARTICLE_TYPE_GENERIC151						252	
#define	PARTICLE_TYPE_GENERIC152						253	
#define	PARTICLE_TYPE_GENERIC153						254	
#define	PARTICLE_TYPE_GENERIC154						255	
#define	PARTICLE_TYPE_GENERIC155						256	
#define	PARTICLE_TYPE_GENERIC156						257	
#define	PARTICLE_TYPE_GENERIC157						258	
#define	PARTICLE_TYPE_GENERIC158						259	
#define	PARTICLE_TYPE_GENERIC159						260	
#define	PARTICLE_TYPE_GENERIC160						261	
#define	PARTICLE_TYPE_GENERIC161						262
#define	PARTICLE_TYPE_GENERIC162						263
#define	PARTICLE_TYPE_GENERIC163						264
#define	PARTICLE_TYPE_GENERIC164						265
#define	PARTICLE_TYPE_GENERIC165						266
#define	PARTICLE_TYPE_GENERIC166						267
#define	PARTICLE_TYPE_GENERIC167						268
#define	PARTICLE_TYPE_GENERIC168						269
#define	PARTICLE_TYPE_GENERIC169						270
#define	PARTICLE_TYPE_GENERIC170						271
#define	PARTICLE_TYPE_GENERIC171						272
#define	PARTICLE_TYPE_GENERIC172						273
#define	PARTICLE_TYPE_GENERIC173						274
#define	PARTICLE_TYPE_GENERIC174						275
#define	PARTICLE_TYPE_GENERIC175						276
#define	PARTICLE_TYPE_GENERIC176						277
#define	PARTICLE_TYPE_GENERIC177						278
#define	PARTICLE_TYPE_GENERIC178						279
#define	PARTICLE_TYPE_GENERIC179						280
#define	PARTICLE_TYPE_GENERIC180						281
#define	PARTICLE_TYPE_GENERIC181						282
#define	PARTICLE_TYPE_GENERIC182						283
#define	PARTICLE_TYPE_GENERIC183						284
#define	PARTICLE_TYPE_GENERIC184						285
#define	PARTICLE_TYPE_GENERIC185						286
#define	PARTICLE_TYPE_GENERIC186						287
#define	PARTICLE_TYPE_GENERIC187						288
#define	PARTICLE_TYPE_GENERIC188						289
#define	PARTICLE_TYPE_GENERIC189						290
#define	PARTICLE_TYPE_GENERIC190						291
#define	PARTICLE_TYPE_GENERIC191						292
#define	PARTICLE_TYPE_GENERIC192						293
#define	PARTICLE_TYPE_GENERIC193						294
#define	PARTICLE_TYPE_GENERIC194						295
#define	PARTICLE_TYPE_GENERIC195						296
#define	PARTICLE_TYPE_GENERIC196						297
#define	PARTICLE_TYPE_GENERIC197						298
#define	PARTICLE_TYPE_GENERIC198						299
#define	PARTICLE_TYPE_GENERIC199						300
#define	PARTICLE_TYPE_GENERIC200						301
#define	PARTICLE_TYPE_REGENERATION_APPEARANCE		302
#define	PARTICLE_TYPE_MINIGUN_BULLET					303
#define	PARTICLE_TYPE_PLAYER_IMPACT_BLOOD			304
#define	PARTICLE_TYPE_FREEZE_EXPLOSION				305
#define	PARTICLE_TYPE_PICKUPSPECIAL1					306
#define	PARTICLE_TYPE_PICKUPSPECIAL2					307
#define	PARTICLE_TYPE_PICKUPSPECIAL3					308
#define	PARTICLE_TYPE_FREEZE_START						309
#define	PARTICLE_TYPE_CHRONOSCEPTER					310
#define	PARTICLE_TYPE_WEFFECT_CHRONOSCEPTER			311
#define	PARTICLE_TYPE_ENEMY_LONGHUNTER				312
#define	PARTICLE_TYPE_CHRONOSCEPTERCHARGE			313
#define	PARTICLE_TYPE_REGENERATION_START				314
#define	PARTICLE_TYPE_GENERIC201						315	
#define	PARTICLE_TYPE_GENERIC202						316	
#define	PARTICLE_TYPE_GENERIC203						317	
#define	PARTICLE_TYPE_GENERIC204						318	
#define	PARTICLE_TYPE_GENERIC205						319	
#define	PARTICLE_TYPE_GENERIC206						320	
#define	PARTICLE_TYPE_GENERIC207						321	
#define	PARTICLE_TYPE_GENERIC208						322	
#define	PARTICLE_TYPE_GENERIC209						323	
#define	PARTICLE_TYPE_GENERIC210						324	
#define	PARTICLE_TYPE_GENERIC211						325	
#define	PARTICLE_TYPE_GENERIC212						326	
#define	PARTICLE_TYPE_GENERIC213						327	
#define	PARTICLE_TYPE_GENERIC214						328	
#define	PARTICLE_TYPE_GENERIC215						329	
#define	PARTICLE_TYPE_GENERIC216						330	
#define	PARTICLE_TYPE_GENERIC217						331	
#define	PARTICLE_TYPE_GENERIC218						332	
#define	PARTICLE_TYPE_GENERIC219						333	
#define	PARTICLE_TYPE_GENERIC220						334	
#define	PARTICLE_TYPE_GENERIC221						335	
#define	PARTICLE_TYPE_GENERIC222						336	
#define	PARTICLE_TYPE_GENERIC223						337	
#define	PARTICLE_TYPE_GENERIC224						338	
#define	PARTICLE_TYPE_GENERIC225						339	
#define	PARTICLE_TYPE_GENERIC226						340	
#define	PARTICLE_TYPE_GENERIC227						341	
#define	PARTICLE_TYPE_GENERIC228						342	
#define	PARTICLE_TYPE_GENERIC229						343	
#define	PARTICLE_TYPE_GENERIC230						344	
#define	PARTICLE_TYPE_GENERIC231						345	
#define	PARTICLE_TYPE_GENERIC232						346	
#define	PARTICLE_TYPE_GENERIC233						347	
#define	PARTICLE_TYPE_GENERIC234						348	
#define	PARTICLE_TYPE_GENERIC235						349	
#define	PARTICLE_TYPE_GENERIC236						350	
#define	PARTICLE_TYPE_GENERIC237						351	
#define	PARTICLE_TYPE_GENERIC238						352	
#define	PARTICLE_TYPE_GENERIC239						353	
#define	PARTICLE_TYPE_GENERIC240						354	
#define	PARTICLE_TYPE_GENERIC241						355	
#define	PARTICLE_TYPE_GENERIC242						356	
#define	PARTICLE_TYPE_GENERIC243						357	
#define	PARTICLE_TYPE_GENERIC244						358	
#define	PARTICLE_TYPE_GENERIC245						359	
#define	PARTICLE_TYPE_GENERIC246						360	
#define	PARTICLE_TYPE_GENERIC247						361	
#define	PARTICLE_TYPE_GENERIC248						362	
#define	PARTICLE_TYPE_GENERIC249						363	
#define	PARTICLE_TYPE_GENERIC250						364	
#define	PARTICLE_TYPE_GENERIC251						365	
#define	PARTICLE_TYPE_GENERIC252						366	
#define	PARTICLE_TYPE_GENERIC253						367	
#define	PARTICLE_TYPE_GENERIC254						368	
#define	PARTICLE_TYPE_GENERIC255						369	
#define	PARTICLE_TYPE_GENERIC256						370	
#define	PARTICLE_TYPE_GENERIC257						371	
#define	PARTICLE_TYPE_GENERIC258						372	
#define	PARTICLE_TYPE_GENERIC259						373	
#define	PARTICLE_TYPE_GENERIC260						374	

#define	PARTICLE_TYPE_MAX									375


#ifdef PARTICLE_DECLARE_STRINGS
char *Particle_Strings[PARTICLE_TYPE_MAX]={
"<none>",
"Arrow",					
"Bullet",					
"Shotgun",				
"Grenade",				
"Plasma1",				
"Plasma2",				
"Bullet shell",			
"Shotgun shell",		
"Blood",					
"Woodchip",				
"Foliage",				
"Explosion",				
"Spark",					
"Watersplash",			
"Test",					
"Acidspit",				
"Breath",					
"Deathrain",				
"Energyblast",			
"Eyefire",				
"Fireball",				
"Shockwave",				
"Spellcast",				
"Super Arrow Flash",			
"Super Arrow Fizzle",			
"Enemy bullet",			
"Enemy plasma1",		
"Dustcloud footfall",	
"Dustcloud bodyfall",	
"Weffect assaultrifle",			
"Weffect automaticshotgun",		
"Weffect grenadelauncher",		
"Weffect knifeforward",			
"Weffect knifeleft",				
"Weffect kniferight",				
"Weffect machinegun",				
"Weffect minigun",					
"Weffect pistol",					
"Weffect riotshotgun",			
"Weffect techweapon1",			
"Weffect techweapon2",			
"Enemy muzzleflash",				
"Hulk blaster",						
"Raptor projectile",				
"Plasma explosion",				
"Fireball explosion",				
"Tech explosion",					
"Greenblood",						
"Robot bullet",						
"Robot rocket",						
"Spurt blood",						
"Gush blood",						
"Fountain blood",					
"Drool",								
"Arterial blood",					
"Spurt greenblood",				
"Gush greenblood",					
"Fountain greenblood",			
"Arterial greenblood",			
"Knifeleft blood",					
"Knifeleft gblood",				
"Knifeleft spark",					
"Kniferight blood",				
"Kniferight gblood",				
"Kniferight spark",				
"Knifeforward blood",				
"Knifeforward gblood",			
"Knifeforward spark",				
"Tomahawkleft blood",				
"Tomahawkleft gblood",			
"Tomahawkleft spark",				
"Tomahawkright blood",			
"Tomahawkright gblood",			
"Tomahawkright spark",			
"Tomahawkforward blood",			
"Tomahawkforward gblood",		
"Tomahawkforward spark",			
"K/T Fatal blood",					
"K/T Fatal gblood",					
"K/T Fatal spark",					
"Enemy plasma2",					
"Water drop",						
"Water foam",						
"Water steam",						
"Auto Shotgun",
"Generic 1",
"Generic 2",
"Generic 3",
"Generic 4",
"Generic 5",
"Generic 6",
"Generic 7",
"Generic 8",
"Generic 9",
"Generic 10",
"Generic 11",
"Generic 12",
"Generic 13",
"Generic 14",
"Generic 15",
"Generic 16",
"Generic 17",
"Generic 18",
"Generic 19",
"Generic 20",
"Generic 21",
"Generic 22",
"Generic 23",
"Generic 24",
"Generic 25",
"Generic 26",
"Generic 27",
"Generic 28",
"Generic 29",
"Generic 30",
"Rocket",
"Shockwave pulse 1 (Weakest)",
"Weffect rocket",
"Weffect Shockwave pulse",
"Longhunter gun pulse",
"Water Bubble",
"Water Ripple",
"Water Splash",
"Generic 31",
"Generic 32",
"Generic 33",
"Generic 34",
"Generic 35",
"Generic 36",
"Generic 37",
"Generic 38",
"Generic 39",
"Generic 40",
"Generic 41",
"Generic 42",
"Generic 43",
"Generic 44",
"Generic 45",
"Generic 46",
"Generic 47",
"Generic 48",
"Generic 49",
"Generic 50",
"Generic 51",
"Generic 52",
"Generic 53",
"Generic 54",
"Generic 55",
"Generic 56",
"Generic 57",
"Generic 58",
"Generic 59",
"Generic 60",
"Generic 61",
"Generic 62",
"Generic 63",
"Generic 64",
"Generic 65",
"Generic 66",
"Generic 67",
"Generic 68",
"Generic 69",
"Generic 70",
"Generic 71",
"Generic 72",
"Generic 73",
"Generic 74",
"Generic 75",
"Generic 76",
"Generic 77",
"Generic 78",
"Generic 79",
"Generic 80",
"Generic 81",
"Generic 82",
"Generic 83",
"Generic 84",
"Generic 85",
"Generic 86",
"Generic 87",
"Generic 88",
"Generic 89",
"Generic 90",
"Generic 91",
"Generic 92",
"Generic 93",
"Generic 94",
"Generic 95",
"Generic 96",
"Generic 97",
"Generic 98",
"Generic 99",
"Generic 100",
"Arrow (Explosive)",
"Shotgun (Explosive)",
"Shotgun Shell (Explosive)",
"Shockwave Charge",
"Shockwave pulse 2",
"Shockwave pulse 3",
"Shockwave pulse 4",
"Shockwave pulse 5 (Strongest)",
"Generic 101",
"Generic 102",
"Generic 103",
"Generic 104",
"Generic 105",
"Generic 106",
"Generic 107",
"Generic 108",
"Generic 109",
"Generic 110",
"Generic 111",
"Generic 112",
"Generic 113",
"Generic 114",
"Generic 115",
"Generic 116",
"Generic 117",
"Generic 118",
"Generic 119",
"Generic 120",
"Generic 121",
"Generic 122",
"Generic 123",
"Generic 124",
"Generic 125",
"Generic 126",
"Generic 127",
"Generic 128",
"Generic 129",
"Generic 130",
"Generic 131",
"Generic 132",
"Generic 133",
"Generic 134",
"Generic 135",
"Generic 136",
"Generic 137",
"Generic 138",
"Generic 139",
"Generic 140",
"Generic 141",
"Generic 142",
"Generic 143",
"Generic 144",
"Generic 145",
"Generic 146",
"Generic 147",
"Generic 148",
"Generic 149",
"Generic 150",
"Generic 151",
"Generic 152",
"Generic 153",
"Generic 154",
"Generic 155",
"Generic 156",
"Generic 157",
"Generic 158",
"Generic 159",
"Generic 160",
"Generic 161",
"Generic 162",
"Generic 163",
"Generic 164",
"Generic 165",
"Generic 166",
"Generic 167",
"Generic 168",
"Generic 169",
"Generic 170",
"Generic 171",
"Generic 172",
"Generic 173",
"Generic 174",
"Generic 175",
"Generic 176",
"Generic 177",
"Generic 178",
"Generic 179",
"Generic 180",
"Generic 181",
"Generic 182",
"Generic 183",
"Generic 184",
"Generic 185",
"Generic 186",
"Generic 187",
"Generic 188",
"Generic 189",
"Generic 190",
"Generic 191",
"Generic 192",
"Generic 193",
"Generic 194",
"Generic 195",
"Generic 196",
"Generic 197",
"Generic 198",
"Generic 199",
"Generic 200",
"Regeneration Appearance",
"Minigun Bullet",
"Player Impact Blood",
"Freeze Explosion",
"Pickup Special 1",
"Pickup Special 2",
"Pickup Special 3",
"Freeze Start",
"Chronscepter",
"Weffect Chronoscepter",
"Enemy longhunter",
"Chronscepter Charge",
"Regeneration Start",
"Generic 201",
"Generic 202",
"Generic 203",
"Generic 204",
"Generic 205",
"Generic 206",
"Generic 207",
"Generic 208",
"Generic 209",
"Generic 210",
"Generic 211",
"Generic 212",
"Generic 213",
"Generic 214",
"Generic 215",
"Generic 216",
"Generic 217",
"Generic 218",
"Generic 219",
"Generic 220",
"Generic 221",
"Generic 222",
"Generic 223",
"Generic 224",
"Generic 225",
"Generic 226",
"Generic 227",
"Generic 228",
"Generic 229",
"Generic 230",
"Generic 231",
"Generic 232",
"Generic 233",
"Generic 234",
"Generic 235",
"Generic 236",
"Generic 237",
"Generic 238",
"Generic 239",
"Generic 240",
"Generic 241",
"Generic 242",
"Generic 243",
"Generic 244",
"Generic 245",
"Generic 246",
"Generic 247",
"Generic 248",
"Generic 249",
"Generic 250",
"Generic 251",
"Generic 252",
"Generic 253",
"Generic 254",
"Generic 255",
"Generic 256",
"Generic 257",
"Generic 258",
"Generic 259",
"Generic 260",
};
#endif
extern char *Particle_Strings[];


/////////////////////////////////////////////////////////////////////////////
// particle impact types - MUST BE SAME ORDER AS REGMAT_ IN  ---- regtype.h

#define	PARTICLE_IMPACTS_GRASS				0
#define	PARTICLE_IMPACTS_WATERSURFACE		1
#define	PARTICLE_IMPACTS_STEEL				2
#define	PARTICLE_IMPACTS_STONE				3
#define	PARTICLE_IMPACTS_FLESH				4
#define	PARTICLE_IMPACTS_ALIENFLESH		5
#define	PARTICLE_IMPACTS_FLESHWATER		6
#define	PARTICLE_IMPACTS_LAVA				7
#define	PARTICLE_IMPACTS_TAR					8
#define	PARTICLE_IMPACTS_FORCEFIELD		9

#define	PARTICLE_IMPACTS_ENDLIFE			10
#define	PARTICLE_IMPACTS_EVERYFRAME		11
#define	PARTICLE_IMPACTS_ENDLIFEWATER		12
#define	PARTICLE_IMPACTS_EVERYFRAMEWATER	13

#define	PARTICLE_IMPACTS_AMT					14


#ifdef PARTICLE_IMPACTS_DECLARE_STRINGS
char *Particle_Impact_Strings[PARTICLE_IMPACTS_AMT]={
	"Grass",
	"Water Surface",
	"Steel",
	"Stone",
	"Flesh",
	"Alien Flesh",
	"Flesh (Water)",
	"Lava",
	"Tar",
	"Force Field",
	"End Of Life",
	"Every Frame",
	"End Of Life (Water)",
	"Every Frame (Water)",
};
#endif
extern char *Particle_Impact_Strings[];
	

#ifdef WIN32
// pre version 32 of particle impact effects serialize
#define	PRE32_PARTICLE_IMPACTS_GRASS					0
#define	PRE32_PARTICLE_IMPACTS_DIRT					1
#define	PRE32_PARTICLE_IMPACTS_WATERSURFACE			2
#define	PRE32_PARTICLE_IMPACTS_STEEL					3
#define	PRE32_PARTICLE_IMPACTS_STONE					4
#define	PRE32_PARTICLE_IMPACTS_WOOD					5
#define	PRE32_PARTICLE_IMPACTS_FOLIAGE				6
#define	PRE32_PARTICLE_IMPACTS_FLESH					7
#define	PRE32_PARTICLE_IMPACTS_ALIENFLESH			8
#define  PRE32_PARTICLE_IMPACTS_WATERFLOOR			9
#define	PRE32_PARTICLE_IMPACTS_FLESHWATER			10
#define	PRE32_PARTICLE_IMPACTS_LAVA					11
#define	PRE32_PARTICLE_IMPACTS_TAR						12
#define	PRE32_PARTICLE_IMPACTS_FORCEFIELD			13
#define	PRE32_PARTICLE_IMPACTS_ENDLIFE				14
#define	PRE32_PARTICLE_IMPACTS_EVERYFRAME			15
#define	PRE32_PARTICLE_IMPACTS_ENDLIFEWATER			16
#define	PRE32_PARTICLE_IMPACTS_EVERYFRAMEWATER		17

// pre version 27 of particle impact effects serialize
#define	PRE27_PARTICLE_IMPACTS_GRASS					0
#define	PRE27_PARTICLE_IMPACTS_DIRT					1
#define	PRE27_PARTICLE_IMPACTS_WATERSURFACE			2
#define	PRE27_PARTICLE_IMPACTS_STEEL					3
#define	PRE27_PARTICLE_IMPACTS_STONE					4
#define	PRE27_PARTICLE_IMPACTS_WOOD					5
#define	PRE27_PARTICLE_IMPACTS_FOLIAGE				6
#define	PRE27_PARTICLE_IMPACTS_FLESH					7
#define	PRE27_PARTICLE_IMPACTS_ALIENFLESH			8
#define  PRE27_PARTICLE_IMPACTS_WATERFLOOR			9
#define	PRE27_PARTICLE_IMPACTS_FLESHWATER			10
#define	PRE27_PARTICLE_IMPACTS_ENDLIFE				11
#define	PRE27_PARTICLE_IMPACTS_EVERYFRAME			12
#define	PRE27_PARTICLE_IMPACTS_ENDLIFEWATER			13
#define	PRE27_PARTICLE_IMPACTS_EVERYFRAMEWATER		14

// pre version 19 of particle impact effects serialize
#define	PRE19_PARTICLE_IMPACTS_GRASS			0
#define	PRE19_PARTICLE_IMPACTS_DIRT			1
#define	PRE19_PARTICLE_IMPACTS_WATERSURFACE	2
#define	PRE19_PARTICLE_IMPACTS_STEEL			3
#define	PRE19_PARTICLE_IMPACTS_STONE			4
#define	PRE19_PARTICLE_IMPACTS_WOOD			5
#define	PRE19_PARTICLE_IMPACTS_FOLIAGE		6
#define	PRE19_PARTICLE_IMPACTS_FLESH			7
#define	PRE19_PARTICLE_IMPACTS_ALIENFLESH	8
#define  PRE19_PARTICLE_IMPACTS_WATERFLOOR	9
#define	PRE19_PARTICLE_IMPACTS_ENDLIFE		10
#define	PRE19_PARTICLE_IMPACTS_EVERYFRAME	11
#define	PRE19_PARTICLE_IMPACTS_ENDLIFEWATER	12

// pre version 17 of particle impact effects serialize
#define	PRE17_PARTICLE_IMPACTS_GRASS			0
#define	PRE17_PARTICLE_IMPACTS_DIRT			1
#define	PRE17_PARTICLE_IMPACTS_WATERSURFACE	2
#define	PRE17_PARTICLE_IMPACTS_STEEL			3
#define	PRE17_PARTICLE_IMPACTS_STONE			4
#define	PRE17_PARTICLE_IMPACTS_WOOD			5
#define	PRE17_PARTICLE_IMPACTS_FOLIAGE		6
#define	PRE17_PARTICLE_IMPACTS_FLESH			7
#define	PRE17_PARTICLE_IMPACTS_ALIENFLESH	8
#define	PRE17_PARTICLE_IMPACTS_ENDLIFE		9
#define	PRE17_PARTICLE_IMPACTS_EVERYFRAME	10
#endif


/////////////////////////////////////////////////////////////////////////////
// particle playback types

#define PARTICLE_PLAYBACK_RUNONCE				0
#define PARTICLE_PLAYBACK_RUNANDHOLD			1
#define PARTICLE_PLAYBACK_LOOP					2
#define PARTICLE_PLAYBACK_PINGPONG				3
#define PARTICLE_PLAYBACK_RANDOM					4
#define PARTICLE_PLAYBACK_RANDOMANDHOLD		5

/////////////////////////////////////////////////////////////////////////////
// particle visibility types

#define PARTICLE_VISIBLE_CLOSE					0
#define PARTICLE_VISIBLE_FAR						1
#define PARTICLE_VISIBLE_ALWAYS					2
#define PARTICLE_VISIBLE_INVISIBLE				3

/////////////////////////////////////////////////////////////////////////////
// particle alignment types

#define PARTICLE_ALIGN_SCREEN						0
#define PARTICLE_ALIGN_UP							1
#define PARTICLE_ALIGN_GROUND						2
#define PARTICLE_ALIGN_WALL						3
#define PARTICLE_ALIGN_AUTO						4
#define PARTICLE_ALIGN_YAXIS						5

/////////////////////////////////////////////////////////////////////////////
// particle behavior flags

#define PARTICLE_BEHAVIOR_FADEOUT						(1 << 0)
//#define PARTICLE_BEHAVIOR_BOUNCEOFFGROUND				(1 << 1)
#define PARTICLE_BEHAVIOR_HOLDANIMONBOUNCE			(1 << 2)
#define PARTICLE_BEHAVIOR_ALIGNONBOTTOM				(1 << 3)
#define PARTICLE_BEHAVIOR_RETAINVELOCITY				(1 << 4)
#define PARTICLE_BEHAVIOR_ADDSOURCEVELOCITY			(1 << 5)
#define PARTICLE_BEHAVIOR_CANNOTHITSOURCE				(1 << 6)
#define PARTICLE_BEHAVIOR_CREATEONCE					(1 << 7)
#define PARTICLE_BEHAVIOR_MIRRORWIDTH					(1 << 8)
#define PARTICLE_BEHAVIOR_MIRRORHEIGHT					(1 << 9)
#define PARTICLE_BEHAVIOR_INTERSECTION					(1 << 10)
#define PARTICLE_BEHAVIOR_BOUNCEONLYONCE				(1 << 11)
#define PARTICLE_BEHAVIOR_SCALELINEAR					(1 << 12)
#define PARTICLE_BEHAVIOR_DRAWONBOTTOM					(1 << 13)
#define PARTICLE_BEHAVIOR_ZBUFFER						(1 << 14)
#define PARTICLE_BEHAVIOR_SPARKLE						(1 << 15)
#define PARTICLE_BEHAVIOR_CROSSFADE						(1 << 16)
#define PARTICLE_BEHAVIOR_DIEONWATERSURFACE			(1 << 17)
#define PARTICLE_BEHAVIOR_AIMATPLAYER					(1 << 18)
#define PARTICLE_BEHAVIOR_HASLENSFLARE					(1 << 19)
#define PARTICLE_BEHAVIOR_GLARE							(1 << 20)
#define PARTICLE_BEHAVIOR_PLAYONLYWHENFAR				(1 << 21)
#define PARTICLE_BEHAVIOR_PLAYONLYWHENNEAR			(1 << 22)
#define PARTICLE_BEHAVIOR_CENTERIMPACTONINSTANCE	(1 << 23)
#define PARTICLE_BEHAVIOR_GLOBALCOORD					(1 << 24)
#define PARTICLE_BEHAVIOR_BLOODEFFECT					(1 << 25)
#define PARTICLE_BEHAVIOR_IMPACTEFFECT					(1 << 26)
#define PARTICLE_BEHAVIOR_RESTRICTAIMATPLAYER		(1 << 27)
#define PARTICLE_BEHAVIOR_NOWALLSPAWN					(1 << 28)
#define PARTICLE_BEHAVIOR_NOGRDSPAWN					(1 << 29)


/////////////////////////////////////////////////////////////////////////////

#define	PARTICLE_DEFAULT_PRIORITY				50


#endif // _INC_PARTTYPE
