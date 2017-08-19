#ifndef UCPPU_H
#define UCPPU_H

#include "UcPPUConfig.h"
#include "UcPPUDriver.h"

#define UCPPU_NO_TILE 0x3FF // Represents a `null` tile, which will be skipped in rendering
#define UCPPU_NO_PALETTE 0xF // Represents a `null` palette, which will be skipped in rendering


enum Layer {
  bg0 = 0, 
  bg1 = 1, 
  fg = 2
};



class UcPPU {
 public:
  UcPPU(UcPPUDriver& ppuDriver, const uint32_t* tileTable, uint16_t numTiles, const uint16_t* paletteTable, uint16_t numPalettes);
  /**
   * Call during setup()
   */
  void begin();
  /**
   * Call at drawing phase of loop()
   */
  void render();
  
  /**
   * Modify the PPU
   */
  void setSpriteTile(uint8_t spriteIndex, uint16_t tileIndex);
  void setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex);
  void setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY);
  void setSprite(uint8_t spriteIndex, uint16_t rawReference);
  void setSpritePalette(uint8_t spriteIndex, uint8_t paletteIndex);
  void setSpriteOrientation(uint8_t spriteIndex, uint8_t flipX, uint8_t flipY);
  void setSpritePosition(uint8_t spriteIndex, uint8_t x, uint8_t y);
  
  void setSpriteOffset(uint8_t x, uint8_t y);


  
  void setBackgroundColor(uint16_t color);
  void setBackgroundColor(uint8_t r, uint8_t g, uint8_t b);

  void setBackgroundTile(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex);
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex);
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY);
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t rawReference);
  void setBackgroundPalette(Layer backgroundIndex, uint8_t x, uint8_t y, uint8_t paletteIndex);
  void setBackgroundOrientation(Layer backgroundIndex, uint8_t x, uint8_t y, uint8_t flipX, uint8_t flipY);
  void setBackgroundOffset(Layer backgroundIndex, uint8_t x, uint8_t y);

  void enableLayer(Layer backgroundIndex);
  void disableLayer(Layer backgroundIndex);
  void enableSprites();
  void disableSprites();

    
 private:
  void renderNametableScanline(Layer bg, uint16_t* scanline, uint8_t screen_y, uint8_t transparent);
  UcPPUDriver* driver;
  
  uint16_t bg_table[3][UCPPU_BG_HEIGHT][UCPPU_WIDTH];
  uint8_t bg_pos[3][2];
  uint8_t spr_offset_x = 0;
  uint8_t spr_offset_y = 0;
  uint16_t sprites[UCPPU_SPRITES][2];
  uint16_t bg_color;
  uint8_t sprites_enabled;
  uint8_t bg_enabled[3];
  
  const uint32_t* tiles;
  uint16_t nTiles;
  const uint16_t* palettes;
  uint16_t nPalettes;
  
};

#endif // UCPPU_H
