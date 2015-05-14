// Base class for protocol VSSDCP implementing serial communication (via serial port or USB)
//
// This class may be inheritated by a class that implements calls for controlling display modules using 
// Teensy/Arduino-libraries.
//
// See folder VSSDCP_base/examples/ for examples.
//

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#ifndef VSSDCP_serial_h
#define VSSDCP_serial_h

// special case for arduino-ide
#if defined(ARDUINO)
  #include "../VSSDCP_base/VSSDCP_base.h"
#else
  #include "VSSDCP_base.h"
#endif


// teensy 3.x
#if defined(__MK20DX128__) || defined(__MK20DX256__)
  //#define MAX_BUFFER_CHUNK 128
  //#define MAX_BUFFER_CHUNK 256
  #define MAX_BUFFER_CHUNK 254
// all other
#else
  #define MAX_BUFFER_CHUNK 64
#endif

class VSSDCP_serial : public VSSDCP_base {
  private:
    long baud;
    byte serialConf;
  protected:


  public:
    VSSDCP_serial (void);
    bool   config (unsigned long = 9600, byte = SERIAL_8N1);

    int    availData  (void);
    int    readByte   (void);
    size_t readData   (byte*, size_t);
    size_t sendByte   (byte);
    size_t sendData   (byte* buf, size_t);

    virtual size_t getDataBufferSize  (void) { return MAX_BUFFER_CHUNK; };
};

#endif /* VSSDCP_serial_h */
