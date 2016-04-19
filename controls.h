#ifndef CONTROLS_H_
#define CONTROLS_H_

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT 	160

#include "sprite.h"

//need to pass in sprite move, frame, and counter for each function because it will be different per sprite

/* move sprite left or right returns if it is at edge of the screen */
int move_left(struct Sprite* sprite) {
	
	/* face left */
	sprite_set_horizontal_flip(sprite, 1);
	
	/* set movement speed */
    //this is for animation, not speed.  Marco starts his walking at frame 128
	sprite->move = sprite->walk_start;

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
		sprite_position(sprite/*,sprite->x, sprite->y*/);
		return 0;
	}
}
int move_right(struct Sprite* sprite) {
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

	/* if we are at the right end, just scroll the screen */
	if ((sprite->x /*>> 8*/) > (SCREEN_WIDTH - 64 - sprite->border)) {
		return 1; 
	} else {
		/* else move right */
		//sprite->x++;

		sprite->x++; //added for jumping and falling
		sprite_position(sprite/*, sprite->x, sprite->y*/);
		return 0;
	}
}

/* stop the sprite from walking left/right */
void move_none(struct Sprite* sprite) {
	sprite->move = sprite->stand_start;
	sprite->counter = 0;
	sprite->frame = sprite->stand_start;

	sprite_set_offset(sprite, sprite->frame);
}


/* sprite shoot */
//first is sprite shooting, second is bullet sprite
void shoot (struct Sprite* mario, struct Sprite* megaman, struct Sprite* bullet, int dist) {
    int tmp;
    // needs work!!!!
//    tmp = megaman->frame;
    megaman->frame = megaman->atk_start; 
    megaman->frame = megaman->frame + megaman->frame_interval; 
    bullet->x +=8;
    sprite_position(bullet); 
//    megaman->frame = tmp;

 
}



#endif
