// Abstract base class for protocol VSSDCP
//
// Base class for protocol VSSDCP, contains generic functionality to control display modules
// connected to Teensy or Arduino-compliant boards.

// This class is not functional, it must be inheritated by classes which implement different ways of communication
// eg: VSSDCP_serial: serial communication (via serial port or USB)
//

// (C) 2014-2015, Wolfgang Astleitner (mrwastl at users.sourceforge.net)
// License: GNU GPL2


#include <VSSDCP_base.h>

static char order2channelID (byte id) {
  switch(id) {
    case 0: return 'A';
    case 1: return 'R';
    case 2: return 'G';
    case 3: return 'B';
  }
  return ' ';
}

VSSDCP_base::VSSDCP_base(void) {
  initialised = false;
  featureBacklight = false;
  featureInvert = false;
  featureSelfemitting = false;
  featureImmediateUpdate = false;
  featureContrast = false;

  featureBrightness = 0;
  featureGPIs = 0;
  featureGPOs = 0;

  featurePixelDependency = 0;

  planes = 0;
  bbc = 0;
  maxChunk = 0;
  nopDelay = 0;
  needsUpdate = false;
  currInvert = false;
  dataBuf = NULL;;
}

void VSSDCP_base::nop (uint32_t _d) {
  if (_d) {
    delay(_d);
  } else if (nopDelay) {
    delay(nopDelay);
  }
}


bool VSSDCP_base::processCommand (char c) {
  char rv = 0;
  bool send_ack = true;

  if ( ( c == 'Q') || ( ( ! initialised ) && ( c != 'I' ) ) ) {
    while (availData()) {
      readByte();
    }
    nop(100);
    sendByte('V');
    sendByte('S');
    sendByte('S');
    sendByte('D');
    sendByte('C');
    sendByte('P');
    sendByte(0);
    return true;
  }

  if ( (! initialised) && (c != 'I') ) {
    nop();
    while (availData()) {
      readByte();
    }
    return false;
  } else if (c != 'I') {
    nop();
  }


  switch(c) {
    case 'I': {
      int cnt;
      /* read and split flags */
      nop(100);
      byte flags = (byte) readByte();
      clientflagsAcktype = (flags & 0x03);
      clientflagsEvents = (flags & 0x04) ? 1 : 0;

      String info = "";
      info += "N=" + getName();
      info += ";V=" + String(((int)VSSDCP_VERSION)) + "." + String(((int)(VSSDCP_VERSION * 10.0)) % 10);
      info += ";W=" + String(width, DEC);
      info += ";H=" + String(height, DEC);
      int i;
      // calculate some values if not already set
      if (planes == 0) {
        byte sum = 0;
        for (i = 0; i < 4; i++) {
          sum += colourChannel[i];
        }
        planes = sum;
        bbc = (planes + 7) / 8;
      }
      if (maxChunk == 0) {
        maxChunk =  ( ((uint16_t)( ( ((uint16_t)getDataBufferSize()) * 8) / planes )) * planes ) / 8;
      }
      if (plmask == 0) { // plane mask for extracting bits (only usable for b/w and greyscale <= 8 planes)
        if (planes <= 8) {
          plmask = (planes == 8) ? 0xFF : ((1 << planes) - 1);
        }
      }
      // add bpp
      info += ";P=" + String(planes, DEC);
      colourLUTlen = 4;
      for (i = 0; i < 4; i++) {
        colourLUT[i] = -1;
      }
      for (i = 0; i < 4; i++) {
        if (colourOrder[i] >= 0) {
          colourLUT[ colourOrder[i] ] = i;
        } else {
          colourLUTlen --;
        }
      }
      // add colour format
      info += ";C=";
      if (colourLUTlen == 1) {
        //featurePixelDependency = 1;  /* preset horizontal pixel dependency */
        info += "K" + String( planes, DEC );
      } else {
        for (i = 0; i < colourLUTlen; i++) {
          info += String( order2channelID( colourLUT[i] ) );
        }
        for (i = 0; i < colourLUTlen; i++) {
          info += String( colourChannel[ colourLUT[i] ] , DEC );
        }
      }
      // add transfer buffer size for payloads
      info += ";B=";
      info += String( getDataBufferSize() - 1, DEC );
      // add flags
      info += ";F=";
      info += "A" + String(clientflagsAcktype, DEC);
      if (featureSelfemitting) {
        info += "e1";
      }
      if (featureImmediateUpdate) {
        info += "u1";
      }
      if (featureInvert) {
        info += "i1";
      }
      if (featureBacklight) {
        info += "b1";
      }
      if (featurePixelDependency) {
        info += "p" + String(featurePixelDependency, DEC);
      }
      if (featureBrightness) {
        info += "B" + String(featureBrightness, DEC);
      }
      if (featureContrast) {
        info += "C1";
      }
      if (clientflagsEvents) {
        if (featureGPIs) {
          info += "I" + String(featureGPIs, DEC);
        }
        if (featureGPOs) {
          info += "O" + String(featureGPOs, DEC);
        }
      }

      // this answer is sent only once: so no speed optimisation (advantage: no extra mem required)
      cnt = 0;
      for (byte i = 0; i < info.length(); i++) {
        if (cnt >= getDataBufferSize() - 1) {
          sendData(dataBuf, cnt);
          cnt = 0;
        }
        dataBuf[cnt] = info[i];
        cnt++;
      }
      dataBuf[cnt] = 0; // null byte
      sendData(dataBuf, cnt+1);

      send_ack = false;
      initialised = true;

      break;
    }
    case 'C': {  /* clear screen */
      rv = this->commandClearScreen();
      break;
    }
    case 'b': {
      rv = commandSetColour(false);
      break;
    }
    case 'f': {
      rv = commandSetColour(true);
      break;
    }
    case 's': {
      rv = commandSetPixel(true);
      break;
    }
    case 'c': {
      rv = commandSetPixel(false);
      break;
    }
    case 'D': {
      rv = commandDrawWindow();
      //send_ack = false;
      break;
    }
    case 'U': {
      rv = commandUpdate();
      break;
    }
    case 'F': {
      rv = commandFeature();
      break;
    }
    default: return sendACK(-2);
  }

  if (send_ack) {
    if (rv >= 0) {
      /* check if event is available */
      //if (event) {
      //  return sendACK(>0);
      //} else {
        return sendACK(0);
      //}
    } else {
      return sendACK(rv);
    }
  }
  return true;
}


int8_t VSSDCP_base::commandUpdate (void) {
  needsUpdate = true;
  return 0;
}


bool VSSDCP_base::sendACK (char ack, VSSDCP_event* ev, bool simpleACK) {
  nop();
  switch (clientflagsAcktype) {
    case 0:
      return true;  /* never send ACKs */
    case 1:
      if (simpleACK)
        return true;  /* never send simpleACKs */
      break;
  }
  dataBuf[0] = 'X';
  dataBuf[1] = ack;
  byte rv = sendData(dataBuf, 2);  delay(100);
  return (rv > 0);
}


int8_t VSSDCP_base::commandSetColour (bool fg) {
  byte rv = readData(colBuf, bbc);
  char i;

  tempcol = 0;

  if (rv == bbc) {
    for (i = 0; i < bbc; i++) {
      tempcol |=  colBuf[i] << ( (bbc - i - 1) * 8);
    }
    if (fg) {
      fgcol = tempcol;
    } else {
      bgcol = tempcol;
    }
    return 0;
  }
  return -1;
}


int8_t VSSDCP_base::commandDrawWindow (void) {
  uint16_t x, y, w, h;
  uint16_t i, j;
  uint8_t p;

  int pos = 0;
  byte chunk;
  
  bool ack = false;

  uint32_t curr = 0;
  uint32_t total = 0;
  
  byte rv = readData(headerBuf, 8);

  if (rv == 8) {

    needsUpdate = true;

    x = (headerBuf[0] << 8) | headerBuf[1];
    y = (headerBuf[2] << 8) | headerBuf[3];
    w = (headerBuf[4] << 8) | headerBuf[5];
    h = (headerBuf[6] << 8) | headerBuf[7];

    if (planes < 8) {
      byte pbb = 8 / planes;
      switch (featurePixelDependency) {
        case 1:
          total = (uint32_t)( (w + pbb - 1) / pbb) * h;
          break;
        case 2:
          total = w * (uint32_t) ( (h + pbb - 1) / pbb);
          break;
        default:
          total = ( (uint32_t)w * h + (pbb - 1) ) / pbb;
      }

      i = x;
      j = y;

      for (curr = 0; curr < total; curr++) {
        if ((curr % maxChunk) == 0) {
          chunk = ( (total - curr) < maxChunk) ? (total - curr) : maxChunk;
          rv = readData(dataBuf, chunk);
          ack = true;
          pos = 0;
        }

        tempcol = dataBuf[pos];
        pos ++;

        if (featurePixelDependency == 0) { // independent pixel drawing
          for (p = 0; p < pbb; p++) {
            drawPixel(i, j, ( tempcol & (plmask << (p * planes)) ) >> (p * planes) );
            
            i++;
            if (i >= (x + w)) {
              i = x;
              j++;
            }
          }
        } else if (featurePixelDependency == 1) { // set pixel + horiz. neighbour-pixels (assertion: correctly aligned)
          drawPixel(i, j, tempcol);
          i += pbb;

          if (i >= (x + w)) {
            i = x;
            j++;
          }
        } else { // featurePixelDependency == 2: vertical drawing + neighbour-pixels
          drawPixel(i, j, tempcol);
          j += pbb;

          if (j >= y + h) {
            j = y;
            i++;
          }

        }

        if (ack) {
          if ( ( total - 1 ) != curr ) {
            sendSimpleACK();
          }
          ack = false;
        }

      }
    } else { // planes >= 8
      total = ((uint32_t) w * h * planes ) / 8;

      for (j = y; j < y + h; j++) {
        for (i = x; i < x + w; i ++) {
          if ((curr % maxChunk) == 0) {
            chunk = ( (total - curr) < maxChunk) ? (total - curr) : maxChunk;
            rv = readData(dataBuf, chunk);
            ack = true;
            pos = 0;
          }
          if (planes == 8) {
            tempcol = dataBuf[pos];
            pos ++;
            curr ++;
          } else if (planes == 16) {
            tempcol = (dataBuf[pos] << 8) | dataBuf[pos+1]; //* [pos++] doesn't work w/ teensy??
            pos += 2;
            curr += 2;
          } else if (planes == 24) {
            tempcol = (dataBuf[pos] << 16) | (dataBuf[pos+1] << 8) | dataBuf[pos+2]; //* [pos++] doesn't work w/ teensy??
            pos += 3;
            curr += 3;
          } else {
            tempcol = 0xFF0000;
          }
          if ((i < width) && (j < height)) {
            drawPixel(i, j, tempcol);
          }
          if (ack) {
            if ( (total- curr) >= maxChunk) {
              sendSimpleACK();
            }
            ack = false;
          }
        }
      }
    }
    return 0;
  }
  return -1;
}


int8_t VSSDCP_base::commandSetPixel (bool fg) {
  uint16_t x;
  uint16_t y;

  byte rv = readData(headerBuf, 4);
  
  if (rv == 4) {
    x = (headerBuf[0] << 8) | headerBuf[1];
    y = (headerBuf[2] << 8) | headerBuf[3];
    if (x < width && y < height ) {
      drawPixel(x, y, ( (fg) ? fgcol : bgcol) );
    }
    needsUpdate = true;
    return 0;
  }
  return -1;
}


int8_t VSSDCP_base::commandFeature (void) {
  byte rv = readData(headerBuf, 2);

  if (rv == 2) {
    switch((char)headerBuf[0]) {
      case 'B':
        if (featureBrightness) {
          changeBrightness( (headerBuf[1] > featureBrightness) ? featureBrightness : headerBuf[1]  );
        }
        break;
      case 'C':
        if (featureContrast) {
         changeContrast( (headerBuf[1] > featureContrast) ? featureContrast : headerBuf[1]  );
        }
        break;
      case 'i':
        if (featureInvert) {
         changeInvert( (headerBuf[1]) ? 1 : 0 );
        }
        break;
    }
    return 0;
  }
  return -1;
}

