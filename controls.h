#ifndef CONTROLS_H_
#define CONTROLS_H_

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT 	160

#include "sprite.h"

/* move sprite left or right returns if it is at edge of the screen */
int move_left(struct Sprite* sprite) {
	
	/* face left */
	sprite_set_horizontal_flip(sprite, 1);
	
	/* set movement speed */
	sprite->move = 128;

	/* walking animation */
	if (sprite->frame >= sprite->walk_start && sprite->frame < sprite->walk_end) {
		sprite->frame = sprite->frame + sprite->frame_interval;
	} else {
		sprite->frame = sprite->walk_start;
	}

	sprite_set_offset(sprite, sprite->frame);
	sprite->counter = 0;

	/* if we are at the left end, just scroll the screen */
	if ((sprite->x/*>> 8*/) < sprite->border) {
		return 1; 
	} else {
		/* else move left */
		//sprite->x--;
		sprite->x--; //added for jumping and falling
		sprite_position(sprite);
		return 0;
	}
}
int move_right(struct Sprite* sprite) {
	/* face right */
	sprite_set_horizontal_flip(sprite, 0);
	sprite->move = 128;

	/* walking animation */
	if (sprite->frame >= sprite->walk_start && sprite->frame < sprite->walk_end) {
		sprite->frame = sprite->frame + sprite->frame_interval;
	} else {
		sprite->frame = sprite->walk_start;
	}

	sprite_set_offset(sprite, sprite->frame);
	sprite->counter = 0;

	/* if we are at the right end, just scroll the screen */
	if ((sprite->x /*>> 8*/) > (SCREEN_WIDTH - 64 - sprite->border)) {
		return 1; 
	} else {
		/* else move right */
		//sprite->x++;

		sprite->x++; //added for jumping and falling
		sprite_position(sprite);
		return 0;
	}
}

/* stop the sprite from walking left/right */
void move_none(struct Sprite* sprite) {
	sprite->move = 0;
	sprite->counter = 7;
	sprite->frame = 0;

	sprite_set_offset(sprite, sprite->frame);
}


#endif
