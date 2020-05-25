The GPS module will probably not have recption inside,
it will probably have to be on a window sill or outside to 
get recption.

A library was used to parse the GPS NMEA messages,
and this library was a bit finicky to set up.
It is included here with the changes made to it. It needs to
be put in the libraries folder in the Arduino folder on your
computer to use it.

While it is painful to read through libraries documentation,
it is definitely less pain than trying to write original code
to parse the GPS messages as efficiently or as well. No point 
wasting time re-inventing the wheel. One of the recommendations
was to use AltSoftSerial instead of softSerial.

For the Uranus 622f, despite what the datasheet says,
(29 sec cold start) it almost always takes me 
two minutes in an open space to attain a GPS signal.