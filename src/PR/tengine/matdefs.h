// matdefs.h

#define MATERIAL_FLAT_SHADED			(1 << 0)	// otherwise flat shaded
#define MATERIAL_SELF_ILLUMINATED	(1 << 1)	// forces flat shaded
#define MATERIAL_TWO_SIDED				(1 << 2)
#define MATERIAL_TEXTURE_MAP			(1 << 3)	// overrides color
#define MATERIAL_REFLECT_MAP			(1 << 4)	// overrides color and texture
#define MATERIAL_DECAL_TEXTURE		(1 << 5)
#define MATERIAL_PRELIT					(1 << 6)
#define MATERIAL_MASK					(1 << 7)
#define MATERIAL_TRANSPARENCY			(1 << 8)	// won't write to z-buffer
#define MATERIAL_INTERSECT				(1 << 9)
#define MATERIAL_PSEUDOCOLOR			(1 << 10)
#define MATERIAL_MIRROR_TEXTURE		(1 << 11)
#define MATERIAL_SHADE_ALPHA			(1 << 12)
