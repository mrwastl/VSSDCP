// Base class for protocol VSSDCP, implementing communication to an ENC28J60-based board via an ethernet connection.
//
// This class may be inheritated by a class that implements calls for controlling display modules using 
// Teensy/Arduino-libraries
//
// See folder VSSDCP_base/examples/ for examples.
//
// Attention: this class is not yet usable (very error-prone and unreliable communication)

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#ifndef VSSDCP_uipethernet_h
#define VSSDCP_uipethernet_h

// define correct VSSDC_* driver and check for multiple includes
#if defined(VSSDCP_DRIVER_INCLUDED)
 #error "Only one VSSDCP_* driver permitted"
#else
  #define VSSDCP_DRIVER_INCLUDED
  #define VSSDCP_UIPETHERNET
#endif

// special case for arduino-ide
#if defined(ARDUINO)
  #include "../VSSDCP_base/VSSDCP_base.h"
#else
  #include "VSSDCP_base.h"
#endif


#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>

#define MAX_BUFFER_CHUNK 254 //1024 //256 //2000


class VSSDCP_uipethernet : public VSSDCP_base {
  private:
    size_t rv;
  protected:
    EthernetServer* server;
    EthernetClient client;

  public:
    VSSDCP_uipethernet(void);
    bool   config     (uint16_t port);

    int    availData  (void);
    int    readByte   (void);
    size_t readData   (byte*, size_t);
    size_t sendByte   (byte);
    size_t sendData   (byte* buf, size_t);

    virtual size_t  getDataBufferSize  (void) { return MAX_BUFFER_CHUNK; };
};

#endif /* VSSDCP_uipethernet_h */
