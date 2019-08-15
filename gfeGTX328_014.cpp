/********************************************************************************************************************************

Version

001		copy template from EDM800
002		implement PNG loader, create textures
003		code Edit routine
004		Edit Mode finished, Flight Timer
005		OAT mode, display invert, redo textures, manual dim
006		resize bezel, scalable
007		panel light control option via hardware using Datarefs
008		requires sufficient battery voltage; OAT Celsius and Fahrenheit
009		implement OpenAL for alert sounds, only supports 10.20 or higher anymore
010		various fixes, FL trend arrows, Count Down timer
011		male and female alert voices, Bezel on/off, texture offset as float to support upscaling
012		always create own OpenAL context; make audio device selectable in INI file
013		battery voltage selectable 12/24 Volts



Manual:
This is a simulation of the ubiqitous Garmin GTX 327,328,330 transponder, found in many of todays GA airplanes.
Functions are closely modelled to the original instrument, such as IDENT button, configurable VFR Code, various timers, 
temperature and density altitude as well as altitude monitor and aural alerts.
The display has night/day mode, brightness is adjustable from X-Planes lighting system, X-Planes ambient light, 
manually or even from a hardware photo cell (for advanced cockpit builders)
Configuration that in the real thing would be performed in the configuration menu, is partially available in a config file.

For proper use of this transponder, please refer to the original Garmin GTX 328 Transponder Pilot's Guide, available anywhere on the Internet.

Technical notes:
Install by unzipping into plugins folder, note that only gfeGTX328.xpl should sit in plugins folder, the GTX328 subfolder should be underneath  
Instrument can be moved around on the panel by dragging in the upper bezel area
Size of the instrument can be configured in the ini file. 
When resizing, make sure that your aspect ratio (width/height) approximates the value 3.8 (original size 159mm/42mm)

Plugin has been tested on X-Plane 9.40 to 10.45, and is currently available as 32bit only. Platform is Windows only.
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


******************************************************************************************************************************************/
#define XPLM200															//required for XPLMFindCommand
																		//runs only on X-Plane 9 or higher
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include "BitmapSupport.h"												//Bitmap Loader, obsolete
#include "XPLMProcessing.h"												//Timed Processing
#include <windows.h>													//required for GL
#include <gl\GL.h>
#include <gl\GLU.h>
#include "png.h"														//PNG routines, is not found when Configuration "Debug"
#include "zlib.h"														//PNG compression lib
#include <al.h>															//OpenAL
#include <alc.h>

/// Texture stuff
#define BEZEL_FILENAME			"gtx_bezel.png"							//Bezel
#define DISPLAY_FILENAME		"gtx_display.png"						//display background
#define INIT_FILENAME			"gtx_init.png"							//Init Screen
#define RCV_FILENAME			"gtx_receive.png"						//reception light
#define ALT_FILENAME			"gtx_ALT.png"							//
#define GND_FILENAME			"gtx_GND.png"							//
#define STBY_FILENAME			"gtx_STBY.png"							//
#define FL_FILENAME				"gtx_FL.png"							//
#define ___LFILENAME			"gtx__l.png"							//inverted "-"
#define FT_FILENAME				"gtx_ft.png"							//ft symbol for Altitude Monitor
#define _0_MFILENAME			"gtx_0m.png"							//numbers medium size
#define _1_MFILENAME			"gtx_1m.png"
#define _2_MFILENAME			"gtx_2m.png"
#define _3_MFILENAME			"gtx_3m.png"
#define _4_MFILENAME			"gtx_4m.png"
#define _5_MFILENAME			"gtx_5m.png"
#define _6_MFILENAME			"gtx_6m.png"
#define _7_MFILENAME			"gtx_7m.png"
#define _8_MFILENAME			"gtx_8m.png"
#define _9_MFILENAME			"gtx_9m.png"
#define _0_LFILENAME			"gtx_0l.png"							//numbers large size
#define _1_LFILENAME			"gtx_1l.png"
#define _2_LFILENAME			"gtx_2l.png"
#define _3_LFILENAME			"gtx_3l.png"
#define _4_LFILENAME			"gtx_4l.png"
#define _5_LFILENAME			"gtx_5l.png"
#define _6_LFILENAME			"gtx_6l.png"
#define _7_LFILENAME			"gtx_7l.png"
#define _0_IFILENAME			"gtx_0i.png"							//numbers large size inverted
#define _1_IFILENAME			"gtx_1i.png"
#define _2_IFILENAME			"gtx_2i.png"
#define _3_IFILENAME			"gtx_3i.png"
#define _4_IFILENAME			"gtx_4i.png"
#define _5_IFILENAME			"gtx_5i.png"
#define _6_IFILENAME			"gtx_6i.png"
#define _7_IFILENAME			"gtx_7i.png"
#define _8_IFILENAME			"gtx_8i.png"
#define _9_IFILENAME			"gtx_9i.png"
#define _A_SFILENAME			"gtx_as.png" 							//A-Z small size characters
#define _B_SFILENAME			"gtx_bs.png" 
#define _C_SFILENAME			"gtx_cs.png" 	
#define _D_SFILENAME			"gtx_ds.png" 
#define _E_SFILENAME			"gtx_es.png" 
#define _F_SFILENAME			"gtx_fs.png" 
#define _G_SFILENAME			"gtx_gs.png" 
#define _H_SFILENAME			"gtx_hs.png" 
#define _I_SFILENAME			"gtx_is.png" 
#define _K_SFILENAME			"gtx_ks.png" 
#define _L_SFILENAME			"gtx_ls.png" 
#define _M_SFILENAME			"gtx_ms.png" 
#define _N_SFILENAME			"gtx_ns.png" 
#define _O_SFILENAME			"gtx_os.png" 
#define _P_SFILENAME			"gtx_ps.png" 
#define _Q_SFILENAME			"gtx_qs.png" 
#define _R_SFILENAME			"gtx_rs.png" 
#define _S_SFILENAME			"gtx_ss.png" 
#define _T_SFILENAME			"gtx_ts.png" 
#define _U_SFILENAME			"gtx_us.png" 
#define _V_SFILENAME			"gtx_vs.png" 
#define _W_SFILENAME			"gtx_ws.png" 
#define _X_SFILENAME			"gtx_xs.png" 
#define _Y_SFILENAME			"gtx_ys.png" 
#define _Z_SFILENAME			"gtx_zs.png" 
#define ___SFILENAME			"gtx__s.png"
#define _A_WFILENAME			"gtx_aw.png" 							//A-Z wide size characters
#define _B_WFILENAME			"gtx_bw.png" 
#define _C_WFILENAME			"gtx_cw.png" 	
#define _D_WFILENAME			"gtx_dw.png" 
#define _E_WFILENAME			"gtx_ew.png" 
#define _F_WFILENAME			"gtx_fw.png" 
#define _G_WFILENAME			"gtx_gw.png" 
#define _H_WFILENAME			"gtx_hw.png" 
#define _I_WFILENAME			"gtx_iw.png" 
#define _K_WFILENAME			"gtx_kw.png" 
#define _L_WFILENAME			"gtx_lw.png" 
#define _M_WFILENAME			"gtx_mw.png" 
#define _N_WFILENAME			"gtx_nw.png" 
#define _O_WFILENAME			"gtx_ow.png" 
#define _P_WFILENAME			"gtx_pw.png" 
#define _Q_WFILENAME			"gtx_qw.png" 
#define _R_WFILENAME			"gtx_rw.png" 
#define _S_WFILENAME			"gtx_sw.png" 
#define _T_WFILENAME			"gtx_tw.png" 
#define _U_WFILENAME			"gtx_uw.png" 
#define _V_WFILENAME			"gtx_vw.png" 
#define _W_WFILENAME			"gtx_ww.png" 
#define _X_WFILENAME			"gtx_xw.png" 
#define _Y_WFILENAME			"gtx_yw.png" 
#define _Z_WFILENAME			"gtx_zw.png" 
#define colMFILENAME			"gtx_col.png"
#define OFF_FILENAME			"gtx_OFF.png"
#define	frameFILENAME			"gtx_frame.png"
#define dashFILENAME			"gtx_dash.png"
#define posFILENAME				"gtx_pos.png"
#define negFILENAME				"gtx_neg.png"
#define POWERDOWN_FILENAME		"gtx_powerdown.png"						//Power Down
#define ON_FILENAME				"gtx_ON.png"							//Mode ON (Mode A)
#define UP_S_FILENAME			"gtx_up_s.png"							//FL Trend arrows
#define UP_L_FILENAME			"gtx_up_l.png"
#define DOWN_S_FILENAME			"gtx_down_s.png"
#define DOWN_L_FILENAME			"gtx_down_l.png"

#define BEZEL_TEXTURE	1
#define DISPLAY_TEXTURE 2
#define INIT_TEXTURE	3
#define RCV_TEXTURE	4
#define ALT_TEXTURE	5
#define GND_TEXTURE	6
#define STBY_TEXTURE 7	
#define FL_TEXTURE	8
#define ___LTEXTURE 9													//inverted "-" for edit mode
#define _0_MTEXTURE 10													//numbers medium size for PRESSURE ALT and Timers
#define _1_MTEXTURE 11
#define _2_MTEXTURE 12
#define _3_MTEXTURE 13
#define _4_MTEXTURE 14
#define _5_MTEXTURE 15
#define _6_MTEXTURE 16
#define _7_MTEXTURE 17
#define _8_MTEXTURE 18
#define _9_MTEXTURE 19
#define _0_LTEXTURE 20													//digits large size for Transponder Code
#define _1_LTEXTURE 21
#define _2_LTEXTURE 22
#define _3_LTEXTURE 23
#define _4_LTEXTURE 24
#define _5_LTEXTURE 25
#define _6_LTEXTURE 26
#define _7_LTEXTURE 27
#define _8_LTEXTURE 28													//no actual textures but required for DrawCode() function
#define _9_LTEXTURE 29													//no actual textures but required for DrawCode() function
#define _0_ITEXTURE 30													//numbers large size inverted
#define _1_ITEXTURE 31
#define _2_ITEXTURE 32
#define _3_ITEXTURE 33
#define _4_ITEXTURE 34
#define _5_ITEXTURE 35
#define _6_ITEXTURE 36
#define _7_ITEXTURE 37
#define _8_ITEXTURE 38
#define _9_ITEXTURE 39
#define _A_STEXTURE 50													//A-Z small size characters
#define _B_STEXTURE 51
#define _C_STEXTURE 52	
#define _D_STEXTURE 53
#define _E_STEXTURE 54
#define _F_STEXTURE 55
#define _G_STEXTURE 56
#define _H_STEXTURE 57
#define _I_STEXTURE 58
#define _K_STEXTURE 59
#define _L_STEXTURE 60
#define _M_STEXTURE 61
#define _N_STEXTURE 62
#define _O_STEXTURE 63
#define _P_STEXTURE 64
#define _Q_STEXTURE 65
#define _R_STEXTURE 66
#define _S_STEXTURE 67
#define _T_STEXTURE 68
#define _U_STEXTURE 69
#define _V_STEXTURE 70
#define _W_STEXTURE 71
#define _X_STEXTURE 72
#define _Y_STEXTURE 73
#define _Z_STEXTURE 74
#define ___STEXTURE 75
#define _A_WTEXTURE 80													//A-Z wide size characters
#define _B_WTEXTURE 81
#define _C_WTEXTURE 82	
#define _D_WTEXTURE 83
#define _E_WTEXTURE 84
#define _F_WTEXTURE 85
#define _G_WTEXTURE 86
#define _H_WTEXTURE 87
#define _I_WTEXTURE 88
#define _K_WTEXTURE 89
#define _L_WTEXTURE 80
#define _M_WTEXTURE 81
#define _N_WTEXTURE 82
#define _O_WTEXTURE 83
#define _P_WTEXTURE 84
#define _Q_WTEXTURE 85
#define _R_WTEXTURE 86
#define _S_WTEXTURE 87
#define _T_WTEXTURE 88
#define _U_WTEXTURE 89
#define _V_WTEXTURE 80
#define _W_WTEXTURE 81
#define _X_WTEXTURE 82
#define _Y_WTEXTURE 83
#define _Z_WTEXTURE 84
#define colMTEXTURE 85													//colon for Timers
#define colSTEXTURE 95													//dummy
#define FT_TEXTURE	96
#define OFF_TEXTURE 97
#define frameTEXTURE 98													//Display and Contrast modes
#define dashTEXTURE 99													//Display and Contrast modes
#define ___TEXTURE 100
#define posTEXTURE 101													//Celsius positive
#define negTEXTURE 102		
#define POWERDOWN_TEXTURE 103											//Power Down
#define ON_TEXTURE 104													//Mode ON (Mode A)
#define UP_S_TEXTURE 105												//FL Trend arrows
#define UP_L_TEXTURE 106
#define DOWN_S_TEXTURE 107
#define DOWN_L_TEXTURE 108

#define MAX_TEXTURES 200

//Operational Modes and Menu Items
#define M_OFF			0
#define M_ON			1												//Mode A
#define M_TEST			2												//startup selftest
#define M_GND			4												//Ground detected on landing
#define M_ALT			8												//Mode C
#define M_STBY			16												//Mode Standby

#define F_PA			1
#define F_FLTTIME		2
#define	F_ALTMON		3
#define	F_OAT			4
#define F_COUNTUP		5
#define F_COUNTDN		6
#define F_DISPLAY		7

#define R	0.28														//colors for GREEN display backlight and characters
#define G	0.72														
#define B	0.10
#define Rb	0.1															//colors for BLACK display backlight and characters 
#define Gb	0.1															//should be somewhere between 0.1 and 0.4
#define Bb	0.1



#define A_ALTMON_M	"gtx_altalert_m.wav"								//male voice "leaving altitude"
#define A_TIMEXP_M	"gtx_timeexp_m.wav"									//male voice: "timer expired"
#define A_ALTMON_F	"gtx_altalert_f.wav"								//female voice "leaving altitude"
#define A_TIMEXP_F	"gtx_timeexp_f.wav"									//female voice: "timer expired"
#define A_ALERT		"gtx_alert.wav"										//bell alert sound

XPLMDataRef	RED = NULL, GREEN = NULL, BLUE = NULL;
XPLMDataRef dFlightTime = NULL;
XPLMDataRef	dBattOn = NULL;
XPLMDataRef	dAvnOn = NULL;
XPLMDataRef dRunTime = NULL;
XPLMDataRef dBat = NULL;
XPLMDataRef dOAT = NULL;
XPLMDataRef dCode = NULL;
XPLMDataRef dIdent = NULL;
XPLMDataRef dLit = NULL;
XPLMDataRef dMode = NULL;
XPLMDataRef dFail = NULL;
XPLMDataRef dBaro = NULL;
XPLMDataRef dBaroAlt = NULL;
XPLMDataRef dOnGround = NULL;
XPLMDataRef dAltitude = NULL;
XPLMDataRef dInstrumentDim = NULL;

XPLMCommandRef dIdentEnter = NULL;										//X-Plane command for transponder Ident
XPLMCommandRef gfe_gtx_Key0 = NULL;
XPLMCommandRef gfe_gtx_Key1 = NULL;
XPLMCommandRef gfe_gtx_Key2 = NULL;
XPLMCommandRef gfe_gtx_Key3 = NULL;
XPLMCommandRef gfe_gtx_Key4 = NULL;
XPLMCommandRef gfe_gtx_Key5 = NULL;
XPLMCommandRef gfe_gtx_Key6 = NULL;
XPLMCommandRef gfe_gtx_Key7 = NULL;
XPLMCommandRef gfe_gtx_Key8 = NULL;
XPLMCommandRef gfe_gtx_Key9 = NULL;
XPLMCommandRef gfe_gtx_KeyIDENT = NULL;
XPLMCommandRef gfe_gtx_KeyVFR = NULL;
XPLMCommandRef gfe_gtx_KeyON = NULL;
XPLMCommandRef gfe_gtx_KeyALT = NULL;
XPLMCommandRef gfe_gtx_KeySTBY = NULL;
XPLMCommandRef gfe_gtx_KeyOFF = NULL;
XPLMCommandRef gfe_gtx_KeyFUNC = NULL;
XPLMCommandRef gfe_gtx_KeyCRSR = NULL;
XPLMCommandRef gfe_gtx_KeySTART = NULL;
XPLMCommandRef gfe_gtx_KeyCLR = NULL;

XPLMTextureID gTexture[MAX_TEXTURES];
XPLMWindowID ggfePanelDisplayWindow = NULL;
XPLMPluginID gPlugin;

XPLMHotKeyID gHotKey = NULL;
XPLMHotKeyID gInvKey = NULL;
XPLMHotKeyID gKey0 = NULL;
XPLMHotKeyID gKey1 = NULL;
XPLMHotKeyID gKey2 = NULL;
XPLMHotKeyID gKey3 = NULL;
XPLMHotKeyID gKey4 = NULL;
XPLMHotKeyID gKey5 = NULL;
XPLMHotKeyID gKey6 = NULL;
XPLMHotKeyID gKey7 = NULL;
XPLMHotKeyID gKey8 = NULL;
XPLMHotKeyID gKey9 = NULL;
XPLMHotKeyID gKeyIDENT = NULL;
XPLMHotKeyID gKeyVFR = NULL;
XPLMHotKeyID gKeyON = NULL;
XPLMHotKeyID gKeyALT = NULL;
XPLMHotKeyID gKeySTBY = NULL;
XPLMHotKeyID gKeyOFF = NULL;
XPLMHotKeyID gKeyFUNC = NULL;
XPLMHotKeyID gKeyCRSR = NULL;
XPLMHotKeyID gKeySTART = NULL;
XPLMHotKeyID gKeyCLR = NULL;

GLfloat glwidth;							//texture width 
GLfloat glheight;							//texture height

int gfeDisplayPanelWindow = 1;				//show panel from start
char gPluginDataFile[255];					//file path and name

char Symbol[4];								//content for LED
int FSMode;									//active Operation Mode we're in
int FSModeStart = 0;						//the Mode that was switched with startup (STBY, ON, ALT)
int FSFunc;									//active Function FUNC mode we're in 
int nTick;									//Counter for Init Mode in Flightloop 
int nTickEdit;								//Counter for Timeout for Edit Mode in Flightloop
int KeyOFFTick = 0;							//Counter for KeyOFF screen
int gaugeX1;								//gauge position bottom left
int gaugeX2;								//gauge position bottom right
int gaugeY1;								//gauge position top left
int gaugeY2;								//gauge position top right
int FirstCall = TRUE;						//first call since plugin start or power recycle
int BattOn;									//X-Plane electrical current on Bus 1
int AvnOn;									//X-Plane electrical current on Avionics
int AutoFlightTimer = 0;					//0 = manual, 1 = clear at every lift off, 2 = accum after lift off
int FlightTime = 0;							//actual time in the air
int RunTime = 0;							//X-Plane elapsed time
int StartTime = 0;							//time at lift off
int CountUp = 0;							//Count Up Timer
int CountUpStart = 0;						//Count Up Timer Start
int CountUpPause = 0;						//is paused = 1
int CountDn = 0;							//Count Down Timer
int CountDnEnd = 0;							//Count Down Timer Target
int CountDnFlag = 0;						//Count Down Target reached
int CountDnPause = 0;						//is paused = 1
int CountDnCursor = -1;						//position when entering Count Down start value
int Code = 0;								//X-Plane Transponder Code
int VFRCode = 0;							//VFR Code (configurable)
int prevCode = 0;							//previous Code before VFR switch was activated or when Edit aborted
char cCode[5];								//Transponder Code			
int Ident = 0;								//X-Plane Transponder Ident on/off
int Lit = 0;								//X-Plane Transponder reply lamp lit
int Mode = 0;								//X-Plane Transponder Mode
int Fail = 0;								//X-Plane Transponder Fail
float fBat[8];								//X-Plane bus volts
int OAT = 0;								//do we have OAT option ?
int FC = 0;									//OAT unit:	0 = Fahrenheit, 1 = Celsius
float fOAT;									//outside air temperature °C
float fBaro = 0;							//X-Plane current sealevel Baro pressure [inHg]
float fBaroAlt = 0;							//X-Plane current indicated Baro altitude [ft]
float fAltitude = 0;						//X-Plane current elevation above MSL [meter]
float fPrevAltitude = 0;					//previous Altitude for finding trend
int FL = 0;									//calculated flight level
int MonFL = 0;								//monitored FL from Alt Monitor function
int Deviation = 0;							//Alt Monitor deviation
int AltAlert = 0;							//Alt Monitor alert active
int ShowAltDev = 0;							//every nTick flash alert 
int OnGround = 0;							//X-Plane airplane is on ground
int Squat = 0;								//Squat switch (for ground detection) (on ground = 1)
int SquatFlag = 0;							//previous state of squat switch (flange detection)
int iCursor = -1;							//cursor active (>0) and position (1-4)
char title[13];								//function titles
char cFL[4];								//displayed Flight Level
int iFLArrow;								//up/down arrows in Flight Leve Mode
int Display;								//0=Auto (ambient light), 1=night (black background), 2=day, 3=hardware
int InvertDisp = 1;							//inverted display (1 = night mode)
float InstrumentDim[16];					//X-Plane instrument brightneas, 0= X-Plane, 14=gfe hardware photo cell
float Red = 0;								//X-Plane ambient light
float Green = 0;
float Blue = 0;
float DimR = R;								//start with full brightness
float DimG = G;
float DimB = B;
int Backlight = 0;							//0=manual, 1=X-Plane lighting bus, 2=X-Plane ambient, 3=hardware photo cell
int Func = 1;								//1=Pressure Alt, 2=Flight Time, 3=Alt Monitor, 4=Count up, 5=dn, 
											//6=OAT, 7=Display
int AlertSound = 0;							//0=bell, 1=male, 2=female voice
int Bezel = 1;								//show bezel
int Voltage = 0;							//Aircraft battery voltage

int i;										//free Counter, can be used anywhere
int n;										//Counter
int a;										//Counter
int b ;										//reserved Counter
int c;
int k = 1;									//Counter for EGT temp filtering
int u = 0;

float PanelL, PanelR, PanelB, PanelT;
float DispL;								//Display position
float DispB;
float Scaler;								//scale factor for resizing window
float CodeL;								//Transporter Code position

ALuint			snd_src[3];					// Sample source and buffer - this is one "sound" we play.
ALuint			snd_buffer[3];
float			pitch		= 1.0f;			// Start with 1.0 pitch - no pitch shift.
ALCdevice *my_dev	= NULL;					// We make our own device and context to play sound through.
ALCcontext *my_ctx	= NULL;
ALCcontext *old_ctx = NULL;					// have to save old AL context
char sAudioDevice[255];						// read device name from INI File

////////////////////////////////////////// PROTOTYPES //////////////////////////////////////////

/// Prototypes for callbacks etc.
GLvoid LoadTextures(GLvoid);
GLuint LoadPNGTexture(const char *pFileName, int TextureId, int *width, int *height);
int GetTextureID(char);
int DrawGLScene(float x, float y);
int gfe_gtx_Key0_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key1_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key2_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key3_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key4_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key5_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key6_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key7_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key8_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_Key9_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyIDENT_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyVFR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyON_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyALT_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeySTBY_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyOFF_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyFUNC_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyCRSR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeySTART_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
int gfe_gtx_KeyCLR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon);
void HotKey(void * refCon);
void InvKey(void * refCon);
void Key0(void * refCon);
void Key1(void * refCon);
void Key2(void * refCon);
void Key3(void * refCon);
void Key4(void * refCon);
void Key5(void * refCon);
void Key6(void * refCon);
void Key7(void * refCon);
void Key8(void * refCon);
void Key9(void * refCon);
void KeyIDENT(void * refCon);
void KeyVFR(void * refCon);
void KeyON(void * refCon);
void KeyALT(void * refCon);
void KeySTBY(void * refCon);
void KeyOFF_1(void * refCon);
void KeyOFF_2();
void KeyFUNC(void * refCon);
void KeyCRSR(void * refCon);
void KeySTART(void * refCon);
void KeyCLR(void * refCon);
void EditCode(int key);
void gfePanelWindowCallback(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon);    
void gfePanelKeyCallback(
                                   XPLMWindowID         inWindowID,    
                                   char                 inKey,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon,    
                                   int                  losingFocus);    
int gfePanelMouseClickCallback(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   XPLMMouseStatus      inMouse,    
                                   void *               inRefcon);    
float	MyFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon);    
int OpenINIFile();
int WriteINIFile();
static	int	CoordInRect(float x, float y, float l, float t, float r, float b);
int PaintTexture2(int texture, float w, float h, float x, float y);
void SwitchDisplay();
void DrawDisplay();
void DrawIdent();			
void DrawCode();
void DrawReply();
void DrawInit();
void DrawMode();
void DrawTitle();
void DrawFL();
int CalcFL();
int FLTrend();
void DrawFlightTime();
void DrawAltMonitor();
void DrawOAT();
void DrawCountUp();
void DrawCountDn();
void CountDnEnter(int key);
void DrawDim();
void DrawMText(float x, float y, char Text[20]);
//OpenAL
float init_sound(float elapsed, float elapsed_sim, int counter, void * ref);

/*
////////////////////////////////////////// XPLM REQUIRED FUNCTIONS //////////////////////////////////////////
*/
PLUGIN_API int XPluginStart(
				char *		outName,
				char *		outSig,
				char *		outDesc)
{
	char *pFileName = "Resources\\Plugins\\GTX328\\";

	/// Setup texture and ini file locations
	XPLMGetSystemPath(gPluginDataFile);
	strcat(gPluginDataFile, pFileName);

	strcpy(outName, "gfeGTX328");
	strcpy(outSig, "gfe.panel.gfeGTX328");
	strcpy(outDesc, "gfe Transponder");

	// Do deferred sound initialization. See http://www.xsquawkbox.net/xpsdk/mediawiki/DeferredInitialization
	XPLMRegisterFlightLoopCallback(init_sound,-1.0,NULL);	

	//Register FlightLoop Callback for every 250msec
	XPLMRegisterFlightLoopCallback(		
			MyFlightLoopCallback,	/* Callback */
			0.25,					/* Interval */
			NULL);					/* refcon not used. */

	//Open INI File and load values, or load default values instead
	if ( !OpenINIFile() )
	{
		gaugeX1 = 100;													//gauge position left
		gaugeX2 = 531;													//gauge position right
		gaugeY1 = 100;													//gauge position bottom
		gaugeY2 = 214;													//gauge position top
		VFRCode = 7000;
		Voltage = 12;
	}
	
	/// Create window, setup datarefs and register  hotkeys
	ggfePanelDisplayWindow = XPLMCreateWindow(gaugeX1, gaugeY2, gaugeX2, gaugeY1, 1, gfePanelWindowCallback, gfePanelKeyCallback, gfePanelMouseClickCallback, NULL);

	dBattOn = XPLMFindDataRef("sim/cockpit/electrical/battery_on"); 
	dAvnOn = XPLMFindDataRef("sim/cockpit/electrical/avionics_on"); 
	//dFlightTime = XPLMFindDataRef("sim/time/total_flight_time_sec");
	RED = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_r");
	GREEN = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_g");
	BLUE = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_b");
	dRunTime = XPLMFindDataRef("sim/time/total_running_time_sec");						//sim time
	dBat = XPLMFindDataRef("sim/cockpit2/electrical/battery_voltage_actual_volts");		//indicated volts (bus volts, not just battery!)
	dOAT = XPLMFindDataRef("sim/cockpit2/temperature/outside_air_temp_degc");
	dCode = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
	dIdent = XPLMFindDataRef("sim/cockpit/radios/transponder_id");
	dLit = XPLMFindDataRef("sim/cockpit/radios/transponder_light");
	dMode = XPLMFindDataRef("sim/cockpit/radios/transponder_mode");
	dFail = XPLMFindDataRef("sim/operation/failures/rel_xpndr");
	//dBaro = XPLMFindDataRef("sim/weather/barometer_current_inhg");
	dBaro = XPLMFindDataRef("sim/weather/barometer_sealevel_inhg");
	dAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
	dBaroAlt = XPLMFindDataRef("sim/flightmodel/misc/h_ind2");
	dOnGround = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
	dInstrumentDim = XPLMFindDataRef("sim/cockpit2/electrical/instrument_brightness_ratio");	//Array 
	dIdentEnter = XPLMFindCommand("sim/transponder/transponder_ident");

	//create custom commands
	gfe_gtx_Key0 = XPLMCreateCommand("gfe/GTX328/Key0", "GTX Key0");
	gfe_gtx_Key1 = XPLMCreateCommand("gfe/GTX328/Key1", "GTX Key1");
	gfe_gtx_Key2 = XPLMCreateCommand("gfe/GTX328/Key2", "GTX Key2");
	gfe_gtx_Key3 = XPLMCreateCommand("gfe/GTX328/Key3", "GTX Key3");
	gfe_gtx_Key4 = XPLMCreateCommand("gfe/GTX328/Key4", "GTX Key4");
	gfe_gtx_Key5 = XPLMCreateCommand("gfe/GTX328/Key5", "GTX Key5");
	gfe_gtx_Key6 = XPLMCreateCommand("gfe/GTX328/Key6", "GTX Key6");
	gfe_gtx_Key7 = XPLMCreateCommand("gfe/GTX328/Key7", "GTX Key7");
	gfe_gtx_Key8 = XPLMCreateCommand("gfe/GTX328/Key8", "GTX Key8");
	gfe_gtx_Key9 = XPLMCreateCommand("gfe/GTX328/Key9", "GTX Key9");
	gfe_gtx_KeyIDENT = XPLMCreateCommand("gfe/GTX328/IDENT", "GTX IDENT");
	gfe_gtx_KeyVFR = XPLMCreateCommand("gfe/GTX328/VFR", "GTX VFR");
	gfe_gtx_KeyON = XPLMCreateCommand("gfe/GTX328/ON", "GTX ON");
	gfe_gtx_KeyALT = XPLMCreateCommand("gfe/GTX328/ALT", "GTX ALT");
	gfe_gtx_KeySTBY = XPLMCreateCommand("gfe/GTX328/STBY", "GTX STBY");
	gfe_gtx_KeyOFF = XPLMCreateCommand("gfe/GTX328/OFF", "GTX OFF");
	gfe_gtx_KeyFUNC = XPLMCreateCommand("gfe/GTX328/FUNC", "GTX FUNC");
	gfe_gtx_KeyCRSR = XPLMCreateCommand("gfe/GTX328/CRSR", "GTX CRSR");
	gfe_gtx_KeySTART = XPLMCreateCommand("gfe/GTX328/START", "GTX START");
	gfe_gtx_KeyCLR = XPLMCreateCommand("gfe/GTX328/CLR", "GTX CLR");
	XPLMRegisterCommandHandler(gfe_gtx_Key0, gfe_gtx_Key0_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key1, gfe_gtx_Key1_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key2, gfe_gtx_Key2_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key3, gfe_gtx_Key3_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key4, gfe_gtx_Key4_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key5, gfe_gtx_Key5_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key6, gfe_gtx_Key6_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key7, gfe_gtx_Key7_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key8, gfe_gtx_Key8_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_Key9, gfe_gtx_Key9_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyIDENT, gfe_gtx_KeyIDENT_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyVFR, gfe_gtx_KeyVFR_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyON, gfe_gtx_KeyON_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyALT, gfe_gtx_KeyALT_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeySTBY, gfe_gtx_KeySTBY_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyOFF, gfe_gtx_KeyOFF_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyFUNC, gfe_gtx_KeyFUNC_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyCRSR, gfe_gtx_KeyCRSR_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeySTART, gfe_gtx_KeySTART_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_gtx_KeyCLR, gfe_gtx_KeyCLR_Proc, 1, (void*)0);
	//create  Hotkeys
	gHotKey = XPLMRegisterHotKey(XPLM_VK_F4, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-F4", HotKey, NULL);
	gInvKey = XPLMRegisterHotKey(XPLM_VK_F5, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-F5", InvKey, NULL);
	gKey0 = XPLMRegisterHotKey(XPLM_VK_0, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-0", Key0, NULL);
	gKey1 = XPLMRegisterHotKey(XPLM_VK_1, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-1", Key1, NULL);
	gKey2 = XPLMRegisterHotKey(XPLM_VK_2, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-2", Key2, NULL);
	gKey3 = XPLMRegisterHotKey(XPLM_VK_3, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-3", Key3, NULL);
	gKey4 = XPLMRegisterHotKey(XPLM_VK_4, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-4", Key4, NULL);
	gKey5 = XPLMRegisterHotKey(XPLM_VK_5, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-5", Key5, NULL);
	gKey6 = XPLMRegisterHotKey(XPLM_VK_6, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-6", Key6, NULL);
	gKey7 = XPLMRegisterHotKey(XPLM_VK_7, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-7", Key7, NULL);
	gKey8 = XPLMRegisterHotKey(XPLM_VK_8, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-8", Key8, NULL);
	gKey9 = XPLMRegisterHotKey(XPLM_VK_9, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-9", Key9, NULL);
	gKeyIDENT = XPLMRegisterHotKey(XPLM_VK_END, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-End", KeyIDENT, NULL);
	gKeyVFR = XPLMRegisterHotKey(XPLM_VK_HOME, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Home", KeyVFR, NULL);
	gKeyON = XPLMRegisterHotKey(XPLM_VK_LEFT, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Left", KeyON, NULL);
	gKeyALT = XPLMRegisterHotKey(XPLM_VK_UP, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Up", KeyALT, NULL);
	gKeySTBY = XPLMRegisterHotKey(XPLM_VK_RIGHT, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Right", KeySTBY, NULL);
	gKeyOFF = XPLMRegisterHotKey(XPLM_VK_DOWN, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Down", KeyOFF_1, NULL);
	gKeyFUNC = XPLMRegisterHotKey(XPLM_VK_DELETE, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Del", KeyFUNC, NULL);
	gKeyCRSR = XPLMRegisterHotKey(XPLM_VK_PRIOR, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-PgUp", KeyCRSR, NULL);
	gKeySTART = XPLMRegisterHotKey(XPLM_VK_NEXT, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-PgDn", KeySTART, NULL);
	gKeyCLR = XPLMRegisterHotKey(XPLM_VK_BACK, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Bksp", KeyCLR, NULL);
	
	// Load the textures and bind them etc.
	LoadTextures();

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	WriteINIFile();

	XPLMUnregisterCommandHandler(gfe_gtx_Key0, gfe_gtx_Key0_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key1, gfe_gtx_Key1_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key2, gfe_gtx_Key2_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key3, gfe_gtx_Key3_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key4, gfe_gtx_Key4_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key5, gfe_gtx_Key5_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key6, gfe_gtx_Key6_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key7, gfe_gtx_Key7_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key8, gfe_gtx_Key8_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_Key9, gfe_gtx_Key9_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyIDENT, gfe_gtx_KeyIDENT_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyVFR, gfe_gtx_KeyVFR_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyON, gfe_gtx_KeyON_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyALT, gfe_gtx_KeyALT_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeySTBY, gfe_gtx_KeySTBY_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyOFF, gfe_gtx_KeyOFF_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyFUNC, gfe_gtx_KeyFUNC_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyCRSR, gfe_gtx_KeyCRSR_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeySTART, gfe_gtx_KeySTART_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_gtx_KeyCLR, gfe_gtx_KeyCLR_Proc, 0, 0); 
	
	XPLMUnregisterHotKey(gHotKey);
	XPLMUnregisterHotKey(gInvKey);
	XPLMUnregisterHotKey(gKey0);
	XPLMUnregisterHotKey(gKey1);
	XPLMUnregisterHotKey(gKey2);
	XPLMUnregisterHotKey(gKey3);
	XPLMUnregisterHotKey(gKey4);
	XPLMUnregisterHotKey(gKey5);
	XPLMUnregisterHotKey(gKey6);
	XPLMUnregisterHotKey(gKey7);
	XPLMUnregisterHotKey(gKey8);
	XPLMUnregisterHotKey(gKey9);
	XPLMUnregisterHotKey(gKeyIDENT);
	XPLMUnregisterHotKey(gKeyVFR);
	XPLMUnregisterHotKey(gKeyON);
	XPLMUnregisterHotKey(gKeyALT);
	XPLMUnregisterHotKey(gKeySTBY);
	XPLMUnregisterHotKey(gKeyOFF);
	XPLMUnregisterHotKey(gKeyFUNC);
	XPLMUnregisterHotKey(gKeyCRSR);
	XPLMUnregisterHotKey(gKeySTART);
	XPLMUnregisterHotKey(gKeyCLR);

	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
	XPLMDestroyWindow(ggfePanelDisplayWindow);
	//Cleanup OpenAl stuff
	if(alcGetCurrentContext() != NULL)
	{
		for (i=0;i++;i<3)
		{
			if(snd_src[i])		alDeleteSources(1, &snd_src[i]);
			if(snd_buffer[i])	alDeleteBuffers(1, &snd_buffer[i]);
		}
	}
	if(my_ctx) 
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(my_ctx);
	}
	if(my_dev) alcCloseDevice(my_dev);
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
				XPLMPluginID	inFromWho,
				long			inMessage,
				void *			inParam)
{
}


void gfePanelWindowCallback(
                XPLMWindowID         inWindowID,    
                void *               inRefcon)
{
	int	PanelWindowLeft, PanelWindowRight, PanelWindowBottom, PanelWindowTop;
    float PanelWidth, PanelHeight;

	Red = XPLMGetDataf (RED);
	Green = XPLMGetDataf (GREEN);
	Blue = XPLMGetDataf (BLUE);

	/// Do the actual drawing, but only if our window is active
	if (gfeDisplayPanelWindow)
	{
		//Set up initializing
		BattOn = XPLMGetDatai(dBattOn);
		AvnOn = XPLMGetDatai(dAvnOn);
		if ((!AvnOn) || (!BattOn) || FSMode == M_OFF)
		{
			FirstCall = TRUE;											//reset at each power recycle
			FSMode = M_OFF;
		}
		if (FirstCall)													//reset on first Call since Plugin Start or Battery or Device on/off
		{
			OpenINIFile();													
			FirstCall = FALSE;
			nTick = 0;													//reset counter after restart
			b = 0;
			c = 0;
			CountDn = 30;												//can't be 0, would be EXPIRED right on startup, needs reset anyway on startup				
			CountDnFlag = 0;											//make sure we're not in CountDn End on startup
			CountUp = 0;												//reset CountUp on restart
			CountUpPause = 1;
			InvertDisp = 0;
			DimR = R;													//reset full brightness for display
			DimG = G;
			DimB = B;
			AltAlert = 0;
			ShowAltDev = 0;
			SquatFlag = -1;												//is undetermined before startup, find out when we switch on
		}
		else
		{
			nTick = nTick;												//just a dummy for debug
		}

		// Need to find out where our window is
		XPLMGetWindowGeometry(ggfePanelDisplayWindow, &PanelWindowLeft, &PanelWindowTop, &PanelWindowRight, &PanelWindowBottom);
		/// Tell Xplane what we are doing
		XPLMSetGraphicsState(0/*Fog*/, 1/*TexUnits*/, 0/*Lighting*/, 0/*AlphaTesting*/, 1/*AlphaBlending*/, 0/*DepthTesting*/, 0/*DepthWriting*/);
		/// Setup our panel and gauge relative to our window
		PanelL = PanelWindowLeft; PanelR = PanelWindowRight; PanelB = PanelWindowBottom; PanelT = PanelWindowTop;
		// Original Panel Size = 431x114, so in case of resize, scale everything accordingly
		Scaler = (PanelR-PanelL)/431;										

		// Row Coordinates
		DispL = PanelL + ((PanelR-PanelL) * 0.32);
		DispB = PanelB + ((PanelT-PanelB) * 0.53);
		CodeL = DispL + 40 * Scaler;		

		// Draw Bezel, handle day/night
		glColor3f(Red, Green, Blue);									//apply ambient light from X-Plane
		if (Bezel)		PaintTexture2(BEZEL_TEXTURE, 0, 0, PanelL, PanelB);

		//get actual data from XPL
		i = XPLMGetDatavf(dBat, fBat, 0, 8);
		fOAT = XPLMGetDataf(dOAT);
		Code = XPLMGetDatai(dCode);
		Ident = XPLMGetDatai(dIdent);
		Lit = XPLMGetDatai(dLit);
		Mode = XPLMGetDatai(dMode);
		Fail = XPLMGetDatai(dFail);
		fBaro = XPLMGetDataf(dBaro);
		fBaroAlt = XPLMGetDataf(dBaroAlt);
		fAltitude = XPLMGetDataf(dAltitude);
		OnGround = XPLMGetDatai(dOnGround);
		RunTime = XPLMGetDataf(dRunTime);
		i = XPLMGetDatavf(dInstrumentDim, InstrumentDim, 0, 16);		//item 0=X-Plane, item 14=gfe hardware photo cell

		FL = CalcFL();

		if ((AvnOn) && (fBat[0] > (Voltage-5)) && (BattOn) && (FSMode != M_OFF))	//required actual bat voltage minimum 7 or 19 Volts
		{
			if ((FSMode == M_TEST) && (nTick < 10))						//init page, first two sec full bright
			{
				DrawDisplay();											//display background
				DrawInit();												//init page 					
			}
			else if (FSMode == M_TEST)
			{
				SwitchDisplay();
				DrawDisplay();
				DrawInit();
			}
			else if (KeyOFFTick > 0)									//OFF pressed, display power down page
			{
				SwitchDisplay();
				DrawDisplay();
				PaintTexture2(POWERDOWN_TEXTURE, 0, 0, DispL, DispB);
			}
			else														//set background light
			{
				SwitchDisplay();
				DrawDisplay();

				if (OnGround)
				{
					if (SquatFlag == 0)									//was in the air, now landed
					{
						FSMode = M_GND;
					}
					SquatFlag = OnGround;
					/*
					if ((FSMode & M_GND) == 0)							//test for M_GND
					{
						FSMode = FSMode + M_GND;						//set M_GND flag
					}		
					FSMode = FSMode & (M_ALT ^ 2047);					//clear M_ALT flag
					*/
				}
				else
				{
					FSMode = FSMode & (M_GND ^ 2047);					//XOR with 0b1111111111 to invert M_GND and clear M_GND bit
					if (SquatFlag == 1)									//changed from ground to air?
					{
						StartTime = RunTime;							//start Timer
						FlightTime = 0;
						SquatFlag = 0;									//reset Flag for later touch down detection
						FSMode = FSMode | M_ALT;						//switch to ALT Mode
						Mode = 2;										//send new Mode to X-Plane
						XPLMSetDatai(dMode, Mode);
					}
					FlightTime = RunTime - StartTime;
					SquatFlag = 0;										//indicate that we were (still are) in the air
				}
				if (CountUpPause == 0)									//while not paused
				{
					CountUp = RunTime - CountUpStart;
				}
				if (CountDnEnd >= RunTime)								//Count Down routine down to zero
				{
					CountDn = CountDnEnd - RunTime;
				}
				//on end of Count Down, start separate Count up
				if ((CountDn == 0) && (CountDnFlag == 0) && (CountDnCursor < 0))	//don't reset while  entering new target time
				{
					CountDnFlag = 1;
					alcMakeContextCurrent(my_ctx);						//change into our AL context
					alSourcef(snd_src[0],AL_PITCH,pitch);				//play sound "timer expired"
					alSourcePlay(snd_src[0]);
					alcMakeContextCurrent(old_ctx);
				}

				if ((FSMode & M_GND) == M_GND)
				{
					PaintTexture2(GND_TEXTURE, 0, 0, DispL + 1*Scaler, DispB + 14*Scaler);
					Mode = 1;
					XPLMSetDatai(dMode, Mode);							//need to switch built in XPDR at least to STBY to make Ident timer work
				}
				else if (FSMode== M_ON)
				{
					PaintTexture2(ON_TEXTURE, 0, 0, DispL + 1*Scaler, DispB + 14*Scaler);
					Mode = 2;
					XPLMSetDatai(dMode, Mode);
				}
				else if ((FSMode & M_ALT) == M_ALT)
				{
					PaintTexture2(ALT_TEXTURE, 0, 0, DispL + 1*Scaler, DispB + 14*Scaler);
					Mode = 2;
					XPLMSetDatai(dMode, Mode);
				}
				else 
				{
					PaintTexture2(STBY_TEXTURE, 0, 0, DispL + 1*Scaler, DispB + 14*Scaler);
					Mode = 1;
					XPLMSetDatai(dMode, Mode);
				}

				//now draw everything
				DrawCode();
				DrawReply();
				if (Ident) 
				{ 
					DrawIdent(); 
				}			
				DrawTitle();

				//draw FUNC key Modes
				if (Func == F_PA)
				{
					DrawFL();
				}
				else if (Func == F_FLTTIME)
				{
					DrawFlightTime();
				}
				else if (Func == F_ALTMON)
				{
					DrawAltMonitor();
				}
				else if (Func == F_OAT)
				{
					DrawOAT();
				}
				else if (Func == F_COUNTUP)
				{
					DrawCountUp();
				}
				else if (Func == F_COUNTDN)
				{
					DrawCountDn();
				}
				else if (Func == F_DISPLAY)
				{
					DrawDim();
				}
				if (KeyOFFTick > 0)
				{
				}
			}
		}
	}
}


////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////


/*
 * Our key handling callback does nothing in this plugin.  This is ok; 
 * we simply don't use keyboard input.
 */
void gfePanelKeyCallback(
                XPLMWindowID         inWindowID,    
                char                 inKey,    
                XPLMKeyFlags         inFlags,    
                char                 inVirtualKey,    
                void *               inRefcon,    
                int                  losingFocus)
{
}                                   

/*
 * 
 * mouse click callback activates buttons and updates the position that the windows is dragged to
 * 
 */
int gfePanelMouseClickCallback(
                XPLMWindowID         inWindowID,    
                int                  x,    
                int                  y,    
                XPLMMouseStatus      inMouse,    
                void *               inRefcon)
{
	static	int	dX = 0, dY = 0;
	static	int	Weight = 0, Height = 0;
	int	Left, Top, Right, Bottom;

	static	int	gDragging = 0;

	if (!gfeDisplayPanelWindow)
		return 0;

	/// Get the windows current position
	XPLMGetWindowGeometry(inWindowID, &Left, &Top, &Right, &Bottom);

	switch(inMouse) 
	{
	case xplm_MouseDown:
		if (CoordInRect(x, y, Left, Top, Right, Top-15))						// test for the mouse anywhere in the top part of the window
		{		
			dX = x - Left;
			dY = y - Top;
			Weight = Right - Left;
			Height = Bottom - Top;
			gDragging = 1;
		}
		if (CoordInRect(x, y, DispL, DispL + 200*Scaler , DispB + 33*Scaler, DispB))	
		{
			InvKey(0);													//click anywhere into display to invert
		}
		if (CoordInRect(x, y, Left+(0.01*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.08*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key0(0);
		}
		if (CoordInRect(x, y, Left+(0.11*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.18*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key1(0);
		}
		if (CoordInRect(x, y, Left+(0.20*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.26*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key2(0);
		}
		if (CoordInRect(x, y, Left+(0.28*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.36*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key3(0);
		}
		if (CoordInRect(x, y, Left+(0.37*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.45*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key4(0);
		}
		if (CoordInRect(x, y, Left+(0.48*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.56*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key5(0);
		}
		if (CoordInRect(x, y, Left+(0.57*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.65*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key6(0);
		}
		if (CoordInRect(x, y, Left+(0.67*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.74*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key7(0);
		}
		if (CoordInRect(x, y, Left+(0.82*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.89*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key8(0);
		}
		if (CoordInRect(x, y, Left+(0.91*(Right-Left)), Top-(0.73*(Top-Bottom)), Left+(0.98*(Right-Left)), Top-(0.91*(Top-Bottom))))	
		{
			Key9(0);
		}
		if (CoordInRect(x, y, Left+(0.01*(Right-Left)), Top-(0.14*(Top-Bottom)), Left+(0.08*(Right-Left)), Top-(0.36*(Top-Bottom))))	
		{
			KeyIDENT(0);
		}
		if (CoordInRect(x, y, Left+(0.01*(Right-Left)), Top-(0.40*(Top-Bottom)), Left+(0.08*(Right-Left)), Top-(0.60*(Top-Bottom))))	
		{
			KeyVFR(0);
		}
		if (CoordInRect(x, y, Left+(0.13*(Right-Left)), Top-(0.08*(Top-Bottom)), Left+(0.20*(Right-Left)), Top-(0.22*(Top-Bottom))))	
		{
			KeyON(0);
		}
		if (CoordInRect(x, y, Left+(0.14*(Right-Left)), Top-(0.30*(Top-Bottom)), Left+(0.18*(Right-Left)), Top-(0.42*(Top-Bottom))))	
		{
			KeyALT(0);
		}
		if (CoordInRect(x, y, Left+(0.10*(Right-Left)), Top-(0.42*(Top-Bottom)), Left+(0.13*(Right-Left)), Top-(0.54*(Top-Bottom))))	
		{
			KeySTBY(0);
		}
		if (CoordInRect(x, y, Left+(0.20*(Right-Left)), Top-(0.43*(Top-Bottom)), Left+(0.22*(Right-Left)), Top-(0.55*(Top-Bottom))))	
		{
			KeyOFF_1(0);
		}
		if (CoordInRect(x, y, Left+(0.82*(Right-Left)), Top-(0.14*(Top-Bottom)), Left+(0.89*(Right-Left)), Top-(0.36*(Top-Bottom))))	
		{
			KeyFUNC(0);
		}
		if (CoordInRect(x, y, Left+(0.91*(Right-Left)), Top-(0.14*(Top-Bottom)), Left+(0.98*(Right-Left)), Top-(0.36*(Top-Bottom))))	
		{
			KeyCRSR(0);
		}
		if (CoordInRect(x, y, Left+(0.82*(Right-Left)), Top-(0.40*(Top-Bottom)), Left+(0.89*(Right-Left)), Top-(0.60*(Top-Bottom))))	
		{
			KeySTART(0);
		}
		if (CoordInRect(x, y, Left+(0.91*(Right-Left)), Top-(0.40*(Top-Bottom)), Left+(0.98*(Right-Left)), Top-(0.60*(Top-Bottom))))	
		{
			KeyCLR(0);
		}
		break;
	case xplm_MouseDrag:
		if (gDragging)															// We are dragging so update the window position
		{
			Left = (x - dX);
			Right = Left + Weight;
			Top = (y - dY);
			Bottom = Top + Height;
			XPLMSetWindowGeometry(inWindowID, Left, Top, Right, Bottom);
		}
		break;
	case xplm_MouseUp:
		gDragging = 0;
		break;
	}
	return 1;
}                     

void HotKey(void * refCon)							//toggle between display and non display
{
	gfeDisplayPanelWindow = !gfeDisplayPanelWindow;
}

void InvKey(void * refCon)							//invert display colors
{
	InvertDisp = !InvertDisp;
}

void Key0(void * refCon)							//why won't Keys 0 and 7 work? check on different machines!
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(0);
	}
	else
	{
		EditCode(0);
	}
}

void Key1(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(1);
	}
	else
	{
		EditCode(1);
	}
}

void Key2(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(2);
	}
	else
	{
		EditCode(2);
	}
}

void Key3(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(3);
	}
	else
	{
		EditCode(3);
	}
}

void Key4(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(4);
	}
	else
	{
		EditCode(4);
	}
}

void Key5(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(5);
	}
	else
	{
		EditCode(5);
	}
}

void Key6(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(6);
	}
	else
	{
		EditCode(6);
	}
}

void Key7(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(7);
	}
	else
	{
		EditCode(7);
	}
}

void Key8(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(8);
	}
	else
	{
		if (Func == F_DISPLAY)
		{
			DimR = DimR - 0.025*R;
			if (DimR < 0.25*R)
			{
				DimR = 0.2*R;
			}
			DimG = DimG - 0.025*G;
			if (DimG < 0.25*G)
			{
				DimG = 0.2*G;
			}
			DimB = DimB - 0.025*B;
			if (DimB < 0.25*B)
			{
				DimB = 0.2*B;
			}

		}
	}
}

void Key9(void * refCon)
{
	if ((Func == F_COUNTDN) && (CountDnCursor > -1))
	{
		CountDnEnter(9);
	}
	else
	{
		if (Func == F_DISPLAY)
		{
			DimR = DimR + 0.025*R;
			if (DimR > R)
			{
				DimR = R;
			}
			DimG = DimG + 0.025*G;
			if (DimG > G)
			{
				DimG = G;
			}
			DimB = DimB + 0.025*B;
			if (DimB > B)
			{
				DimB = B;
			}
		}
	}
}

void KeyIDENT(void * refCon)
{
	if (FSMode != M_OFF)
	{
		Ident = 1;
		XPLMSetDatai(dIdent, Ident);									//X-Plane's timer works only when built in XPDR is at least STBY
		XPLMCommandOnce(dIdentEnter);									//press Ident 
	}
}

void KeyVFR(void * refCon)
{
	if (Code == VFRCode)												//when VFR key pressed with VFR Code active
	{																	
		Code = prevCode;												//switch back to previous Code
	}
	else
	{
		prevCode = Code;
		Code = VFRCode;
	}
	XPLMSetDatai(dCode, Code);
}

void KeyON(void * refCon)
{
	/*
		if ((FSMode & M_ON) == 0)
		{
			//FSMode = M_ON;											//brings shortly code before init page
			FSMode = M_TEST;
			FirstCall = TRUE;
		}
	*/
	if (FSMode > M_OFF)													//we were already switched on
	{
		FSMode = M_ON;
		FSMode = 1;
	}
	else																//fresh start
	{
		FSModeStart = M_ON;
		FSMode = M_TEST;
		FirstCall = TRUE;
	}
}

void KeyALT(void * refCon)
{
	/*
	if ((FSMode & M_ON) == 0)
	{
		FSMode = M_ON;
		FirstCall = TRUE;
	}
	if (FSMode != M_OFF) 
	{
		FSMode = FSMode | M_ALT;
		Mode = 2;
		XPLMSetDatai(dMode, Mode);
	}
	*/
	if (FSMode == M_OFF)
	{
		FSModeStart = M_ALT;
		FSMode = M_TEST;
		FirstCall = TRUE;
	}
	else
	{
		FSMode = M_ALT;
		Mode = 2;
		XPLMSetDatai(dMode, Mode);
	}
}

void KeySTBY(void * refCon)
{
	/*
	if ((FSMode & M_ON) == 0)
	{
		FSMode = M_ON;
		FirstCall = TRUE;
	}
	if (FSMode != M_OFF)
	{
		FSMode = FSMode & (M_ALT ^ 2047);
		Mode = 1;
		XPLMSetDatai(dMode, Mode);
	}
	*/
	if (FSMode == M_OFF)
	{
		FSModeStart = M_STBY;
		FSMode = M_TEST;
		FirstCall = TRUE;
	}
	else
	{
		FSMode = M_STBY;
		Mode = 1;
		XPLMSetDatai(dMode, Mode);
	}

}

void KeyOFF_1(void * refCon)											//KeyOFF just triggers Power Down screen
{
	KeyOFFTick = nTick;	
}

void KeyOFF_2()															//this is actual Shut Off sequence
{
	{
		KeyOFFTick = 0;
		FSMode = M_OFF;
		Mode = 0;
		if (XPLMCanWriteDataRef(dMode))
		{
			XPLMSetDatai(dMode, Mode);
		}
	}
}

void KeyFUNC(void * refCon)
{
	Func++;
	if (Func > 7)
	{
		Func = 1;
	}
	if ((Func == 7) && (Backlight > 0))
	{
		Func = 1;
	}
}

void KeyCRSR(void * refCon)
{
	if (iCursor > -1)													//cancel when in Edit Mode 
	{
		iCursor = -1;
		Code = prevCode;
		XPLMSetDatai(dCode, Code);
	}
	if ((Func == F_COUNTDN) && (CountDnCursor == -1))					//CRSR starts Enter mode for CountDown
	{
		CountDnCursor = 0;
	}
}

void KeySTART(void * refCon)
{
	if (Func == F_ALTMON)
	{
		if (MonFL == 0)
		{
			MonFL = FL;													//save current FL to monitor
		}
		else
		{
			MonFL = 0;													//clear Alt Monitor											
		}
	}
	if (Func == F_COUNTUP)
	{
		if (CountUp == 0)
		{
			CountUpStart = RunTime;										//Start Count Up
			CountUpPause = 0;
		}
		else 
		{
			CountUpPause = ! CountUpPause;								//toggle pause
		}
	}
	if (Func == F_COUNTDN)
	{
		CountDnFlag = 0;												//turns to 1 once Count Down target is reached
		//CountDn = 30;													//default start value, can't be 0, otherwise EXPIRED goes of
		CountDnEnd = RunTime + CountDn;
	}
}

void KeyCLR(void * refCon)
{
	if (iCursor == 0)													//CLR at first digit: revert to previous Code
	{
		Code = prevCode;
		XPLMSetDatai(dCode, Code);
		iCursor = -1;
	}
	else if (iCursor > 0)													//when in Edit Mode, step one digit back
	{
		sprintf(cCode, "%04d \n", Code);
		iCursor--;
		cCode[iCursor] = '-';
	}
	else																	//within 5sec after last digit allow return to Edit Mode
	{
		if (nTickEdit < 20)
		{
			iCursor = 3;
			cCode[iCursor] = '-';
		}
	}
	if (Func == F_COUNTUP)
	{
		CountUp = 0;
	}

}

//Edit Transponder Code
void EditCode(int key)
{
	char d;																//correspondig character for a specific digit
	
	//start timeout counter
	nTickEdit = 0;
	if (iCursor == -1)
	{
		iCursor = 0;
		prevCode = Code;												//save Code before Edit to revert to in case of CRSR press
	}

	d = (char)(((int)'0') + key);										//to get char for this key, add 48 as starting point in ASCII table
	cCode[iCursor] = d;													//set new character at cursor position
	Code = atoi(cCode);													//convert Code back to integer...
	XPLMSetDatai(dCode, Code);											//...just in order to write it back to X-Plane

	iCursor++;															//move cursor to next digit
	if (iCursor < 4)
	{
		cCode[iCursor] = '-';
	}
	if (iCursor == 4)													//fourth digit? reset Mode counter 
	{
		iCursor = -1;
	}
}


//Main flight loop
float	MyFlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
								   int                  inCounter,    
                                   void *               inRefcon)
{	
	static int AltAlertSound = 0;										//flag for sound being played only once
	nTickEdit++;	
	if (nTickEdit > 20)													//counter for 5sec timeout in Edit Mode
	{
		iCursor = -1;
	}

	if (KeyOFFTick > 0)													//KeyOFF clicked ?
	{
		if (nTick == KeyOFFTick + 4)		KeyOFF_2();						//1 sec wait on KeyOFF
	}

	char s[11];
	char sa[11];
	char sb[11];


	nTick = nTick + 1;

	if ((nTick < 25) && (FSMode != M_OFF))													//start with self test sequence 5 sec
	{		
		FSMode = M_TEST;
	}
	else if (nTick == 25)												//end of self test
	{	
		FSMode = FSModeStart;											//switch into the mode that was selected on startup (STBY, ON, ALT)				
		Func = F_PA;													//default to PRESSURE ALT display
	}
	else if (nTick > 30)
	{
		if ((nTick % 4) == 0)											//every second check altitude deviation
		{
			if (MonFL > 0)												//<=== to be checked, prevents start
			{
				Deviation = (FL - MonFL) * 100;
				if ((abs(Deviation) > 100)	&& (abs(Deviation) < 1100))		//if 100 < altitude deviation < 1100ft
				{
					AltAlert = 1;											//set off alert
					Func = F_ALTMON;										//bring up Alt Monitor page
					if (! AltAlertSound)
					{
						alcMakeContextCurrent(my_ctx);						//change into our AL context
						alSourcef(snd_src[1],AL_PITCH,pitch);				//configure sound source
						alSourcePlay(snd_src[1]);							//play sound "leaving altitude"
						alcMakeContextCurrent(old_ctx);						
						AltAlertSound = 1;									//only play once
					}
				}
				else 
				{
					AltAlert = 0;
					ShowAltDev = 1;
					AltAlertSound = 0;										//reset				}
				}
			}
		}
		if ((nTick % 8) == 0)											//every 2 sec refresh time
		{
			RunTime = XPLMGetElapsedTime();
			if ((nTick % 40) == 0)										//every 10 sec save current window position
			{			
				WriteINIFile();											//write back variables
			}
		}
		if ((nTick % 16) == 0)											//every 4 sec refresh time
		{
			iFLArrow = FLTrend();										//find Arrow for FL trend indication
		}

		if (AltAlert)
		{
			ShowAltDev = !ShowAltDev;									//every 0.25 sec flash Alert
		}
	}

	return 0.25;														// Return time interval after that we want to be called again
}																		// if this interval is changed, all nTick based timers will change!

/// Loads all our textures
GLvoid LoadTextures(GLvoid)
{
	if (!LoadPNGTexture(BEZEL_FILENAME, BEZEL_TEXTURE, 0, 0))	XPLMDebugString("Panel texture BEZEL_TEXTURE failed to load\n");
	if (!LoadPNGTexture(DISPLAY_FILENAME, DISPLAY_TEXTURE, 0, 0))	XPLMDebugString("Panel texture DISPLAY_TEXTURE failed to load\n");
	if (!LoadPNGTexture(RCV_FILENAME, RCV_TEXTURE, 0, 0))	XPLMDebugString("Panel texture RCV_TEXTURE failed to load\n");
	if (!LoadPNGTexture(INIT_FILENAME, INIT_TEXTURE, 0, 0))	XPLMDebugString("Panel texture INIT_TEXTURE failed to load\n");
	if (!LoadPNGTexture(ALT_FILENAME, ALT_TEXTURE, 0, 0))	XPLMDebugString("Panel texture ALT_TEXTURE failed to load\n");
	if (!LoadPNGTexture(GND_FILENAME, GND_TEXTURE, 0, 0))	XPLMDebugString("Panel texture GND_TEXTURE failed to load\n");
	if (!LoadPNGTexture(STBY_FILENAME, STBY_TEXTURE, 0, 0))	XPLMDebugString("Panel texture STBY_TEXTURE failed to load\n");	
	if (!LoadPNGTexture(FL_FILENAME, FL_TEXTURE, 0, 0))		XPLMDebugString("Panel texture FL_TEXTURE failed to load\n");	
	if (!LoadPNGTexture(___LFILENAME, ___LTEXTURE, 0, 0))	XPLMDebugString("Panel texture ___TEXTURE failed to load\n");	
	if (!LoadPNGTexture(_0_MFILENAME, _0_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _0_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_1_MFILENAME, _1_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _1_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_2_MFILENAME, _2_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _2_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_3_MFILENAME, _3_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _3_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_4_MFILENAME, _4_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _4_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_5_MFILENAME, _5_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _5_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_6_MFILENAME, _6_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _6_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_7_MFILENAME, _7_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _7_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_8_MFILENAME, _8_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _8_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_9_MFILENAME, _9_MTEXTURE, 0, 0))	XPLMDebugString("Panel texture _9_MTEXTURE failed to load\n");
	if (!LoadPNGTexture(_0_LFILENAME, _0_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _0_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_1_LFILENAME, _1_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _1_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_2_LFILENAME, _2_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _2_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_3_LFILENAME, _3_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _3_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_4_LFILENAME, _4_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _4_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_5_LFILENAME, _5_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _5_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_6_LFILENAME, _6_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _6_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_7_LFILENAME, _7_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _7_LTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_0_IFILENAME, _0_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _0_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_1_IFILENAME, _1_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _1_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_2_IFILENAME, _2_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _2_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_3_IFILENAME, _3_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _3_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_4_IFILENAME, _4_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _4_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_5_IFILENAME, _5_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _5_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_6_IFILENAME, _6_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _6_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_7_IFILENAME, _7_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _7_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_8_IFILENAME, _8_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _8_ITEXTURE failed to load\n");
	//if (!LoadPNGTexture(_9_IFILENAME, _9_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _9_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_A_SFILENAME, _A_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _A_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_B_SFILENAME, _B_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _B_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_C_SFILENAME, _C_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _C_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_D_SFILENAME, _D_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _D_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_E_SFILENAME, _E_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _E_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_F_SFILENAME, _F_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _F_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_G_SFILENAME, _G_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _G_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_H_SFILENAME, _H_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _H_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_I_SFILENAME, _I_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _I_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_K_SFILENAME, _K_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _K_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_L_SFILENAME, _L_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _L_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_M_SFILENAME, _M_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _M_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_N_SFILENAME, _N_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _N_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_O_SFILENAME, _O_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _O_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_P_SFILENAME, _P_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _P_STEXTURE failed to load\n");
	//if (!LoadPNGTexture(_Q_SFILENAME, _Q_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Q_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_R_SFILENAME, _R_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _R_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_S_SFILENAME, _S_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _S_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_T_SFILENAME, _T_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _T_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_U_SFILENAME, _U_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _U_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_V_SFILENAME, _V_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _V_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_W_SFILENAME, _W_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _W_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_X_SFILENAME, _X_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _X_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_Y_SFILENAME, _Y_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Y_STEXTURE failed to load\n");
	//if (!LoadPNGTexture(_Z_SFILENAME, _Z_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Z_STEXTURE failed to load\n");
	if (!LoadPNGTexture(___SFILENAME, ___STEXTURE, 0, 0))	XPLMDebugString("Panel texture ___STEXTURE failed to load\n");
	//if (!LoadPNGTexture(_A_WFILENAME, _A_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _A_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_B_WFILENAME, _B_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _B_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_C_WFILENAME, _C_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _C_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_D_WFILENAME, _D_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _D_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_E_WFILENAME, _E_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _E_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_F_WFILENAME, _F_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _F_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_G_WFILENAME, _G_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _G_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_H_WFILENAME, _H_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _H_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_I_WFILENAME, _I_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _I_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_K_WFILENAME, _K_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _K_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_L_WFILENAME, _L_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _L_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_M_WFILENAME, _M_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _M_WTEXTURE failed to load\n");
	////if (!LoadPNGTexture(_N_WFILENAME, _N_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _N_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_O_WFILENAME, _O_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _O_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_P_WFILENAME, _P_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _P_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_Q_WFILENAME, _Q_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Q_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_R_WFILENAME, _R_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _R_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_S_WFILENAME, _S_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _S_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_T_WFILENAME, _T_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _T_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_U_WFILENAME, _U_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _U_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_V_WFILENAME, _V_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _V_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_W_WFILENAME, _W_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _W_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_X_WFILENAME, _X_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _X_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_Y_WFILENAME, _Y_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Y_WTEXTURE failed to load\n");
	//if (!LoadPNGTexture(_Z_WFILENAME, _Z_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Z_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(colMFILENAME, colMTEXTURE, 0, 0))	XPLMDebugString("Panel texture colMTEXTURE failed to load\n");
	if (!LoadPNGTexture(FT_FILENAME, FT_TEXTURE, 0, 0))		XPLMDebugString("Panel texture FT_TEXTURE failed to load\n");
	if (!LoadPNGTexture(OFF_FILENAME, OFF_TEXTURE, 0, 0))	XPLMDebugString("Panel texture OFF_TEXTURE failed to load\n");
	if (!LoadPNGTexture(dashFILENAME, dashTEXTURE, 0, 0))	XPLMDebugString("Panel texture dashTEXTURE failed to load\n");
	if (!LoadPNGTexture(frameFILENAME, frameTEXTURE, 0, 0))	XPLMDebugString("Panel texture frameTEXTURE failed to load\n");
	if (!LoadPNGTexture(posFILENAME, posTEXTURE, 0, 0))	XPLMDebugString("Panel texture posTEXTURE failed to load\n");
	if (!LoadPNGTexture(negFILENAME, negTEXTURE, 0, 0))	XPLMDebugString("Panel texture negTEXTURE failed to load\n");
	if (!LoadPNGTexture(POWERDOWN_FILENAME, POWERDOWN_TEXTURE, 0, 0))	XPLMDebugString("Panel texture POWERDOWN_TEXTURE failed to load\n");
	if (!LoadPNGTexture(ON_FILENAME, ON_TEXTURE, 0, 0))	XPLMDebugString("Panel texture ON_TEXTURE failed to load\n");
	if (!LoadPNGTexture(UP_S_FILENAME, UP_S_TEXTURE, 0, 0))	XPLMDebugString("Panel texture UP_S_TEXTURE failed to load\n");
	if (!LoadPNGTexture(UP_L_FILENAME, UP_L_TEXTURE, 0, 0))	XPLMDebugString("Panel texture UP_L_TEXTURE failed to load\n");
	if (!LoadPNGTexture(DOWN_S_FILENAME, DOWN_S_TEXTURE, 0, 0))	XPLMDebugString("Panel texture DOWN_S_TEXTURE failed to load\n");
	if (!LoadPNGTexture(DOWN_L_FILENAME, DOWN_L_TEXTURE, 0, 0))	XPLMDebugString("Panel texture DOWN_L_TEXTURE failed to load\n");
}


//get id of texture corresponding to character
int GetTextureID(char irg)
{
	if (irg == ':') return colSTEXTURE;
	if (irg == ' ') return ___STEXTURE;
	if (irg == '-') return ___LTEXTURE;
	if (irg == '0') return _0_LTEXTURE;
	if (irg == '1') return _1_LTEXTURE;
	if (irg == '2') return _2_LTEXTURE;
	if (irg == '3') return _3_LTEXTURE;
	if (irg == '4') return _4_LTEXTURE;
	if (irg == '5') return _5_LTEXTURE;
	if (irg == '6') return _6_LTEXTURE;
	if (irg == '7') return _7_LTEXTURE;
	if (irg == '8') return _8_LTEXTURE;
	if (irg == '9') return _9_LTEXTURE;
	if (irg == 'A') return _A_STEXTURE;
	if (irg == 'B') return _B_STEXTURE;
	if (irg == 'C') return _C_STEXTURE;
	if (irg == 'D') return _D_STEXTURE;
	if (irg == 'E') return _E_STEXTURE;
	if (irg == 'F') return _F_STEXTURE;
	if (irg == 'G') return _G_STEXTURE;
	if (irg == 'H') return _H_STEXTURE;
	if (irg == 'I') return _I_STEXTURE;
	if (irg == 'K') return _K_STEXTURE;
	if (irg == 'L') return _L_STEXTURE;
	if (irg == 'M') return _M_STEXTURE;
	if (irg == 'N') return _N_STEXTURE;
	if (irg == 'O') return _O_STEXTURE;
	if (irg == 'P') return _P_STEXTURE;
	if (irg == 'Q') return _Q_STEXTURE;
	if (irg == 'R') return _R_STEXTURE;
	if (irg == 'S') return _S_STEXTURE;
	if (irg == 'T') return _T_STEXTURE;
	if (irg == 'U') return _U_STEXTURE;
	if (irg == 'V') return _V_STEXTURE;
	if (irg == 'W') return _W_STEXTURE;
	if (irg == 'X') return _X_STEXTURE;
	if (irg == 'Y') return _Y_STEXTURE;
	if (irg == 'Z') return _Z_STEXTURE;
	return ___TEXTURE;
}

/*
parameters to save:
VFR Code			local autority VFR Code
OAT feature			0 or 1
°C / °F				0 = Fahrenheit, 1 = Celsius
AutoFlightTimer		0 = Man, 1 = Clear at every lift off, 2 = Accum counting after lift off
Display				0 = Auto, 1 = black background, 2 = light background
Backlight			0 = manual, 1 = X-Plane lighting bus, 2 = X-Plane ambient, 3 = hardware photo cell
Bezel				0 = don't show bezel
'*/
//This function is Windows only
int OpenINIFile()
{
	char INIFile[255];
	char sLeft[255];
	char sRight[255];	
	char sBottom[255];
	char sTop[255];
	char sVFRCode[5];
	char sOAT[2];
	char sFC[2];
	char sAutoFlightTimer[2];
	char sDisplay[2];
	char sBacklight[2];
	char sAlertSound[2];
	char sBezel[2];
	//char sAudioDevice[255];						//is already defined public
	char sVoltage[3];

	strcpy(INIFile, gPluginDataFile);
	strcat(INIFile, "gtx328.ini");
	GetPrivateProfileString("common","Left","100",sLeft,255,INIFile);
	GetPrivateProfileString("common","Right","860",sRight,255,INIFile);
	GetPrivateProfileString("common","Bottom","100",sBottom,255,INIFile);
	GetPrivateProfileString("common","Top","300",sTop,255,INIFile);
	GetPrivateProfileString("common","VFRCode","",sVFRCode,255,INIFile);
	GetPrivateProfileString("common","OAT","1",sOAT,255,INIFile);
	GetPrivateProfileString("common","FC","1",sFC,255,INIFile);
	GetPrivateProfileString("common","AutoFlightTimer","1",sAutoFlightTimer,255,INIFile);
	GetPrivateProfileString("common","Display","0",sDisplay,255,INIFile);
	GetPrivateProfileString("common","Backlight","0",sBacklight,255,INIFile);
	GetPrivateProfileString("common","AlertSound","1",sAlertSound,255,INIFile);
	GetPrivateProfileString("common","Bezel","1",sBezel,255,INIFile);
	GetPrivateProfileString("common", "AudioDevice", "1", sAudioDevice, 255, INIFile); 
	GetPrivateProfileString("common", "Voltage", "12", sVoltage, 255, INIFile);
	gaugeX1 = atof(sLeft);
	gaugeX2 = atof(sRight);
	gaugeY1 = atof(sBottom);
	gaugeY2 = atof(sTop);
	VFRCode = atoi(sVFRCode);
	OAT = atoi(sOAT);
	FC = atoi(sFC);
	AutoFlightTimer = atoi(sAutoFlightTimer);
	Display = atoi(sDisplay);
	Backlight = atoi(sBacklight);
	AlertSound = atoi(sAlertSound);
	Bezel = atoi(sBezel);
	Voltage = atoi(sVoltage);
	return TRUE;
}


//This function is Windows only
//save window position
//Transponder Code and Callsign need not be saved seperately
int WriteINIFile()
{
	char INIFile[255];
	char sLeft[255];
	char sRight[255];	
	char sBottom[255];
	char sTop[255];

	int Left;
	int Right;
	int Bottom;
	int Top;

	//write back window position
	XPLMGetWindowGeometry(ggfePanelDisplayWindow, &Left, &Top, &Right, &Bottom);
	sprintf(sLeft,"%-i",Left);
	sprintf(sRight,"%-i",Right);
	sprintf(sBottom,"%-i",Bottom);
	sprintf(sTop,"%-i",Top);
	
	strcpy(INIFile, gPluginDataFile);
	strcat(INIFile, "gtx328.ini");
	WritePrivateProfileString("common","Left",sLeft,INIFile);
	WritePrivateProfileString("common","Right",sRight,INIFile);
	WritePrivateProfileString("common","Bottom",sBottom,INIFile);
	WritePrivateProfileString("common","Top",sTop,INIFile);

	return TRUE;
}


// Used for dragging plugin panel window and mouse clickable spots
int	CoordInRect(float x, float y, float l, float t, float r, float b)
{	return ((x >= l) && (x < r) && (y < t) && (y >= b)); }


int PaintTexture2(int texture, float w, float h, float x, float y)
{	
	XPLMBindTexture2d(gTexture[texture], 0);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &glwidth);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &glheight);
	if (w == 0) {w = glwidth * Scaler;}									//default to original texture size
	if (h == 0) {h = glheight * Scaler;}
	glBegin(GL_QUADS);
			glTexCoord2f(1, 0.0f); glVertex2f(x + w, y);				// Bottom Right 
			glTexCoord2f(0, 0.0f); glVertex2f(x, y);					// Bottom Left 		
			glTexCoord2f(0, 1.0f); glVertex2f(x, y + h);				// Top Left 
			glTexCoord2f(1, 1.0f); glVertex2f(x + w, y + h);			// Top Right 
	glEnd();
	return TRUE;
}

//switch Display night/day mode
void SwitchDisplay()
{
	if (Display == 0)										//night/day mode controlled by ambient light
	{
		if ((Red + Green + Blue) < 1.5)						//X-Plane ambient light is low
		{
			InvertDisp = 1;									//draw on black background
		}
		else
		{
			InvertDisp = 0;
		}
	}
	else if (Display == 1)									//night
	{
		InvertDisp = 1;										//draw on black background
	}
	else if (Display == 2)									//day
	{
		InvertDisp = 0;										
	}

	if (Backlight == 1)										//X-Plane lighting bus controls display
	{
		DimR = InstrumentDim[0] * R;						
		DimG = InstrumentDim[0] * G;
		DimB = InstrumentDim[0] * B;
	}
	else if (Backlight == 2)								//X-Plane ambient light controls display
	{
		DimR = R * Red;
		DimG = G * Green;
		DimB = B * Blue;
	}
	else if (Backlight == 3)								//hardware photo cell controls display
	{
		DimR = InstrumentDim[14] * R;						//used X-Plane array item 15 for that
		DimG = InstrumentDim[14] * G;
		DimB = InstrumentDim[14] * B;
	}
	glColor3f(DimR, DimG, DimB);
}

//draw Display Background
void DrawDisplay()
{
	if (InvertDisp)
	{
		glColor3f(Rb, Gb, Bb);											//draw display black
		PaintTexture2(DISPLAY_TEXTURE, 0, 0, DispL, DispB);
		glColor3f(DimR, DimG, DimB);									//switch back to colored characters
	}
	else
	{
		glColor3f(DimR, DimG, DimB);									//draw display colored
		PaintTexture2(DISPLAY_TEXTURE, 0, 0, DispL, DispB);
		glColor3f(Rb, Gb, Bb);											//switch back to black characters
	}	
}

//draw Init Page
void DrawInit()
{
	PaintTexture2(INIT_TEXTURE, 0, 0, DispL, DispB);
}

/*
//draw Mode
void DrawMode(const char *cMode)
{
	int ModeOffset = 0;
	for (i=0; i < (strlen(cMode) + 1); i++)
	{
		a = GetTextureID(cCode[i]);										//mid size textures
		if (i > 0)
		{
			ModeOffset = ModeOffset + glwidth * Scaler;
		}
		PaintTexture2(a, 0, 0, DispL + ModeOffset, 1.2 * DispB);
	}
}
*/

//draw Transponder Code
void DrawCode()
{
	int CodeLoffset = 0;

	if (iCursor == -1)													//if not in Edit Mode, show X-Plane's Code
	{
		if ((Code >= 0) || (Code < 7778))								//accept only valid codes from X-Plane 
		{
			sprintf(cCode, "%04d \n", Code);
		}
	}
	for  (i=0; i<4; i++)												//left digits												
	{
		if (iCursor > -1)												//if cursor active
		{
			//a = GetTextureID(cCode[i]) + 10;							//get inverted large textures
			a = GetTextureID(cCode[i]);									//standard large textures
		}
		else
		{
			a = GetTextureID(cCode[i]);									//standard large textures
		}
		if (i > 0)														//only for digits 2 to 4
		{
			CodeLoffset = CodeLoffset + glwidth * Scaler;				//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, CodeL + CodeLoffset, DispB + 3 * Scaler);
	}
}

//draw reply symbol (R) ony in XPDR Modes A and C (M_ON and M_ALT)
void DrawReply()
{
	if (Lit)															//X-Plane Dataref
	{
		if ((FSMode == M_ON) || (FSMode == M_ALT))
		{
			PaintTexture2(RCV_TEXTURE, 0, 0, DispL + 1 * Scaler, DispB);
		}
	}
}

//draw title 
void DrawTitle()
{
	int i = 0;
	int offset = 0;
	if (Func == F_PA)
	{
		strcpy(title, "PRESSURE ALT");
	}
	else if (Func == F_FLTTIME)
	{
		strcpy(title, " FLIGHT TIME");
	}
	else if (Func == F_ALTMON)
	{
		strcpy(title, " ALT MONITOR");
	}
	else if (Func == F_OAT)
	{
		strcpy(title, " OAT        ");
	}
	else if (Func == F_COUNTUP)
	{
		strcpy(title, "    COUNT UP");
	}
	else if ((Func == F_COUNTDN) && (CountDnFlag) && (nTick % 2 == 0))	//flashing every 0.5 sec
	{
		strcpy(title, "    EXPIRED ");
	}
	else if ((Func == F_COUNTDN) && (!CountDnFlag))						//don't show on flashing EXPIRED
	{
		strcpy(title, "  COUNT DOWN");
	}
	else if (Func == F_DISPLAY)
	{
		strcpy(title, "    DISPLAY  ");
	}
	else
	{
		strcpy(title, "            ");									//blank on flashing EXPIRED
	}

	for (i=0; i<12;i++)
	{
		a = GetTextureID(title[i]);
		if (i > 0)
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;
		}
		PaintTexture2(a, 0, 0, DispL + offset + 125*Scaler, DispB + 20*Scaler);
	}
}

//draw FL (Pressure Altitude)
void DrawFL()
{
	float offset = 0;
	sprintf(cFL, "%03u", FL);
	for  (i=0; i<3; i++)												//left digits												
	{
		a = GetTextureID(cFL[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 3
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 153 * Scaler, DispB + 5 * Scaler);
	}
	PaintTexture2(FL_TEXTURE, 0, 0, DispL + 137 * Scaler, DispB + 5 * Scaler);

	if (iFLArrow == 1)			PaintTexture2(UP_S_TEXTURE, 0, 0, DispL + 184 * Scaler, DispB + 5 * Scaler);
	if (iFLArrow == 2)			PaintTexture2(UP_L_TEXTURE, 0, 0, DispL + 184 * Scaler, DispB + 3 * Scaler);
	if (iFLArrow == -1)			PaintTexture2(DOWN_S_TEXTURE, 0, 0, DispL + 184 * Scaler, DispB + 5 * Scaler);
	if (iFLArrow == -2)			PaintTexture2(DOWN_L_TEXTURE, 0, 0, DispL + 184 * Scaler, DispB + 3 * Scaler);

}

//calculate flight level from current baro setting and indicated altitude
int CalcFL()
{ return floor(((fAltitude * 3.28 +  ((29.92 - fBaro) * 1000)) / 100) + 0.5); }	

//find Altitude trend by comparing current with previous Altitude, calculates Arrow up/down and size
//called from main flight loop every 4 sec, m/s converted to feet per minute
int FLTrend()
{
	int iTrend;
	float fTrendArrow;
	fTrendArrow = fAltitude - fPrevAltitude;
	fPrevAltitude = fAltitude;

	if ((fTrendArrow >= -1) && (fTrendArrow <= 1))				iTrend = 0;		//less than 50 fpm, no trend arrow	
	if ((fTrendArrow > 1) && (fTrendArrow < 10))			iTrend = 1;		//50 fpm to 500 fpm, small arrow
	if (fTrendArrow >= 10)										iTrend = 2;		//greater 500 fpm, large arrow
	if ((fTrendArrow < -1) && (fTrendArrow > -10))			iTrend = -1;
	if (fTrendArrow < -10)										iTrend = -2;

	return iTrend;
}

//draw time
void DrawFlightTime()
{
	int h, m, s;
	char cTime[9];
	float offset = 0;

	h = FlightTime / 3600;
	m = (FlightTime % 3600) / 60;
	s = (FlightTime % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}

}

//draw Altitude Monitor 
void DrawAltMonitor()
{
	float offset = 0;
	char cDeviation[10];
	if (MonFL > 0)
	{
		Deviation = (FL - MonFL) * 100;
		sprintf(cDeviation, "%4u \n", abs(Deviation));
		if (abs(Deviation) < 1100)												//stop drawing once deviation is too large
		{
			if (abs(Deviation) > 0)
			{
				for  (i=0; i<4; i++)											//left digits												
				{
					a = GetTextureID(cDeviation[i]) - 10;						//get smaller textures _?_MTEXTURE
					if (i > 0)													//only for digits 1 to 3
					{
						offset = offset + glwidth * Scaler;						//get last texture width for character spacing 	
					}
					PaintTexture2(a, 0, 0, DispL + offset + 124 * Scaler, DispB + 5 * Scaler);
				}
				offset = offset + 1 * Scaler + glwidth * Scaler;
				PaintTexture2(FT_TEXTURE, 0, 0, DispL + offset + 124 * Scaler, DispB + 5 * Scaler);
			}
			else
			{
				a = GetTextureID('0') - 10;										//get smaller textures _?_MTEXTURE
				PaintTexture2(a, 0, 0, DispL + offset + 148 * Scaler, DispB + 5 * Scaler);
				offset = offset + 1 * Scaler + glwidth * Scaler;
				PaintTexture2(FT_TEXTURE, 0, 0, DispL + offset + 148 * Scaler, DispB + 5 * Scaler);
			}
			if ((Deviation > 0) && ShowAltDev)									//care for flashing flag
			{
				strcpy(cDeviation, "ABOVE");
			}
			else if ((Deviation < 0) && ShowAltDev)
			{
				strcpy(cDeviation, "BELOW");
			}
			else
			{
				strcpy(cDeviation, "     ");
			}
			offset = 0;
			for  (i=0; i<5; i++)												//draw "ABOVE" or "BELOW"											
			{
				a = GetTextureID(cDeviation[i]);								//get smaller textures _?_MTEXTURE
				if (i > 0)														//only for digits 2 to 5
				{
					offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
				}
				PaintTexture2(a, 0, 0, DispL + offset + 164 * Scaler, DispB + 5 * Scaler);
			}
		}
	}
	else
	{
		PaintTexture2(OFF_TEXTURE, 0, 0, DispL + 148 * Scaler, DispB + 5 * Scaler);
	}
}

//draw outside air temperature and density altitude
void DrawOAT()
{
	float offset = 0;
	char cOAT[4];
	char cDA[7];
	char cText[5];
	int iDA = ((fAltitude * 3.28 +  ((29.92 - fBaro) * 1000)) * 1.24) + (118.8 * fOAT) - 1782 ;
	int iOAT = abs((int)fOAT);

	if (FC == 0)														//option Fahrenheit
	{
		iOAT = (fOAT * 1.8) + 32;										//convert to Fahrenheit
	}

	sprintf(cOAT, "%3u", abs(iOAT));											
	for  (i=0; i<3; i++)																							
	{
		a = GetTextureID(cOAT[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 3
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 160 * Scaler, DispB + 20 * Scaler);
	}
	if (iOAT > 0)
	{
		PaintTexture2(posTEXTURE, 0, 0, DispL + offset + 169 * Scaler, DispB + 27 * Scaler);
	}
 	else
	{
		PaintTexture2(negTEXTURE, 0, 0, DispL + offset + 169 * Scaler, DispB + 27 * Scaler);
	}
	if (FC == 1)
	{
		PaintTexture2(_C_STEXTURE, 0, 0, DispL + offset + 173 * Scaler, DispB + 20 * Scaler);
	}
	else
	{
		PaintTexture2(_F_STEXTURE, 0, 0, DispL + offset + 173 * Scaler, DispB + 20 * Scaler);
	}
	strcpy(cText, "DALT");
	for (i=0; i<4;i++)
	{
		a = GetTextureID(cText[i]);
		if (i > 0)
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;
		}
		PaintTexture2(a, 0, 0, DispL + offset + 115*Scaler, DispB + 5*Scaler);
	}
	sprintf(cDA, "%5u", abs(iDA));
	for  (i=0; i<5; i++)																								
	{
		a = GetTextureID(cDA[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 5
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 121 * Scaler, DispB + 5 * Scaler);
	}
	PaintTexture2(FT_TEXTURE, 0, 0, DispL + offset + 9 * Scaler + 121 * Scaler, DispB + 5 * Scaler);
}

void DrawCountUp()
{
	int h, m, s;
	char cTime[9];
	float offset = 0;

	h = CountUp / 3600;
	m = (CountUp % 3600) / 60;
	s = (CountUp % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}

}

void DrawCountDn()
{
	int h, m, s;
	char cTime[9];
	float offset = 0;

	h = CountDn / 3600;
	m = (CountDn % 3600) / 60;
	s = (CountDn % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + glwidth * Scaler;							//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}
}

//build Count Down starting point
void CountDnEnter(int key)
{
	static int h, m, s;
	
	switch (CountDnCursor)												//character position
	{
	case 0:																//hh
		h = 0;
		h = 10 * key;
		break;
	case 1:
		h = h + key;
		break;
	case 2:																//mm
		m = 0;
		if (key < 6)
		{
			m = 10 * key;
		}
		break;
	case 3:
		m = m + key;
		break;
	case 4:																//ss
		s = 0;
		if (key < 6)
		{
			s = 10 * key;
		}
		break;
	case 5:
		s = s + key;
		break;
	}
	
	CountDn = (3600 * h) + (60 * m) + s;
	CountDnCursor++;
	if (CountDnCursor == 6)
	{
		CountDnCursor = -1;
	}
}

//draw Display Dim box
void DrawDim()
{
	float z;
	PaintTexture2(frameTEXTURE, 0, 0, DispL + 129 * Scaler, DispB + 5 * Scaler);
	for (z=0;z<(DimR/R)-0.25;z+=0.025)
	{
		PaintTexture2(dashTEXTURE, 0, 0, DispL + 129 * Scaler + z*80*Scaler, DispB + 6 * Scaler);
	}
}

//draw Ident symbol
void DrawIdent()
{
	int offset = 0;
	char c[6];
	strcpy(c, "IDENT");

	for (i=0; i < (strlen(c)); i++)
	{
		a = GetTextureID(c[i]);											//mid size textures
		if (i > 0)
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//1 px spacing 
		}
		PaintTexture2(a, 0, 0, DispL + offset, DispB + 25*Scaler);
	}
}

//draw Text Array with MTEXTURE
void DrawMText(float x, float y, char Text[20])
{
}

int gfe_gtx_Key0_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key0(0);
	}
	return 0;
}

int gfe_gtx_Key1_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key1(0);
	}
	return 0;
}

int gfe_gtx_Key2_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key2(0);
	}
	return 0;
}

int gfe_gtx_Key3_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key3(0);
	}
	return 0;
}

int gfe_gtx_Key4_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key4(0);
	}
	return 0;
}

int gfe_gtx_Key5_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key5(0);
	}
	return 0;
}

int gfe_gtx_Key6_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key6(0);
	}
	return 0;
}

int gfe_gtx_Key7_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key7(0);
	}
	return 0;
}

int gfe_gtx_Key8_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key8(0);
	}
	return 0;
}

int gfe_gtx_Key9_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		Key9(0);
	}
	return 0;
}

int gfe_gtx_KeyIDENT_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyIDENT(0);
	}
	return 0;
}

int gfe_gtx_KeyVFR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyVFR(0);
	}
	return 0;
}

int gfe_gtx_KeyON_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyON(0);
	}
	return 0;
}

int gfe_gtx_KeyALT_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyALT(0);
	}
	return 0;
}

int gfe_gtx_KeySTBY_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeySTBY(0);
	}
	return 0;
}

int gfe_gtx_KeyOFF_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyOFF_1(0);
	}
	return 0;
}

int gfe_gtx_KeyFUNC_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyFUNC(0);
	}
	return 0;
}

int gfe_gtx_KeyCRSR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyCRSR(0);
	}
	return 0;
}

int gfe_gtx_KeySTART_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeySTART(0);
	}
	return 0;
}

int gfe_gtx_KeyCLR_Proc(
	XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void *                inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		KeyCLR(0);
	}
	return 0;
}


/**************************************************************************************************************
 * PNG FILE LOADING
**************************************************************************************************************/
/*
libpng info:
http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf
http://signbit01.wordpress.com/2012/05/27/libpng-1-5-10-access-violation-on-visual-studio-2010/
http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/

source: 
http://forums.x-plane.org/index.php?showtopic=66819
http://forums.x-plane.org/index.php?showtopic=51503
alternate use http://forums.x-plane.org/index.php?showtopic=44573	

building info:
Generated Code Setting (/MD, /MTd...) must be the same for target, libpng16.lib and zlib.lib 
no Optimization (/GL)
Configuration must be Release (even though I cant see any differences in configs)
*/
GLuint LoadPNGTexture(const char *pFileName, int TextureId, int *width, int *height)
{
    png_byte header[8];

	char file_name[255];
	strcpy(file_name, gPluginDataFile);
	strcat(file_name, pFileName);

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        return 0;
    }
 
    // read the header
    fread(header, 1, 8, fp);
 
    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }
 
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }
 
    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }
 
    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }
 
    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }
 
    // init png reading
    png_init_io(png_ptr, fp);
 
    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);
 
    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);
 
    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
 
    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);
 
    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }
 
    //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);
 
    if (bit_depth != 8)
    {
        fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
        return 0;
    }
 
    GLint format;
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        break;
    default:
        fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
        return 0;
    }
 
    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);
 
    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
 
    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);
 
    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }
 
    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }
 
    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }
 
    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);
 
    // Generate the OpenGL texture object with XPLM functions
	XPLMGenerateTextureNumbers(&gTexture[TextureId], 1);
	XPLMBindTexture2d(gTexture[TextureId], 0);
	glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    //return texture;
	return TextureId;
}


/**************************************************************************************************************
 * WAVE FILE LOADING
**************************************************************************************************************/
// this is just copied straight from SDK
// You can just use alutCreateBufferFromFile to load a wave file, but there seems to be a lot of problems with 
// alut not beign available, being deprecated, etc.  So...here's a stupid routine to load a wave file.  I have
// tested this only on x86 machines, so if you find a bug on PPC please let me know.

// Macros to swap endian-values.
#define SWAP_32(value)                 \
        (((((unsigned short)value)<<8) & 0xFF00)   | \
         ((((unsigned short)value)>>8) & 0x00FF))

#define SWAP_16(value)                     \
        (((((unsigned int)value)<<24) & 0xFF000000)  | \
         ((((unsigned int)value)<< 8) & 0x00FF0000)  | \
         ((((unsigned int)value)>> 8) & 0x0000FF00)  | \
         ((((unsigned int)value)>>24) & 0x000000FF))

// Wave files are RIFF files, which are "chunky" - each section has an ID and a length.  This lets us skip
// things we can't understand to find the parts we want.  This header is common to all RIFF chunks.
struct chunk_header { 
	int			id;
	int			size;
};

// WAVE file format info.  We pass this through to OpenAL so we can support mono/stereo, 8/16/bit, etc.
struct format_info {
	short		format;				// PCM = 1, not sure what other values are legal.
	short		num_channels;
	int			sample_rate;
	int			byte_rate;
	short		block_align;
	short		bits_per_sample;
};

// This utility returns the start of data for a chunk given a range of bytes it might be within.  Pass 1 for
// swapped if the machine is not the same endian as the file.
static char *	find_chunk(char * file_begin, char * file_end, int desired_id, int swapped)
{
	while(file_begin < file_end)
	{
		chunk_header * h = (chunk_header *) file_begin;
		if(h->id == desired_id && !swapped)
			return file_begin+sizeof(chunk_header);
		if(h->id == SWAP_32(desired_id) && swapped)
			return file_begin+sizeof(chunk_header);
		int chunk_size = swapped ? SWAP_32(h->size) : h->size;
		char * next = file_begin + chunk_size + sizeof(chunk_header);
		if(next > file_end || next <= file_begin)
			return NULL;
		file_begin = next;		
	}
	return NULL;
}

// Given a chunk, find its end by going back to the header.
static char * chunk_end(char * chunk_start, int swapped)
{
	chunk_header * h = (chunk_header *) (chunk_start - sizeof(chunk_header));
	return chunk_start + (swapped ? SWAP_32(h->size) : h->size);
}

//#define FAIL(X) { XPLMDebugString(X); free(mem); return 0; }

#define RIFF_ID 0x46464952			// 'RIFF'
#define FMT_ID  0x20746D66			// 'FMT '
#define DATA_ID 0x61746164			// 'DATA'

ALuint load_wave(const char * file_name)
{
	// First we open the file and copy it into a single large memory buffer for processing.
	FILE * fi = fopen(file_name,"rb");
	if(fi == NULL)
	{
		XPLMDebugString("WAVE file load failed - could not open.\n");	
		return 0;
	}
	fseek(fi,0,SEEK_END);
	int file_size = ftell(fi);
	fseek(fi,0,SEEK_SET);
	char * mem = (char*) malloc(file_size);
	if(mem == NULL)
	{
		XPLMDebugString("WAVE file load failed - could not allocate memory.\n");
		fclose(fi);
		return 0;
	}
	if (fread(mem, 1, file_size, fi) != file_size)
	{
		XPLMDebugString("WAVE file load failed - could not read file.\n");	
		free(mem);
		fclose(fi);
		return 0;
	}
	fclose(fi);
	char * mem_end = mem + file_size;
	
	// Second: find the RIFF chunk.  Note that by searching for RIFF both normal
	// and reversed, we can automatically determine the endian swap situation for
	// this file regardless of what machine we are on.
	int swapped = 0;
	char * riff = find_chunk(mem, mem_end, RIFF_ID, 0);
	if(riff == NULL)
	{
		riff = find_chunk(mem, mem_end, RIFF_ID, 1);
		if(riff)
			swapped = 1;
		else
			XPLMDebugString("Could not find RIFF chunk in wave file.\n");
	}
	
	// The wave chunk isn't really a chunk at all. It's just a "WAVE" tag followed by more chunks.  
	// confirm the WAVE ID and move on
	if (riff[0] != 'W' ||
		riff[1] != 'A' ||
		riff[2] != 'V' ||
		riff[3] != 'E')
		XPLMDebugString("Could not find WAVE signature in wave file.\n");

	char * format = find_chunk(riff+4, chunk_end(riff,swapped), FMT_ID, swapped);
	if(format == NULL)
		XPLMDebugString("Could not find FMT  chunk in wave file.\n");
	
	// Find the format chunk, and swap the values if needed.  This gives us our real format.	
	format_info * fmt = (format_info *) format;
	if(swapped)
	{
		fmt->format = SWAP_16(fmt->format);
		fmt->num_channels = SWAP_16(fmt->num_channels);
		fmt->sample_rate = SWAP_32(fmt->sample_rate);
		fmt->byte_rate = SWAP_32(fmt->byte_rate);
		fmt->block_align = SWAP_16(fmt->block_align);
		fmt->bits_per_sample = SWAP_16(fmt->bits_per_sample);
	}
	
	// Reject things we don't understand...expand this code to support weirder audio formats.	
	if(fmt->format != 1) 
		XPLMDebugString("Wave file is not PCM format data.\n");
	if(fmt->num_channels != 1 && fmt->num_channels != 2) 
		XPLMDebugString("Must have mono or stereo sound.\n");
	if(fmt->bits_per_sample != 8 && fmt->bits_per_sample != 16) 
		XPLMDebugString("Must have 8 or 16 bit sounds.\n");
	char * data = find_chunk(riff+4, chunk_end(riff,swapped), DATA_ID, swapped) ;
	if(data == NULL)
		XPLMDebugString("I could not find the DATA chunk.\n");
	
	int sample_size = fmt->num_channels * fmt->bits_per_sample / 8;
	int data_bytes = chunk_end(data,swapped) - data;
	int data_samples = data_bytes / sample_size;
	
	// If the file is swapped and we have 16-bit audio, we need to endian-swap the audio too or we'll 
	// get something that sounds just astoundingly bad!	
	if(fmt->bits_per_sample == 16 && swapped)
	{
		short * ptr = (short *) data;
		int words = data_samples * fmt->num_channels;
		while(words--)
		{
			*ptr = SWAP_16(*ptr);
			++ptr;
		}
	}
	
	// Finally, the OpenAL crud.  Build a new OpenAL buffer and send the data to OpenAL, passing in
	// OpenAL format enums based on the format chunk.	
	ALuint buf_id = 0;
	alGenBuffers(1, &buf_id);
	if(buf_id == 0) 
		XPLMDebugString("Could not generate buffer id.\n");
	
	alBufferData(buf_id, fmt->bits_per_sample == 16 ? 
							(fmt->num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16) :
							(fmt->num_channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8),
					data, data_bytes, fmt->sample_rate);
	free(mem);
	return buf_id;
}

// This is a stupid logging error function...useful for debugging, but not good error checking.
#define CHECK_ERR() __CHECK_ERR(__FILE__,__LINE__)
static void __CHECK_ERR(const char * f, int l)
{
	ALuint e = alGetError();
	if (e != AL_NO_ERROR)
		printf("ERROR: %d (%s:%d\n", e, f, l);
}

// Initialization code.
float init_sound(float elapsed, float elapsed_sim, int counter, void * ref)
{
	char* s;

	char buf[255];
	char filepath[255];
	char wavfile[3][30];		

	if (AlertSound == 1)												//male voice
	{
		strcpy(wavfile[0], A_TIMEXP_M);
		strcpy(wavfile[1], A_ALTMON_M);
	}
	else if (AlertSound == 2)											//female voice
	{
		strcpy(wavfile[0], A_TIMEXP_F);
		strcpy(wavfile[1], A_ALTMON_F);
	}
	else																//no voice, just bell
	{
		strcpy(wavfile[0], A_ALERT);
	    strcpy(wavfile[1], A_ALERT);
	}

	CHECK_ERR();
	
	// We have to save the old context and restore it later, so that we don't interfere with X-Plane and other plugins.
	old_ctx = alcGetCurrentContext();
	
	if (old_ctx)
	{
		//printf("0x%08x: I found someone else's context 0x%08x.\n",XPLMGetMyID(), old_ctx);
		XPLMDebugString("found someone else's context\n");
	}

	//if(old_ctx == NULL)
	{
		//printf("0x%08x: I found no OpenAL, I will be the first to init.\n",XPLMGetMyID());

		// Enumerate Audio Devices
		s = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);

		//try open specific device, if unavailable, revert to default device
		//my_dev = alcOpenDevice("Lautsprecher (Realtek High Definition Audio)");
		my_dev = alcOpenDevice(sAudioDevice);
		if (my_dev == NULL)			
		{
			my_dev = alcOpenDevice(NULL);					
		}
		if (my_dev == NULL)
		{
			XPLMDebugString("Could not open the default OpenAL device.\n");
			return 0;		
		}	
		my_ctx = alcCreateContext(my_dev, NULL);
		if(my_ctx == NULL)
		{
			if(old_ctx)
				alcMakeContextCurrent(old_ctx);
			alcCloseDevice(my_dev);
			my_dev = NULL;
			XPLMDebugString("Could not create a context.\n");
			return 0;				
		}
		
		// Make our context current, so that OpenAL commands affect our, um, stuff.		
		alcMakeContextCurrent(my_ctx);

/*		printf("0x%08x: I created the context.\n",XPLMGetMyID(), my_ctx);

		ALCint		major_version, minor_version;
		const char * al_hw=alcGetString(my_dev,ALC_DEVICE_SPECIFIER	);
		const char * al_ex=alcGetString(my_dev,ALC_EXTENSIONS);
		alcGetIntegerv(NULL,ALC_MAJOR_VERSION,sizeof(major_version),&major_version);
		alcGetIntegerv(NULL,ALC_MINOR_VERSION,sizeof(minor_version),&minor_version);
		
		printf("OpenAL version   : %d.%d\n",major_version,minor_version);
		printf("OpenAL hardware  : %s\n", (al_hw?al_hw:"(none)"));
		printf("OpenAL extensions: %s\n", (al_ex?al_ex:"(none)"));
*/
		CHECK_ERR();
	} 
	//else
	{
		//printf("0x%08x: I found someone else's context 0x%08x.\n",XPLMGetMyID(), old_ctx);
	//	XPLMDebugString("found someone else's context\n");
	}
	
	ALfloat	zero[3] = { 0 } ;
	strcpy(filepath, gPluginDataFile);

	for (i=0;i<2;i++)
	{
		strcpy(buf, filepath);
		strcat(buf,wavfile[i]);
	
		// Generate 1 source and load a buffer of audio.
		alGenSources(1,&snd_src[i]);
		CHECK_ERR();
		snd_buffer[i] = load_wave(buf);
		//printf("0x%08x: Loaded %d from %s\n", XPLMGetMyID(), snd_buffer[i],buf);
		CHECK_ERR();

		// Basic initializtion code to play a sound: specify the buffer the source is playing, as well as some 
		// sound parameters. This doesn't play the sound - it's just one-time initialization.
		alSourcei(snd_src[i],AL_BUFFER,snd_buffer[i]);
		alSourcef(snd_src[i],AL_PITCH,1.0f);
		alSourcef(snd_src[i],AL_GAIN,1.0f);	
		alSourcei(snd_src[i],AL_LOOPING,0);
		alSourcefv(snd_src[i],AL_POSITION, zero);
		alSourcefv(snd_src[i],AL_VELOCITY, zero);
		CHECK_ERR();
	}

	// Finally: put back the old context _if_ we had one.  If old_ctx was null, X-Plane isn't using OpenAL.
	if (old_ctx)
	{
		alcMakeContextCurrent(old_ctx);
	}
		
	return 0.0f;
}


