#ifndef UCPPUCONFIG_H
#define UCPPUCONFIG_H
// Config

#define UCPPU_SPRITES 128 // Slots for sprites, can be 1..256.
#define UCPPU_SPRITES_PER_SCAN 32 // Number of sprites allowed per scanline, 1..ucPPU_SPRITES.  consumes 128 bytes of RAM per.
#define UCPPU_BG_WIDTH 32 // BG table size.  32x32 is 2 screens by 2 screens.  Could get away with 17x17 for scrolling.
#define UCPPU_BG_HEIGHT 32

#define UCPPU_WIDTH 128 // Physical size of screen; Pixel 2.0 is 128x128
#define UCPPU_HEIGHT 128

#endif //UCPPUCONFIG_H
