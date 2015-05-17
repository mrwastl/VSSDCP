// Using protocol VSSDCP from serdisplib with a UTFT-based display module
//
// (C) 2014-2015 by Wolfgang Astleitner
// License: GNU GPL2
//
// serdisplib: https://svn.code.sf.net/p/serdisplib/code/serdisplib/branches/vssdcp  by Wolfgang Astleitner
// fork of SmartMatrix: https://github.com/mrwastl/SmartMatrix by Wolfgang Astleitner (orig. version by Louis Beaudoin)

// include the header file of VSSDCP base class (or arduino IDE's build process will fail)
#include <VSSDCP_base.h>

// SmartMatrix library
#include <SmartMatrix.h>


// START OF DRIVER SELECTION
// uncomment only required #include-lines
// comment out ALL other #include-lines !
// if more than one driver is enabled an #error will be thrown when compiling
// this is an arduino-IDE specific limitation that can't be solved nicely with #if 0 / #if 1 ...

// // VSSDCP_SERIAL - communication via serial / USB
#include <VSSDCP_serial.h>

// // VSSDCP_ETHERNET - communication via EtherShield-Board (W5100)
//#include <VSSDCP_ethernet.h>
//#include <Ethernet.h>
//#include <SPI.h>

// // VSSDCP_UIPETHERNET - communication via ENC28J60 ethernet board
//#include <VSSDCP_uipethernet.h>
//#include <UIPEthernet.h>

// END OF DRIVER SELECTION


#if defined(VSSDCP_SERIAL)
  #define BAUD           0    // BAUD 0 is fine for Teensy 3.x
  //#define BAUD        9600
  //#define BAUD       57600
  //#define BAUD      115200
  //#define BAUD      230400
  //#define BAUD      460800
  //#define BAUD      500000
  //#define BAUD      576000
  //#define BAUD      921600
  //#define BAUD     1000000
  //#define BAUD     1152000
  //#define BAUD     1500000
#elif defined(VSSDCP_ETHERNET) || defined(VSSDCP_UIPETHERNET)
  byte mac[] =  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  IPAddress ip(10,0,0,177);
  //EthernetServer server(9876);
  #define PORT 9876
#else
 #error "No VSSDCP_* driver defined"
#endif


// other defines

//#define DEBUG_FPS


// limit max. brightness for SmartMatrix-library (serdisplib's brightness [0,11] is relative to this max. brightness)
#define MAX_BRIGHTNESS 30

const int defaultBrightness = MAX_BRIGHTNESS;


class VSSDCP_smartmatrix : public
#if defined(VSSDCP_ETHERNET)
                                  VSSDCP_ethernet
#elif defined(VSSDCP_UIPETHERNET)
                                  VSSDCP_uipethernet
#else
                                  VSSDCP_serial
#endif
 {
  private:
    SmartMatrix* matrix;
    rgb24 tempcol;
    void   col2rgb24              (uint32_t col);

  public:
    VSSDCP_smartmatrix            (void) { }

    void   setup                  (void);
    String getName                (void);
  
    void   drawPixel              (uint16_t, uint16_t, uint32_t);
    int8_t commandClearScreen     (void);
    bool   commandCommit          (void);   
    void   changeBrightness       (uint8_t);
    void   changeInvert           (uint8_t);

#ifdef DEBUG_FPS
  private:
    void   draw_bar               (int x, int y, int w, int h, int hor, int type, rgb24 sdcol);
    void   draw_digit             (int x, int y, int digit, int segwidth, int thick, rgb24 sdcol);
    void   draw_number            (int x, int y, int number, int segwidth, int thick, int alignr, rgb24 sdcol, byte radixpos = 0);

  public:
    rgb24  COL_WHITE = {0xFF, 0xFF, 0xFF};
    rgb24  COL_BLACK = {0x00, 0x00, 0x00};

    unsigned long stime, etime;
    int fps;
    int segsize;
#endif
};


// [0, 100] -> [0, MAX_BRIGHTNESS]
static byte shiftBrightness (byte b) {
  return (byte)(b * ((float)(MAX_BRIGHTNESS / 100.0)));
}

String VSSDCP_smartmatrix::getName (void) {
  return "RGB Matrix Panel " + String(width, DEC) + "x" + String(height, DEC);
}


void VSSDCP_smartmatrix::setup(void) {
  featureSelfemitting = true;
  featureInvert = true;
  featureBrightness = 255;

  //                 A                     R                     G                     B
  colourOrder[0] =  -1; colourOrder[1] =   0; colourOrder[2] =   1; colourOrder[3] =   2; // -> RGB
  colourChannel[0] = 0; colourChannel[1] = 8; colourChannel[2] = 8; colourChannel[3] = 8; // ->    888

  // planes and maxChunk will be calculated when calling command 'I'

  bgcol = 0x000000;
  fgcol = 0xFFFFFF;


  matrix = new SmartMatrix();
  width = matrix->getScreenWidth();
  height = matrix->getScreenHeight();

  matrix->begin();
  matrix->setColorCorrection(cc24);

  // testpatterns (white / red / green / blue)
  col2rgb24(fgcol);    matrix->fillScreen(tempcol);  matrix->swapBuffers(true); delay (100);
  col2rgb24(0xFF0000); matrix->fillScreen(tempcol);  matrix->swapBuffers(true); delay (100);
  col2rgb24(0x00FF00); matrix->fillScreen(tempcol);  matrix->swapBuffers(true); delay (100);
  col2rgb24(0x0000FF); matrix->fillScreen(tempcol);  matrix->swapBuffers(true); delay (100);

  // clear background
  col2rgb24(bgcol);    matrix->fillScreen(tempcol);  matrix->swapBuffers(true);
  matrix->setBrightness(shiftBrightness(defaultBrightness));

#if defined(VSSDCP_ETHERNET)
  Ethernet.begin(mac, ip);
  config(PORT);
  delay(100);
#elif defined(VSSDCP_UIPETHERNET)
  Ethernet.begin(mac, ip);
  config(PORT);
  delay(100);
#else
  config(BAUD);
#endif
}


bool VSSDCP_smartmatrix::commandCommit (void) {
  if (needsUpdate) {

#ifdef DEBUG_FPS
    matrix->fillRectangle(width-((segsize+2)*5), height-(segsize * 2) - 1, width-1, height -1, COL_BLACK);
    draw_number( width - 4, height - (segsize * 2), fps, segsize, 1, 1, COL_WHITE, 1);
#endif

    matrix->swapBuffers(true);
    needsUpdate = false;
  }
  return true;
}


int8_t VSSDCP_smartmatrix::commandClearScreen (void) {
  col2rgb24(bgcol);
  matrix->fillScreen( tempcol );
  needsUpdate = true;
  return 0;
}


void VSSDCP_smartmatrix::drawPixel (uint16_t i, uint16_t j, uint32_t col) {
  if (currInvert) {
    col ^= 0xFFFFFF;
  }
  col2rgb24(col);
  matrix->drawPixel(i, j, tempcol );
  needsUpdate = true;
}


void VSSDCP_smartmatrix::col2rgb24(uint32_t col) {
  tempcol = { (uint8_t)((col & 0xFF0000) >> 16), (uint8_t)((col & 0x00FF00) >> 8), (uint8_t)((col & 0x0000FF)) };
}


void VSSDCP_smartmatrix::changeBrightness (uint8_t v) {
  matrix->setBrightness(shiftBrightness(v));
}


void VSSDCP_smartmatrix::changeInvert (uint8_t v) {
  bool vbool = (v) ? true : false;
  int idx;
  rgb24 c;
  rgb24* b = matrix->backBuffer();

  if (vbool != currInvert) {
    for (idx = 0; idx < height*width; idx ++) {
      c = b[idx];
      c.red = c.red ^ 0xFF;
      c.green = c.green ^ 0xFF;
      c.blue = c.blue ^ 0xFF;
      b[idx] = c;
    }
    currInvert = (currInvert) ? false : true;
    needsUpdate = true;
  }
}


#ifdef DEBUG_FPS
void VSSDCP_smartmatrix::draw_bar(int x, int y, int w, int h, int hor, int type, rgb24 sdcol) {
  int i,j;
  for (j = y; j < y + ((hor)? h : w); j += ((type) ? 2 : 1))
    for (i = x; i < x + ((hor)? w : h); i += ((type) ? 2 : 1))
      if ( ! ( (w == 1 || h == 1) && (type == 1)) ) /* don't draw dotted bar if thickness of w or h is only 1 */
        matrix->drawPixel(i, j, sdcol);
}

void VSSDCP_smartmatrix::draw_digit(int x, int y, int digit, int segwidth, int thick, rgb24 sdcol) {
  if (digit < 0 || digit > 9) return;

  draw_bar(x, y,                    segwidth, thick, 1, !(digit != 1 && digit != 4), sdcol);
  draw_bar(x, y + segwidth - thick, segwidth, thick, 1, !(digit != 1 && digit != 7 && digit != 0), sdcol);
  draw_bar(x, y + 2*(segwidth - thick), segwidth, thick, 1, !(digit != 1 && digit != 4 && digit != 7), sdcol);

  draw_bar(x, y,                    segwidth, thick, 0, !(digit == 4 || digit == 5 || digit == 6 || digit == 8 || digit ==  9 || digit == 0), sdcol);
  draw_bar(x + segwidth - thick, y,  segwidth, thick, 0, !(digit != 5 && digit != 6), sdcol);
  draw_bar(x, y + segwidth - thick,                    segwidth, thick, 0, !(digit == 2 || digit == 6 || digit == 8 || digit == 0), sdcol);
  draw_bar(x + segwidth - thick, y + segwidth - thick,  segwidth, thick, 0, !(digit != 2), sdcol);
  
}


void VSSDCP_smartmatrix::draw_number(int x, int y, int number, int segwidth, int thick, int alignr, rgb24 sdcol, byte radixpos) {
  int digits, i, posx;

  int num = number;

  if (num < 0 && num > 9999) return;

  if (num >= 1000) digits = 4;
  else if (num >= 100) digits = 3;
  else if (num >= 10) digits = 2;
  else digits = 1;

  if (alignr) {
    posx = x - (segwidth - thick);
    x -= digits *segwidth+digits*(thick-1);
    if (radixpos) {
      x -= thick* 3;
    }
  } else {
    posx = x;
  }

  for (i = digits-1; i >= 0; i--) {
    draw_digit(posx, y, num % 10, segwidth, thick, sdcol);
    if (radixpos && ((digits - i) == radixpos)) {
      matrix->drawPixel(posx - (2 * thick), y + segwidth * 2 - 2, sdcol);
      posx-= 2;
    }
    posx -= segwidth + thick;
    num /= 10;
  }
}

  #define MAX_ITER 10
  int iter;
#endif //  DEBUG_FPS



/* program start */

VSSDCP_smartmatrix disp;  /* instance of VSSDCP_smartmatrix */


void setup() {
  disp.setup();

  delay(100);

#ifdef DEBUG_FPS
  disp.fps = 0;
  iter = 0;
  disp.segsize = 3;
#endif
}

void loop() {
  bool rv;
  if (disp.availData()) {

#ifdef DEBUG_FPS
    if (iter == 0) {
      disp.stime = micros();
    }
#endif

    char c = disp.readByte();
    rv = disp.processCommand (c);

    disp.commandCommit();

#ifdef DEBUG_FPS
    if (iter == MAX_ITER - 1) {
      disp.etime = micros();
      /* (1 sec * 10 * MAX_ITER) / timediff;  * 10: avoid floating point calc (-> one digit after decimal point) */
      disp.fps =  (int)((long)(1000000 * 10 * MAX_ITER) /  (long)(disp.etime - disp.stime));
      iter = 0;
    } else {
      iter++;
    }
#endif
  }
}


