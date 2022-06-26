
/*
	NOTE: you must: #include <SoftwareSerial.h>
	BEFORE including the class header file

				allen joslin
				payson productions
				allen@joslin.net
*/

#ifndef NHDLCD9_h
#define NHDLCD9_h

#include "Arduino.h"
//#include "SoftwareSerial.h"

/******************************************************************************************************/
/* NHDLCD9 -- manages the NewHaven Design SerLCD, based on SoftwareSerial to aid pinning and printing */
/*                                                                                                    */
/*     some cmds are cached so repeated calls will not actually be sent which can cause               */
/*     flickering of the display, printed values are not cached and are always sent                   */
/*                                                                                                    */
/*     autoOn: turn off the display and turn it back on with the next command                         */
/*                                                                                                    */
/*     posBase: cursor positioning via 0x0 or 1x1                                                     */
/*                                                                                                    */
/*     on/off: display of characters, not backlight                                                   */
/*                                                                                                    */
/*     bright: backlight control, by percentage                                                       */
/*                                                                                                    */
/*     scrolling: scrolling is slow because of the amount of time the LCD takes to redraw.            */
/*     scrolling is persistant and moves the x-origin a single column at a time                       */
/*                                                                                                    */
/******************************************************************************************************/

class NHDLCD9 : public SoftwareSerial {
  private:
    uint8_t _bv[10];
    uint8_t _ro[5];
    void command(uint8_t);
    void command(uint8_t, uint8_t);

    enum _bv : uint8_t {
      PINOUT       = 0,
      POSBASE      = 1,
      BOUNCE       = 2,
      NUMROWS      = 3,
      NUMCOLS      = 4,
      LASTROW      = 5,
      LASTCOL      = 6,
      LASTBRIGHT   = 8,
      BOUNCEMicros = 9
    };

  public:
    NHDLCD9 ( uint8_t pin, uint8_t numRows, uint8_t numCols, uint8_t posBase = 1 );
    void setup ( uint8_t brightPcnt = 100, boolean startEmpty = true );

    void on ();
    void off ();
    void empty ();

    //void scrollLeft ();
    //void scrollRight ();

    void bright ( uint8_t pcnt );
    void oldbright ( uint8_t pcnt );
    void contrast ( uint8_t contrastval );
    void pos ( uint8_t row, uint8_t col );

    void cursorUnderline();
    void cursorBlock();
    void cursorOff ();

    // shortcuts for printing at particular positions
    void at ( uint8_t row, uint8_t col, char );
    void at ( uint8_t row, uint8_t col, const char[] );
    void at ( uint8_t row, uint8_t col, int8_t );
    void at ( uint8_t row, uint8_t col, uint8_t );
    void at ( uint8_t row, uint8_t col, int16_t );
    void at ( uint8_t row, uint8_t col, uint16_t );
    void at ( uint8_t row, uint8_t col, int32_t );
    void at ( uint8_t row, uint8_t col, uint32_t );
    void at ( uint8_t row, uint8_t col, int32_t, int16_t );
    void at ( uint8_t row, uint8_t col, String );
};


#endif
