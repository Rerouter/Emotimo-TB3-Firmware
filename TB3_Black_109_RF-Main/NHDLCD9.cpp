/*
	NOTE: you must: #include "SoftwareSerial.h"
	BEFORE including the class header file

				allen joslin
				payson productions
				allen@joslin.net
*/

#include "SoftwareSerial.h"
#include "NHDLCD9.h"

/* ======================================================== */

//--------------------------
NHDLCD9::NHDLCD9 ( uint8_t pin, uint8_t numRows, uint8_t numCols, uint8_t posBase )
  : SoftwareSerial(pin, pin) {
  _bv[PINOUT] = pin;
  _bv[POSBASE] = posBase;
  _bv[BOUNCE] = 1;
  _bv[NUMROWS] = numRows;
  _bv[NUMCOLS] = numCols;
  _bv[LASTROW] = 1;
  _bv[LASTCOL] = 1;
  _bv[LASTBRIGHT] = 8;
  _bv[BOUNCEMicros] = 20;
  _ro[0] = 0;
  _ro[1] = 64;
  _ro[2] = numCols;
  _ro[3] = _ro[1] + numCols;
}


//--------------------------
void NHDLCD9::setup( uint8_t brightPcnt, boolean startEmpty ) {
  pinMode(_bv[PINOUT], OUTPUT);
  delay(_bv[BOUNCE]);

  begin(9600);
  uint8_t baud = 7;
  switch (baud)
  {
    case 1: command(0x61, 1); end(); begin(300); break;
    case 2: command(0x61, 2); end(); begin(1200); break;
    case 3: command(0x61, 3); end(); begin(2400); break;
    //case 4: command(0x61,4); end(); begin(9600); break;
    case 4: break;
    case 5: command(0x61, 5); end(); begin(14400); break;
    case 6: command(0x61, 6); end(); begin(19200); break;
    case 7: command(0x61, 7); end(); begin(57600); break;
    case 8: command(0x61, 8); end(); begin(115200); break;
    default: break;
  }
  delay(_bv[BOUNCE]);
  if (startEmpty) {
    empty();
  }
  bright(8);
  //cursorOff();
}

//--------------------------

void NHDLCD9::on () {
  command(0x41);
}


void NHDLCD9::off () {
  command(0x43);
}


void NHDLCD9::empty () {
  command(0x51); delayMicroseconds(2500);
}


void NHDLCD9::cursorOff() {
  command(0x4c);
  command(0x48);
}


void NHDLCD9::cursorBlock () {
  command(0x4b);
}


void NHDLCD9::cursorUnderline () {
  command(0x47);
}


//--------------------------
void NHDLCD9::oldbright ( uint8_t pcnt ) {
  if (_bv[LASTBRIGHT] == pcnt)	return;

  if (pcnt < 1)	pcnt = 1;
  else		pcnt = (((pcnt) * (7)) / (100)) + 1; //basically a map command map(value,0,100,1,8)
  //value =  1 byte Set the LCD backlight brightness level, value between 1 to 8
  command(0x53, pcnt);

  _bv[LASTBRIGHT] = pcnt;
}


void NHDLCD9::bright ( uint8_t pcnt ) {
  if (_bv[LASTBRIGHT] == pcnt)	return;
  if (pcnt < 1)	pcnt = 1;
  if (pcnt > 8)	pcnt = 8;
  command(0x53, pcnt);

  _bv[LASTBRIGHT] = pcnt;
}


void NHDLCD9::contrast ( uint8_t contrastval ) {
  command(0x52, contrastval);
}


//--------------------------
void NHDLCD9::pos ( uint8_t row, uint8_t col )
{
  col--;
  if (row == 2) col += 64;
  command(0x45, col);
}


// Functions for sending the special command values
void NHDLCD9::command(uint8_t value) {
  write(0xFE);
  write(value);
  //delay(_bv[BOUNCE]);
  delayMicroseconds(_bv[BOUNCEMicros]);    //_bv[BOUNCEMicros]
}

// Functions for sending the special command values
void NHDLCD9::command(uint8_t value, uint8_t value2) {
  write(0xFE);
  write(value);
  write(value2);
  //delay(_bv[BOUNCE]);
  delayMicroseconds(_bv[BOUNCEMicros]);    //_bv[BOUNCEMicros]
}


// shortcuts

void NHDLCD9::at ( uint8_t row, uint8_t col, char v )			        	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, const char v[] )	    	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint8_t v )		      	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint16_t v )		      	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint32_t v )           { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int8_t v )             { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int16_t v )            { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int32_t v )			      { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int32_t v, int16_t t )	{ pos(row,col); print(v,t); }
void NHDLCD9::at ( uint8_t row, uint8_t col, String v)			      	{ pos(row,col); print(v); }


/* ======================================================== */
