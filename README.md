# Ground-Control-Station-Tracking-sys
Ground Control Station Tracking system :

Arduino will be used in the control system to track CubeSat for efficient data
receiving. So, this code aims to develop a ground station control system.
This code is uploaded to an Arduino for accurately tracking  satellites by pointing an antenna in the direction of the satellite. 
Communication with the satellite is outside the scope of this system. However, this project's focus was on creating a
method for tracking satellites using different modes, such as real time, built-in tables, and manualmode.
In built in table mode we use a pointing table, which contains azimuth, elevation angles,
and time, to be used to adjust the antenna position for upcoming CubeSat passes. The
GCS will track the CubeSat during the pass time, return to its standby position, and then
prepare for the next communication session.
In real time tracking, the system outlined in this project uses the Orbitron program to
calculate the real-time positions of low-Earth-orbit (LEO) satellites. These generated locations are used to control the orientation of a helical directional antenna that can be used
to communicate with and receive data from NGS.
Manual mode forces motors to point the antenna manually towards the Satellite using a Joystick.

