#ifndef SPRITE_H_
#define SPRITE_H_

#include <stdlib.h>
#include <stdio.h>

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

struct Sprite {

	/* Hardware Attributes */
	unsigned short attribute0;
	unsigned short attribute1;
	unsigned short attribute2;
	unsigned short attribute3;

	
	int x, y, leftHit, rightHit, bottomHit, topHit; /* x and y position in 1/256 px */
	int yvel;				/* y-velocity in 1/256 px/sec */
	int gravity; 			/* y-acceleration in 1/256px/sec^2 */
	int frame; 				/* current animation frame */
	int animation_delay; 	/* number of frames to wait before flipping */
	int counter; 			/* how many frames until flip */
	int move; 				/* boolean, whether moving or not */
	int border; 			/* pixel distance from edge of screen */
	int falling; 			/* boolean, whether falling or not */
	char* name; 			/* callsign */

};

struct Sprite* new_Sprite(char* name, enum SpriteSize size, int x, int y, int h, int v, int tile_index, int priority) {

	struct Sprite* sprite = malloc(sizeof(struct Sprite));

  	int size_bits, shape_bits;
  	switch (size) {
		case SIZE_8_8:   size_bits = 0; shape_bits = 0; break;
		case SIZE_16_16: size_bits = 1; shape_bits = 0; break;
		case SIZE_32_32: size_bits = 2; shape_bits = 0; break;
		case SIZE_64_64: size_bits = 3; shape_bits = 0; break;
		case SIZE_16_8:  size_bits = 0; shape_bits = 1; break;
		case SIZE_32_8:  size_bits = 1; shape_bits = 1; break;
		case SIZE_32_16: size_bits = 2; shape_bits = 1; break;
		case SIZE_64_32: size_bits = 3; shape_bits = 1; break;
		case SIZE_8_16:  size_bits = 0; shape_bits = 2; break;
		case SIZE_8_32:  size_bits = 1; shape_bits = 2; break;
		case SIZE_16_32: size_bits = 2; shape_bits = 2; break;
		case SIZE_32_64: size_bits = 3; shape_bits = 2; break;
	}


	/* attribute0 setup */
	sprite->attribute0 = y |	/* y-coord */
		(0 <<  8) |				/* rendering mode */
		(0 << 10) |				/* gfx mode */
		(0 << 12) |				/* mosaic */
		(1 << 13) |				/* color mode, 0:16, 1:256 */
		(shape_bits << 14);		/* shape */

	/* attribute1 setup */
	sprite->attribute1 = x | 	/* x-coord */
		(0 <<  9) |				/* affine flag */
		(h << 12) |				/* horizontal flip flag */
		(v << 13) |				/* vertical flip flag */
		(size_bits << 14);		/* size */

	/* attribute2 setup */
	sprite->attribute2 = tile_index | /* tile index */
		(priority << 10) |		/* priority */
		(0 << 12);				/* palette bank (16 color only) */

	/* initialization */
	sprite->name = name;		/* name */
	sprite->x 	 = x;			/* initial x-pos */
	sprite->y 	 = y;			/* initial y-pos */
	sprite->frame = tile_index;	/* tile index */
	sprite->counter = 0;		/* set in sprite_set_flip_counter */
	sprite->yvel = 0;			/* initially not falling */
	sprite->falling = 0;		/* initially not falling */
	sprite->animation_delay = 0;/* set in sprite_set_animation_delay */
	sprite->move = 0;			/* initially not moving */

	/* return a pointer */
	return sprite;
}


void sprite_set_animation_delay(struct Sprite* sprite, int delay) {
	sprite->animation_delay = delay;
}

void sprite_set_flip_counter(struct Sprite* sprite, int count) {
	sprite->counter = count;
}

void sprite_collision_init(struct Sprite* sprite, int l, int r, int u, int d, int b) {
	sprite->leftHit = l;
	sprite->rightHit = r;
	sprite->topHit = u;
	sprite->bottomHit = d;
	sprite->border = b;
}

/* set a sprite postion */
void sprite_position(struct Sprite* sprite, int x, int y) {
  /* clear out the y coordinate */
  sprite->attribute0 &= 0xff00;

  /* set the new y coordinate */
  sprite->attribute0 |= (y & 0xff);

  /* clear out the x coordinate */
  sprite->attribute1 &= 0xfe00;

  /* set the new x coordinate */
  sprite->attribute1 |= (x & 0x1ff);
}

/* move a sprite in a direction */
void sprite_move(struct Sprite* sprite, int dx, int dy) {
  /* get the current y coordinate */
  int y = sprite->attribute0 & 0xff;

  /* get the current x coordinate */
  int x = sprite->attribute1 & 0x1ff;

  /* move to the new location */
  sprite_position(sprite, x + dx, y + dy);
}

/* change the vertical flip flag */
void sprite_set_vertical_flip(struct Sprite* sprite, int vertical_flip) {
  if (vertical_flip) {
    /* set the bit */
    sprite->attribute1 |= 0x2000;
  } else {
    /* clear the bit */
    sprite->attribute1 &= 0xdfff;
  }
}

/* change the vertical flip flag */
void sprite_set_horizontal_flip(struct Sprite* sprite, int horizontal_flip) {
  if (horizontal_flip) {
    /* set the bit */
    sprite->attribute1 |= 0x1000;
  } else {
    /* clear the bit */
    sprite->attribute1 &= 0xefff;
  }
}

/* change the tile offset of a sprite */
void sprite_set_offset(struct Sprite* sprite, int offset) {
  /* clear the old offset */
  sprite->attribute2 &= 0xfc00;

  /* apply the new one */
  sprite->attribute2 |= (offset & 0x03ff);
}

void sprite_update(struct Sprite* sprite, int xscroll) {
	if (sprite->move) {
    	sprite->counter++;
    	if (sprite->counter >= sprite->animation_delay) {
      		sprite->frame = sprite->frame + 128;
      		if (sprite->frame > 640) {
      			sprite->frame = 128;
      		}
      		sprite_set_offset(sprite, sprite->frame);
      		sprite->counter = 0;
    	}
    }
}


#endif
