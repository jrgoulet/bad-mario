/*
███████╗██╗     ██╗   ██╗ ██████╗ 
██╔════╝██║     ██║   ██║██╔════╝ 
███████╗██║     ██║   ██║██║  ███╗
╚════██║██║     ██║   ██║██║   ██║
███████║███████╗╚██████╔╝╚██████╔╝
╚══════╝╚══════╝ ╚═════╝  ╚═════╝ 
Kenny Lyon and Joe Goulet
CPSC 305 01 - Finlayson
Spring 2016
*/



/* ====== Header Files ================================================================================ */

#include "background.h"
#include "sprites.h"
#include "controls.h"
#include "sprite.h"
#include "map.h"
#include "maptrans.h"
#include "text.h"
#include "slug_title.h"
#include "main_bg.h"
#include <time.h>
#include <stdlib.h>

/* ====== Global Vars ================================================================================= */

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT 	160
#define MODE0			0x00 		/* the tile mode flags needed for display control register */
#define MODE3			0x0003
#define BG0_ENABLE 		0x100
#define BG1_ENABLE 		0x200
#define BG2_ENABLE		0x400


#define SPRITE_MAP_2D 	0x0 		/* flags to set sprite handling in display control register */
#define SPRITE_MAP_1D 	0x40
#define SPRITE_ENABLE 	0x1000

#define PALETTE_SIZE 	256			/* palette is always 256 colors */

#define NUM_SPRITES 	128			/* there are 128 sprites on the GBA */

#define BUTTON_A 		(1 << 0)	/* the bit positions indicate each button */
#define BUTTON_B 		(1 << 1)	/* the first bit is for A, second for */
#define BUTTON_SELECT 	(1 << 2)	/* B, and so on, each constant below can be  */
#define BUTTON_START 	(1 << 3)	/* ANDED into the register to get the */
#define BUTTON_RIGHT 	(1 << 4)	/* status of any one button */
#define BUTTON_LEFT 	(1 << 5)
#define BUTTON_UP 		(1 << 6)
#define BUTTON_DOWN 	(1 << 7)
#define BUTTON_R 		(1 << 8)
#define BUTTON_L 		(1 << 9)

#define DMA_ENABLE 		0x80000000 	/* flag for turning on DMA */

#define DMA_16 			0x00000000 	/* flags for the sizes to transfer, 16 or 32 bits */
#define DMA_32 			0x04000000

/* ====== Declarations ================================================================================ */

struct Sprite* Marco;
struct Sprite_MEM sprites_m[NUM_SPRITES];
struct Sprite* sprites[NUM_SPRITES];


/* ====== Control Registers =========================================================================== */

/* the control registers for the four tile layers */
volatile unsigned short* bg0_control = (volatile unsigned short*) 0x4000008;
volatile unsigned short* bg1_control = (volatile unsigned short*) 0x400000a;
volatile unsigned short* bg2_control = (volatile unsigned short*) 0x400000c;
volatile unsigned short* bg3_control = (volatile unsigned short*) 0x400000e;

/* the display control pointer points to the gba graphics register */
volatile unsigned long* display_control = (volatile unsigned long*) 0x4000000;

/* the memory location which controls sprite attributes */
volatile unsigned short* sprite_attribute_memory = (volatile unsigned short*) 0x7000000;

/* the memory location which stores sprite image data */
volatile unsigned short* sprite_image_memory = (volatile unsigned short*) 0x6010000;

/* the address of the color palettes used for backgrounds and sprites */
volatile unsigned short* bg_palette = (volatile unsigned short*) 0x5000000;
volatile unsigned short* sprite_palette = (volatile unsigned short*) 0x5000200;

/* the button register holds the bits which indicate whether each button has
 * been pressed - this has got to be volatile as well
 */
volatile unsigned short* buttons = (volatile unsigned short*) 0x04000130;

/* scrolling registers for backgrounds */
volatile short* bg0_x_scroll = (unsigned short*) 0x4000010;
volatile short* bg0_y_scroll = (unsigned short*) 0x4000012;
volatile short* bg1_x_scroll = (unsigned short*) 0x4000014;
volatile short* bg1_y_scroll = (unsigned short*) 0x4000016;

/* the scanline counter is a memory cell which is updated to indicate how
 * much of the screen has been drawn */
volatile unsigned short* scanline_counter = (volatile unsigned short*) 0x4000006;

/* pointer to the DMA source location */
volatile unsigned int* dma_source = (volatile unsigned int*) 0x40000D4;

/* pointer to the DMA destination location */
volatile unsigned int* dma_destination = (volatile unsigned int*) 0x40000D8;

/* pointer to the DMA count/control */
volatile unsigned int* dma_count = (volatile unsigned int*) 0x40000DC;

/* return a pointer to one of the 4 character blocks (0-3) */
volatile unsigned short* char_block(unsigned long block) {
	/* they are each 16K big */
	return (volatile unsigned short*) (0x6000000 + (block * 0x4000));
}

/* return a pointer to one of the 32 screen blocks (0-31) */
volatile unsigned short* screen_block(unsigned long block) {
	/* they are each 2K big */
	return (volatile unsigned short*) (0x6000000 + (block * 0x800));
}

/* ====== GBA Functions ================================================================================ */

/* wait for the screen to be fully drawn so we can do something during vblank */
void wait_vblank( ) {
	/* wait until all 160 lines have been updated */
	while (*scanline_counter < 160) { }
}

/* copy data using DMA */
void memcpy16_dma(unsigned short* dest, unsigned short* source, int amount) {
	*dma_source = (unsigned int) source;
	*dma_destination = (unsigned int) dest;
	*dma_count = amount | DMA_16 | DMA_ENABLE;
}

volatile unsigned short* screen = (volatile unsigned short*) 0x6000000;
void put_pixel(int row, int col, unsigned short color) {
		/* set the screen location to this color */
		screen[row * SCREEN_WIDTH + col] = color;
}

/* function to setup background 0 for this program */
void setup_background() {

	/* load the palette from the image into palette memory*/
	memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) background_palette, PALETTE_SIZE);

	/* load the image into char block 0 */
	memcpy16_dma((unsigned short*) char_block(0), (unsigned short*) background_data,
		(background_width * background_height) / 2);

	memcpy16_dma((unsigned short*) char_block(1), (unsigned short*) text_data,
		(text_width * text_height) / 2);

	/* set all control the bits in this register */
	*bg0_control = 2 |    /* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |       /* the char block the image data is stored in */
	(0 << 6)  |       /* the mosaic flag */
	(1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
	(16 << 8) |       /* the screen block the tile data is stored in */
	(1 << 13) |       /* wrapping flag */
	(0 << 14);        /* bg size, 0 is 256x256 */

	/* load the tile data into screen block 16 */
	memcpy16_dma((unsigned short*) screen_block(16), (unsigned short*) map, map_width * map_height);

	/* set all control the bits in this register */
	*bg1_control = 1 |    /* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |         /*the char block the image data is stored in */
	(0 << 6)  |         /* the mosaic flag */
	(1 << 7)  |         /* color mode, 0 is 16 colors, 1 is 256 colors */
	(24 << 8) |         /* the screen block the tile data is stored in */
	(1 << 13) |         /* wrapping flag */
	(0 << 14);          /* bg size, 0 is 256 */

	/* load the tile data into screen block 24 */
	memcpy16_dma((unsigned short*) screen_block(24), (unsigned short*) maptrans, maptrans_width * maptrans_height);

		/* load the palette from the image into palette memory*/
		//memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) t_palette, PALETTE_SIZE);

	/* set all control the bits in this register */
	*bg2_control = 0 |    /* priority, 0 is highest, 3 is lowest */
	(1 << 2)  |         /*the char block the image data is stored in */
	(0 << 6)  |         /* the mosaic flag */
	(1 << 7)  |         /* color mode, 0 is 16 colors, 1 is 256 colors */
	(28 << 8) |         /* the screen block the tile data is stored in */
	(1 << 13) |         /* wrapping flag */
	(0 << 14);          /* bg size, 0 is 256 */


	/* clear the tile map in screen block 25 to all black tile*/
		volatile unsigned short* ptr = screen_block(28);

		/* clear the text map to be all blanks */
	ptr = screen_block(28);
		for (int i = 0; i < 32 * 32; i++) {
			ptr[i] = 0;
		}

}

void setup_background1() {

	/* loop through each column of the screen */
	for (int row = 0; row < SCREEN_HEIGHT; row++) { 
			for (int col = 0; col < SCREEN_WIDTH; col++) {
					put_pixel(row, col, slug_title_data[row * SCREEN_WIDTH + col]);
			}
	}

}

void setup_background2() {

	/* loop through each column of the screen */
	for (int row = 0; row < SCREEN_HEIGHT; row++) { 
			for (int col = 0; col < SCREEN_WIDTH; col++) {
					put_pixel(row, col, main_bg_data[row * SCREEN_WIDTH + col]);
			}
	}

}


/* function to set text on the screen at a given location */
void set_text(char* str, int row, int col) {                    
		/* find the index in the texmap to draw to */
		int index = row * 32 + col;

		/* the first 32 characters are missing from the map (controls etc.) */
		int missing = 32; 

		/* pointer to text map */
		volatile unsigned short* ptr = screen_block(28);

		/* for each character */
		while (*str) {
				/* place this character in the map */
				ptr[index] = *str - missing;

				/* move onto the next character */
				index++;
				str++;
		}   
}

/* just kill time */
void delay(unsigned int amount) {
	for (int i = 0; i < amount * 10; i++);
}

/* setup all sprites */
void sprite_clear() {

	/* move all sprites offscreen to hide them */
	for(int i = 0; i < NUM_SPRITES; i++) {
	sprites[i]->sprite_m.attribute0 = SCREEN_HEIGHT;
	sprites[i]->sprite_m.attribute1 = SCREEN_WIDTH;
	}
}

/* update all of the sprites on the screen */
void sprite_update_all() {
	/* copy them all over */
	memcpy16_dma((unsigned short*) sprite_attribute_memory, (unsigned short*) sprites_m, NUM_SPRITES * 4);
}


/* this function checks whether a particular button has been pressed */
unsigned char button_pressed(unsigned short button) {
	/* and the button register with the button constant we want */
	unsigned short pressed = *buttons & button;

	/* if this value is zero, then it's not pressed */
	if (pressed == 0) {
	return 1;
	} else {
	return 0;
	}
}

/* setup the sprite image and palette */
//FIGURE OUT HOW TO ADD SECOND SPRITE
void setup_sprite_image() {
	/* load the palette from the image into palette memory*/
	memcpy16_dma((unsigned short*) sprite_palette, (unsigned short*) sprites_palette, PALETTE_SIZE);

	/* load the image into char block 0 */
	memcpy16_dma((unsigned short*) sprite_image_memory, (unsigned short*) sprites_data, (sprites_width * sprites_height) / 2);
}




/* ====== Main ========================================================================================= */

int main( ) {

	/* title display */
	*display_control = MODE3 | BG2_ENABLE;
	setup_background1();
	delay(70000);

	/* main background */
	*display_control = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | SPRITE_ENABLE | SPRITE_MAP_1D;
	setup_background();

	/* setup the sprite image data */
	setup_sprite_image();

	/* initiate random number generator */
	srand(0);

	/*						 *\
	 * sprite initialization * =============================================================================
	\*						 */

	sprites[0] = new_Sprite("Mario", SIZE_32_64, 200, 88, 0, 0, 0, 0);
	sprite_set_floor(sprites[0],85);

	sprites[1] = new_Sprite("Megaman", SIZE_32_32, 100, 120, 1, 0, 384,0);
	sprite_set_player(sprites[1]);

	//need to draw this off screen, then change x, y to come from the gun
	sprites[2] = new_Sprite("Bullet", SIZE_8_8, 130, 130, 0, 0, 608, 0);

	/* sprite collisions */
	sprite_collision_init(sprites[0],6, 25, 20, 64,40);
	sprite_collision_init(sprites[1], 5, 27, 8, 32, 40);

	/* to-do:
	   marco, mario, bullet animation cycles
	   sprite_animation_init(sprites[0],128,640,0,128,896,3328,0,0);
	*/

	/* sprite animations */
	sprite_animation_init(sprites[0], 64, 320, 0, 0, 0, 0, 0, 64);
	sprite_animation_init(sprites[1], 416, 480, 544, 576, 0, 0, 384, 416); 
	sprite_animation_init(sprites[2], 608, 648, 0, 0, 0, 0, 0, 0);

	/*						 *\
	 * game variables        * =============================================================================
	\*						 */

	/* for clearing text */
	volatile unsigned short * ptr = screen_block(28);

	/* set initial scroll to 0 */
	int xscroll = 0;
	int sprite_scroll = 0;
	int bulletTravel = 0;

	/* AI vars */
	int ai_move = 0;
	int ai_jump = 0;

	int start_counter = 200;
	int title_counter = 200;
	int clear = 0;

	sprite_set_pos(sprites[0],-64,90);
	sprite_set_pos(sprites[1],-64,115);

	/*						 *\
	 * title sequence        * =============================================================================
	\*						 */

	while (sprites[0]->x < SCREEN_WIDTH/2 - 20) {    
		sprite_move_right(sprites[0]);
		title_counter--;
		for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
		}
		wait_vblank();
		*bg0_x_scroll = xscroll/2;
		*bg1_x_scroll = xscroll*2;
		sprite_update_all();
		delay(800);
	}

	delay(10000);

	sprite_move_none(sprites[0]);
	for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
	}

	wait_vblank();
	sprite_update_all();

	char msg_01 [32] = "Mario is mad.";
	set_text(msg_01, 6, 6); 

	delay(100000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	char msg_02 [32]  = "You're in danger.";
	set_text(msg_02, 6, 6); 

	delay(100000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	char msg_03 [32] = "There is no escape.";
	set_text(msg_03, 6, 6); 

	delay(100000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	char msg_04 [32] = "Try to survive.";
	set_text(msg_04, 6, 6); 

	delay(100000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	sprites[0]->falling = 1;
	sprites[1]->falling = 1;

	sprite_set_pos(sprites[0],200,-10);
	sprite_set_pos(sprites[1],20,-10);

	while (sprites[1]->falling == 1) {    


		for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
		}

		wait_vblank();
		*bg0_x_scroll = xscroll/2;
		*bg1_x_scroll = xscroll*2;
		sprite_update_all();
		delay(800);
	}


	/*						 *\
	 * main sequence         * =============================================================================
	\*						 */

	/* loop forever */
	while (1) {

		/* AI decisions */
		ai_move = rand() % 100;
		ai_jump = rand() % 100;

		/* update sprites */
		for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
		}

		/* ? */
		sprite_scroll = 0;
	
		/* User Controls */
		if (button_pressed(BUTTON_RIGHT)) {
			if (move_right(sprites[1])) {
				xscroll++;
				sprite_scroll--;
			}
		} 
		else if (button_pressed(BUTTON_LEFT)) {
			if (move_left(sprites[1])) {
				xscroll--;
				sprite_scroll++;
			}
		} 
		else {
			move_none(sprites[1]);
		}
		if (button_pressed(BUTTON_A)) {
			shoot(sprites[0], sprites[1], sprites[2], bulletTravel);
		}
		if (button_pressed(BUTTON_UP)) {
				jump(sprites[1]);
		}
		
		/* sprite behavior */							
		sprite_ai(sprites[0],sprites[1],ai_move,ai_jump);

		/* wait for vblank before scrolling and moving sprites */
		wait_vblank();
		*bg0_x_scroll = xscroll/2;
		*bg1_x_scroll = xscroll*2;
		sprite_update_all();

		/* delay some */
		delay(400);

	}

}

/* the game boy advance uses "interrupts" to handle certain situations
 * for now we will ignore these */
void interrupt_ignore( ) {
	/* do nothing */
}

/* this table specifies which interrupts we handle which way
 * for now, we ignore all of them */
typedef void (*intrp)( );
const intrp IntrTable[13] = {
	interrupt_ignore,   /* V Blank interrupt */
	interrupt_ignore,   /* H Blank interrupt */
	interrupt_ignore,   /* V Counter interrupt */
	interrupt_ignore,   /* Timer 0 interrupt */
	interrupt_ignore,   /* Timer 1 interrupt */
	interrupt_ignore,   /* Timer 2 interrupt */
	interrupt_ignore,   /* Timer 3 interrupt */
	interrupt_ignore,   /* Serial communication interrupt */
	interrupt_ignore,   /* DMA 0 interrupt */
	interrupt_ignore,   /* DMA 1 interrupt */
	interrupt_ignore,   /* DMA 2 interrupt */
	interrupt_ignore,   /* DMA 3 interrupt */
	interrupt_ignore,   /* Key interrupt */
};

