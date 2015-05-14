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


#include "VSSDCP_uipethernet.h"

VSSDCP_uipethernet::VSSDCP_uipethernet (void) : VSSDCP_base() {
  dataBuf = (byte*) malloc( getDataBufferSize() );
}

bool   VSSDCP_uipethernet::config     (uint16_t port) {
  if (!dataBuf) return false;

  server = new EthernetServer(port);
  server->begin();
  //server->begin();
  //server->write("started");
  //client = server->available();
  //client.flush();
  return true;
}

int VSSDCP_uipethernet::availData  (void) {
  client = server->available();
  if (client && client.connected()) {
    return client.available();
  }
  return 0;
}

int VSSDCP_uipethernet::readByte   (void) {
  return client.read();
}

size_t VSSDCP_uipethernet::readData   (byte* buf, size_t len) {
  rv = client.readBytes((char*)buf, len);
  Ethernet.maintain();
  //Ethernet.update();
  return rv;
}

size_t VSSDCP_uipethernet::sendByte   (byte elem) {
  rv = client.write(elem);
  return rv;
}

size_t VSSDCP_uipethernet::sendData   (byte* buf, size_t len) {
  rv = client.write(buf, len);
  Ethernet.maintain();
//  Ethernet.update();
  return rv;
}
