/*
██████╗  █████╗ ██████╗     ███╗   ███╗ █████╗ ██████╗ ██╗ ██████╗ 
██╔══██╗██╔══██╗██╔══██╗    ████╗ ████║██╔══██╗██╔══██╗██║██╔═══██╗
██████╔╝███████║██║  ██║    ██╔████╔██║███████║██████╔╝██║██║   ██║
██╔══██╗██╔══██║██║  ██║    ██║╚██╔╝██║██╔══██║██╔══██╗██║██║   ██║
██████╔╝██║  ██║██████╔╝    ██║ ╚═╝ ██║██║  ██║██║  ██║██║╚██████╔╝
╚═════╝ ╚═╝  ╚═╝╚═════╝     ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝ ╚═════╝ 
Kenny Lyon and Joe Goulet
CPSC 305 01 - Finlayson
Spring 2016
*/



/* ====== Header Files ================================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sprites.h"
#include "controls.h"
#include "sprite.h"
#include "text.h"

#include "title_01.h"
#include "title_02.h"
#include "game_over.h"

#include "dg_map.h"
#include "dg_tiles.h"
#include "dg_platform.h"

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
	memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) dg_map_palette, PALETTE_SIZE);

	/* load the image into char block 0 */
	memcpy16_dma((unsigned short*) char_block(0), (unsigned short*) dg_map_data,
		(80 * 128) / 2);

	/* load text int char block 1 */
	memcpy16_dma((unsigned short*) char_block(1), (unsigned short*) text_data,
		(text_width * text_height) / 2);

	/* set all control the bits in this register */
	*bg0_control = 2 |  /* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |       	/* the char block the image data is stored in */
	(0 << 6)  |       	/* the mosaic flag */
	(1 << 7)  |       	/* color mode, 0 is 16 colors, 1 is 256 colors */
	(16 << 8) |       	/* the screen block the tile data is stored in */
	(1 << 13) |       	/* wrapping flag */
	(0 << 14);        	/* bg size, 0 is 256x256 */

	/* load the tile data into screen block 16 */
	memcpy16_dma((unsigned short*) screen_block(16), (unsigned short*) dg_tiles, 32 * 20);
	
    /* set all control the bits in this register */
	*bg1_control = 1 |	/* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |         /* the char block the image data is stored in */
	(0 << 6)  |         /* the mosaic flag */
	(1 << 7)  |         /* color mode, 0 is 16 colors, 1 is 256 colors */
	(24 << 8) |         /* the screen block the tile data is stored in */
	(1 << 13) |         /* wrapping flag */
	(0 << 14);          /* bg size, 0 is 256 */
    
    /* load other background */  
    memcpy16_dma((unsigned short*) screen_block(24), (unsigned short*) dg_platform, 32 * 21);

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

/* start sequence */
void setup_background1() {

	/* loop through each column of the screen */
	for (int row = 0; row < SCREEN_HEIGHT; row++) { 
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			put_pixel(row, col, title_01_data[row * SCREEN_WIDTH + col]);
		}
	}

}

/* start sequence */
void setup_background2() {

	/* loop through each column of the screen */
	for (int row = 0; row < SCREEN_HEIGHT; row++) { 
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			put_pixel(row, col, title_02_data[row * SCREEN_WIDTH + col]);
		}
	}

}

/* game over */
void setup_background3() {

	/* loop through each column of the screen */
	for (int row = 0; row < SCREEN_HEIGHT; row++) { 
		for (int col = 0; col < SCREEN_WIDTH; col++) {
			put_pixel(row, col, game_over_data[row * SCREEN_WIDTH + col]);
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

/* function to set text on the screen at a given location */
void set_text_int(int num, int row, int col) {    

	/* count number of digits */
	int digit = 0; 
	int n = num;

	while (n != 0) {
	    n /= 10;
	    digit++;
	}

	char n_str[digit];
	digit = 0;    
	n = num;

	/* extract each digit */
	while (n != 0) {
	    n_str[digit] = '0' + (n % 10);
	    n /= 10;
	    digit++;
	}

	/* find the index in the text map to draw to */
	int index = row * 32 + col;

	/* the first 32 characters are missing from the map (controls etc.) */
	int missing = 32; 

	/* pointer to text map */
	volatile unsigned short* ptr = screen_block(28);

	/* for each character */
	for (int i = digit-1; i >= 0; i--) {
		/* place this character in the map */
		ptr[index] = n_str[i] - missing;
		index++;
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
void setup_sprite_image() {
	/* load the palette from the image into palette memory*/
	memcpy16_dma((unsigned short*) sprite_palette, (unsigned short*) sprites_palette, PALETTE_SIZE);

	/* load the image into char block 0 */
	memcpy16_dma((unsigned short*) sprite_image_memory, (unsigned short*) sprites_data, (sprites_width * sprites_height) / 2);
}

/* prevents delay when pressing start button */
int exit_title() {
	for (int i = 0; i < 80000; i++) {
		if (button_pressed(BUTTON_A)) return 1;
		if (button_pressed(BUTTON_START)) return 1;
	}
	return 0;
}

int restart_game() {
	while(1) {
		if (button_pressed(BUTTON_A)) return 1;
		if (button_pressed(BUTTON_START)) return 1;
		if (button_pressed(BUTTON_B)) return 0;
	}
	return 0;
}


/* ====== Main ========================================================================================= */

int main() {

	/* title display */
	*display_control = MODE3 | BG2_ENABLE;
	
	/* flashing start screen */
	while (1) {
		wait_vblank();
		setup_background1();
		if (exit_title()) break;
		wait_vblank();
		setup_background2();
		if (exit_title()) break;
	}
       
	/* setup main background */
	*display_control = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | SPRITE_ENABLE | SPRITE_MAP_1D;
	setup_background();

	/* setup the sprite image data */
	setup_sprite_image();

	/* initiate random number generator */
	srand(0);

	/*						 *\
	 * sprite initialization * =============================================================================
	\*						 */

	/* Mario */
	sprites[0] = new_Sprite("Mario", SIZE_32_64, 200, 69, 0, 0, 0, 0);
	sprite_set_floor(sprites[0],69);

	/* Megaman */
	sprites[1] = new_Sprite("Megaman", SIZE_32_32, 100, 120, 1, 0, 384,0);
	sprite_set_player(sprites[1]);

    
    /* Bullets */
    int bullets = 12; 	/* bullets start at sprites[2] */
    int z = 2;    
    while (z <= bullets) {
        sprites[z] = new_Sprite("Bullet", SIZE_8_8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 608, 0);
        sprite_animation_init(sprites[z], 608, 648, 0, 0, 0, 0, 0, 0);
        z++;
    }

	/* sprite collisions */
	sprite_collision_init(sprites[0],6, 25, 20, 64,40);
	sprite_collision_init(sprites[1], 5, 27, 8, 32, 40);

	/* sprite animations */
	sprite_set_animation_delay(sprites[1],50);
	sprite_animation_init(sprites[0], 64, 320, 0, 0, 0, 0, 0, 64);
	sprite_animation_init(sprites[1], 416, 480, 544, 576, 0, 0, 384, 416); 

	/*						 *\
	 * game variables        * =============================================================================
	\*						 */

	/* for clearing text */
	volatile unsigned short * ptr = screen_block(28);


	int xscroll = 0;			/* set initial scroll to 0 */
	int sprite_scroll = 0;		/* used to handle sprites outside of visible window */
	int bulletTravel = 4;		/* bullet speed */
    int bulletDist = 140;		/* maximum bullet distance */		
    int has_moved = 0;			/* used to check whether a sprite has moved */
    int collide = 1;			/* used to check bullet collision */
    int game_over_flag = 0;		/* game over check */

	/* AI vars */
	int ai_move = 0;			/* AI decision-making */
	int ai_jump = 0;			/* AI decision-making */
    int marioHitCount = 1;		/* used for score */
    int marioKnockback = 0;		/* knockback counter */	
	int exit_loop = 0;    		/* exits while loop for finding inactive bullet */
	int restart = 1;			/* in-game restart */
	int play_again = 1;			/* continue? */

    /* set characters off-screen */
	sprite_set_pos(sprites[0],-64,74);
	sprite_set_pos(sprites[1],-64,115);

	/* text messages */
	char score_text [5] = "Score";
	char* score = "0";
	char game_over_text [16] = "Game Over";
	char continue_text [16]	 = "Continue?";

	/*						 *\
	 * title sequence        * =============================================================================
	\*						 */

	wait_vblank();
	sprite_update_all();

	/* clear any text */
	delay(50000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	/* dialogue */
	char msg_02 [32]  = "You're in danger.";
	set_text(msg_02, 13, 7); 

	delay(50000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	char msg_03 [32] = "Don't let Mario touch you...";
	set_text(msg_03, 13, 1); 

	delay(50000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	/* Mario walks to center */
	while (sprites[0]->x < SCREEN_WIDTH/2 - 20) {    
		sprite_move_right(sprites[0]);
		for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
		}
		wait_vblank();
		sprite_update_all();
		delay(800);
	}

	/* Mario stops walking */
	sprite_move_none(sprites[0]);
	for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
	}

	wait_vblank();
	sprite_update_all();
	delay(10000);
	
	/* Mario taunts */
	char msg_04 [32] = "I'm gonna";
	set_text(msg_04, 13, 16); 
	char msg_05 [32] = "get you!";
	set_text(msg_05, 14, 16); 

	/* clear text */
	delay(50000);
	for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }

	/*						 *\
	 * main sequence         * =============================================================================
	\*						 */

	/* loop forever */
	while (play_again == 1) {

		if (restart == 1) {
			marioHitCount = 0;
			restart = 0;

			/* start falling */
			sprite_set_floor(sprites[1],99);
			sprite_set_floor(sprites[0],69);
			sprites[0]->falling = 1;
			sprites[1]->falling = 1;
			sprite_set_vertical_flip(sprites[1],0);
			sprite_set_pos(sprites[0],200,-10);
			sprite_set_pos(sprites[1],20,-10);

			/* fall into start position */
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
		}

        collide = 1;
        has_moved = 0;
        exit_loop = 0;  //for leaving shooting loop

        /* Scoreboard */
        for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }
        set_text(score_text, 1, 1);
        set_text_int(marioHitCount,2,1);

		/* AI decisions */
		ai_move = rand() % 100;
		ai_jump = rand() % 100;

		/* update sprites */
		for (int i = 0; i < NUM_SPRITES; i++) {
			sprite_update(sprites[i],sprite_scroll);
			sprites_m[i] = sprites[i]->sprite_m;
		}

		/* reset sprite scroll */
		sprite_scroll = 0;
	
		/* User Controls */
		if (button_pressed(BUTTON_RIGHT)) {
			if (move_right(sprites[1])) {
				xscroll++;
				sprite_scroll--;
			}
			has_moved = 1;
		} 
		else if (button_pressed(BUTTON_LEFT)) {
			if (move_left(sprites[1])) {
				xscroll--;
				sprite_scroll++;
			}
			has_moved = 1;
		} 		
		/* shooting */
		if (button_pressed(BUTTON_A)) {
            if (sprites[1]->lastFired > 0) {
                sprites[1]->lastFired -= 1;
            }     
            z = 2;
 		}
        while(z <= bullets && exit_loop == 0) {
            if (sprites[z]->bulletActive == 0) {
                 exit_loop = 1;
                 if (sprites[1]->lastFired == 0) {
                 	/* sets bullet active state in ARM assembly code */
                    sprites[z]->bulletActive = activeBullet(sprites[z]->bulletActive); 
                    shoot(sprites[0], sprites[1], sprites[z], bulletTravel,bulletDist);
                    sprites[1]->lastFired = 5;
                 } 
            } 
            z++;
       	}

        /* jump */
		if (button_pressed(BUTTON_UP)) {
				jump(sprites[1]);
		}	
        
        /* update active bullets, and mario's hitbox */
        sprite_collision_init(sprites[0],6, 25, 20, 64,40);
        z = 2;
        while (z <= bullets) {
            if (sprites[z]->bulletActive == 1) {
                collide = mario_collide(sprites[z], sprites[0]);               
                /* knockback calculation */
                if(collide == 0) {
                   /* assembly function to update knockback */
                   marioKnockback = stagger(marioKnockback);
                   marioHitCount = update_hitCount(marioHitCount);
                   mario_knockdown(sprites[0], marioKnockback);
                }
                update_bullet(sprites[z], sprites[0], bulletTravel, sprites[z]->facing, collide, bulletDist);
            }   
            z++; 
        }

		/* sprite behavior */						
		sprite_ai(sprites[0],sprites[1],ai_move,ai_jump);
        
        /*check for game over */
		if (sprite_check_collision(sprites[1],sprites[0]) == 1) {
			game_over_flag = 1;
		}

		/* wait for vblank before scrolling and moving sprites */
		wait_vblank();
		*bg0_x_scroll = xscroll/2;
		*bg1_x_scroll = xscroll*2;
		sprite_update_all();

		/* delay some */
		delay(400);

		/* check game_over */
		if (game_over_flag == 1) {

			/* jump */
			sprites[1]->airtime = 5;
			sprites[1]->ymin = 180;
			sprite_set_pos(sprites[0],-64,74);

			/* fall to death */
			while (sprites[1]->y < sprites[1]->ymin) {
				for (int i = 0; i < NUM_SPRITES; i++) {
					sprite_set_vertical_flip(sprites[1],1);
					sprite_update(sprites[i],sprite_scroll);
					sprites_m[i] = sprites[i]->sprite_m;
				}
				wait_vblank();
				*bg0_x_scroll = xscroll/2;
				*bg1_x_scroll = xscroll*2;
				sprite_update_all();
				delay(5000);
			}

			set_text(game_over_text,13,9); 
			delay(50000);
			for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }
			set_text(continue_text, 13,9);
			restart = restart_game();
			if (restart == 1) play_again = 1;
			else play_again = 0;
			for (int i = 0; i < 32 * 32; i++) { ptr[i] = 0; }
			game_over_flag = 0;
		}


	}	

	/*						 *\
	 * death sequence        * =============================================================================
	\*						 */

	delay(40000);

	*display_control = MODE3 | BG2_ENABLE;
	
	wait_vblank();
	setup_background3();

	while(1) {}

} /* end main */


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

