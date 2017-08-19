# μC PPU

μC PPU is an NES-like Picture Processing Unit, originally designed for the 
[Pixel 2.0](https://www.kickstarter.com/projects/rabidprototypes/pixel-20-the-arduino-compatible-smart-display), 
which is a neat little Cortex M0 based Arduino-compatible board with 
OLED SSD1351 (16-bit, 128x128) screen.

Currently, I'm planning to generalize it to multiple microcontrollers and screens - 
and have separated the code appropriately to achieve this - however, at its 
present state, it only works for the Pixel 2.0.

### Note:

This is a low-priority work in progress, and I'm aware it's slow for the 
moment (but no longer broken!  yay! see 
[open issues](https://github.com/Fordi/P2PPU/issues)).  
`[Free-for-all]`-tagged issues are free for anyone to fix (though I'll probably 
get to them).  Tickets for which I want to reserve responsibility
will be marked `[Hands-off]`.  That said, all pull requests you can offer 
will be looked at immediately, regardless of the ticket's tag.

This is being actively developed, but I do have a day job, so work may be slower 
than I like.

## Motivation

When I got my Pixel 2.0 in the mail, I was _super_ excited.  I was gonna totes
write a game specific to this device, using all the framebuffer-fu I'd learned
from various projects I've worked on over the years, starting with some VGA 
programming in the 90's.

Sadly, framebuffer-style deployment of pixel data through the narrow little SPI 
channel proved problematic - at first getting me a mere ~6 FPS using 
Adafruit's SSD1351 library to just throw rectangles at the screen.

With some tweaks to the SSD1351 lib, ultimately breaking its cross-compatibility,
I was able to get a full-screen framerate of around 9.5 FPS - that's fast enough
for some jerky gaming action!

But there's another problem: ain't no way you can fit a 128x128x16 bit framebuffer
in the Pixel's ram; that's 32k and it's only _got_ 32k.  So, I figured I'd look at 
how memory-constrained systems solved it in the past.  I knew peripherally about 
the NES' PPU, and thought that'd be a good place to start.

## Breakdown

Note: this isn't detailed or accurate, just a discussion on how the 
original device worked at a high-level.

The NES' PPU was laid out simply:

* In ROM:
     * Pattern tables, containing 8x8 sprite image data
     * Palettes, containing the color data to be applied to the patterns
* In PPU RAM
     * A pair of name tables, indicating which pattern/palettes go where, 
         aligned to 8x8 boundaries

     * A sprite table, specifying the positions of the sprites, their 
         palettes, and their patterns

The NES did rendering on a per-scanline basis, which meant that it could skip
a lot of the memory needs required of a full framebuffer.

This library implements a similar scheme.  To make use of it, you need to first 
set up PPUConfig.h and Resources.h:

`P2PPU_TILES` (PPUConfig.h) tells the PPU how many tiles you'll have in your table.

`tiles` (Resources.h) should be an array of `P2PPU_TILES` `Tile` structures, where a `Tile` 
is an 8x4 array of bytes, each byte representing a big-endian pixel pair.

`P2PPU_PALETTES` (PPUConfig.h) tells the PPU how many palettes you've got.

`palettes` (Resources.h) should be an array of `P2PPU_PALETTES` `Palette` structures, where a
`Palette` is a 16-element array of 16-bit words, representing rgb565 colors.  Though, I'm finding 
that if the low-order green bit is not set, the SSD1351 flickers.  Which is odd.

`P2PPU_SPRITES` tells the PPU how many sprites you intend to have on-screen at a time.  
More is slower.  Each sprite slot consumes 4 bytes.

`P2PPU_SPRITES_PER_SCAN` tells it how many sprites you're allowed on the same scanline.  Each slot 
consumes 128 bytes of RAM, so don't go crazy.  This should never be larger than `P2PPU_SPRITES`

`P2PPU_BG_WIDTH` and `P2PPU_BG_HEIGHT` tells the PPU how much space to allocate for the 
background layer.  Each tile slot consumes 2 bytes.

`P2PPU_WIDTH` and `P2PPU_HEIGHT` tells the PPU the physical size of the screen.  This should 
match whatever driver you're using.

There's a sample PPUConfig.h / Resources.h in the repository; use that as your guide for 
resource preparation.

Once you have that ready, you can run the boring sample program, `PPUTest.ino`.  
All it does is fill the screen with tiles, set up a sprite then scroll both
around.  Fun stuff!

Using the `ppu` local, you can place and palette background tiles and sprites, 
set the background's offset, etc.  Once your code is happy with its composition, 
it can call ppu.render() to throw it at the screen.

Remember, any issues / feature requests, please fill out an [issue](https://github.com/Fordi/P2PPU/issues), and I'll have a look!

