# Emotimo-TB3-Firmware
My work on improving the firmware from the last release, I'm not opposed to easy enough requests, 

Rewrite Progress:

Panoramas class now handles Timelapses, Panoramas, Keyframe Moves and any combination of the above, so if your after a Keyframed Hyperlapse of Panoramas, I have you covered and would really love to know who you are :)
- Needs better documentation for plumbing as it has too much packed in that brick
- It needs the GUI parts to set up and run it fleshed out, including a pause menu
- As part of the GUI I want to do a dry run option where it will show you how it plans to move if there are keyframes or the corners of the panorama

LCD class talks via Bit-Bang Serial, its as non blocking as I can make it, you queue up a screen of content and check in when its done,
- Each function checks if there is room in the buffer to write before it does, its there more to catch any issues in defining the menu's where I did not wait long enoug
- I remade the clear functions, as printing 2x20 characters takes less time than the proper clear command interestingly,
- Every command the LCD supports is covered, everything is non blocking,

Motion Control, Took Multi/AccelStepper and converted the whole thing over to non blocking integer math and added better acceleration profiles
- It still needs polishing as it really could be shifted over to compile time sizing of the number of instances,
- the handle function might need to pass off things to the task schedualer as it does not really need to babysit them
- Should move the powersaving functionality into AccelStepper, to keep Multistepper as a glorified group of for loops
- It can now do proper linear acceleration and torque limited curves for steppers, it needs to be tested but should be very fast
- I don't like using SQRT in loops, but the approximation method is fast enough for most use cases, you can drop the approximation counter down to 8 if its really needed, 10 is better

Task Schedualer,
- It exists so that I don't need to hammer calling each libraries run / handle functions, when something is due to be called it will run,
- you pass in a libraries handle function and it will keep calling it until the timestamp returned is 0,
- It sorts the tasks as they are added, moving some of the computation cost so that the check that something is due to run is a single check per loop for arbitary many items
- Eventually I would want to hook it up to a hardware timer such that I'm not needing to hammer anything at all,

GUI
- I've got the core fleshed out, its structure definition will not be that different from the older style, but it will be much more dense and easier to change things around on.
- Currently it falls into 3 cases, a Menu. Setting or Jogging, where if you want to just call a function you point it at the function to call.
- It needs the old structure migrated across and plumbed into the different classes,

Nunchuck
- Its been fleshed out, but it needs further simplification for the button logic, enums in the GUI code will be nice, 
- I'm working on the button states being released, pressed or held, which gives quite a few possible combinations to hopefully simplify some things,
- Joystick direction code is about as good as I can get it, Accelerometer code now can get pitch / roll, less sure how I want to use these
- Does proper drift correction and the deadband is correcly approached so that there is no sharp knee when you cross it.

Camera class handles the camera related stuff, its effectivly just a state machine
- I have connected up the external trigger code in here, but needs some thinking on plumbing
- e.g. if you hooked the trigger signal up to the flash shoe to have things iterate forward as fast as the camera allows and would lead to pausing on focus fail

What else:
- Floating point is gone apart from 1 place
- Array accesses now use sizeof() to make sure things are hard to take out of bounds
- There are very few if any globals at present, and I will likely need to slice up the EEPROM to store a settings item per class
- The EEPROM will likely end up a struct of structs to keep reading and writing fairly clean as things shift about
- It easily fits on an uno now, its night and day in terms of program space and RAM requirements, but the GUI will chew back some of the program space
- I need to re-add the dragonFrame serial protocol, I'm also planning to make it togglable for LX200 protocol for widefeild astrophotography
