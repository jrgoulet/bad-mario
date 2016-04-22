#ifndef CONTROLS_H_
#define CONTROLS_H_

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT 	160

#include "sprite.h"


/* move sprite left or right returns if it is at edge of the screen */
int move_left(struct Sprite* sprite) {

    /* face left */
    sprite_set_horizontal_flip(sprite, 1);

    /* set animation frame */
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
    if (sprite->x < sprite->border) {
        return 1; 
    } else {
        /* else move left */

        sprite->x--;
        if (sprite->airtime > 0) sprite->x -= 3;
        sprite_position(sprite);
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
    if ((sprite->x) > (SCREEN_WIDTH - 32 - sprite->border)) {
        return 1; 
    } else {
        /* else move right */

        sprite->x++; 
        if (sprite->airtime > 0) sprite->x += 3;
        sprite_position(sprite);
        return 0;
    }
}

/* stop the sprite from walking left/right */
void move_none(struct Sprite* sprite) {
    sprite->move = sprite->stand_start;
    sprite->counter = 14;
    sprite->frame = sprite->stand_start;

    sprite_set_offset(sprite, sprite->frame);
}

void jump(struct Sprite* sprite) {
    if (sprite->airtime == 0 && sprite->falling == 0) {
        sprite->airtime = 30;
    }
}

/* sprite shoot */
void shoot (struct Sprite* mario, struct Sprite* megaman, struct Sprite* bullet, int travel, int dist) {

    bullet->facing = megaman->facing;

    if (bullet->facing == 1) {
        bullet->x = megaman->x + 2;
        bullet->y = megaman->y + 8;
    } else {
        bullet->x = megaman->x + 24;
        bullet->y = megaman->y + 8;
    }

    megaman->frame = megaman->atk_start;
    sprite_set_offset(megaman, megaman->frame);

    sprite_bullet(mario, megaman, bullet, travel, dist);
}



#endif
