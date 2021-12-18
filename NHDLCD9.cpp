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

#define PINOUT      0
#define POSBASE     1
#define BOUNCE      2
#define NUMROWS     3
#define NUMCOLS     4
#define LASTROW     5
#define LASTCOL     6
#define LASTBRIGHT  8
#define BOUNCEMicros 9

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
	_bv[BOUNCEMicros] = 500;
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
	delay(_bv[BOUNCE]);
	if (startEmpty) {
		empty();
	}
	bright(8);
	//cursorOff();
}

//--------------------------

void NHDLCD9::on () {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x41); delay(_bv[BOUNCE]);
	command(0x41);
}


void NHDLCD9::off () {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x42); delay(_bv[BOUNCE]);
	command(0x43);
}


void NHDLCD9::empty () {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x51); delay(_bv[BOUNCE]*10);
	command(0x51); delay(10);
}


void NHDLCD9::cursorOff() {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x4c); delay(_bv[BOUNCE]);
	command(0x4c);

	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x48); delay(_bv[BOUNCE]);
	command(0x48);
}


void NHDLCD9::cursorBlock () {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x4b); delay(_bv[BOUNCE]);
	command(0x4b);
}


void NHDLCD9::cursorUnderline () {
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x47); delay(_bv[BOUNCE]);
	command(0x47);
}


//--------------------------
void NHDLCD9::oldbright ( uint8_t pcnt ) {
	if (_bv[LASTBRIGHT] == pcnt)	return;

	if (pcnt<1)	pcnt=1;
	else		pcnt= (((pcnt) * (7)) / (100)) + 1; //basically a map command map(value,0,100,1,8)
	//value =  1 byte Set the LCD backlight brightness level, value between 1 to 8
	//write(0xfe); delay(_bv[BOUNCE]);
	//write(0x53); delay(_bv[BOUNCE]);
	command(0x53);
	write((uint8_t) pcnt); delay(_bv[BOUNCE]);

  	_bv[LASTBRIGHT] = pcnt;
}


void NHDLCD9::bright ( uint8_t pcnt ) {
	if (_bv[LASTBRIGHT] == pcnt)	return;
	if (pcnt<1)	pcnt=1;
	if (pcnt>8)	pcnt=8;
	command(0x53);
	write((uint8_t) pcnt); delay(_bv[BOUNCE]);

	_bv[LASTBRIGHT] = pcnt;
}


void NHDLCD9::contrast ( uint8_t contrastval ) {
	command(0x52);
	write((uint8_t) contrastval); delay(_bv[BOUNCE]);
}


//--------------------------
void NHDLCD9::pos ( uint8_t row, uint8_t col )
{
	col--;
	if (row == 2) col += 64;

	command(0x45);
	write((uint8_t)  col );   delay(_bv[BOUNCE]);
}


// Functions for sending the special command values
void NHDLCD9::command(uint8_t value){
	write(0xFE);
	write(value);
	//delay(_bv[BOUNCE]);
	delayMicroseconds(_bv[BOUNCEMicros]);    //_bv[BOUNCEMicros]
}


// shortcuts

void NHDLCD9::at ( uint8_t row, uint8_t col, char v )			        	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, const char v[] )	    	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint8_t v )		      	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int16_t v )		      	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint16_t v )		      	{ pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int32_t v )			      { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, uint32_t v )			      { pos(row,col); print(v); }
void NHDLCD9::at ( uint8_t row, uint8_t col, int32_t v, int16_t t )	{ pos(row,col); print(v,t); }
void NHDLCD9::at ( uint8_t row, uint8_t col, String v)			      	{ pos(row,col); print(v); }


/* ======================================================== */
