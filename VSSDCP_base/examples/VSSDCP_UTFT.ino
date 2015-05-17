// Using protocol VSSDCP from serdisplib with a UTFT-based display module
//
// (C) 2014-2015 by Wolfgang Astleitner
// License: GNU GPL2
//
// serdisplib: https://svn.code.sf.net/p/serdisplib/code/serdisplib/branches/vssdcp  by Wolfgang Astleitner
// UTFT: http://www.rinkydinkelectronics.com/library.php?id=51 by Henning Karlsen

//Protocol name: VSSDCP

// include the header file of VSSDCP base class (or arduino IDE's build process will fail)
#include <VSSDCP_base.h>

// Hardware-specific library
#include <UTFT.h>

//#define DISP_480x320
#define DISP_320x240


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


#ifdef DISP_480x320
  #define DISPLAY_NAME "Adafruit HX8357"
  #define DISPLAY_ID HX8340B_8
#else
  #define DISPLAY_NAME "Adafruit ILI938X"
  #define DISPLAY_ID ILI9325C
#endif
                
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#if defined(__MK20DX128__) || defined(__MK20DX256__)
  //  For 8 bit mode:    For 16 bit mode, these connections are *also* required
  //  LCD    Teensy3     LCD    Teensy3
  //  ---    -------     ---    -------
  //  DB8       2        DB0      16
  //  DB9      14        DB1      17
  //  DB10      7        DB2      19
  //  DB11      8        DB3      18
  //  DB12      6        DB4       0
  //  DB13     20        DB5       1
  //  DB14     21        DB6      32  (bottom side)
  //  DB15      5        DB7      25  (bottom side)

  #define RS  23
  #define WR  22
  #define CS  3
  #define RST 4

  #define YP A2  // must be an analog pin, use "An" notation!
  #define XM A3  // must be an analog pin, use "An" notation!
  #define YM A1 //8   // can be a digital pin
  #define XP A4 //9   // can be a digital pin
  
#else
  #define RS  23
  #define WR  22
  #define CS  3
  #define RST 4
#endif


// other defines


class VSSDCP_utft : public
#if defined(VSSDCP_ETHERNET)
                                  VSSDCP_ethernet
#elif defined(VSSDCP_UIPETHERNET)
                                  VSSDCP_uipethernet
#else
                                  VSSDCP_serial
#endif
{
  private:
    UTFT* tft;
    regtype *P_CS;
    regsize B_CS;

  public:
    VSSDCP_utft                   (void) {  }

    void   setup                  (void);
    String getName                (void) { return DISPLAY_NAME; }
  
    void   drawPixel              (uint16_t, uint16_t, uint32_t);
    int8_t commandClearScreen     (void);
    bool   commandCommit          (void);
    int8_t commandDrawWindow      (void);
};

void VSSDCP_utft::setup(void) {
  featureSelfemitting = true;

  //                 A                     R                     G                     B
  colourOrder[0] =  -1; colourOrder[1] =   0; colourOrder[2] =   1; colourOrder[3] =   2;
  colourChannel[0] = 0; colourChannel[1] = 5; colourChannel[2] = 6; colourChannel[3] = 5;

  // planes and maxChunk will be calculated when calling command 'I'

  bgcol = 0x0000;
  fgcol = 0xFFFF;

  // initialising display module
  P_CS    = portOutputRegister(digitalPinToPort(CS));
  B_CS    = digitalPinToBitMask(CS);

  tft = new UTFT(DISPLAY_ID, RS, WR, CS, RST);

  tft->InitLCD(PORTRAIT);
  
  width = tft->disp_x_size+1;
  height = tft->disp_y_size+1;

  // test patterns (white / red / green / blue)
  tft->fillScr(0xFFFF); delay (100);
  tft->fillScr(0xF800); delay (100);
  tft->fillScr(0x07E0); delay (100);
  tft->fillScr(0x001F); delay (100);

  // clear background
  tft->fillScr(bgcol);

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
  //nopDelay=1; // 1000;
}

bool VSSDCP_utft::commandCommit (void) {
  return true;
}

int8_t VSSDCP_utft::commandClearScreen (void) {
  tft->fillScr( bgcol );
  return 0;
}

void VSSDCP_utft::drawPixel (uint16_t i, uint16_t j, uint32_t col) {
  tft->setColor((word)col);
  tft->drawPixel((int)i, (int)j);
}


int8_t VSSDCP_utft::commandDrawWindow (void) {
  uint16_t x, y, w, h;
  uint16_t i, j;

  int pos = 0;
  byte chunk;
  
  bool ack = false;

  uint32_t curr = 0;
  uint32_t total = 0;
  
  size_t rv = readData(headerBuf, 8);

  if (rv == 8) {
    needsUpdate = true;

    x = (headerBuf[0] << 8) | headerBuf[1];
    y = (headerBuf[2] << 8) | headerBuf[3];
    w = (headerBuf[4] << 8) | headerBuf[5];
    h = (headerBuf[6] << 8) | headerBuf[7];

    cbi(P_CS, B_CS);
    tft->setXY((word)x, (word)y, (word)(x + w - 1), (word)(y + h - 1));

    total = ( (uint32_t)w * (uint32_t)h * 2);
    
    do {
      byte _tmp;

      chunk = ( (total - curr) < maxChunk) ? (total - curr) : maxChunk;

      rv = readData(dataBuf, chunk);

      if (rv > 0) {
        for (i = 0 ; i < rv; i+=2) {
          tft->LCD_Write_DATA(dataBuf[i], dataBuf[i+1]);
        }
        curr += chunk;

        if ( (total - curr) >= maxChunk) {
          sendSimpleACK();
        }
      } else {
        curr += chunk;
      }
    } while (curr < total);
    sbi(P_CS, B_CS);
    tft->clrXY();
    //nop(100);
    return 0;
  }
  return -1;
};


/* program start */

VSSDCP_utft disp;

void setup() {
  disp.setup();
}

void loop() {
  bool rv;
  if (disp.availData()) {
    char c = disp.readByte();
    rv = disp.processCommand (c);
    
    disp.commandCommit();
  }
}


