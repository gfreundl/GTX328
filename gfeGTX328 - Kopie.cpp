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


/*


Version

001		copy template from EDM800
002		implement PNG loader, create textures
003		code Edit routine
004		Edit Mode finished, Flight Timer
005		OAT mode, display invert, redo textures, manual dim
006		resize


Manual:



Pseudocodes:



functions:



parameters to save:
FLT_ID				Callsign
VFR Code			local autority VFR Code
OAT feature			0 or 1
°C / °F				0 = Fahrenheit, 1 = Celsius
manual contrast		0 or 1
AutoFlightTimer		0 = Man, 1 = Clear at every lift off, 2 = Accum counting after lift off
Display				0 = Auto, 1 = black background, 2 = light background
backlight			0 = manual, 1 = X-Plane lighting bus, 2 = X-Plane ambient, 3 = hardware


Open Issues:
- no shut off while init phase
- no STBY or ALT while on ground
- IDENT draw: symbol missing
- switch to STBY or GND on Squat or other sensor, default delay 24sec
- FL should show "___" in GND and STBY
- Alerts
- Contrast Dimmer
- INI Files
- Reply icon stays on after change from ALT to GND ?
- AltMonitor 
	flashes > 100ft deviation
	breaks through other modes
	what is displayed on > 1000ft + deviation
- panel light control (VARIABLE ALREADY THERE)



GIMP: 
find bezel size in px for display size 198x33:  431x114, 680x179 for aspect ratio 3.8 (original size 42mm/159mm)
all less black
medium size digits 4,7,9 need be verified
arrows


not implemented:
- FLT ID setup
- configuration modes
- ADS-B
- Audio Alerts


checks on the real thing:
- take Pilots Manual, photo
- init run 
	duration
	ambient light sensor
	how many steps do we have for Dim and Contrast setting (if available anyway)
	photo hires
- startup into ON, GND, STBY ?
- is there an ON mode (responding w/o ALT encoding), or any mode other than ALT, STBY, GND
- can we switch to ALT, STBY while on ground, is there then an ALT indication
- is there a delay in OFF switching (like in GNS430)
- can be shut off while still initialzing ?
- reply symbol while on GND, STBY ?
- photos on edit: one dash or all dash (or inverted)
- IDENT
	photo on position IDENT
	is IDENT possible while Edit and vice versa
	are keys blocked during 18sec Ident interval
- function of keys 8 & 9 (photos)
- Count Down Timer
	- start default
	- maximum start value
	- CRSR function?
	- EXPIRED signage,record sound, volume
	- switch over to Count Up, stays at 00:00:00 int CountDn ?


- Altitude Trend indicator: photo of up/down arrow in ALT mode (single/ double)
- photo on FL digits 4, 7, 9
- what FL is shown in modes GND, STBY, when is ___ shown
- can Flight Timer in Automatic Mode be START/STOPped and CLRed ?
- test Altitude Monitor 
		- find configured allowed deviation
		- flashing interval
		- what is shown on initial FUNC press
		- what is shown on 0 ft deviation
		- what is shown when exceeding max. deviation
		- record Audio Alerts
- OAT available
	photo pos/neg Celsius/Fahrenheit
	Density Altitude resolution, update interval
- on landing switch to STBY or GND ?

*/


/// Texture stuff
#define BEZEL_FILENAME			"gtx_bezel.png"				//Bezel
#define DISPLAY_FILENAME		"gtx_display.png"			//display background
#define INIT_FILENAME			"gtx_init.png"				//Init Screen
#define RCV_FILENAME			"gtx_receive.png"			//reception light
#define ALT_FILENAME			"gtx_ALT.png"				//
#define GND_FILENAME			"gtx_GND.png"				//
#define STBY_FILENAME			"gtx_STBY.png"				//
#define FL_FILENAME				"gtx_FL.png"				//
#define ___LFILENAME			"gtx__l.png"				//inverted "-"
#define FT_FILENAME				"gtx_ft.png"				//ft symbol for Altitude Monitor
#define _0_MFILENAME			"gtx_0m.png"				//numbers medium size
#define _1_MFILENAME			"gtx_1m.png"
#define _2_MFILENAME			"gtx_2m.png"
#define _3_MFILENAME			"gtx_3m.png"
#define _4_MFILENAME			"gtx_4m.png"
#define _5_MFILENAME			"gtx_5m.png"
#define _6_MFILENAME			"gtx_6m.png"
#define _7_MFILENAME			"gtx_7m.png"
#define _8_MFILENAME			"gtx_8m.png"
#define _9_MFILENAME			"gtx_9m.png"
#define _0_LFILENAME			"gtx_0l.png"				//numbers large size
#define _1_LFILENAME			"gtx_1l.png"
#define _2_LFILENAME			"gtx_2l.png"
#define _3_LFILENAME			"gtx_3l.png"
#define _4_LFILENAME			"gtx_4l.png"
#define _5_LFILENAME			"gtx_5l.png"
#define _6_LFILENAME			"gtx_6l.png"
#define _7_LFILENAME			"gtx_7l.png"
#define _0_IFILENAME			"gtx_0i.png"				//numbers large size inverted
#define _1_IFILENAME			"gtx_1i.png"
#define _2_IFILENAME			"gtx_2i.png"
#define _3_IFILENAME			"gtx_3i.png"
#define _4_IFILENAME			"gtx_4i.png"
#define _5_IFILENAME			"gtx_5i.png"
#define _6_IFILENAME			"gtx_6i.png"
#define _7_IFILENAME			"gtx_7i.png"
#define _8_IFILENAME			"gtx_8i.png"
#define _9_IFILENAME			"gtx_9i.png"
#define _A_SFILENAME			"gtx_as.png" 					//A-Z small size characters
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
#define _A_WFILENAME			"gtx_aw.png" 					//A-Z wide size characters
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
#define posCFILENAME			"gtx_posC.png"
#define negCFILENAME			"gtx_negC.png"

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
#define posCTEXTURE 101													//Celsius positive
#define negCTEXTURE 102		

#define MAX_TEXTURES 200

//Operational Modes and Menu Items
#define M_OFF			0
#define M_ON			1												//Mode A
#define M_TEST			2
#define M_GND			4
#define M_ALT			8												//Mode C
#define M_IDENT			32												//IDENT pressed

#define F_PA			1
#define F_FLTTIME		2
#define	F_ALTMON		3
#define	F_OAT			4
#define F_COUNTUP		5
#define F_COUNTDN		6
#define F_DISPLAY		7

#define R	0.28														//colors for display backlight
#define G	0.72														
#define B	0.10

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
int FSFunc;									//active Function FUNC mode we're in 
int nTick;									//Counter for Init Mode in Flightloop 
int nTickEdit;								//Counter for Timeout for Edit Mode in Flightloop
int gaugeX1;								//gauge position bottom left
int gaugeX2;								//gauge position bottom right
int gaugeY1;								//gauge position top left
int gaugeY2;								//gauge position top right
int FirstCall = TRUE;						//first call since plugin start or power recycle
int BattOn;									//X-Plane electrical current on Bus 1
int AvnOn;									//X-Plane electrical current on Avionics
int FlightTime = 0;							//actual time in the air
int RunTime = 0;							//X-Plane elapsed time
int StartTime = 0;							//time at lift off
int CountUp = 0;							//Count Up Timer
int CountUpStart = 0;						//Count Up Timer Start
int CountUpPause = 0;						//is paused = 1
int CountDn = 0;							//Count Down Timer
int CountDnEnd = 0;							//Count Down Timer Target
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
float fBaro = 0;							//X-Plane current sealevel Baro pressure [inHg]
float fBaroAlt = 0;							//X-Plane current indicated Baro altitude [ft]
float fAltitude = 0;						//X-Plane current elevation above MSL [meter]
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
int Display;								//0=Auto (ambient light), 1=night (black background), 2=day, 3=hardware
int InvertDisp = 1;							//inverted display (1 = night mode)
float InstrumentDim[16];					//X-Plane instrument brightneas
float DimR = R;								//start with full brightness
float DimG = G;
float DimB = B;
float BackLight = 0;						//0=manual, 1=X-Plane lighting bus, 2=X-Plane ambient, 3=hardware
int Func = 1;								//1=Pressure Alt, 2=Flight Time, 3=Alt Monitor, 4=Count up, 5=dn, 
											//6=OAT, 7=Display

int i;										//free Counter, can be used anywhere
int n;										//Counter
int a;										//Counter
int b ;										//reserved Counter
int c;
int k = 1;									//Counter for EGT temp filtering
int u = 0;

float fBat[8];								//bus volts
float fOAT;									//outside air temperature °C

float PanelL, PanelR, PanelB, PanelT;
float DispL;								//Display position
float DispB;
float Scaler;								//scale factor for resizing window
float CodeL;								//Transporter Code position


////////////////////////////////////////// PROTOTYPES //////////////////////////////////////////

/// Prototypes for callbacks etc.
GLvoid LoadTextures(GLvoid);
GLuint LoadPNGTexture(const char *pFileName, int TextureId, int *width, int *height);
int GetTextureID(char);
int DrawGLScene(float x, float y);
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
void KeyOFF(void * refCon);
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
void DrawDisplay();
void DrawIdent();			
void DrawCode();
void DrawReply();
void DrawInit();
void DrawMode();
void DrawTitle();
void DrawFL();
int CalcFL();
void DrawFlightTime();
void DrawAltMonitor();
void DrawOAT();
void DrawCountUp();
void DrawCountDn();
void CountDnEnter(int key);
void DrawDim();
void DrawMText(float x, float y, char Text[20]);


/*
////////////////////////////////////////// XPLM REQUIRED FUNCTIONS //////////////////////////////////////////
*/
PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	/// Handle cross platform differences
	#if IBM
		char *pFileName = "Resources\\Plugins\\GTX328\\";
	#elif LIN
		char *pFileName = "Resources/plugins/gfeGTX328/";
	#else
		char *pFileName = "Resources:Plugins:gfeGTX328:";
	#endif
	/// Setup texture and ini file locations
	XPLMGetSystemPath(gPluginDataFile);
	strcat(gPluginDataFile, pFileName);

	strcpy(outName, "gfeGTX328");
	strcpy(outSig, "gfe.panel.gfeGTX328");
	strcpy(outDesc, "gfe Transponder");

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
	dBat = XPLMFindDataRef("sim/cockpit2/electrical/battery_voltage_indicated_volts");	//indicated volts (bus volts, not just battery!)
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
	dInstrumentDim = XPLMFindDataRef("sim/cockpit2/electrical/instrument_brightness_ratio");
	dIdentEnter = XPLMFindCommand("sim/transponder/transponder_ident");

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
	gKeyOFF = XPLMRegisterHotKey(XPLM_VK_DOWN, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-Down", KeyOFF, NULL);
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
/*	XPLMUnregisterDrawCallback(
					gfeFS450DrawCallback,
					xplm_Phase_Gauges, 
					0,
					NULL);	
*/	
	WriteINIFile();

	XPLMUnregisterHotKey(gHotKey);
	
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
	XPLMDestroyWindow(ggfePanelDisplayWindow);
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
	float Red = XPLMGetDataf (RED);
	float Green = XPLMGetDataf (GREEN);
	float Blue = XPLMGetDataf (BLUE);
	int	PanelWindowLeft, PanelWindowRight, PanelWindowBottom, PanelWindowTop;
    float PanelWidth, PanelHeight;

	/// Do the actual drawing, but only if our window is active
	if (gfeDisplayPanelWindow)
	{
		//Set up initializing
		BattOn = XPLMGetDatai(dBattOn);
		AvnOn = XPLMGetDatai(dAvnOn);
		if (!AvnOn)
		{
			FirstCall = TRUE;											//reset at each battery switch recycle
			FSMode = M_OFF;
		}
		if (FirstCall)													//reset on first Call since Plugin Start or Battery Switch on/off
		{
			OpenINIFile();													
			FirstCall = FALSE;
			FSMode = M_TEST;
			nTick = 0;													//reset counter after restart
			b = 0;
			c = 0;
			//CountDn = -1;												//to prevent auto run after startup					
			CountUpPause = 1;
			InvertDisp = 0;
			DimR = R;													//reset full brightness
			DimG = G;
			DimB = B;
			AltAlert = 0;
			ShowAltDev = 0;

			Display = 0;												// <------ TEST ONLY
			BackLight = 2;												// <------ TEST ONLY

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
		PaintTexture2(BEZEL_TEXTURE, 0, 0, PanelL, PanelB);

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
		i = XPLMGetDatavf(dInstrumentDim, InstrumentDim, 0, 16);

		FL = CalcFL();

		if ((AvnOn) && (FSMode != M_OFF))
		{
			if (FSMode == M_TEST) 										//Selftest sequence - Init page
			{
				DrawDisplay();
				DrawInit();												//init screen is always bright						
			}
			else	
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

				if (BackLight == 1)										//X-Plane lighting bus controls display
				{
					DimR = InstrumentDim[0] * R;						
					DimG = InstrumentDim[0] * G;
					DimB = InstrumentDim[0] * B;
				}
				else if (BackLight == 2)								//X-Plane ambient light controls display
				{
					DimR = R * Red;
					DimG = G * Green;
					DimB = B * Blue;
				}
				glColor3f(DimR, DimG, DimB);
				DrawDisplay();

				if (OnGround)
				{
					SquatFlag = OnGround;
					if ((FSMode & M_GND) == 0)							//test for M_GND
					{
						FSMode = FSMode + M_GND;						//set M_GND flag
					}		
					FSMode = FSMode & (M_ALT ^ 2047);					//clear M_ALT flag
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
				}
				if (CountUpPause == 0)									//while not paused
				{
					CountUp = RunTime - CountUpStart;
				}
				if (CountDnEnd > RunTime)								//Count Down routine
				{
					CountDn = CountDnEnd - RunTime;
				}
				if (CountDn == 0)										//on end of Count Down, start Count up
				{
					//not yet working
					//CountDn = -1;										//to prevent this to run after Count Dn reversal
					//CountUpStart = RunTime;							//Start Count Up
					//CountUpPause = 0;
				}

				if ((FSMode & M_GND) == M_GND)
				{
					PaintTexture2(GND_TEXTURE, 0, 0, DispL + 1*Scaler, DispB + 14*Scaler);
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
			KeyOFF(0);
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
		FSMode = FSMode | M_IDENT;										//set M_IDENT bit
		Ident = 1;
		XPLMSetDatai(dIdent, Ident);	
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
	if ((FSMode & M_ON) == 0)
	{
		FSMode = M_ON;
		FirstCall = TRUE;
	}
}

void KeyALT(void * refCon)
{
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
}

void KeySTBY(void * refCon)
{
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
}

void KeyOFF(void * refCon)
{
	FSMode = M_OFF;
	Mode = 0;
	if (XPLMCanWriteDataRef(dMode))
	{
		XPLMSetDatai(dMode, Mode);
	}
}

void KeyFUNC(void * refCon)
{
	Func++;
	if (Func > 7)
	{
		Func = 1;
	}
	if ((Func == 7) && (BackLight > 0))
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
		CountDnEnd = RunTime + CountDn;
		//CountDnEnd = RunTime + 3600;
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
	float FloatVals[8];	
	char s[11];
	char sa[11];
	char sb[11];


	nTick = nTick + 1;
	nTickEdit++;	

	if (nTickEdit > 20)															//counter for 5sec timeout in Edit Mode
	{
		iCursor = -1;
	}

	if (nTick < 30)														//start with self test sequence
	{		
		FSMode = M_TEST;
	}
	else if (nTick == 30)
	{	
		FSMode = M_ON;
		Func = F_PA;													//default to PRESSUE ALT display
	}
	else if (nTick > 30)
	{
		if ((nTick % 4) == 0)											//every second
		{
			Deviation = (FL - MonFL) * 100;
			if ((abs(Deviation) > 100)	&& (abs(Deviation) < 1100))		//if 100 < altitude deviation < 1100ft
			{
				AltAlert = 1;											//set off alert
			}
			else 
			{
				AltAlert = 0;
				ShowAltDev = 1;
			}
		}
		if ((nTick % 8) == 0)											//every 2 sec refresh current fuel state
		{
			RunTime = XPLMGetElapsedTime();
			if ((nTick % 40) == 0)										//every 10 sec save current RemFuel into File
			{			
				WriteINIFile();											//write back variables
			}
		}
		if (AltAlert)
		{
			ShowAltDev = !ShowAltDev;								//every 0.25 sec flash Alert
		}
	}

	return 0.25;																// Return time interval after that we want to be called again
}																				// if this interval is changed, all nTick based timers will change!

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
	if (!LoadPNGTexture(_0_IFILENAME, _0_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _0_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_1_IFILENAME, _1_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _1_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_2_IFILENAME, _2_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _2_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_3_IFILENAME, _3_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _3_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_4_IFILENAME, _4_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _4_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_5_IFILENAME, _5_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _5_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_6_IFILENAME, _6_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _6_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_7_IFILENAME, _7_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _7_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_8_IFILENAME, _8_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _8_ITEXTURE failed to load\n");
	if (!LoadPNGTexture(_9_IFILENAME, _9_ITEXTURE, 0, 0))	XPLMDebugString("Panel texture _9_ITEXTURE failed to load\n");
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
	if (!LoadPNGTexture(_Q_SFILENAME, _Q_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Q_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_R_SFILENAME, _R_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _R_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_S_SFILENAME, _S_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _S_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_T_SFILENAME, _T_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _T_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_U_SFILENAME, _U_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _U_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_V_SFILENAME, _V_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _V_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_W_SFILENAME, _W_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _W_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_X_SFILENAME, _X_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _X_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_Y_SFILENAME, _Y_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Y_STEXTURE failed to load\n");
	if (!LoadPNGTexture(_Z_SFILENAME, _Z_STEXTURE, 0, 0))	XPLMDebugString("Panel texture _Z_STEXTURE failed to load\n");
	if (!LoadPNGTexture(___SFILENAME, ___STEXTURE, 0, 0))	XPLMDebugString("Panel texture ___STEXTURE failed to load\n");
	if (!LoadPNGTexture(_A_WFILENAME, _A_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _A_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_B_WFILENAME, _B_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _B_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_C_WFILENAME, _C_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _C_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_D_WFILENAME, _D_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _D_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_E_WFILENAME, _E_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _E_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_F_WFILENAME, _F_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _F_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_G_WFILENAME, _G_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _G_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_H_WFILENAME, _H_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _H_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_I_WFILENAME, _I_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _I_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_K_WFILENAME, _K_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _K_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_L_WFILENAME, _L_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _L_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_M_WFILENAME, _M_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _M_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_N_WFILENAME, _N_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _N_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_O_WFILENAME, _O_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _O_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_P_WFILENAME, _P_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _P_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_Q_WFILENAME, _Q_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Q_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_R_WFILENAME, _R_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _R_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_S_WFILENAME, _S_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _S_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_T_WFILENAME, _T_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _T_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_U_WFILENAME, _U_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _U_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_V_WFILENAME, _V_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _V_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_W_WFILENAME, _W_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _W_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_X_WFILENAME, _X_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _X_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_Y_WFILENAME, _Y_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Y_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(_Z_WFILENAME, _Z_WTEXTURE, 0, 0))	XPLMDebugString("Panel texture _Z_WTEXTURE failed to load\n");
	if (!LoadPNGTexture(colMFILENAME, colMTEXTURE, 0, 0))	XPLMDebugString("Panel texture colMTEXTURE failed to load\n");
	if (!LoadPNGTexture(FT_FILENAME, FT_TEXTURE, 0, 0))		XPLMDebugString("Panel texture FT_TEXTURE failed to load\n");
	if (!LoadPNGTexture(OFF_FILENAME, OFF_TEXTURE, 0, 0))	XPLMDebugString("Panel texture OFF_TEXTURE failed to load\n");
	if (!LoadPNGTexture(dashFILENAME, dashTEXTURE, 0, 0))	XPLMDebugString("Panel texture dashTEXTURE failed to load\n");
	if (!LoadPNGTexture(frameFILENAME, frameTEXTURE, 0, 0))	XPLMDebugString("Panel texture frameTEXTURE failed to load\n");
	if (!LoadPNGTexture(posCFILENAME, posCTEXTURE, 0, 0))	XPLMDebugString("Panel texture posCTEXTURE failed to load\n");
	if (!LoadPNGTexture(negCFILENAME, negCTEXTURE, 0, 0))	XPLMDebugString("Panel texture negCTEXTURE failed to load\n");
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


//This function is Windows only
int OpenINIFile()
{
	char INIFile[255];
	char sLeft[255];
	char sRight[255];	
	char sBottom[255];
	char sTop[255];
	char sVFRCode[5];
	strcpy(INIFile, gPluginDataFile);
	strcat(INIFile, "gtx328.ini");
	GetPrivateProfileString("common","Left","100",sLeft,255,INIFile);
	GetPrivateProfileString("common","Right","860",sRight,255,INIFile);
	GetPrivateProfileString("common","Bottom","100",sBottom,255,INIFile);
	GetPrivateProfileString("common","Top","300",sTop,255,INIFile);
	GetPrivateProfileString("common","VFRCode","7000",sVFRCode,5,INIFile);
	gaugeX1 = atof(sLeft);
	gaugeX2 = atof(sRight);
	gaugeY1 = atof(sBottom);
	gaugeY2 = atof(sTop);
	VFRCode = atoi(sVFRCode);
	return TRUE;
}


//This function is Windows only
int WriteINIFile()
{
	char INIFile[255];
	char sLeft[255];
	char sRight[255];	
	char sBottom[255];
	char sTop[255];
	char sFuelUnit[255];
	char sStartFuel[255];

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

//draw Display Background
void DrawDisplay()
{
	if (InvertDisp)
	{
		glColor3f(0, 0, 0);												//draw display black
		PaintTexture2(DISPLAY_TEXTURE, 0, 0, DispL, DispB);
		glColor3f(DimR, DimG, DimB);									//switch back to colored
	}
	else
	{
		glColor3f(DimR, DimG, DimB);									//draw display colored
		PaintTexture2(DISPLAY_TEXTURE, 0, 0, DispL, DispB);
		glColor3f(0, 0, 0);												//switch back
	}	
}

//draw Init Page
void DrawInit()
{
	PaintTexture2(INIT_TEXTURE, 0, 0, DispL, DispB);
}

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

//draw reply symbol (R)
void DrawReply()
{
	if ((Lit) && ((FSMode & M_ALT) != 0))
	{
		PaintTexture2(RCV_TEXTURE, 0, 0, DispL + 1 * Scaler, DispB);
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
	else if (Func == F_COUNTDN)
	{
		strcpy(title, "  COUNT DOWN");
	}
	else if (Func == F_DISPLAY)
	{
		strcpy(title, "    DISPLAY  ");
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
	int offset = 0;
	sprintf(cFL, "%03u", FL);
	for  (i=0; i<3; i++)												//left digits												
	{
		a = GetTextureID(cFL[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 3
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 153 * Scaler, DispB + 5 * Scaler);
	}
	PaintTexture2(FL_TEXTURE, 0, 0, DispL + 137 * Scaler, DispB + 5 * Scaler);
}

//calculate flight level from current baro setting and indicated altitude
int CalcFL()
{ return floor(((fAltitude * 3.28 +  ((29.92 - fBaro) * 1000)) / 100) + 0.5); }	

//draw time
void DrawFlightTime()
{
	int h, m, s;
	char cTime[9];
	int offset = 0;

	h = FlightTime / 3600;
	m = (FlightTime % 3600) / 60;
	s = (FlightTime % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}

}

//draw Altitude Monitor 
void DrawAltMonitor()
{
	int offset = 0;
	char cDeviation[10];
	if (MonFL > 0)
	{
		Deviation = (FL - MonFL) * 100;
		sprintf(cDeviation, "%4u \n", abs(Deviation));
		for  (i=0; i<4; i++)											//left digits												
		{
			a = GetTextureID(cDeviation[i]) - 10;						//get smaller textures _?_MTEXTURE
			if (i > 0)													//only for digits 1 to 3
			{
				offset = offset + 1 * Scaler + glwidth * Scaler;		//get last texture width for character spacing 	
			}
			PaintTexture2(a, 0, 0, DispL + offset + 124 * Scaler, DispB + 5 * Scaler);
		}
		offset = offset + 1 * Scaler + glwidth * Scaler;
		PaintTexture2(FT_TEXTURE, 0, 0, DispL + offset + 124 * Scaler, DispB + 5 * Scaler);
		if ((Deviation > 0) && ShowAltDev)								//care for flashing flag
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
		for  (i=0; i<5; i++)												//left digits												
		{
			a = GetTextureID(cDeviation[i]);								//get smaller textures _?_MTEXTURE
			if (i > 0)														//only for digits 2 to 5
			{
				offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
			}
			PaintTexture2(a, 0, 0, DispL + offset + 164 * Scaler, DispB + 5 * Scaler);
		}
	}
	else
	{
		PaintTexture2(OFF_TEXTURE, 0, 0, DispL + 150 * Scaler, DispB + 5 * Scaler);
	}
}

//draw outside air temperature and density altitude
void DrawOAT()
{
	int offset = 0;
	char cOAT[4];
	char cDA[6];
	char cText[5];
	int iDA = ((fAltitude * 3.28 +  ((29.92 - fBaro) * 1000)) * 1.24) + (118.8 * fOAT) - 1782 ;
	int iOAT = abs(fOAT);

	sprintf(cOAT, "%3u", iOAT);
	for  (i=0; i<2; i++)																							
	{
		a = GetTextureID(cFL[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 3
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 160 * Scaler, DispB + 20 * Scaler);
	}
	if (fOAT > 0)
	{
		PaintTexture2(posCTEXTURE, 0, 0, DispL + offset + 169 * Scaler, DispB + 20 * Scaler);
	}
 	else
	{
		//PaintTexture2(negTEXTURE, 0, 0, DispL + offset + 145 * Scaler, DispB + 20 * Scaler);
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
	sprintf(cDA, "%5u", iDA);
	for  (i=0; i<5; i++)																								
	{
		a = GetTextureID(cDA[i]);										//get smaller textures _?_MTEXTURE
		a = a - 10;
		if (i > 0)														//only for digits 2 to 5
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 121 * Scaler, DispB + 5 * Scaler);
	}
	PaintTexture2(FT_TEXTURE, 0, 0, DispL + offset + 9 * Scaler + 121 * Scaler, DispB + 5 * Scaler);
}

void DrawCountUp()
{
	int h, m, s;
	char cTime[9];
	int offset = 0;

	h = CountUp / 3600;
	m = (CountUp % 3600) / 60;
	s = (CountUp % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}

}

void DrawCountDn()
{
	int h, m, s;
	char cTime[9];
	int offset = 0;

	h = CountDn / 3600;
	m = (CountDn % 3600) / 60;
	s = (CountDn % 3600) % 60;

	sprintf(cTime, "%02d:%02d:%02d", h, m, s);
	for  (i=0; i<9; i++)												//left digits												
	{
		a = GetTextureID(cTime[i]) - 10;								//get smaller textures _?_MTEXTURE
		if (i > 0)														//only for digits 2 to 8
		{
			offset = offset + 1 * Scaler + glwidth * Scaler;			//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, DispL + offset + 129 * Scaler, DispB + 5 * Scaler);
	}
}

//build Count Down starting point
void CountDnEnter(int key)
{
	static int h, m, s;


	switch (CountDnCursor)
	{
	case 0:
		h = 0;
		h = 10 * key;
		break;
	case 1:
		h = h + key;
		break;
	case 2:
		m = 0;
		if (key < 7)
		{
			m = 10 * key;
		}
		break;
	case 3:
		m = m + key;
		break;
	case 4:
		s = 0;
		if (key < 7)
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
	PaintTexture2(INIT_TEXTURE, 0, 0, DispL + 1 * Scaler, DispB + 22 * Scaler);
}

//draw Text Array with MTEXTURE
void DrawMText(float x, float y, char Text[20])
{
}


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