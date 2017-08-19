/************************************************************************
 * Hardware-hugging NES-style Picture Processing Unit for the Pixel 2.0 *
 ************************************************************************/

#include "UcPPU.h"
#include "UcPPUConfig.h"

#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

#define TILE_LINE(index,y) (tiles[(index << 3) + y])
#define PALETTE_ENTRY(num,index) (palettes[(num << 4) + index])

// Convert 0..15 to r (0..31), g (0..63), b (0..31) and pack into rgb565
// e.g., dcba -> dcbad dcbadc dcbad is a close approximation for this
#define GRAY_ENTRY(index) ((index<<12) | ((index >> 3) << 11) | (index << 7) | ((index >> 2) << 5) | (index << 1) | (index >> 3))

#define TILE(ref) (ref & 0x3FF)
#define PALETTE(ref) ((ref >> 10) & 0xF)
#define FLIPY(ref) ((ref >> 14) & 0x1)
#define FLIPX(ref) ((ref >> 15) & 0x1)
#define HIBYTE(w) (w >> 8)
#define LOBYTE(w) (w & 0xFF)


#define UCPPU_PIXELS UCPPU_WIDTH * UCPPU_HEIGHT
#define UCPPU_NONE ((UCPPU_NO_PALETTE << 10) | UCPPU_NO_TILE)


void UcPPU::renderNametableScanline(Layer layer, uint16_t* line, uint8_t screen_y, uint8_t trans) {

  uint16_t y = screen_y + bg_pos[layer][1];
  uint16_t table_y = y >> 3;
  uint16_t tile_y = y & 7;
  uint16_t x, screen_x, table_x, tile_x;
  uint16_t entry;
  uint16_t tile;
  uint8_t palette;
  uint32_t mask, shift;
  uint8_t pi;
  
  
  for (x = bg_pos[layer][0], screen_x = 0; screen_x < UCPPU_WIDTH; x++, screen_x++) {
    
    
    // Draw the background tiles
    // X index into the background table
    table_x = x >> 3;
    if (table_x >= UCPPU_BG_WIDTH) { return; }
    // X pixel within the tile (0..7)
    tile_x = x & 7;
    // Get the background table entry
    entry = bg_table[layer][table_y][table_x];
    // Extract the tile index
    tile = (entry & 0x3ff);
    if (tile == 0x3ff) { continue; }
      
    //Extract the palette index
    palette = PALETTE(entry);
    if (FLIPX(entry)) { tile_x = 7 - tile_x; }
    if (FLIPY(entry)) { tile_y = 7 - tile_y; }
    
    // Get the line of pixels
    shift = 32 - (tile_x << 2);
    mask = 0xF << shift;
    pi = ((TILE_LINE(tile, tile_y) & mask) >> shift) & 0xF;
    if (pi != 0 || !trans) {
      
      if (palette != UCPPU_NO_PALETTE && palette < nPalettes) {
        line[screen_x] = PALETTE_ENTRY(palette, pi);
      } else {
        line[screen_x] = GRAY_ENTRY(pi);
      }
    }
    
    
    
  }
  
}

void UcPPU::render() {
    // Allocate enough space for one {spriteIndex} per-line
    // The structure of this is scanlineSprite[y][i], where `i` will grow with more sprites on the same scanlines.
    // Similar limit to the SNES of 32 sprites per scanline
    uint8_t scanSprites[UCPPU_HEIGHT][UCPPU_SPRITES_PER_SCAN];
    uint8_t ssi;
    uint32_t pixelLine;
    uint16_t x, y;
    uint8_t index;
    int16_t tile_y, tile_x;
    uint16_t tile;
    uint16_t scanline[UCPPU_WIDTH];
    int16_t table_y, table_x;
    uint8_t palette;
    uint8_t pi;
    uint16_t entry;
    uint16_t screen_x, screen_y;
    uint16_t color;
    uint32_t mask;
    uint32_t shift;
    memset(&scanSprites, 0xFF, UCPPU_HEIGHT * UCPPU_SPRITES_PER_SCAN);
    
    
    // Run over each sprite; don't bother doing this if we don't need to.
    if (sprites_enabled) {
      for (index = 0; index < UCPPU_SPRITES; index++) {
          // Pull the palette/tile entry
          entry = sprites[index][0];
          
          // No tile; skip this sprite
          if ((entry & 0x7FF) == UCPPU_NO_TILE) { 
            continue; 
          }
          entry = sprites[index][1];
          tile_x = (entry >> 8) - spr_offset_x;
          if (tile_x + 7 < 0 || tile_x >= UCPPU_WIDTH) { continue; }
          tile_y = (entry & 0xFF) - spr_offset_y;
          if (tile_y + 7 < 0 || tile_y >= UCPPU_WIDTH) { continue; }
          
          for (y = 0, screen_y = tile_y; y < 8; y++, screen_y++) {
              if (screen_y < 0) { continue; }
              if (screen_y >= UCPPU_HEIGHT) { break; }
              // Find the next empty slot
              for (ssi = 0; ssi < UCPPU_SPRITES_PER_SCAN; ssi++) {
                if (scanSprites[screen_y][ssi] == 0xFF) { break; }
              }
              // Long as we didn't walk past the end of the buffer, add the sprite reference
              if (ssi < UCPPU_SPRITES_PER_SCAN) {
                scanSprites[screen_y][ssi] = index;
              }
          }
      }
    }
    
    
    
    // Tell the screen it's going to be recieving data
    driver->startData();
    for (screen_y = 0; screen_y < UCPPU_HEIGHT; screen_y++) {
        //Initialize the scanline
        memset(scanline, bg_color, UCPPU_WIDTH << 1);
        if (bg_enabled[bg0]) {
          renderNametableScanline(bg0, &scanline[0], screen_y, false);
        }
        if (bg_enabled[bg1]) {
          renderNametableScanline(bg1, &scanline[0], screen_y, true);
        }
        if (sprites_enabled) {
          for (ssi = 0; ssi < UCPPU_SPRITES_PER_SCAN; ssi++) {
            index = scanSprites[screen_y][ssi];
            
            if (index == 0xFF) { break; }
            //Grab the sprite's tile/palette reference
            entry = sprites[index][0];
            // grab the tile; tile and palette can not at this point be NO_TILE 
            // or NO_PALETTE
            tile = TILE(entry);
            palette = PALETTE(entry);
            
            
            // sprite position (may be negative
            table_x = HIBYTE(sprites[index][1]) - spr_offset_x;
            table_y = LOBYTE(sprites[index][1]) - spr_offset_y;
            // Position of current scanline on the screen
            tile_y = screen_y - table_y;
            if (FLIPY(entry)) { tile_y = 7 - tile_y; }
            pixelLine = TILE_LINE(tile, tile_y);
            
            for (x = 0, screen_x = table_x; x < 8; x++, screen_x++) {
              if (screen_x < 0) { continue; }
              if (screen_x >= UCPPU_WIDTH) { break; }
              tile_x = x;
              if (FLIPX(entry)) { tile_x = 7 - tile_x; }
              
              shift = 32 - (tile_x << 2);
              mask = 0xF << shift;
              pi = ((pixelLine & mask) >> shift) & 0xF;
              if (pi) {
                if (palette != UCPPU_NO_PALETTE) {
                  scanline[screen_x] = PALETTE_ENTRY(palette, pi);
                } else {
                  scanline[screen_x] =  GRAY_ENTRY(pi);
                }
              }
            }
            
          }
        }
        if (bg_enabled[fg]) {
          renderNametableScanline(fg, &scanline[0], screen_y, true);
        }
        driver->transmit(scanline, UCPPU_WIDTH);
    }
    driver->endData();
}

void UcPPU::setSpriteTile(uint8_t spriteIndex, uint16_t tileIndex) {
  sprites[spriteIndex][0] = (sprites[spriteIndex][0] & 0xF800) | (tileIndex & 0x3FF);
}
void UcPPU::setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex) {
  sprites[spriteIndex][0] = (sprites[spriteIndex][0] & 0xC000) | (tileIndex & 0x3FF) | ((paletteIndex & 0xF) << 10);
}
void UcPPU::setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY) {
  sprites[spriteIndex][0] = ((flipX & 1) << 15) | ((flipY & 1) << 14) | ((paletteIndex & 0xF) << 10) | (tileIndex & 0x3FF);
}
void UcPPU::setSprite(uint8_t spriteIndex, uint16_t rawReference) {
  sprites[spriteIndex][0] = rawReference;
}
void UcPPU::setSpritePalette(uint8_t spriteIndex, uint8_t paletteIndex) {
  sprites[spriteIndex][0] = (sprites[spriteIndex][0] & 0xC3FF) | ((paletteIndex & 0xF) << 10);
}
void UcPPU::setSpriteOrientation(uint8_t spriteIndex, uint8_t flipX, uint8_t flipY) {
  sprites[spriteIndex][0] = (sprites[spriteIndex][0] & 0x3FFF) | ((flipX & 1) << 15) | ((flipY & 1) << 14);
}
void UcPPU::setSpritePosition(uint8_t spriteIndex, uint8_t x, uint8_t y) {
  sprites[spriteIndex][1] = (x << 8) | y;
}


void UcPPU::setBackgroundTile(Layer bg, uint8_t x, uint8_t y, uint16_t tileIndex) {
  bg_table[bg][y][x] = (bg_table[bg][y][x] & 0xF800) | (tileIndex & 0x3FF);
}
void UcPPU::setBackground(Layer bg, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex) {
  bg_table[bg][y][x] = (bg_table[bg][y][x] & 0xC000) | (tileIndex & 0x3FF) | ((paletteIndex & 0xF) << 10);
}
void UcPPU::setBackground(Layer bg, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY) {
  bg_table[bg][y][x] = ((flipX & 1) << 15) | ((flipY & 1) << 14) | ((paletteIndex & 0xF) << 10) | (tileIndex & 0x3FF);
}
void UcPPU::setBackground(Layer bg, uint8_t x, uint8_t y, uint16_t rawReference) {
  bg_table[bg][y][x] = rawReference;
}
void UcPPU::setBackgroundPalette(Layer bg, uint8_t x, uint8_t y, uint8_t paletteIndex) {
  bg_table[bg][y][x] = (bg_table[bg][y][x] & 0xC3FF) | ((paletteIndex & 0xF) << 10);
}
void UcPPU::setBackgroundOrientation(Layer bg, uint8_t x, uint8_t y, uint8_t flipX, uint8_t flipY) {
  bg_table[bg][y][x] = (bg_table[bg][y][x] & 0x3FFF) | ((flipX & 1) << 15) | ((flipY & 1) << 14);
}

  void UcPPU::enableLayer(Layer bg) {
    bg_enabled[bg] = 1;
  }
  void UcPPU::disableLayer(Layer bg) {
    bg_enabled[bg] = 0;
  }
  void UcPPU::enableSprites() {
    sprites_enabled = 1;
  }
  void UcPPU::disableSprites() {
    sprites_enabled = 0;
  }

void UcPPU::setBackgroundOffset(Layer bg, uint8_t x, uint8_t y) {
  bg_pos[bg][0] = x;
  bg_pos[bg][1] = y;
}

void UcPPU::setSpriteOffset(uint8_t x, uint8_t y) {
  spr_offset_x = x;
  spr_offset_y = y;
}


void UcPPU::setBackgroundColor(uint16_t color) {
  bg_color = color;
}
void UcPPU::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b) {
  bg_color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

UcPPU::UcPPU(UcPPUDriver& ppuDriver, const uint32_t* tileTable, uint16_t numTiles, const uint16_t* paletteTable, uint16_t numPalettes) {
    driver = &ppuDriver;
    
    for (uint8_t layer = 0; layer < 3; layer++) {
      for (uint8_t y = 0; y < UCPPU_BG_HEIGHT; y++) {
        for (uint8_t x = 0; x < UCPPU_BG_WIDTH; x++) {
          bg_table[layer][y][x] = UCPPU_NONE;
        }
      }
    }
    for (uint8_t index = 0; index < UCPPU_SPRITES; index++) {
      sprites[index][0] = UCPPU_NONE;
      sprites[index][1] = 0x0000;
    }
    spr_offset_x = 0;
    spr_offset_y = 0;
    bg_color = 0x0000;
    tiles = tileTable;
    nTiles = numTiles;
    palettes = paletteTable;
    nPalettes = numPalettes;
    bg_enabled[bg0] = 0;
    bg_enabled[bg1] = 0;
    bg_enabled[fg] = 0;
    sprites_enabled = 0;
}
void UcPPU::begin() {
  driver->begin();
}


