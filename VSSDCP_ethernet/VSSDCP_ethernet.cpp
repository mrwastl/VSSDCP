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


#include "VSSDCP_ethernet.h"

VSSDCP_ethernet::VSSDCP_ethernet (void) : VSSDCP_base() {
  dataBuf = (byte*) malloc( getDataBufferSize() );
}

bool   VSSDCP_ethernet::config     (uint16_t port) {
  if (!dataBuf) return false;

  server = new EthernetServer(port);
  server->begin();
  return true;
}

int VSSDCP_ethernet::availData  (void) {
  client = server->available();
  if (client && client.connected()) {
    return client.available();
  }
  return 0;
}

int VSSDCP_ethernet::readByte   (void) {
  return client.read();
}

size_t VSSDCP_ethernet::readData   (byte* buf, size_t len) {
  return client.readBytes((char*)buf, len);
}

size_t VSSDCP_ethernet::sendByte   (byte elem) {
  return client.write(elem);
}

size_t VSSDCP_ethernet::sendData   (byte* buf, size_t len) {
  return client.write(buf, len);
}
