#ifndef UCPPUDRIVER_H
#define UCPPUDRIVER_H
// Provides uint*
#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

class UcPPUDriver {
  public:
    // Should initialize the screen
    virtual void begin();
    // Should prepare the screen to accept pixel data
    virtual void startData();
    // Should shuffle pixel data to the screen
    virtual void transmit(uint16_t* data, uint16_t count);
    // Clean-up after a data transmission
    virtual void endData();
    
    // Return the screen's physical width and height
    virtual uint8_t getWidth() = 0;
    virtual uint8_t getHeight() = 0;
};
#endif // UCPPUDRIVER_H
