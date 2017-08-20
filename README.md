# μC PPU

μC PPU is an Picture Processing Unit - like those used in older console gaming systems - originally designed for the 
[Pixel 2.0](https://www.kickstarter.com/projects/rabidprototypes/pixel-20-the-arduino-compatible-smart-display),
but slowly being generalized for other arduino-compatible hardware.

At its present state, it only works for the Pixel 2.0, or the Arduino Zero with an attached SSD1351-controlled display.

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

This library implements a similar scheme.  To make use of it, you need to check out a [screen driver](https://github.com/Fordi/UcPPU_SSD1351), and set up PPUConfig.h and Resources.h:

`tiles` is an array of uint32_t[8], each representing a tile, with one horizontal line of pixels per `uint32_t`, one pixel per nibble, each nibble referencing an index in the active palette.

`palettes` is an array of uint16_t[16], each representing a palette, where each `uint16_t` is a rgb565 color (packed, bitwise, as (bit 15) RRRRR GGGGGG BBBBB (0)) to be referenced by a member of the `tiles` table.  

I'm finding that if the low-order green bit is not set, the SSD1351 flickers.  Which is weird, but keep it in mind.

There's a sample Resources.h in the boring sample program [P2PPU](https://github.com/Fordi/P2PPU); use that as your guide for 
resource preparation. It populates three layers of tiles and a few sprites, (all just numbers), and moves them all around.  Fun stuff!

Using the `ppu` local, you can place and palette background tiles and sprites, 
set the background's offset, etc.  Once your code is happy with its composition, 
it can call ppu.render() to throw it at the screen.

Remember, any issues / feature requests, please fill out an [issue](https://github.com/Fordi/P2PPU/issues), and I'll have a look!  Also, if something works, but is confusing, please create a ticket there with the prefix `[Docs] `.
