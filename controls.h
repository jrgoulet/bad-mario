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

void jump(struct Sprite* sprite) {
	if (sprite->airtime == 0 && sprite->falling == 0) {
      sprite->airtime = 20;
  }
}

/* sprite shoot */
//first is sprite shooting, second is bullet sprite
void shoot (struct Sprite* marco,struct Sprite* mario, struct Sprite* bullet, int dist) {
    bullet->x +=8;
    sprite_position(bullet); 
    /*
    // find which way sprite is facing
    if (marco->facing) {
        // facing left
        // shoot bullet to left, get x, y coord for gun to control bullet animation
        while(dist > travelL) {
            if(bullet->x < mario->leftHit) {
                bullet->sprite_m.attribute0 = SCREEN_HEIGHT;
                bullet->sprite_m.attribute1 = SCREEN_WIDTH;
            }else {
            bullet->x = bullet->x - 8;
            dist-=8;
            //at the end, draw bullet off screen, or if collision
            sprite_position(bullet);
            }
            if(dist <= -80) {
                bullet->sprite_m.attribute0 = SCREEN_HEIGHT;
                bullet->sprite_m.attribute1 = SCREEN_WIDTH;
            }
        }
    } else {
        //facing right
        //shoot bullet to right
        while(dist < travelR) {
            if(bullet->x > mario->rightHit) {
                bullet->sprite_m.attribute0 = SCREEN_HEIGHT;
                bullet->sprite_m.attribute1 = SCREEN_WIDTH;
            } else {
            bullet->x += 8;
            dist +=8;
            //at the end, draw bullet off screen, or if collision
            // if (collision) {
                //draw bullet off screen, end while
            sprite_position(bullet);
            }
            if (dist >= 80) {
                bullet->sprite_m.attribute0 = SCREEN_HEIGHT;
                bullet->sprite_m.attribute1 = SCREEN_WIDTH;
            }
        }

    }    */
 
}



#endif
