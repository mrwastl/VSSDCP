// Base class for protocol VSSDCP, implementing communication to an Ethershield board via an ethernet connection.
//
// This class may be inheritated by a class that implements calls for controlling display modules using 
// Teensy/Arduino-libraries
//
// See folder VSSDCP_base/examples/ for examples.
//
// Attention: this class is not yet usable (very error-prone and unreliable communication)

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#ifndef VSSDCP_ethernet_h
#define VSSDCP_ethernet_h

// special case for arduino-ide
#if defined(ARDUINO)
  #include "../VSSDCP_base/VSSDCP_base.h"
#else
  #include "VSSDCP_base.h"
#endif


#include <EthernetServer.h>
#include <EthernetClient.h>

#define MAX_BUFFER_CHUNK 4000


class VSSDCP_ethernet : public VSSDCP_base {
  private:
  protected:
    EthernetServer* server;
    EthernetClient client;

  public:
    VSSDCP_ethernet   (void);
    bool   config     (uint16_t port);

    int    availData  (void);
    int    readByte   (void);
    size_t readData   (byte*, size_t);
    size_t sendByte   (byte);
    size_t sendData   (byte* buf, size_t);

    virtual size_t  getDataBufferSize  (void) { return MAX_BUFFER_CHUNK; };
};

#endif /* VSSDCP_ethernet_h */
