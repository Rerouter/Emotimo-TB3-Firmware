# Emotimo-TB3-Firmware
My work on improving the firmware from the last release, I'm not opposed to easy enough requests, 

Changes so far:
- Currently broken the X point moves.... working on it, my usual work is panoramas so took a while to notice
- Standardised all Button and joystick handling
- Standardised min-max in user controlled settings and made options roll over instead of constraining at the ends
- Rescoped a lot of variables that didnt need to be global, and moved globals that didnt need to be user accessed
- Started work on reducing most of the floating point math that did not need to be floating point.
- Made all functions use the same set max speed limits, allowing the user to instead control.
- Started trying to shift code to files where it makes sense for them to live

Nunchuck Related:
- Wrapped the button handling up into its own function to make behaviour consistant
- Cleaned the joystick handling into a smaller set of functions, and made user control follow the speed limits
- Added deadband to all joystick handling, and cleaned up the exponential functions
- Made it that if the nunchuck disconnects while user is controlling motion, it will stop moving
- Stopped hammering on the nunchuck faster than it can actually update, greatly reducing read errors.

LCD Related:
- Increased LCD baud rate up to 57600 instead of the original 9600
- Standardised all commands issued to the lcd
- Fixed some LCD text offsets / typos

Camera Related:
- Wrapped the camera up into self contained functions so taking photos in functions is simplified
- Laid the groundwork for a minimum focus time before taking a photo, to make the time an image is taken more consistant
- Started working on external trigger modes in all modes, my hope being to connect the trigger to the camera flash shoe pin, and only update after the camera has completed a shot, e.g. if a focus takes longer than expected.

Panorama Related:
- Improved accuracy of time estimates significantly, now should be within 5% as it does try and account for move time, not just image time
- Extended maximum exposure time out to 1 hour, for astrophotography related things, in reality 15 minutes would be enough, but why limit 
- Built a proper pause menu, you can now skip forward or backward to any image, change the exposure time, and hopefully more soon.
