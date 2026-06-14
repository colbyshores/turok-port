// cartdir.h
/////////////////////////////////////////////////////////////////////////////
//
//						Directory indices for data collections
//
/////////////////////////////////////////////////////////////////////////////


// root
#define CART_ROOT_nItems							11
															
#define CART_ROOT_isGraphicObjects				0
#define CART_ROOT_usObjectAttributes			1
#define CART_ROOT_usObjectTypes					2
#define CART_ROOT_isTextureSets					3
#define CART_ROOT_isParticleEffects				4
#define CART_ROOT_isSoundEffects					5
#define CART_ROOT_isLevels							6
#define CART_ROOT_isPersistantCounts			7
#define CART_ROOT_isWarpDests						8
#define CART_ROOT_isBinaries						9
#define CART_ROOT_usBinaryTypes					10
															
															
// graphic object										
#define CART_GRAPHOBJ_nItems						4
															
#define CART_GRAPHOBJ_isObjectInfo				0
#define CART_GRAPHOBJ_isGeometry					1
#define CART_GRAPHOBJ_isAnims						2
#define CART_GRAPHOBJ_usStaticEvents			3
															
															
// object info											
#define CART_OBJECTINFO_nItems					2
															
#define CART_OBJECTINFO_bounds					0
#define CART_OBJECTINFO_usAnimTypes				1
															
															
// geometry												
#define CART_GEOMETRY_nItems						2
															
#define CART_GEOMETRY_info							0
#define CART_GEOMETRY_isNodes						1
															
															
// node													
#define CART_NODE_nItems							3
															
#define CART_NODE_usChildNodeIndices			0
#define CART_NODE_usPartIndices					1
#define CART_NODE_isParts							2
															
															
// sections												
#define CART_SECTION_nItems						3
															
#define CART_SECTION_section						0
#define CART_SECTION_usPStream					1
#define CART_SECTION_usVertices					2
															
															
// animation											
#define CART_ANIM_nItems							7
															
#define CART_ANIM_usYRotations					0
#define CART_ANIM_usNodeAnimIndices				1
#define CART_ANIM_usInitialOrients				2
#define CART_ANIM_usTranslationSets				3
#define CART_ANIM_usRotationSets					4
#define CART_ANIM_isTransitionTable				5
#define CART_ANIM_usEvents							6
															
// transition table									
#define CART_TRANTABLE_nItems						2
															
#define CART_TRANTABLE_transition				0
#define CART_TRANTABLE_usTableEntries			1
															
															
// texture set											
#define CART_TEXTURESET_nItems					3
															
#define CART_TEXTURESET_format					0
#define CART_TEXTURESET_isBitmaps				1
#define CART_TEXTURESET_isPalettes				2
															

// particle effects															
#define CART_PARTICLEEFFECTS_nItems				3

#define CART_PARTICLEEFFECTS_usTypes			0
#define CART_PARTICLEEFFECTS_usParticles		1
#define CART_PARTICLEEFFECTS_usImpacts			2


// sound effects										
#define CART_SOUNDEFFECTS_nItems					2
															
#define CART_SOUNDEFFECTS_usTypes				0
#define CART_SOUNDEFFECTS_isSounds				1
															
															
// level													
#define CART_LEVEL_nItems							6
															
#define CART_LEVEL_level							0
#define CART_LEVEL_isCollision					1
#define CART_LEVEL_usInstances					2
#define CART_LEVEL_isGridBounds					3
#define CART_LEVEL_isGridSections				4
#define CART_LEVEL_isSkyTextureSet				5
															
															
// grid bounds

#define CART_GRIDBOUNDS_nItems					5

#define CART_GRIDBOUNDS_usMinXs					0
#define CART_GRIDBOUNDS_usMaxXs					1
#define CART_GRIDBOUNDS_usMinZs					2
#define CART_GRIDBOUNDS_usMaxZs					3
#define CART_GRIDBOUNDS_usBounds					4


// grid section										
#define CART_GRIDSECTION_nItems					3
															
#define CART_GRIDSECTION_usEStaticInstances	0
#define CART_GRIDSECTION_usStaticInstances	1
#define CART_GRIDSECTION_usSimpleInstances	2


// collision
#define CART_COLLISION_nItems						4
															
#define CART_COLLISION_usRegionAttributes		0
#define CART_COLLISION_usCorners					1
#define CART_COLLISION_usRegions					2
#define CART_COLLISION_usBlockBounds			3


// persistant counts
#define CART_PERSIST_nItems						3

#define CART_PERSIST_usPickupCounts				0
#define CART_PERSIST_usAnimCounts				1
#define CART_PERSIST_MaxRegionCount				2


// warp destinations
#define CART_WARPDESTS_nItems						2

#define CART_WARPDESTS_usIDs						0
#define CART_WARPDESTS_usWarpDests				1
