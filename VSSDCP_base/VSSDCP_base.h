// Abstract base class for protocol VSSDCP
//
// Base class for protocol VSSDCP, contains generic functionality to control display modules
// connected to Teensy or Arduino-compliant boards.

// This class is not functional, it must be inheritated by classes which implement different ways of communication
// eg: VSSDCP_serial: serial communication (via serial port or USB)
//

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#ifndef VSSDCP_base_h
#define VSSDCP_base_h

//#if ARDUINO >= 100
 #include "Arduino.h"
// #include "Print.h"
//#else
// #include "WProgram.h"
//#endif


#define VSSDCP_VERSION 0.1


class VSSDCP_event {
};


class VSSDCP_base {
  protected:
    bool     initialised;

    /* flags sent from client when calling 'I' */
    byte     clientflagsAcktype;      /* 0: no ACKS, 1: ACKs only, 2: ACKs and simpleACKs */
    byte     clientflagsEvents;       /* 0 (client doesn't support events) or 1 (client supports events) */

    /* display features */
    bool     featureBacklight;        /* true: backlight supported, false: not supported */
    bool     featureInvert;           /* true: module/display supports inverting of display */
    bool     featureSelfemitting;     /* true: display is self emitting (displays with a dark background) */
    bool     featureImmediateUpdate;  /* true: paint commands update display content, false: extra update command needed */
    bool     featureContrast;         /* true: display supports setting contrast (values: [0..10]), false: not supported */

    byte     featureBrightness;       /* 0: not supported, else max. supported value */
    byte     featureGPIs;             /* amount of supported GPIs */
    byte     featureGPOs;             /* amount of supported GPOs */

    byte     featurePixelDependency;  /* single pixel depenency to neighbour pixels (only relevant for greyscale displays):
                                         0: no pixel dependency (single pixel can be set autonomously)
                                         1: horizontal pixel dep. (single pixel cannot be set without sibling pixels)
                                         2: vertical pixel dep. (single pixel cannot be set without pixels from same page)
                                       */

    /* colour formats:
     * note:
     *  if only R is active: [ -1, 0, -1, -1 ] => display is either monochrome (planes == 1) or greyscale (planes > 1)
     *  colour format for these cases is: K1 (monochrome), K2 - K8  (greyscale)   (K stands for blacK)
     *
     * eg:
     *  colourOrder =   [ 3, 2, 1, 0 ]    => order B, G, R, A is used
     *  colourChannel = [ 8, 8, 8, 8 ]    => all 4 channels have 8 planes
     *  ==> colour format was 'BGRA8888'
     * 
     *  colourOrder =   [ -1, 0, 1, 2 ]   => RGB is used, alpha channel is not used ( = -1 )
     *  colourChannel = [ 0, 5, 6, 5]     => alpha: not used, red: 5 planes, green: 6 planes, blue: 5 planes
     *  ==> colour format was 'RGB565'
     * 
     *  colourOrder =   [ -1, 0, -1, -1]  => only red is active -> monochrome or greyscale
     *  colourChannel = [ 0, 1, 0, 0]     => only one plane -> monochrome
     *  ==> colour format was 'K1'
     * 
     */
    int8_t   colourOrder[4];    /* [0] = order of A, [1] = R, [2] = G, [3] = B */
    byte     colourChannel[4];  /* [0] = planes for A, [1] = R, [2] = G, [3] = B */

    byte     planes;
    byte     plmask;       // mask for planes < 8 (eg. planes = 1: 0000 0001, 2: 0000 0011, ...)
    uint16_t width;
    uint16_t height;

    uint32_t bgcol;
    uint32_t fgcol;
    byte     colBuf[4];    // buffer for storing a single colour entry
    byte     headerBuf[8];
    byte*    dataBuf;      // will be assigned in constructor

    bool     currInvert;   // is display currently inverted (only req. if featureInvert is true) */
    bool     needsUpdate;


    /* internal use*/
    byte     bbc;       /* bytes per colour (for single colour transmissions. eg. planes=12 => bbc = 2) */
    byte     maxChunk;  /* max. bytes per data chunk sent in command 'D'
                          one single colour item must not be cut between two transmissions ==>
                          if one colour item would have to be split it must be transferred within the next transmission
                          */
    int8_t   colourLUT[4];
    byte     colourLUTlen;
    uint32_t tempcol;
    uint32_t nopDelay;

  public:
                    VSSDCP_base            (void);
             bool   processCommand         (char);
    virtual  void   setup                  (void)          {};

    virtual  String getName                (void)          = 0;

    virtual  size_t getDataBufferSize      (void)          = 0;

    virtual  void   drawPixel              (uint16_t, uint16_t, uint32_t) = 0;
    virtual  void   changeBrightness       (uint8_t)       {};
    virtual  void   changeContrast         (uint8_t)       {};
    virtual  void   changeInvert           (uint8_t)       {};

    virtual  bool   commandCommit          (void)          = 0;
    virtual  int8_t commandClearScreen     (void)          = 0;

    virtual  int8_t commandUpdate          (void);
    virtual  int8_t commandDrawWindow      (void);
             int8_t commandSetPixel        (bool = true);
    virtual  int8_t commandFeature         (void);

    virtual  int    availData              (void)          = 0;
    virtual  int    readByte               (void)          = 0;
    virtual  size_t readData               (byte*, size_t) = 0;
    virtual  size_t sendByte               (byte)          = 0;
    virtual  size_t sendData               (byte*, size_t) = 0;

             void   nop                    (uint32_t = 0);

             bool   sendACK                (char, VSSDCP_event* = NULL, bool = false);
             bool   sendSimpleACK          (void)        { sendACK(0, NULL, true); };

             int8_t commandSetColour       (bool = true);
};

#endif /* VSSDCP_base_h */