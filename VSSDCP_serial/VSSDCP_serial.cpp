// Base class for protocol VSSDCP implementing serial communication (via serial port or USB)
//
// This class may be inheritated by a class that implements calls for controlling display modules using 
// Teensy/Arduino-libraries.
//
// See folder VSSDCP_base/examples/ for examples.
//

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#include "VSSDCP_serial.h"

VSSDCP_serial::VSSDCP_serial (void) : VSSDCP_base() {
  dataBuf = (byte*) malloc( getDataBufferSize() );
}

bool   VSSDCP_serial::config     (unsigned long _baud, byte _config) {
  if (!dataBuf) return false;

  baud = _baud;
  serialConf = _config;

  nopDelay = ((uint32_t)baud / 100000);

#if !defined(__MSP430_CPU__)
  // wait for USB CDC serial port to connect.  Arduino Leonardo only
  while (!Serial) {
  }
#endif

//Serial.flush();
  Serial.begin(baud/*, config*/);
  Serial.flush();
  return true;
}

int VSSDCP_serial::availData  (void) {
  return Serial.available();
}

int VSSDCP_serial::readByte   (void) {
  return Serial.read();
}

size_t VSSDCP_serial::readData   (byte* buf, size_t len) {
  return Serial.readBytes((char*)buf, len);
}

size_t VSSDCP_serial::sendByte   (byte elem) {
  return Serial.write(elem);

}

size_t VSSDCP_serial::sendData   (byte* buf, size_t len) {
  return Serial.write(buf, len);
}
