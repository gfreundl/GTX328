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
#include "BitmapSupport.h"			//Bitmap Loader
#include "XPLMProcessing.h"			//Timed Processing
#include <windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "png.h"   //not found when Configuration "Debug"
#include "zlib.h"


/*


Version

001				copy template from EDM800




next:	

exakte Positionen von Code, Mode etc.
Mode  höher 3px, links 2px





Manual:



Pseudocodes:



functions:



parameterws to save:
FLT_ID			active/inactive Callsign	
FLT_ID			Callsign
PWR_UP ENTRY	
VFR Code		regional VFR Code


Open Issues:
- no shut off while init phase
- ON when on resets to init phase
- timeout on cursor reset still missing
- no Mode writeback to XPL


GIMP: 
inverted characters
find bezel size in px for display size 198x33:  760x200 for aspect ratio 3.8 (original 42/159)
all less black


checks on the real thing:
- init run duration
- is there an ON mode (responding w/o ALT encoding), or any mode other than ALT, STBY, GND
- can we switch to ALT, STBY while on ground, is there then an ALT indication
- is there a delay in OFF switching (like in GNS430)



*/

/// Texture stuff
#define BEZEL_FILENAME			"gtx_bezel.png"				//Bezel
#define DISPLAY_FILENAME		"gtx_display.png"				//display background
#define INIT_FILENAME			"gtx_init.png"				//Init Screen
#define RCV_FILENAME			"gtx_receive.png"			//reception light
//#define _FILENAME				"gtx_.png"					//
#define ALT_FILENAME			"gtx_ALT.png"					//
#define GND_FILENAME			"gtx_GND.png"					//
#define STBY_FILENAME			"gtx_STBY.png"					//
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
#define _8_LFILENAME			"gtx_8l.png"
#define _9_LFILENAME			"gtx_9l.png"
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

#define BEZEL_TEXTURE	1
#define DISPLAY_TEXTURE 2
#define INIT_TEXTURE	3
#define RCV_TEXTURE	4
#define ALT_TEXTURE	5
#define GND_TEXTURE	6
#define STBY_TEXTURE 7	
#define _0_MTEXTURE 10							//numbers medium size
#define _1_MTEXTURE 11
#define _2_MTEXTURE 12
#define _3_MTEXTURE 13
#define _4_MTEXTURE 14
#define _5_MTEXTURE 15
#define _6_MTEXTURE 16
#define _7_MTEXTURE 17
#define _8_MTEXTURE 18
#define _9_MTEXTURE 19
#define _0_LTEXTURE 20							//numbers large size
#define _1_LTEXTURE 21
#define _2_LTEXTURE 22
#define _3_LTEXTURE 23
#define _4_LTEXTURE 24
#define _5_LTEXTURE 25
#define _6_LTEXTURE 26
#define _7_LTEXTURE 27
#define _8_LTEXTURE 28
#define _9_LTEXTURE 29
#define _0_ITEXTURE 30							//numbers large size inverted
#define _1_ITEXTURE 31
#define _2_ITEXTURE 32
#define _3_ITEXTURE 33
#define _4_ITEXTURE 34
#define _5_ITEXTURE 35
#define _6_ITEXTURE 36
#define _7_ITEXTURE 37
#define _8_ITEXTURE 38
#define _9_ITEXTURE 39
#define _A_STEXTURE 50							//A-Z small size characters
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
#define _A_WTEXTURE 80							//A-Z wide size characters
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
#define ___TEXTURE 100

#define MAX_TEXTURES 200

//Operational Modes and Menu Items
#define M_TEST			2
#define M_GND			4
#define	M_PA			64
#define M_TIME			128
#define M_ALT			8
#define M_CNTDN			256
#define M_CNTUP			512
#define M_ON			1
#define M_OFF			0
#define M_IDENT			32
//#define M_STBY			8
#define M_ENTER			1024

#define M_0				0									//cursor position
#define M_1				1	
#define M_2				2
#define M_3				3

XPLMDataRef	RED = NULL, GREEN = NULL, BLUE = NULL;
XPLMDataRef dFlightTime = NULL;
XPLMDataRef	dBattOn = NULL;
XPLMDataRef	dAvnOn = NULL;
XPLMDataRef dTime = NULL;
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

XPLMTextureID gTexture[MAX_TEXTURES];
XPLMWindowID ggfePanelDisplayWindow = NULL;
XPLMPluginID gPlugin;

XPLMHotKeyID gHotKey = NULL;
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
char gPluginDataFile[255];					//

char Symbol[4];								//content for LED
int FSMode;									//active Operation Mode we're in
int FSStep;									//active step in sequence 
int nTick;									//Counter for Seconds in Flightloop
int nTick2;									//Counter for Timeout for Edit Mode in Flightloop
char cText0[4];								//Text in Row 1
char cText1[5];								//Text in Row 1
char cText2[4];								//Text in Row 2
int gaugeX1;								//gauge position bottom left
int gaugeX2;								//gauge position bottom right
int gaugeY1;								//gauge position top left
int gaugeY2;								//gauge position top right
int FirstCall = TRUE;						//first call since plugin start or power recycle
int BattOn;									//electrical current on Bus 1
int AvnOn;									//electrical current on Avionics
int RealMode = 0;							//1: work with fuel added on startup, 0: ignore added fuel, work with XPL fuel state data only
float RunTime;							
float DeltaTime;
float LastTime = 0;
int Code = 0;								//Transponder Code
char cCode[5];								// Transponder Code			
int Ident = 0;								//Transponder Ident on/off
int Lit = 0;								//Transponder lit
int Mode = 0;								//Transponder Mode
int Fail = 0;								//Transponder Fail
float fBaro = 0;							//current actual Baro pressue
float fBaroAlt = 0;							//current indicated Baro altitide
int OnGround = 0;							//airplane is on ground
int iCursor = 0;							//cursor active  and position

int i;										//free Counter, can be used anywhere
int n;										//Counter
int a;										//Counter
int b ;										//reserved Counter
int c;
int k = 1;									//Counter for EGT temp filtering
int u =0;

float fTime = 0;							//seconds since sim startup
int iTime = 0;								//fractions of fTime multiplied with 1000 for timing purposes
float fBat[8];								//bus volts
float fOAT;									//outside air temperature °C

float PanelL, PanelR, PanelB, PanelT;
float DispWidth = 199;
float DispL;								//Display position
float DispB;
float Scaler;
float CodeL;								//Transporter Code position


////////////////////////////////////////// PROTOTYPES //////////////////////////////////////////

/// Prototypes for callbacks etc.
GLvoid LoadTextures(GLvoid);
GLuint LoadPNGTexture(const char *pFileName, int TextureId, int *width, int *height);
int GetTextureID(char);
int DrawGLScene(float x, float y);
void HotKey(void * refCon);
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
void Draw_R();
void DrawInit();
void DrawMode();
int CalcFL();


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
		gaugeX1 = 100;															//gauge position left
		gaugeX2 = 531;															//gauge position right
		gaugeY1 = 100;															//gauge position bottom
		gaugeY2 = 214;															//gauge position top
		RealMode = 0;
	}
	
	/// Create window, setup datarefs and register  hotkeys
	ggfePanelDisplayWindow = XPLMCreateWindow(gaugeX1, gaugeY2, gaugeX2, gaugeY1, 1, gfePanelWindowCallback, gfePanelKeyCallback, gfePanelMouseClickCallback, NULL);

	dBattOn = XPLMFindDataRef("sim/cockpit/electrical/battery_on"); 
	dAvnOn = XPLMFindDataRef("sim/cockpit/electrical/avionics_on"); 
	dFlightTime = XPLMFindDataRef("sim/time/total_flight_time_sec");
	RED = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_r");
	GREEN = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_g");
	BLUE = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_b");
	dTime = XPLMFindDataRef("sim/time/total_running_time_sec");											//sim time
	dBat = XPLMFindDataRef("sim/cockpit2/electrical/battery_voltage_indicated_volts");					//indicated volts (bus volts, not just battery!)
	dOAT = XPLMFindDataRef("sim/cockpit2/temperature/outside_air_temp_degc");
	dCode = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
	dIdent = XPLMFindDataRef("sim/cockpit/radios/transponder_id");
	dLit = XPLMFindDataRef("sim/cockpit/radios/transponder_light");
	dMode = XPLMFindDataRef("sim/cockpit/radios/transponder_mode");
	dFail = XPLMFindDataRef("sim/operation/failures/rel_xpndr");
	dBaro = XPLMFindDataRef("sim/weather/barometer_current_inhg");
	dBaroAlt = XPLMFindDataRef("sim/flightmodel/misc/h_ind2");
	dOnGround = XPLMFindDataRef("sim/flightmodel/failures/onground_any");

	gHotKey = XPLMRegisterHotKey(XPLM_VK_F4, xplm_UpFlag + xplm_ControlFlag + xplm_ShiftFlag, "Ctrl-Shift-F4", HotKey, NULL);
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

		BattOn = XPLMGetDatai(dBattOn);
		AvnOn = XPLMGetDatai(dAvnOn);
		if (!AvnOn)
		{
			FirstCall = TRUE;													//reset at each battery switch recycle
			FSMode = M_OFF;
		}
		if (FirstCall)															//reset on first Call since Plugin Start or Battery Switch on/off
		{
			if (RealMode == 1)
			{
				OpenINIFile();													
			}
			else
			{
				FirstCall = FALSE;
				FSMode = M_TEST;
				nTick = 0;														//reset counter after restart
				b = 0;
				c = 0;
			}
		}
		else
		{
			nTick = nTick;														//just a dummy for debug
		}

		// Need to find out where our window is
		XPLMGetWindowGeometry(ggfePanelDisplayWindow, &PanelWindowLeft, &PanelWindowTop, &PanelWindowRight, &PanelWindowBottom);
		/// Tell Xplane what we are doing
		XPLMSetGraphicsState(0/*Fog*/, 1/*TexUnits*/, 0/*Lighting*/, 0/*AlphaTesting*/, 1/*AlphaBlending*/, 0/*DepthTesting*/, 0/*DepthWriting*/);
		/// Handle day/night
		glColor3f(Red, Green, Blue);
		/// Setup our panel and gauge relative to our window
		PanelL = PanelWindowLeft; PanelR = PanelWindowRight; PanelB = PanelWindowBottom; PanelT = PanelWindowTop;
		// Original Panel Size = 760x200, so in case of resize, scale everything accordingly
		Scaler = (PanelR-PanelL)/431;										

		// Row Coordinates
		//BezelWidth = 
		DispWidth = 199 * Scaler;
		DispL = PanelL + ((PanelR-PanelL) * 0.32 * Scaler);
		DispB = PanelB + ((PanelT-PanelB) * 0.53 * Scaler);
		CodeL = DispL + 50 * Scaler;		

		// Draw Bezel
		XPLMBindTexture2d(gTexture[BEZEL_TEXTURE], 0);
		glBegin(GL_QUADS);
				glTexCoord2f(1, 0.0f); glVertex2f(PanelR, PanelB);	// Bottom Right Of The Texture and Quad
				glTexCoord2f(0, 0.0f); glVertex2f(PanelL, PanelB);	// Bottom Left Of The Texture and Quad		
				glTexCoord2f(0, 1.0f); glVertex2f(PanelL, PanelT);	// Top Left Of The Texture and Quad
				glTexCoord2f(1, 1.0f); glVertex2f(PanelR, PanelT);	// Top Right Of The Texture and Quad
		glEnd();

		//get actual data from XPL
		i = XPLMGetDatavf(dBat, fBat, 0, 8);
		fOAT = XPLMGetDataf(dOAT);
		Code = XPLMGetDatai(dCode);
		Ident = XPLMGetDatai(dIdent);
		Lit = XPLMGetDatai(dLit);
		Mode = XPLMGetDatai(dMode);
		Fail = XPLMGetDatai(dFail);
		fBaro = XPLMGetDatai(dBaro);
		fBaroAlt = XPLMGetDatai(dBaroAlt);
		OnGround = XPLMGetDatai(dOnGround);

		CalcFL();

		if ((AvnOn) && (FSMode != M_OFF))
		{
			DrawDisplay();
			if (FSMode == M_TEST) 								//Selftest sequence - Init page
			{
				DrawInit();																		
			}
			else	
			{
				if (OnGround)
				{
					if ((FSMode & M_GND) == 0)
					{
						FSMode = FSMode + M_GND;
					}		
				}
				else
				{
					FSMode = FSMode & (M_GND ^ 2047);
				}

				if ((FSMode & M_GND) == M_GND)
				{
					PaintTexture2(GND_TEXTURE, 0, 0, DispL + 4*Scaler, DispB + 10*Scaler);
				}
				else if ((FSMode & M_ALT) == M_ALT)
				{
					PaintTexture2(ALT_TEXTURE, 0, 0, DispL + 4*Scaler, DispB + 10*Scaler);
				}
				else 
				{
					PaintTexture2(STBY_TEXTURE, 0, 0, DispL + 4*Scaler, DispB + 10*Scaler);
				}

				if ((Code >= 0) || (Code < 7778))					//accept only valid codes from X-Plane 
				{
					sprintf(cCode, "%04d \n", Code);
				}

				//now draw everything
				DrawCode();
				Draw_R();
				if (Ident) { DrawIdent(); }			
				
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

	switch(inMouse) {
	case xplm_MouseDown:
		if (CoordInRect(x, y, Left, Top, Right, Top-15))						// test for the mouse anywhere in the top part of the window
		{		
			dX = x - Left;
			dY = y - Top;
			Weight = Right - Left;
			Height = Bottom - Top;
			gDragging = 1;
		}
		if (CoordInRect(x, y, Left+(0.2*(Right-Left)), Top-(0.9*(Top-Bottom)), Left+(0.3*(Right-Left)), Top-(1.0*(Top-Bottom))))	
		{
		}
		if (CoordInRect(x, y, Left+(0.7*(Right-Left)), Top-(0.9*(Top-Bottom)), Left+(0.8*(Right-Left)), Top-(1.0*(Top-Bottom))))	
		{
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

/// Toggle between display and non display
void	HotKey(void * refCon)
{
	gfeDisplayPanelWindow = !gfeDisplayPanelWindow;
}

void Key0(void * refCon)							//why won't Keys 0 and 7 work? check on different machines!
{
	EditCode(0);
}

void Key7(void * refCon)
{
	EditCode(0);
}

void Key8(void * refCon)
{
	EditCode(0);
}

void Key1(void * refCon)
{
	EditCode(1);
}

void Key2(void * refCon)
{
	EditCode(2);
}

void Key3(void * refCon)
{
	EditCode(3);
}

void Key4(void * refCon)
{
	EditCode(4);
}

void Key5(void * refCon)
{
	EditCode(5);
}

void Key6(void * refCon)
{
	EditCode(6);
}

void Key9(void * refCon)
{
	EditCode(7);
}


void KeyIDENT(void * refCon)
{
	if (FSMode != M_OFF)
	{
		FSMode = FSMode + M_IDENT;
	}
}

void KeyVFR(void * refCon)
{
	if (XPLMCanWriteDataRef(dCode))
	{
		
		XPLMSetDatai(dCode, 7000);
	}
}

void KeyON(void * refCon)
{
	FSMode = M_ON;
	FirstCall = TRUE;
}

void KeyALT(void * refCon)
{
	if (FSMode != M_OFF) 
	{
		FSMode = FSMode | M_ALT;
	}
}

void KeySTBY(void * refCon)
{
	if (FSMode != M_OFF)
	{
		FSMode = FSMode & (M_ALT ^ 2047);
		Mode = 1;
		if (XPLMCanWriteDataRef(dMode))
		{
			XPLMSetDatai(dMode, Mode);
		}
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

}

void KeyCRSR(void * refCon)
{

}

void KeySTART(void * refCon)
{

}

void KeyCLR(void * refCon)
{

}
//Edit Transponder Code
void EditCode(int key)
{
	char d;
	
	//start timeout counter
	nTick2 = 0;

	iCursor++;															//move cursor to first digit
	d = (char)(((int)'0') + key);										//add 48 as starting point in ASCII table
	cCode[iCursor - 1] = d;												//set new character at cursor position
	Code = atoi(cCode);													//convert to integer...
	if (XPLMCanWriteDataRef(dCode))										//...in order to write it back to X-Plane
	{	
		XPLMSetDatai(dCode, Code);
	}
	//timeout? next digit
	
	if (iCursor == 4)													//fourth digit? reset Mode counter 
	{
		iCursor = 0;
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
	nTick2 = nTick2 + 1;	

	if (nTick < 30)																//start with self test sequence
	{		
		FSMode = M_TEST;
	}
	else if (nTick == 30)
	{	
		FSMode = M_ON;
	}
	else if (nTick > 30)
	{
		if ((RealMode == 1) && ((nTick % 8) == 0))								// every 2 sec refresh current fuel state
		{
			RunTime = XPLMGetElapsedTime();
			DeltaTime = RunTime - LastTime;
			LastTime = RunTime;


			if ((nTick % 40) == 0)												//every 10 sec save current RemFuel into File
			{			
				WriteINIFile();													//write back variables
			}
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
	if (!LoadPNGTexture(_8_LFILENAME, _8_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _8_LTEXTURE failed to load\n");
	if (!LoadPNGTexture(_9_LFILENAME, _9_LTEXTURE, 0, 0))	XPLMDebugString("Panel texture _9_LTEXTURE failed to load\n");
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
}


//get id of texture corresponding to character
int GetTextureID(char irg)
{
	if (irg == ' ') return ___TEXTURE;
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
	char sFillMain[255];
	char sFillTop[255];
	char sLeft[255];
	char sRight[255];	
	char sBottom[255];
	char sTop[255];
	char sFuelUnit[255];
	char sIndexRate[255];
	char sRealMode[255];
	char sStartFuel[255];
	char sFuelThres[255];
	char sTimeThres[255];
	strcpy(INIFile, gPluginDataFile);
	strcat(INIFile, "gtx328.ini");
	GetPrivateProfileString("common","Left","100",sLeft,255,INIFile);
	GetPrivateProfileString("common","Right","860",sRight,255,INIFile);
	GetPrivateProfileString("common","Bottom","100",sBottom,255,INIFile);
	GetPrivateProfileString("common","Top","300",sTop,255,INIFile);
	GetPrivateProfileString("common","RealMode","0",sRealMode,255,INIFile);
	gaugeX1 = atof(sLeft);
	gaugeX2 = atof(sRight);
	gaugeY1 = atof(sBottom);
	gaugeY2 = atof(sTop);
	RealMode = atoi(sRealMode);
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
	char sRealMode[255];
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
	if (w == 0) {w = glwidth;}										//default to original texture size
	if (h == 0) {h = glheight;}
	glBegin(GL_QUADS);
			glTexCoord2f(1, 0.0f); glVertex2f(x + w, y);			// Bottom Right 
			glTexCoord2f(0, 0.0f); glVertex2f(x, y);				// Bottom Left 		
			glTexCoord2f(0, 1.0f); glVertex2f(x, y + h);			// Top Left 
			glTexCoord2f(1, 1.0f); glVertex2f(x + w, y + h);		// Top Right 
	glEnd();
	return TRUE;
}


//calculate flight level from current baro setting and indicated altitude
int CalcFL()
{ return (fBaroAlt - (((fBaro * 33.86) - 1013) * 27) / 100); }


//draw Display Background
void DrawDisplay()
{
	PaintTexture2(DISPLAY_TEXTURE, 0, 0, DispL, DispB);
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
			ModeOffset = ModeOffset + glwidth;
		}
		PaintTexture2(a, 0, 0, DispL + ModeOffset, 1.2 * DispB);
	}
}


//draw Transponder Code
void DrawCode()
{
	int CodeLoffset = 0;

	for  (i=0; i<4; i++)												//left digits												
	{
		if (iCursor == 1)												//if cursor active
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
			CodeLoffset = CodeLoffset + glwidth;						//get last texture width for character spacing 	
		}
		PaintTexture2(a, 0, 0, CodeL + CodeLoffset, DispB + 3 * Scaler);
	}
}

//draw reception symbol (R)
void Draw_R()
{
	if (Lit)
	{
		PaintTexture2(RCV_TEXTURE, 0, 0, 1.005 * DispL, DispB);
	}
}

//draw title

//draw FL

//draw time

//draw Ident symbol
void DrawIdent()
{
	//PaintTexture(a, LSymbolWidth, PanelL + i*LSymbolWidth*0.6, PanelB + DigitB);
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