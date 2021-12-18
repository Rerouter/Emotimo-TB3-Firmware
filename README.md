# Emotimo-TB3-Firmware
My work on improving the firmware from the last release, I'm not opposed to easy enough requests, 

This is not the same as the last proper release, as I only thought to move to git a good way in to working on it

So far most of the changes have been related to the pano modes, button handling, joystick handling and tweaking things for astrophotography,

E.g. now if the joystick looses connection in a joystick controlled setup item, it will stop updating the motors after about 2.5 seconds, and will go to a safe default

Reworked all the button input handing to go through the ButtonHandler() where it wont register a button press until it first sees a released state, also removed a lot of the hacky ways this was originally being handled, similar for the joystick position constraining and deadband handling.

Pano mode, and potentially others now have much more accurate time predictions for how long a capture will take, its not perfect, but generally within about 3%

Rescoped a lot of things that didnt need to be global variables, reduced the data type sizes where it was not needed and added extra functionality to the EEPROM code to handle more of those reduced types, particuarly trying to remove any float math that did not need to be float math.

Started moving menu structures into there own functions, in the hope of simplifying the process of adding more as needed. e.g. I plan to build in a sky tracking mode for astrophotography, and possibly satellite / comet tracking.

Fixed up some LCD text typos / offset issues, 

Wrapped the camera stuff up into a clearner implementation

Planning to remake things so external input triggering can work in all modes (if static time is set to 0)
