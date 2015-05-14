# VSSDCP
Experimental communication protocol used by serdisplib to control display modules


Introduction
------------

Communication libraries 

* VSSDCP_base is an abstract class and cannot be used directly but is required for all derived classes.

* VSSDCP_serial implements serial communication (via serial port or USB).
* VSSDCP_ethernet implements communication to an 'Ethernet Shield' via ethernet.
* VSSDCP_uipethernet implements communicaiton to an ENC28J60-based ethernet board.


To be able to get a working VSSDCP implementation on a Teensy or Arduino-board, at least VSSDCP_base must be installed
and one of the other VSSDCP_* libraries.


Required libraries (PC / Linux):

* **serdisplib**

  Branch of serdisplib with very experimental support for VSSDCP  (author: Wolfgang Astleitner)


Required additional libraries on a Teensy or Arduino-board:

* **SmartMatrix**

  either original version:  max. 32x32 pixels (author: Louis Beaudoin)

  or my *fork* with *multi-panel support*

* display library

  library that supports your display module


Getting these libraries:

* **serdisplib**

  `svn checkout  https://svn.code.sf.net/p/serdisplib/code/serdisplib/branches/vssdcp serdisplib.vssdcp`

* **SmartMatrix** (original version)

  `git clone https://github.com/pixelmatix/SmartMatrix SmartMatrix`  &nbsp;&nbsp;&nbsp;#original version

* **SmartMatrix** (my fork with multi-panel support)

  `git clone https://github.com/mrwastl/SmartMatrix SmartMatrix`

Important
---------

Only VSSDCP_serial is working in a usable way, VSSDCP_ethernet and VSSDCP_uipethernet are very error-prone and unreliable
(either because of errors in the VSSDCP-implementation of serdisplib or because of the VSSDCP_* libraries).
Improvements and/or ideas for better implementation of communication are very appreciated.



Installation
------------

The folder 'VSSDCP' contains several libraries.
At least library VSSDCP_base has to be installed and one of the other VSSDCP_* libraries.


Usage
-----

One of the VSSDCP_* classes (VSSDCP_serial, VSSDCP_ethernet, VSSDCP_uipethernet) needs to initialise a class that passes
the commands to the display library.

E.g.:

```
class VSSDCP_somedisplay : public VSSDCP_serial {
         VSSDCP_somedisplay (void) { }
  void   setup              (void) { /* setup display module, setup VSSDCP */ }
}

```

VSSDCP_base contains some examples.



Testing
-------

Testing the correct communication with serdisplib (serial communication, Teensy only (baud-rate is not required)):

```src/testserdisp -n vssdcp```

```src/testserdisp -n vssdcp -o "brightness=30;acknowledge=0" ```

If an arduino-based board is used, you have to set an appropriate baud-rate (both in your arduino-sketch and in serdisplib):

```src/testserdisp -n vssdcp -o "brightness=30;baud=9600" ```



mplayer
-------

I've programmed a batch for mplayer to support my library:

https://github.com/mrwastl/mplayer-vo_serdisp

The README.md contains the instructions for patching mplayer-soruce and compilation.


Command line example:

```LD_LIBRARY_PATH=/path_to_serdisplib_with_vssdcp_support/lib  ./mplayer -vo "serdisp:name=vssdcp:viewmode=1:options=brightness=30;acktype=0"  -framedrop -ao null somevideo.mp4```
