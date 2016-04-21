#ifndef SPRITE_H_
#define SPRITE_H_

#define JUMP_SPEED  5
#define FALL_SPEED  5
#define FLOOR       99
#define POSMAX      260
#define POSMIN      -64
#define AI_WALKDELAY  10

#include <stdlib.h>
#include <stdio.h>
#include "map.h"
#include "controls.h"

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
	int facing;       /* which way sprite is facing */
    char* name; 		/* callsign */
    
    int airtime;
    int player;
    int scroll;
    int move_timer;
    int jump_timer;
    int ymin;

    
    int bulletActive;       /* if bullet is active */     
    int distTravel;         /* for bullet distance traveled */
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
		case SIZE_32_32: sprite->frame_interval = 32; break;
		case SIZE_64_64: sprite->frame_interval = 128; break;
		case SIZE_16_8:  sprite->frame_interval = 24; break;
		case SIZE_32_8:  sprite->frame_interval = 40; break;
		case SIZE_32_16: sprite->frame_interval = 48; break;
		case SIZE_64_32: sprite->frame_interval = 96; break;
		case SIZE_8_16:  sprite->frame_interval = 24; break;
		case SIZE_8_32:  sprite->frame_interval = 40; break;
		case SIZE_16_32: sprite->frame_interval = 48; break;
		case SIZE_32_64: sprite->frame_interval = 64; break;
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
  sprite->airtime = 0;
	sprite->sprite_m = sprite_mem_init(sprite,h,v,size,tile_index,priority);
  sprite->player = 0;
  sprite->scroll = 0;
  sprite->move_timer = 0;
  sprite->jump_timer = 0;
  sprite->ymin = FLOOR;
    sprite->leftHit = 0;
    sprite->rightHit = 0;
    sprite->topHit = 0;
    sprite->bottomHit = 0;
    sprite->bulletActive = 0;
    sprite->distTravel = 0;

	/* return a pointer */
	return sprite;
}

void sprite_set_animation_delay(struct Sprite* sprite, int delay) {
	sprite->animation_delay = delay;
}

void sprite_set_player(struct Sprite* sprite) {
  sprite->player = 1;
}

void sprite_set_pos(struct Sprite* sprite, int x, int y) {
  sprite->x = x;
  sprite->y = y;

  /* clear out the y coordinate */
  sprite->sprite_m.attribute0 &= 0xff00;

  /* set the new y coordinate */
  sprite->sprite_m.attribute0 |= (y & 0xff);

  /* clear out the x coordinate */
  sprite->sprite_m.attribute1 &= 0xfe00;

  /* set the new x coordinate */
  sprite->sprite_m.attribute1 |= (x & 0x1ff);

}

void sprite_set_flip_counter(struct Sprite* sprite, int count) {
	sprite->counter = count;
}

void sprite_collision_init(struct Sprite* sprite, int l, int r, int u, int d, int b) {
	sprite->leftHit = sprite->x + l;
	sprite->rightHit = sprite->x +r;
	sprite->topHit = sprite->y + u;
	sprite->bottomHit = sprite->y + d;
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

void sprite_set_floor(struct Sprite* sprite, int y) {
  sprite->ymin = y;
}

void sprite_update(struct Sprite* sprite, int xscroll) {

    xscroll = xscroll * 2;

    /* update y position and speed if falling */
    if (sprite->airtime == 1) {
      sprite->airtime = 0;
      sprite->falling = 1;
    }
    else if (sprite->airtime > 0) {
      sprite->airtime -= 1;
      sprite->y -= JUMP_SPEED;
    }
    else if (sprite->falling) {
        if ((sprite->y + FALL_SPEED) >= sprite->ymin) {
          sprite->falling = 0;
          sprite->y = sprite->ymin;
        }
        sprite->y += FALL_SPEED;
    }

    if (sprite->player != 1) {
      sprite->x = sprite->x + xscroll;
    }

    if (sprite->x > POSMAX) {
      sprite->x = POSMAX;
      sprite->scroll += xscroll;
    }

    if (sprite->x < POSMIN) {
      sprite->x = POSMIN;
      sprite->scroll -= xscroll;
    }

    if (sprite->scroll > 0 && xscroll < 0) {
      sprite->scroll += xscroll;
    }

    if (sprite->scroll < 0 && xscroll > 0) {
      sprite->scroll += xscroll;
    }

    if (sprite->scroll > 0) {
      sprite->x = POSMAX;
    }

    if (sprite->scroll < 0) {
      sprite->x = POSMIN;
    }
    /* set on screen position */
    sprite_position(sprite/*, sprite->x, sprite->y*/);
}

int sprite_move_right(struct Sprite* sprite) {
  /* face right */
  sprite_set_horizontal_flip(sprite, 0);
  sprite->move = sprite->walk_start;

  /* walking animation */
  if (sprite->frame >= sprite->walk_start && sprite->frame < sprite->walk_end) {
    sprite->frame = sprite->frame + sprite->frame_interval;
  } else {
    sprite->frame = sprite->walk_start;
  }

    sprite_set_offset(sprite, sprite->frame);
    sprite->counter = 0;


    sprite->x++; //added for jumping and falling
    sprite_position(sprite/*, sprite->x, sprite->y*/);
    return 0;
}

int sprite_move_left(struct Sprite* sprite) {
  /* face right */
  sprite_set_horizontal_flip(sprite, 1);
  sprite->move = sprite->walk_start;

  /* walking animation */
  if (sprite->frame >= sprite->walk_start && sprite->frame < sprite->walk_end) {
    sprite->frame = sprite->frame + sprite->frame_interval;
  } else {
    sprite->frame = sprite->walk_start;
  }

    sprite_set_offset(sprite, sprite->frame);
    sprite->counter = 0;


    sprite->x--; //added for jumping and falling
    sprite_position(sprite/*, sprite->x, sprite->y*/);
    return 0;
}

void sprite_move_none(struct Sprite* sprite) {
  sprite->move = sprite->stand_start;
  sprite->counter = 0;
  sprite->frame = sprite->stand_start;
  sprite_set_offset(sprite, sprite->frame);
}

void sprite_jump(struct Sprite* sprite) {
  if (sprite->airtime == 0 && sprite->falling == 0) {
      sprite->airtime = 20;
  }
}

void sprite_ai(struct Sprite* com, struct Sprite* player, int move, int jump) {

  if (com->move_timer == 0) {
    sprite_move_none(com);
  }

  if (com->move_timer == 0 && move < 2) {
    com->move_timer = 20;
  }

  if (com->jump_timer == 0 && jump < 1) {
    com->jump_timer = 20;
  }

  if (com->x > player->x && com->move_timer > 0) {
    sprite_move_left(com);
    com->move_timer--;
  }

  if (com->x < player->x && com->move_timer > 0) {
    sprite_move_right(com);
    com->move_timer--;
  }

  if (com->jump_timer > 0 && jump < 1) {
    sprite_jump(com);
    com->jump_timer--;
  }

  if (com->move_timer < 0) com->move_timer = 0;
  if (com->jump_timer < 0) com->jump_timer = 0;

}


int mario_collide(struct Sprite* bullet, struct Sprite* mario) {

    if (bullet->x <= mario->leftHit) { 
        // && (bullet->y >= mario->topHit && bullet->y <= mario->bottomHit)) {
        return 1;    
    } else if (bullet->x >= mario->rightHit) { 
        // && (bullet->y >= mario->topHit && bullet->y <= mario->bottomHit)){
        return 1;
    } else {
        return 0;
    }
}

void update_bullet(struct Sprite* bullet, struct Sprite* mario, int travel, int dir, int coll, int dist) {
    if (dir == 0) {
        if (bullet->distTravel <= dist && coll == 1) {  
            bullet->x = bullet->x + travel;
            bullet->distTravel = bullet->distTravel + travel;
        } else { 
            bullet->bulletActive = 0;
            bullet->x = SCREEN_WIDTH;
            bullet->y = SCREEN_HEIGHT;
            bullet->distTravel = 0;
        }
    } else if (dir == 1) {
        if (bullet->distTravel <= dist && coll == 1) {  
            bullet->x = bullet->x - travel;
            bullet->distTravel = bullet->distTravel + travel;
        } else { 
            bullet->bulletActive = 0;
            bullet->distTravel = 0;
            bullet->x = SCREEN_WIDTH;
            bullet->y = SCREEN_HEIGHT;
        }
    }   
}


void sprite_bullet(struct Sprite* mario,struct Sprite* megaman, struct Sprite* bullet, int travel, int dist) {
    
    // use horizontal flip to see which way bullet is traveling
    if (bullet->bulletActive == 1 && bullet->facing == 0) {
      
        update_bullet(bullet, mario, travel, bullet->facing, 1, dist);
        
    } else if (bullet->bulletActive == 1 && bullet->facing == 1) {
       
        update_bullet(bullet, mario, travel, bullet->facing, 1, dist);
       
    } 

}//end sprite_bullet



#endif
