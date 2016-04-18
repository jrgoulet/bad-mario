#ifndef SPRITE_H_
#define SPRITE_H_

#include <stdlib.h>
#include <stdio.h>
#include "map.h"

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

struct Sprite_MEM {

	/* Hardware Attributes */
	unsigned short attribute0;
	unsigned short attribute1;
	unsigned short attribute2;
	unsigned short attribute3;

};


struct Sprite {

	/* Hardware Attributes */
	struct Sprite_MEM sprite_m;

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

	/* Animation Frames */
	int frame_interval;
	int walk_start;
	int walk_end;
	int atk_start;
	int atk_end;
	int death_start;
	int death_end;
	int stand_start;
	int stand_end;
};




struct Sprite_MEM sprite_mem_init (struct Sprite* sprite, int h, int v, enum SpriteSize size, int tile_index, int priority) {

	struct Sprite_MEM sprite_m;

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

	int x = sprite->x;
	int y = sprite->y;


	/* attribute0 setup */
	sprite_m.attribute0 = y |	/* y-coord */
		(0 <<  8) |				/* rendering mode */
		(0 << 10) |				/* gfx mode */
		(0 << 12) |				/* mosaic */
		(1 << 13) |				/* color mode, 0:16, 1:256 */
		(shape_bits << 14);		/* shape */

	/* attribute1 setup */
	sprite_m.attribute1 = x | 	/* x-coord */
		(0 <<  9) |				/* affine flag */
		(h << 12) |				/* horizontal flip flag */
		(v << 13) |				/* vertical flip flag */
		(size_bits << 14);		/* size */

	/* attribute2 setup */
	sprite_m.attribute2 = tile_index | /* tile index */
		(priority << 10) |		/* priority */
		(0 << 12);				/* palette bank (16 color only) */

	return sprite_m;
}
// h = horizontal flip
// v = vertical flip

struct Sprite* new_Sprite(char* name, enum SpriteSize size, int x, int y, int h, int v, int tile_index, int priority) {

	struct Sprite* sprite = malloc(sizeof(struct Sprite));

	switch (size) {
		case SIZE_8_8:   sprite->frame_interval = 8; break;
		case SIZE_16_16: sprite->frame_interval = 32; break;
		case SIZE_32_32: sprite->frame_interval = 64; break;
		case SIZE_64_64: sprite->frame_interval = 128; break;
		case SIZE_16_8:  sprite->frame_interval = 24; break;
		case SIZE_32_8:  sprite->frame_interval = 40; break;
		case SIZE_32_16: sprite->frame_interval = 48; break;
		case SIZE_64_32: sprite->frame_interval = 96; break;
		case SIZE_8_16:  sprite->frame_interval = 24; break;
		case SIZE_8_32:  sprite->frame_interval = 40; break;
		case SIZE_16_32: sprite->frame_interval = 48; break;
		case SIZE_32_64: sprite->frame_interval = 96; break;
	}

	/* initialization */
	sprite->name = name;		/* name */
	sprite->x 	 = x;			/* initial x-pos */
	sprite->y 	 = y;			/* initial y-pos */
	sprite->frame = tile_index;	/* tile index */
	sprite->counter = 0;		/* set in sprite_set_flip_counter */
	sprite->yvel = 0;			/* initially not falling */
	sprite->falling = 0;		/* initially not falling */
	sprite->animation_delay = 32;/* set in sprite_set_animation_delay */
	sprite->move = 0;			/* initially not moving */
    sprite->facing = h;         /* for knowing which way sprite is facing */

	sprite->sprite_m = sprite_mem_init(sprite,h,v,size,tile_index,priority);

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

void sprite_animation_init(struct Sprite* sprite, int ws, int we, int as, int ae, int ds, int de, int ss, int se) {
	sprite->walk_start = ws;
	sprite->walk_end = we;
	sprite->atk_start = as;
	sprite->atk_end = ae;
	sprite->death_start = ds;
	sprite->death_end = de;
	sprite->stand_start = ss;
	sprite->stand_end = se;
}

/* set a sprite postion */
void sprite_position(struct Sprite* sprite/*, int x, int y*/) {
 
  int x = sprite->x;
  int y = sprite->y;

  /* clear out the y coordinate */
  sprite->sprite_m.attribute0 &= 0xff00;

  /* set the new y coordinate */
  sprite->sprite_m.attribute0 |= (y & 0xff);

  /* clear out the x coordinate */
  sprite->sprite_m.attribute1 &= 0xfe00;

  /* set the new x coordinate */
  sprite->sprite_m.attribute1 |= (x & 0x1ff);

}

/* move a sprite in a direction */
void sprite_move(struct Sprite* sprite, int dx, int dy) {
  /* get the current y coordinate */
  int y = sprite->sprite_m.attribute0 & 0xff;

  /* get the current x coordinate */
  int x = sprite->sprite_m.attribute1 & 0x1ff;

  /* move to the new location */
  sprite_position(sprite);
}

/* change the vertical flip flag */
void sprite_set_vertical_flip(struct Sprite* sprite, int vertical_flip) {
  if (vertical_flip) {
    /* set the bit */
    sprite->sprite_m.attribute1 |= 0x2000;
  } else {
    /* clear the bit */
    sprite->sprite_m.attribute1 &= 0xdfff;
  }
}

/* change the vertical flip flag */
void sprite_set_horizontal_flip(struct Sprite* sprite, int horizontal_flip) {
  if (horizontal_flip) {
    /* set the bit */
    sprite->sprite_m.attribute1 |= 0x1000;
    sprite->facing = 1;
  } else {
    /* clear the bit */
    sprite->sprite_m.attribute1 &= 0xefff;
    sprite->facing = 0;
  }
}

/* change the tile offset of a sprite */
void sprite_set_offset(struct Sprite* sprite, int offset) {
  /* clear the old offset */
  sprite->sprite_m.attribute2 &= 0xfc00;

  /* apply the new one */
  sprite->sprite_m.attribute2 |= (offset & 0x03ff);
}


/*
void update_sprite(struct Sprite* sprite, int xscroll) {

    if (sprite->move) {
        sprite->counter++;
        if (sprite->counter >= sprite->animation_delay) {
            sprite->frame = sprite->frame + sprite->frame_interval;
            if (sprite->frame > sprite->walk_end) {
                sprite->frame = sprite->walk_start;
            }
        sprite_set_offset(sprite, sprite->frame);
        sprite->counter = 0;
        }
    }
    sprite_position(sprite, sprite->x, sprite->y);
}
*/


/* finds which tile a screen coordinate maps to, taking scroll into account */
unsigned short tile_lookup(int x, int y, int xscroll, int yscroll,
        const unsigned short* tilemap, int tilemap_w, int tilemap_h) {

    /* adjust for the scroll */
    x += xscroll;
    y += yscroll;

    /* convert from screen coordinates to tile coordinates */
    x >>= 3;
    y >>= 3;

    /* account for wraparound */
    while (x >= tilemap_w) {
        x -= tilemap_w;
    }
    while (y >= tilemap_h) {
        y -= tilemap_h;
    }
    while (x < 0) {
        x += tilemap_w;
    }
    while (y < 0) {
        y += tilemap_h;
    }

    /* lookup this tile from the map */
    int index = y * tilemap_w + x;

    /* return the tile */
    return tilemap[index];
}

void sprite_update(struct Sprite* sprite, int xscroll) {
    /* update y position and speed if falling */
    if (sprite->falling) {
        sprite->y += sprite->yvel;
        sprite->yvel += sprite->gravity;
    }

    /* check which tile the sprite's feet are over */
    unsigned short tile = tile_lookup((sprite->x >> 8) + 8, (sprite->y >> 8) + 32, xscroll,
            0, map, map_width, map_height);

    /* if it's block tile
     * these numbers refer to the tile indices of the blocks the sprite can walk on */
    if ((tile >= 1 && tile <= 6) || 
        (tile >= 12 && tile <= 17)) {
        /* stop the fall! */
        sprite->falling = 0;
        sprite->yvel = 0;

        /* make him line up with the top of a block
         * works by clearing out the lower bits to 0 */
        sprite->y &= ~0x7ff;

        /* move him down one because there is a one pixel gap in the image */
        sprite->y++;

    } else {
        /* he is falling now */
        sprite->falling = 1;
    }


    /* update animation if moving */
    if (sprite->move) {
        sprite->counter++;
        if (sprite->counter >= sprite->animation_delay) {
            sprite->frame = sprite->frame + 16;
            if (sprite->frame > 16) {
                sprite->frame = 0;
            }
            sprite_set_offset(sprite, sprite->frame);
            sprite->counter = 0;
        }
    }


    /* set on screen position */
    sprite_position(sprite/*, sprite->x, sprite->y*/);
}



#endif
