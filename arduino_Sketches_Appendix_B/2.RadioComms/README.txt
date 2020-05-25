1. Setup

Various radio setup applications could not be made to work, so an
arduino sketch was used to set them up. It was found online
at: https://allodox.wordpress.com/2013/05/01/configuring-an-apc220-rf-transceiver-with-arduino/

In the unlikely situation this webpage no longer exists, screenshots of it are in the 2.1.RadioSetup folder

When searching for info online, these radios are called "RF7020 V4.0"
and also "APC220". See the relevant appendix for more details.


2. Communication

There is no new code for the radio communication.
The radios take a serial input, and emit a serial output.

The radio transceivers have been set up to work at 57600 baud, input & output. 
The radios have been set up to communicate between themselves at 19200 baud.
These are the highest baud rates the radios can do.

Frank Duignan informed me this will reduce the range, and range has been an issue,
so using them at various baud rates should be looked into, although they
will be slower.