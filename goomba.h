#ifndef GOOMBA_H_
#define GOOMBA_H_


/* a struct for goomba's logic and behavior */
struct Goomba {
    /* the actual sprite attribute info */
    struct Sprite* sprite;

    /* the x and y postion in 1/256 pixels */
    int x, y, leftHit, rightHit, bottomHit, topHit;

    /* which frame of the animation he is on */                
    int frame;

    /* the number of frames to wait before flipping */
    int animation_delay;                       

    /* the animation counter counts how many frames until we flip */
    int counter;

    /* whether goomba is moving right now or not */
    int move;

    /* the number of pixels away from the edge of the screen goomba stays */
    int border;

}; //end goomba struct


/* initialize goomba */
void goomba_init(struct Goomba* goomba) {
    goomba->x = 200;
    goomba->y = 120;
    goomba->leftHit = goomba->x + 5;
    goomba->rightHit = goomba->x + 27;
    goomba->bottomHit = goomba->y + 32;
    goomba->topHit = goomba->y + 10;
    goomba->border = 30;
    //marco takes up 6 * 128 frames so goomba starts there
    //goomba takes up 5 * 64 frames so next sprite STARTS at 1088!!!!!!!!
    goomba->frame = 768;
    goomba->move = 768;
    goomba->counter = 0;
    goomba->animation_delay = 16;
    //change SPRITE SIZE HERE!!!
    goomba->sprite = sprite_init(goomba->x, goomba->y, SIZE_32_32, 0, 0, goomba->frame, 0);
}//end goomba_init


/* move goomba left returns if it is at edge of the screen */
void goomba_left(struct Goomba* goomba) {
    /* face left */
    sprite_set_horizontal_flip(goomba->sprite, 1);
    goomba->move = 768;

    if (goomba->x > goomba->border) {
        goomba->x -= 1;
    }
    /*    
    // if we are at the left end, just scroll the screen 
    if (goomba->x < goomba->border) {
    return 1;
    } else {
    // else move left 
    //marco->x--;
    marco->x -= 256; //added for jumping and falling
    return 0;
    }                                   */
} //end goomba_left

//MIGHT NEED CODE BELOW LATER!!!!


#if 0
/* finds which tile a screen coordinate maps to, taking scroll into account */
unsigned short tile_lookup(int x, int y, int xscroll, int yscroll,
        const unsigned short* maptrans, int maptrans_w, int maptrans_h) {

    /* adjust for the scroll */
    x += xscroll; //x left bound value
    //xr += xscroll; //x right bound value
    y += yscroll; //y top bound value
    //yb += yscroll; //y bottom bound value

    /* convert from screen coordinates to tile coordinates */

    x >>= 3;
    y >>= 3;

    /* account for wraparound */
    while (x >= maptrans_w) {
        x -= maptrans_w;
    }
    while (y >= maptrans_h) {
        y -= maptrans_h;
    }
    while (x < 0) {
        x += maptrans_w;
    }
    while (y < 0) {
        y += maptrans_h;
    }

    /* lookup this tile from the map */
    int index = y * maptrans_w + x;

    /* return the tile */
    return maptrans[index];
}
#endif


// updates goomba's movement
void goomba_update(struct Goomba* goomba) {

    /* update animation if moving */
    if (goomba->move) {
        goomba->counter++;
        if (goomba->counter >= goomba->animation_delay) {
            //FRAME ANIMATION HERE, add double the number of frames for the next
            // animation.  For 64, add 128. 
            goomba->frame = goomba->frame + 32;
            if (goomba->frame > 928) {
                goomba->frame = 768;
            }
            sprite_set_offset(goomba->sprite, goomba->frame);
            goomba->counter = 0;
        }
    }

    sprite_position(goomba->sprite, goomba->x /*>> 8*/, goomba->y /*>> 8*/);
}// end goomba_update






#endif
