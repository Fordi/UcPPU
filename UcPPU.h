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
   * Modify the PPU state
   **/
  
  /**
   * Set the tile index of a sprite.  A tile index of UCPPU_NO_TILE will disable the sprite.
   **/
  void setSpriteTile(uint8_t spriteIndex, uint16_t tileIndex);
  /**
   * Set the tile and palette for a sprite
   **/
  void setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex);
  /**
   * Set the tile, palette, and orientation for a sprite
   **/
  void setSprite(uint8_t spriteIndex, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY);
  /**
   * Set the tile, palette, and orientation for a sprite using a prepared reference int
   **/
  void setSprite(uint8_t spriteIndex, uint16_t rawReference);
  /**
   * Set the palette for a sprite. A palette index of UCPPU_NO_PALETTE will use a grayscale palette.
   */
  void setSpritePalette(uint8_t spriteIndex, uint8_t paletteIndex);
  /**
   * Set the orientation for a sprite
   **/
  void setSpriteOrientation(uint8_t spriteIndex, uint8_t flipX, uint8_t flipY);
  /**
   * Set the X and Y position of a sprite
   **/
  void setSpritePosition(uint8_t spriteIndex, uint8_t x, uint8_t y);
  /**
   * Set the global sprite offset (the entire sprite layer will be shifted left and up by this amount
   **/
  void setSpriteOffset(uint8_t x, uint8_t y);


  /**
   * Set the background color to an rgb565 color.  This will be the "Bottom" of the rendered image
   **/
  void setBackgroundColor(uint16_t color);
  /**
   * Set the background color to an r, g, b value.  R, G, and B are 0..255
   **/
  void setBackgroundColor(uint8_t r, uint8_t g, uint8_t b);

  /**
   * Set the tile index of a tile on a layer. A tile index of UCPPU_NO_TILE will disable the sprite.
   **/
  void setBackgroundTile(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex);
  /**
   * Set the tile and palette indices of a tile on a layer
   **/
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex);
  /**
   * Set the tile and palette indices, and orientation of a tile on a layer
   **/
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t tileIndex, uint8_t paletteIndex, uint8_t flipX, uint8_t flipY);
  /**
   * Set the tile and palette indices, and orientation of a tile on a layer by prepared int
   **/  
  void setBackground(Layer backgroundIndex, uint8_t x, uint8_t y, uint16_t rawReference);
  /**
   * Set the palette index of a tile on a layer.  A palette index of UCPPU_NO_PALETTE will use a grayscale palette.
   **/
  void setBackgroundPalette(Layer backgroundIndex, uint8_t x, uint8_t y, uint8_t paletteIndex);
  /**
   * Set the orientation of a tile on a layer.
   **/
  void setBackgroundOrientation(Layer backgroundIndex, uint8_t x, uint8_t y, uint8_t flipX, uint8_t flipY);
  /**
   * Set the position of a layer
   **/
  void setBackgroundOffset(Layer backgroundIndex, uint8_t x, uint8_t y);
  /**
   * Enable a tile layer
   **/
  void enableLayer(Layer backgroundIndex);
  /**
   * Disable a tile layer
   **/
  void disableLayer(Layer backgroundIndex);
  /**
   * Enable the sprites
   **/
  void enableSprites();
  /**
   * Disable the sprites
   **/
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
