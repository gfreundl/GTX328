**GTX328 Simulation for X-Plane**

This is a simulation of the ubiqitous Garmin GTX 327,328,330 transponder, found in many of todays GA airplanes.
Functions are closely modelled to the original instrument, such as IDENT button, configurable VFR Code, various timers, 
temperature and density altitude as well as altitude monitor and aural alerts.
The display has night/day mode, brightness is adjustable from X-Planes lighting system, X-Planes ambient light, 
manually or even from a hardware photo cell (for advanced cockpit builders)
Configuration that in the real thing would be performed in the configuration menu, is partially available in a config file.

For proper use of this transponder, please refer to the original Garmin GTX 328 Transponder Pilot's Guide, available anywhere on the Internet.

**Technical notes**

Install by unzipping GTX328 directory into plugins folder, 
Instrument can be moved around on the panel by dragging in the upper bezel area
Size of the instrument can be configured in the ini file. 
When resizing, make sure that your aspect ratio (width/height) approximates the value 3.8 (original size 159mm/42mm)

Plugin has been tested on X-Plane 9.40 to 11.32, and is available as 64bit only. Platform is Windows only.
For running under versions below 10.20 openal32.dll needs to be in the plugin directory. This DLL is available everywhere on the net.
For anyone interested in compiling a Mac or Linux version, full sourcecode is included (plain C code). OpenAL, libpng and zlib are required. 

Disclaimer:
This software is only for recreational purposes. It is not intended to fully simulate every function of the original device.
It is not endorsed by the original manufacturer.


Configuration parameters stored in GTX328.ini:
Left				screen coordinates define position and instrument size
Right				screen coordinates define position and instrument size
Bottom				screen coordinates define position and instrument size
Top					screen coordinates define position and instrument size
VFR Code			local authority VFR Code
OAT option			0 or 1
°F / °C				0 = Fahrenheit, 1 = Celsius
AutoFlightTimer		0 = Man, 1 = Clear at every lift off, 2 = Accum counting after lift off
Display				0 = Auto, 1 = black background, 2 = light background
Backlight			0 = manual, 1 = X-Plane lighting bus, 2 = X-Plane ambient, 3 = hardware photo cell
AlertSound			0 = bell, 1 = male voice, 2 = female voice
Bezel				0 = don't show bezel
AudioDevice			Display Name from Device Manager
Voltage				12 or 24 volts

As per now, these features are not implemented:
- FLT ID setup (can be done from within X-Plane)
- maintenance configuration modes (some are available through the config file)
- ADS-B	(see no point in that for a sim)
- Contrast Dimmer
- flight time accumulate mode 
- metric altitudes


Open Issues:
- OAT sketchy
- switch to GND on landing, default delay 24sec
- FL show "___" in GND and STBY after some time
- FLT TIME saved on reset (for a limited time)
- FLT TIME can be paused and CLRed in flight
- FLT TIME pauses on GND detect
- Count Down Timer
	- switch over to Count Up at 00:00:00 with "EXPIRED" independent from CountDn Timer
	- invert current character on enter
- Altitude Monitor should show deviation in tens, not hundreds
- ALT MON alert threshold configurable
- contrast configurable
- arrow thresholds: small up/dn even with straight&level, large at around 200fpm, lagging

Notes: 
bezel size in px for display size 198x33:  431x114, 680x179 for aspect ratio 3.8 (original device size 159mm/42mm)
