Protocol name: VSSDCP
=====================


Important
---------
* **multi-byte values**:  **most significant bytes** are transferred at first, ```0x1234``` ->  ```0x12``` at first, then ```0x34``` (*big endian*)


Acknowledge
-----------
* ACKs are optional and may be disabled or enabled by the client.
* ACKs are sent from the server to a client and may contain a payload (events, ...)
* Command *draw window* may send *simpleACK*s between chunks (if requested by the client). A *simpleACK* must not contain a payload.
* An ACK is a `signed char` in `[-32, +32]` and is defined as following:

  | &nbsp; ACK code &nbsp; | description
  |:--------------:|:------------- 
  |   `ack < 0`    | error occured (nACK)
  |   `ack = 0`    | acknowledge, no event attached
  |   `ack > 0`    | acknowledge, event with &lt;ack&gt; bytes attached

---

Commands supported by the protocol:
-----------------------------------

| command ID | description
|:----------:|:------------- 
|    `I`     | initialise and get information
|    `C`     | clear screen
|    `b`     | set background colour
|    `f`     | set foreground colour
|    `s`     | set pixel (using foreground colour)
|    `c`     | clear pixel (sets background colour)
|    `D`     | draw window
|    `U`     | execute updates
|    `F`     | set feature value
|    `X`     | close display

---

I ... initialise and get information
------------------------------------
    -> 'I' <flags>
    <- item {';' item} '\0'


initialises the display and fetches information about it.


* **flags**

  * `<flags>` is one byte wide.

  * the bitmask in `<flags>` can be used by the client to give some hints to the server:

   | bit(s) | description
   |-------:|:---------------
   |    7   | (reserved)
   |    6   | (reserved)
   |    5   | (reserved)
   |    4   | (reserved)
   |    3   | (reserved)
   |    2   | event support
   |        | `xxxx x0xx`: client doesn't support events (GPIs)
   |        | `xxxx x1xx`: client supports events
   |  1-0   | acknowledge type
   |        | `xxxx xx00`: no acknowledge  (=> events must be queried using a command or an interrupt is triggered at the client)
   |        | `xxxx xx01`: ACKs only  (ACK may contain a reference to an event)
   |        | `xxxx xx10`: ACKs and simpleACKs (ACK may contain a reference to an event)
   |        | `xxxx xx11`: (reserved)


* **item**

  | definition                                     | #&nbsp;bytes  | description
  |:-----------------------------------------------|---------:|:-------------------
  |  `'N' '=' <up to 32 chars>`                    |   `<=32` | **name** of display / module
  |  `'V' '=' <major version> '.' <minor version>` |   `<= 4` | **version** of VSSDCP, major version in `[0, 99]`, minor version in `[0, 9]`
  |  `'W' '=' <width>`                             |    `= 2` | **width** of display, width in `[0, 65535]`
  |  `'H' '=' <height>`                            |    `= 2` | **height** of display, height in `[0, 65535]`
  |  `'P' '=' <bpp>`                               |    `= 1` | **bitplanes** used, bpp in `[0, 255]`
  |  `'C' '=' <colour format>`                     |   `<= 8` | **colour format**, eg. `RGB565`, sum of colour channel depths must match bpp
  |  `'B' '=' <buffersize times 8>`                |    `= 1` | **buffersize * 8**: max buffersize for a single payload
  |  `'F' '=' features`                            |          | *see features*


  * **colour format**

    also see additional documentation for class arrays  `colour_order[]` and `colour_channel[]`

  * **buffer size**

    buffer size for payload: `<buffersize> == 8`:  `buffersize = 8 * 8 => 64`


  * **features**
    * `{ <feature code> <value>  }`

    * range for `<value>`: `[0, 255]`

    * **feature code**

      | feature | description
      |:-----------------|:------------------------------------------------
      |  `'A'<value>`   | acknowledge-type (see flags)
      |  `'b'(0 | 1)`   | backlight support yes (1) or no (0)
      |  `'e'(0 | 1)`   | self-emitting yes (1) or no (0)
      |  `'i'(0 | 1)`   | server can invert image (1) or not (0)
      |  `'p'(0 | 1)`   | server cannot set a single pixel (1) or can set a single pixel (0)
      |  `'u'(0 | 1)`   | commit changes: extra update command needed (0) or drawing commands update immediately (1)
      |  `'B'<value>`   | brightness support, 0: not supported, > 0: max supported level
      |  `'C'(0 | 1)`   | contrast support yes (1) or no (0)
      |  `'I'<value>`   | amount of GPIs, 0: not supported / no GPIs, > 1: amount of GPIs
      |  `'O'<value>`   | amount of GPOs, 0: not supported / no GPOs, > 1: amount of GPOs

    * **note**:

      features not given default to 0

    * **examples**:  (strings must be null terminated)

       `N=Some fancy name;V=1.0;W=320;H=240;P=16;C=RGB565;F=b1I3`

       `N=RGB Matrix panel;V=1.0;W=64;H=32;P=16;C=RGB565;F=e1`

---

C ... clear screen
------------------
    -> 'C'
    <- ACK

clears the screen

* **note**:

  display is NOT updated (call command 'U' to do so)

---

b ... set background colour
---------------------------
    -> 'b' <colour>
    <- ACK

sets the background colour

* **colour**

  number of bytes according to BPP => `(bpp + 7) % 8`

---

f ... set foreground colour
---------------------------
    -> 'f' <colour>
    <- ACK

sets the background colour

* **colour**

  number of bytes according to BPP => `(bpp + 7) % 8`

---

s ... set pixel
----------------
    -> 's' <x> <y>
    <- ACK

sets a pixel at the given position using foreground colour.

* **x**

  x-position, x in `[0, 65535]`

* **y**

  y-position, y in `[0, 65535]`


* **note**:

  display is NOT updated (call command 'U' to do so)

---

c ... clear pixel
-----------------
    -> 'c' <x> <y>
    <- ACK

clears a pixel at the given position (pixel is drawn using the background colour).

* **x**

  x-position, x in `[0, 65535]`

* **y**

  y-position, y in `[0, 65535]`


* **note**:

  display is NOT updated (call command 'U' to do so)

---

D ... draw window
-----------------
    -> 'D' <x> <y> <w> <h> {<colourblock> simpleACK } <colourblock>
    <- ACK

draws a window.

* **x**

  x start position, x in `[0, 65535]`

* **y**

  y start position, y in `[0, 65535]`

* **w**

  width of window,  w in `[0, 65535]`

* **h**

  height of window, h in `[0, 65535]`

* **colourblock**

  serial of colour representations, one block contains at most `<buffersize>` bytes, followed by a `simpleACK` (if enabled)

  only the very last ACK may contain a payload (if ACKs are enabled)


* **note**:

  display is NOT updated (call command 'U' to do so)

---

U ... update screen
-------------------
    -> 'U'
    <- ACK

commits all changes and updates to the display.

---

F ... set feature value
-----------------------
    -> 'F' <feature ID> <value>
    <- ACK

en/disables a feature / setting.

* **feature ID**

  | feature ID   | description
  |:------------:|:-----------
  |    `'b'`     | backlight, 0: backlight off, 1: backlight on
  |    `'c'`     | contrast, `[0 .. 10]`, normalisation a.s.o. is done by the server
  |    `'i'`     | invert screen, 0: normal display, 1: inverted display
  |    `'B'`     | brightness, `[0 .. max value]`   (see above)
  |    `'I'`     | enable/disable GPIs, 0: disable GPI events, 1: enable GPI events
  |    `'O'`     | enable/disable GPOs, 0: disable GPO events, 1: enable GPO events


* **note**:

  a feature ID is only handled, if supported by driver (see command `'I'`, section 'flags').

---

X ... close display
-------------------
    -> 'X'
    <- ACK

closes the display.

* **note**:

  screen is NOT cleared by this command!


---
---

colour formats
--------------

* the *colour format* is set by defining two arrays:

  * `int8_t colourOrder[4]`

    * the four values contain the **order** of the colour channels `A`, `R`, `G`, `B`
    * `colourOrder[0]` is for `A`, `[1]` for `R`, `[2]` for `G`, and `[3]` for `B`
    * `-1` is set if a channel is unused (eg. `colorOrder[0] = -1` ... no alpha channel)

  * `byte colourChannel[4]`

    * amount of planes used for this channel



* **note**:
  if only `R` is active: `[ -1, 0, -1, -1 ]` => display is either monochrome (`planes == 1`) or greyscale (`planes > 1`)

  colour format for these cases is: `K1` (monochrome), `K2`, `K4`, `K8`  (greyscale)   (`K` as in _blac**K**_)

* **examples**:

  | colourOrder              | colourChannel            | resulting colour format
  |:-------------------------|:-------------------------|:-----------------------
  | `[ 3, 2, 1, 0 ]`         |                          | order `B`, `G`, `R`, `A` is used
  |                          | `[ 8, 8, 8, 8 ]`         | all 4 channels have 8 planes
  |                          |                          | => colour format is `BGRA8888`
  | ------------------------ | ------------------------ | ---------------------------------------------
  | `[ -1, 0, 1, 2 ]`        |                          | RGB is used, alpha channel is not used ( = -1 )
  |                          | `[ 0, 5, 6, 5]`          | alpha: not used, red: 5 planes, green: 6 planes, blue: 5 planes
  |                          |                          | => colour format is `RGB565`
  | ------------------------ | ------------------------ | ---------------------------------------------
  | `[ -1, 0, -1, -1]`       |                          | only red is active -> monochrome or greyscale
  |                          | `[ 0, 1, 0, 0]`          | only one plane -> monochrome
  |                          |                          | => colour format is `K1`
  | ------------------------ | ------------------------ | ---------------------------------------------
  | `[ -1, 0, -1, -1]`       |                          | only red is active -> monochrome or greyscale
  |                          | `[ 0, 4, 0, 0]`          | four planes -> greyscale
  |                          |                          | => colour format is `K4`


* only the **following combinations** are supported at the moment:

  * **supported monochrome and greyscale modes**:
    * `K1` (monochrome)
    * `K2` (4 grey levels)
    * `K4` (16 grey levels)
    * `K8` (256 grey levels)

  * **supported colour modes and colour orders**:
    * `ARGB`
    * `ABGR`
    * `RGBA`
    * `BGRA`
    * `RGB`
    * `BGR`
