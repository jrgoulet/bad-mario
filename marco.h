#ifndef MARCO_H_
#define MARCO_H_

//#include "sprite_init.h"

struct Marco {

	/* sprite attribute info */
	struct Sprite* sprite;

	/* x and y position in 1/256 px */
	int x, y, leftHit, rightHit, bottomHit, topHit;

	/* y-velocity in 1/256 px/sec */
	int yvel;

	/* y-acceleration in 1/256px/sec^2 */
	int gravity;

	/* current animation frame */
	int frame;

	/* number of frames to wait before flipping */
	int animation_delay;

	/* how many frames until flip */
	int counter;
	
	/* boolean, whether moving or not */
	int move;

	/* pixel distance from edge of screen */
	int border;

	/* boolen, whether falling or not */
	int falling;

};



















#endif
