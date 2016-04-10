#ifndef SPRITE_INIT_H_
#define SPRITE_INIT_H_

#include "sprite.h"

/* all possible sprite sizes */
enum SpriteSize {
	SIZE_8_8,
	SIZE_16_16,
	SIZE_32_32,
	SIZE_64_64,
	SIZE_16_8,
	SIZE_32_8,
	SIZE_32_16,
	SIZE_64_32,
	SIZE_8_16,
	SIZE_8_32,
	SIZE_16_32,
	SIZE_32_64
};

/* sprites */
//struct Sprite {
//	unsigned short attribute0;
//	unsigned short attribute1;
//	unsigned short attribute2;
//	unsigned short arrribute3;
//};

/* all available sprites */
//struct Sprite sprites[128]; /* NUM_SPRITES */
//int next_sprite_index = 0;

struct Sprite sprite_init (int x, int y, enum SpriteSize size, int horizontal_flip, int vertical_flip, int tile_index, int priority) {

	struct Sprite sprite;

	/* grab the next index */
	//int index = next_sprite_index++;

	/* setup the bits used for each shape/size possible */
	int size_bits, shape_bits;
	switch (size) {
		case SIZE_8_8:		size = 0; shape_bits = 0; break;
		case SIZE_16_16:	size = 1; shape_bits = 0; break;
		case SIZE_32_32:	size = 2; shape_bits = 0; break;
		case SIZE_64_64:	size = 3; shape_bits = 0; break;
		case SIZE_16_8:		size = 0; shape_bits = 1; break;
		case SIZE_32_8:		size = 1; shape_bits = 1; break;
		case SIZE_32_16:	size = 2; shape_bits = 1; break;
		case SIZE_64_32:	size = 3; shape_bits = 1; break;
		case SIZE_8_16:		size = 0; shape_bits = 2; break;
		case SIZE_8_32:		size = 1; shape_bits = 2; break;
		case SIZE_16_32:	size = 2; shape_bits = 2; break;
		case SIZE_32_64:	size = 3; shape_bits = 2; break;
	}

	int h = horizontal_flip ? 1 : 0;
	int v = vertical_flip ? 1 : 0;

	/* attribute0 setup */
	sprite.attribute0 = y |	/* y-coord */
		(0 <<  8) |		/* rendering mode */
		(0 << 10) |		/* gfx mode */
		(0 << 12) |		/* mosaic */
		(1 << 13) |		/* color mode, 0:16, 1:256 */
		(shape_bits << 14);	/* shape */

	/* attribute1 setup */
	sprite.attribute1 = x | 	/* x-coord */
		(0 <<  9) |		/* affine flag */
		(h << 12) |		/* horizontal flip flag */
		(v << 13) |		/* vertical flip flag */
		(size_bits << 14);	/* size */

	/* attribute2 setup */
	sprite.attribute2 = tile_index | /* tile index */
		(priority << 10) |	/* priority */
		(0 << 12);		/* palette bank (16 color only) */

	/* return pointer */
	return sprite;
}




#endif
