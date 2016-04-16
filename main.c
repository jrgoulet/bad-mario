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

/* ====== Global Vars ================================================================================= */

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT 	160
#define MODE0			0x00 		/* the tile mode flags needed for display control register */
#define BG0_ENABLE 		0x100
#define BG1_ENABLE 		0x200


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


/* function to setup background 0 for this program */
void setup_background() {

  /* load the palette from the image into palette memory*/
  memcpy16_dma((unsigned short*) bg_palette, (unsigned short*) background_palette, PALETTE_SIZE);

  /* load the image into char block 0 */
  memcpy16_dma((unsigned short*) char_block(0), (unsigned short*) background_data,
	  (background_width * background_height) / 2);

  /* set all control the bits in this register */
  *bg0_control = 1 |    /* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |       /* the char block the image data is stored in */
	(0 << 6)  |       /* the mosaic flag */
	(1 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
	(16 << 8) |       /* the screen block the tile data is stored in */
	(1 << 13) |       /* wrapping flag */
	(0 << 14);        /* bg size, 0 is 256x256 */

  /* load the tile data into screen block 16 */
  memcpy16_dma((unsigned short*) screen_block(16), (unsigned short*) map, map_width * map_height);

  /* set all control the bits in this register */
  *bg1_control = 0 |    /* priority, 0 is highest, 3 is lowest */
	(0 << 2)  |         /*the char block the image data is stored in */
	(0 << 6)  |         /* the mosaic flag */
	(1 << 7)  |         /* color mode, 0 is 16 colors, 1 is 256 colors */
	(24 << 8) |         /* the screen block the tile data is stored in */
	(1 << 13) |         /* wrapping flag */
	(0 << 14);          /* bg size, 0 is 256 */

  /* load the tile data into screen block 24 */
  memcpy16_dma((unsigned short*) screen_block(24), (unsigned short*) maptrans, maptrans_width * maptrans_height);

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
  /* we set the mode to mode 0 with bg0 on */
  *display_control = MODE0 | BG0_ENABLE | BG1_ENABLE | SPRITE_ENABLE | SPRITE_MAP_1D;

  /* setup the background 0 */
  setup_background();

  /* setup the sprite image data */
  setup_sprite_image();

  /* clear all the sprites on screen now */
  //`sprite_clear();

  /* sprite initialization */
  sprites[0] = new_Sprite("Marco", SIZE_64_64, 100, 88, 0, 0, 0, 0);

 
  //sprites_m[0] = sprites[0]->sprite_m;


  sprite_collision_init(sprites[0],21,47,64,24,40);
  sprite_animation_init(sprites[0],128,640,0,128,896,3328,0,0);
    //Goomba
  sprites[1] = new_Sprite("Goomba", SIZE_32_32, 200, 120, 0, 0, 768, 0);
  sprite_collision_init(sprites[1], 5, 27, 10, 32, 30);
  sprite_animation_init(sprites[1], 768, 928, 768, 768, 768, 768, 768, 768);
  /* set initial scroll to 0 */
  int xscroll = 0;

  /* loop forever */
  while (1) {

  	for (int i = 0; i < NUM_SPRITES; i++) {
  		sprites_m[i] = sprites[i]->sprite_m;
  	}
	
	/* User Controls */
	if (button_pressed(BUTTON_RIGHT)) {
	  if (move_right(sprites[0])) {
		xscroll++;
	  }
	} else if (button_pressed(BUTTON_LEFT)) {
	  if (move_left(sprites[0])) {
		xscroll--;
	  }
	} else {
	  move_none(sprites[0]);
	}

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

