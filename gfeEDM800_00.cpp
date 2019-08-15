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


/// Handle cross platform differences
#if IBM
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#elif LIN
#define TRUE 1
#define FALSE 0
#include <GL/gl.h>
#include <GL/glu.h>
#else
#define TRUE 1
#define FALSE 0
#if __GNUC__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <gl.h>
#include <glu.h>
#endif
#include <string.h>
#include <stdlib.h>
#endif



/*


Version

001				copy template from FS450, start with calculating randomized CHT and EGT
002 20130925	final concept for bar display and missing segments




Manual:

Dependent on the actual physical layout of the engine, each individual cylinder has a different CHT and EGT.
Since X-Plane does not provide EGT and CHT values per cylinder, we have to preset an arbitraty temperature distribution within the engine. 
To make things more complicated, the distribu


Pseudocodes:











functions:



parameterws to save:
°F / °C
TIT option installed
Oil temp option installed
fuel option installed
display HP / RPM



checks on the real thing:

- is first EGT bar always shown, even when EGT lower EGTmax / 2
- how is first CHT missing bar displayed, if anyway



*/

/// Texture stuff
#define MAX_TEXTURES 50

#define PANEL_FILENAME			"Panel.bmp"											//Bezel
#define LBAR_FILENAME			"3y_bar_8l.bmp"								//single bargraph segment
#define RBAR_FILENAME			"3y_bar_8r.bmp"								//single bargraph segment
#define RGAP_FILENAME			"gap_8r.bmp"								//single bargraph segment
#define LGAP_FILENAME			"gap_8l.bmp"								//single bargraph segment
#define GAP_FILENAME			"1y_bar_segment.bmp"								//single bargraph segment
#define GAP2_FILENAME			"2y_bar_segment.bmp"								//single bargraph segment
#define HP_FILENAME				"HP.bmp"	
#define RPM_FILENAME			"RPM.bmp"
#define CYL1_FILENAME			"cyl1.bmp"
#define CYL2_FILENAME			"cyl2.bmp"
#define CYL3_FILENAME			"cyl3.bmp"
#define CYL4_FILENAME			"cyl4.bmp"
#define CYL5_FILENAME			"cyl5.bmp"
#define CYL6_FILENAME			"cyl6.bmp"
#define T_FILENAME				"T.bmp"
#define CYL_DOT_FILENAME		"cyl_dot.bmp"	
#define _A_FILENAME				"_A.bmp"
#define _b_FILENAME				"_b.bmp"
#define _d_FILENAME				"_d.bmp"
#define _E_FILENAME				"_E.bmp"
#define _F_FILENAME				"_F.bmp"
#define _G_FILENAME				"_G.bmp"
#define _I_FILENAME				"_1.bmp"
#define _K_FILENAME				"_K.bmp"			//same as H, X
#define _L_FILENAME				"_L.bmp"
#define _M_FILENAME				"_M.bmp"
#define _n_FILENAME				"_n.bmp"
#define _o_FILENAME				"_o.bmp"
#define _P_FILENAME				"_P.bmp"
#define _r_FILENAME				"_r.bmp"
#define _S_FILENAME				"_5.bmp"			//same as 5
#define _t_FILENAME				"_t.bmp"
#define _U_FILENAME				"_U.bmp"
#define _y_FILENAME				"_y.bmp"
#define _0_FILENAME				"_0.bmp"
#define _1_FILENAME				"_1.bmp"
#define _2_FILENAME				"_2.bmp"
#define _3_FILENAME				"_3.bmp"
#define _4_FILENAME				"_4.bmp"
#define _5_FILENAME				"_5.bmp"
#define _6_FILENAME				"_6.bmp"
#define _7_FILENAME				"_7.bmp"
#define _8_FILENAME				"_8.bmp"
#define _9_FILENAME				"_9.bmp"
#define _Q_FILENAME				"_Q.bmp"			// "?" Question Mark 
#define _x_FILENAME				"_x.bmp"			// "=" Sign
#define ___FILENAME				"__.bmp"			// " " (blank) character
#define _c_FILENAME				"_c.bmp"			// "." decimal point
#define _i_FILENAME				"_i.bmp"			// "-" dash 


#define PANEL_TEXTURE	0
#define LBAR_TEXTURE	1
#define GAP_TEXTURE		2
#define GAP2_TEXTURE	3
#define CYL1_TEXTURE	4
#define CYL2_TEXTURE	5
#define CYL3_TEXTURE	6
#define CYL4_TEXTURE	7
#define CYL5_TEXTURE	8
#define CYL6_TEXTURE	9
#define T_TEXTURE		10
#define CYL_DOT_TEXTURE	11
#define _A_TEXTURE 12
#define _b_TEXTURE 13
#define _d_TEXTURE 14
#define _E_TEXTURE 15
#define _F_TEXTURE 16
#define _G_TEXTURE 17
#define _I_TEXTURE 18
#define _K_TEXTURE 19
#define _L_TEXTURE 20
#define _M_TEXTURE 21
#define _n_TEXTURE 22
#define _o_TEXTURE 23
#define _P_TEXTURE 24
#define _r_TEXTURE 25
#define _S_TEXTURE 26
#define _t_TEXTURE 27
#define _U_TEXTURE 28
#define _y_TEXTURE 29
#define _0_TEXTURE 30
#define _1_TEXTURE 31
#define _2_TEXTURE 32
#define _3_TEXTURE 33
#define _4_TEXTURE 34
#define _5_TEXTURE 35
#define _6_TEXTURE 36
#define _7_TEXTURE 37
#define _8_TEXTURE 38
#define _9_TEXTURE 39
#define _Q_TEXTURE 40
#define _x_TEXTURE 41
#define ___TEXTURE 42
#define _c_TEXTURE 43	
#define _i_TEXTURE 44
#define RBAR_TEXTURE	45	
#define RGAP_TEXTURE	46	
#define LGAP_TEXTURE	47	
#define HP_TEXTURE	48	
#define RPM_TEXTURE	49	

//Operational Modes and Menu Items
#define	M_TEST		1
#define M_USD		2
#define M_REM		3
#define M_HM		4
#define M_REQ		5
#define M_RES		6
#define M_MPG		7
#define M_AUTOSEQ	8
#define M_MANSEQ	9
#define M_INIT		10
#define M_FILL_N	11
#define M_FILL_MAIN	12
#define M_FILL_TOP	13
#define M_FILL_ADD	14
#define M_FILL_ADD_1	15
#define M_PROG		16
#define	M_TEST_1	17
#define	M_TEST_2	18
#define	M_TEST_3	19
#define	M_TEST_4	20
#define	M_TEST_5	21
#define M_REM_ALARM	30
#define M_HM_ALARM	31
#define M_CYL1		40
#define M_CYL2		41
#define M_CYL3		42
#define M_CYL4		43
#define M_CYL5		44
#define M_CYL6		45
#define M_TIT		46
#define M_OIL		47
#define M_SHOCK 	48
#define M_BAT		49
#define M_OAT		50
#define M_DIF		51


#define FUEL_GAL	0
#define FUEL_LTR	1
#define FUEL_PDS	2
#define FUEL_OPT	3
#define OIL_OPT		4
#define	TIT_OPT		5
#define HP_OPT		6
#define TEMP_CF		7

#define GPS_XPL		0
#define GPS_RXP		1

XPLMDataRef	RED = NULL, GREEN = NULL, BLUE = NULL;
XPLMDataRef dFuelFlow = NULL;
XPLMDataRef dFlightTime = NULL;
XPLMDataRef	dFuelTotal = NULL;
XPLMDataRef	dFuel1 = NULL;
XPLMDataRef	dFuel2 = NULL;
XPLMDataRef	dDistGPS = NULL;
XPLMDataRef	dTimeGPS = NULL;
XPLMDataRef	dTrack = NULL;
XPLMDataRef	dGroundSpeed = NULL;
XPLMDataRef	dBearingGPS = NULL;
XPLMDataRef dRelBearingGPS = NULL;
XPLMDataRef	dBattOn = NULL;
XPLMDataRef dGPS = NULL;
XPLMDataRef dCourseGPS = NULL;
XPLMDataRef dxplEGT = NULL;
XPLMDataRef dxplCHT = NULL;
XPLMDataRef dxplOIL = NULL;
XPLMDataRef dmaxEGT = NULL; 
XPLMDataRef dmaxCHT = NULL; 
XPLMDataRef dmaxOIL = NULL;
XPLMDataRef dPmax = NULL;
XPLMDataRef dPower = NULL;
XPLMDataRef dRPM = NULL;

XPLMTextureID gTexture[MAX_TEXTURES];
XPLMWindowID ggfePanelDisplayWindow = NULL;
XPLMPluginID gPlugin;

XPLMHotKeyID ggfeEDM800HotKey = NULL;
XPLMHotKeyID gHotKeySTEP = NULL;
XPLMHotKeyID gHotKeySTEPHold = NULL;
XPLMHotKeyID gHotKeyLF = NULL;
XPLMHotKeyID gHotKeyLFHold = NULL;

int gfeDisplayPanelWindow = 1;			//show panel from start
char gPluginDataFile[255];					//
float StartFuel;							//fuel on start
float fUsdFuel;								//used fuel
char cUsdFuel[5];							//used fuel on display
float fRemFuel;								//remaining fuel
char cRemFuel[5];							//remaining fuel on dispaly
int iRemTime;								//remaining time 
int iRemTimeHH;								//hours
int iRemTimeMM;								//minutes
char cRemTime[5];							//remaing time on display
int nGPS = 1;								//type of GPS (for method of deriving relative bearing)
int nGPSfix = 0;							//GPS available
float fRelBearingGPS;						//RB to GPS fix
float fDistGPS;								//distance in nm to GPS fix
float fTimeGPS;								//time in seconds to GPS
float fReqFuel;								//fuel to next GPS Fix
char cReqFuel[5];							//fuel to next GPS Fix on display
float fResFuel;								//fuel at next GPS fix, with GPS link only
char cResFuel[5];							//fuel at next GPS fix on display
float fMPG;									//miles per gallon, with GPS only
char cMPG[5];								//miles per gallon or per liter or per pound on display
float fFlow = 0;							//<units> per hour
char cFlow[5];								//fuel flow on display
char Symbol[4];								//content for LED
int FSMode;									//active Operation Mode we're in
int FSStep;									//active step in sequence 
int nTick;									//Counter for Seconds in Flightloop
int nTick2;									//Counter for Seconds in Flightloop
char cText0[4];								//Text in Row 1
char cText1[5];								//Text in Row 1
char cText2[6];								//Text in Row 2
int iFillAdd = 0;							//fuel added, gallons*10
float fFillAdd;								//fuel added or subtracted
float fFillMain;							//fuel capacity to tabs
float fFillTop;								//max fuel capacity
int gaugeX1;								//gauge position bottom left
int gaugeX2;								//gauge position bottom right
int gaugeY1;								//gauge position top left
int gaugeY2;								//gauge position top right
int FuelUnit;								//Gallons, Liters, Pounds
float fFuelFactor;							//factor for fuel unit conversion
int IndexRate = 1;							//Index Rate in Auto Index Mode [seconds]
int FirstCall = TRUE;						//first call since plugin start
int BattOn;									//electrical current on Bus 1
int RealMode = 0;							//1: work with fuel added on startup, 0: ignore added fuel, work with XPL fuel state data only
float RunTime;							
float DeltaTime;
float LastTime = 0;
int nFuelAlarm;								//alarm state for low REM fuel: 0 = no alarm, 1 = alarm, 2 = alarm inhibit, 3 = alarm off 
int nTimeAlarm;								//alarm state for low HM time: 0 = no alarm, 1 = alarm, 2 = alarm inhibit, 3 = alarm off 
float fFuelThres;							//threshold FuelAlarm
int iTimeThres;								//threshold TimeAlarm

int i;										//Counter
int n;										//Counter
int a;										//Counter
int k = 1;									//Counter for EGT temp filtering
int iCylinders;								//number of cylinders on plane (only 4 or 6 possible)
float fCylVariation[6];						//arbitrary factor for uneven temperature distribution within engine
int iEGT[6];								//number of segments to display EGT
int iCHT[6];								//number of segments to display CHT
int iOIL;									//number of segments to display CHT
float fEGT[6];								//current EGT in cylinder
float fCHT[6];								//current CHT in cylinder
float fmaxEGT;								//from Dataref
float fmaxCHT;								//from Dataref
float fmaxOIL;
float fxplEGT[8];							//actual x-plane EGT per engine
float fxplCHT[8];							//actual x-plane CHT per engine
float fxplOIL[8];							//actual x-plane OIL temp per engine
int FuelOption;								//1 if Fuel Measurement Option is installed
int OILOption;								//1 if Oil Temperature Option is installed
int TITOption;								//1 if Turbine Intake Temperature Option is installed
int HPOption;								//1 if Power display option is installed
int RPMOption;								//1 if RPM Option is installed
int TempUnit;								//1 for temperature measurement is °F, 0 is °C
float fPmax;
float fPower[8];
float fRPM[8];


/*
////////////////////////////////////////// PROTOTYPES //////////////////////////////////////////
*/

/// Prototypes for callbacks etc.
GLvoid LoadTextures(GLvoid);
int GetTextureID(char);
int LoadGLTexture(char *pFileName, int TextureId);
int DrawGLScene(float x, float y);

void gfeEDM800HotKey(void * refCon);
void HotKeyLFCallback(void * inRefcon);   
void HotKeyLFHoldCallback(void * inrefCon);
void HotKeySTEPCallback(void * inRefcon);   
void HotKeySTEPHoldCallback(void * inRefcon);   

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

int PaintTexture(int texture, float w, float x, float y);

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
		char *pFileName = "Resources\\Plugins\\gfeEDM800\\";
	#elif LIN
		char *pFileName = "Resources/plugins/gfeEDM800/";
	#else
		char *pFileName = "Resources:Plugins:gfeEDM800:";
	#endif
	/// Setup texture and ini file locations
	XPLMGetSystemPath(gPluginDataFile);
	strcat(gPluginDataFile, pFileName);

	strcpy(outName, "EDM800");
	strcpy(outSig, "gfe.panel.EDM800");
	strcpy(outDesc, "gfe Engine Monitor");
	
	gPlugin = XPLMFindPluginBySignature("RealityXP.GNS.WAAS");					//detect if RXP GPS is installed
	if (gPlugin == XPLM_NO_PLUGIN_ID)
	{
		nGPS = GPS_XPL;
	}
	else
	{
		nGPS = GPS_RXP;
	}


	//Register FlightLoop Callback for every 250msec
	XPLMRegisterFlightLoopCallback(		
			MyFlightLoopCallback,	/* Callback */
			0.25,					/* Interval */
			NULL);					/* refcon not used. */

	//Open INI File and load values, or load default values instead
	if ( !OpenINIFile() )
	{
		fFillMain = 0;								
		fFillTop = 120;
		gaugeX1 = 768;															//gauge position left
		gaugeX2 = 1024;															//gauge position right
		gaugeY1 = 256;															//gauge position bottom
		gaugeY2 = 512;															//gauge position top
		FuelUnit = FUEL_GAL;													//gallons
		RealMode = 0;
		fFuelThres = 10;														//10 gallons
		iTimeThres = 45;														//45 minutes
		FuelOption = 0;															//not yet implemented							
		OILOption = 1;								
		TITOption = 1;															//not yet implemented
		HPOption = 1;														
		TempUnit = 1;															//Cesius (Centigrade) conversion not yet available
	}

	//Debug only !!!
	OILOption = 1;								
	HPOption = 0;
	RPMOption = 1;


	if (FuelUnit == FUEL_LTR)
	{
		fFuelFactor = 1.3986;													//liters = kg * 1.3986 for Avgas 100LL at 15°C
	}
	else if (FuelUnit == FUEL_PDS)
	{
		fFuelFactor = 2.2045;													//pounds = kg / 0.4536
	}
	else 
	{
		fFuelFactor = 0.3695;													//gallons = kg * 1.3986 / 3.7854
	}					

	
	/// Create window, setup datarefs and register  hotkeys
	ggfePanelDisplayWindow = XPLMCreateWindow(gaugeX1, gaugeY2, gaugeX2, gaugeY1, 1, gfePanelWindowCallback, gfePanelKeyCallback, gfePanelMouseClickCallback, NULL);


	dBattOn = XPLMFindDataRef("sim/cockpit/electrical/battery_on"); 
	dFuelFlow = XPLMFindDataRef("sim/flightmodel/engine/ENGN_FF_");
	dFlightTime = XPLMFindDataRef("sim/time/total_flight_time_sec");
	dFuelTotal = XPLMFindDataRef("sim/flightmodel/weight/m_fuel_total");
	dFuel1 = XPLMFindDataRef("sim/flightmodel/weight/m_fuel1");
	dFuel2 = XPLMFindDataRef("sim/flightmodel/weight/m_fuel2");
	dGPS = XPLMFindDataRef("sim/cockpit/radios/gps_has_dme");											//RXP GNS430: always 0
	dDistGPS = XPLMFindDataRef("sim/cockpit/radios/gps_dme_dist_m");									//RXP GNS430: ok
	dTimeGPS = XPLMFindDataRef("sim/cockpit/radios/gps_dme_time_secs");									//RXP GNS430: alway 0
	dBearingGPS = XPLMFindDataRef("sim/cockpit/radios/gps_dir_degt");									//RXP GNS430: unusable; XPL GNS430: relative bearing to fix
	dRelBearingGPS = XPLMFindDataRef("sim/cockpit2/radios/indicators/gps_relative_bearing_deg");		//XPL GNS430: relative bearing to fix
	dCourseGPS =XPLMFindDataRef("sim/cockpit/radios/gps_course_degtm");									//RXP GNS430: course to fix
	dTrack = XPLMFindDataRef("sim/flightmodel/position/hpath");											//specified as heading, but is track
	dGroundSpeed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");								//GS in m/s
	RED = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_r");
	GREEN = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_g");
	BLUE = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_b");
	dxplEGT = XPLMFindDataRef("sim/flightmodel/engine/ENGN_EGT_c");										//temperatures in °F
	dxplCHT = XPLMFindDataRef("sim/flightmodel/engine/ENGN_CHT_c");
	dxplOIL = XPLMFindDataRef("sim/flightmodel/engine/ENGN_oil_temp_c"); 
	dmaxEGT = XPLMFindDataRef("sim/aircraft/engine/acf_max_EGT");
	dmaxOIL = XPLMFindDataRef("sim/aircraft/engine/acf_max_OILT");
	dmaxCHT = XPLMFindDataRef("sim/aircraft/engine/acf_max_CHT");
	dPmax = XPLMFindDataRef("sim/aircraft/engine/acf_pmax");											//max engine power (watts)
	dPower = XPLMFindDataRef("sim/cockpit2/engine/indicators/power_watts");								//actual engine power indicated (watts)
	dRPM = XPLMFindDataRef("sim/cockpit2/engine/indicators/engine_speed_rpm");							//engine RPM

	ggfeEDM800HotKey = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag, "F8", gfeEDM800HotKey, NULL);
	gHotKeySTEP = XPLMRegisterHotKey(XPLM_VK_F1, xplm_DownFlag, "F1 STEP", HotKeySTEPCallback,	NULL);
	gHotKeySTEPHold = XPLMRegisterHotKey(XPLM_VK_F1, NULL, "F1 STEP Hold", HotKeySTEPHoldCallback,	NULL);
	gHotKeyLF = XPLMRegisterHotKey(XPLM_VK_F2, xplm_DownFlag,	"F2 AUTO", HotKeyLFCallback, NULL);
	gHotKeyLFHold = XPLMRegisterHotKey(XPLM_VK_F2, NULL,	"F2 AUTO Hold", HotKeyLFHoldCallback, NULL);

	//inital SelfTest Routine
	FSStep = M_TEST;
	FSMode = M_TEST;
	FSStep = M_CYL1;

	//no EGT and CHT available per cylinder, so we use random factors
	for (i = 0; i< 7; i++)
	{
		fCylVariation[i] = (rand() % 10);									//create random number between 0 and 9
		fCylVariation[i] = fCylVariation[i] / 100 + 0.95 ;					//random factor between 0.95 and 1.05 				
	}

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

	XPLMUnregisterHotKey(ggfeEDM800HotKey);
	XPLMUnregisterHotKey(gHotKeyLF);
	XPLMUnregisterHotKey(gHotKeySTEP);
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
    float GaugeWidth, GaugeHeight, GaugeWidthRatio, GaugeHeightRatio;
	float PanelL, PanelR, PanelB, PanelT;
	float GaugeLeft, GaugeRight, GaugeBottom, GaugeTop;
	float HdWidth = 16;
	float SymbolWidth = 32;
	float SymbolWidth2 = 24;
	float BarWidth = 8;
	float DotWidth = 8;
	float Scaler;
	float BarL;
	float BarB;
	float DigitL;
	float DigitB;
	float HeaderL;
	float HeaderB;
	float HPL;
	float HPB;

	/// Do the actual drawing, but only if our window is active
	if (gfeDisplayPanelWindow)
	{
		// Need to find out where our window is
		XPLMGetWindowGeometry(ggfePanelDisplayWindow, &PanelWindowLeft, &PanelWindowTop, &PanelWindowRight, &PanelWindowBottom);

		/// Setup our panel and gauge relative to our window
		PanelL = PanelWindowLeft; PanelR = PanelWindowRight; PanelB = PanelWindowBottom; PanelT = PanelWindowTop;

		// Original Panel Size = 256x256, so in case of resize, scale everything accordingly
		Scaler = (PanelR-PanelL)/256;

		// Row Coordinates
		SymbolWidth = SymbolWidth * Scaler;									//Seven Segment Glyphs
		SymbolWidth2 = SymbolWidth2 * Scaler;								//Seven Segment Glyphs for HP and RPM
		BarWidth = BarWidth * Scaler;
		DotWidth = DotWidth * Scaler;
		HdWidth = HdWidth * Scaler * 0.75;
		BarL = 60 * Scaler;
		BarB = 102 * Scaler;
		DigitL = 50 * Scaler;
		DigitB = 65 * Scaler;
		HeaderL = 67 * Scaler;
		HeaderB = 179 * Scaler;
		HPL = 94 * Scaler;
		HPB = 190 * Scaler; 

		/// Tell Xplane what we are doing
		XPLMSetGraphicsState(0/*Fog*/, 1/*TexUnits*/, 0/*Lighting*/, 0/*AlphaTesting*/, 0/*AlphaBlending*/, 0/*DepthTesting*/, 0/*DepthWriting*/);

		/// Handle day/night
		glColor3f(Red, Green, Blue);

		// Draw Panel
		XPLMBindTexture2d(gTexture[PANEL_TEXTURE], 0);
		glBegin(GL_QUADS);
				glTexCoord2f(1, 0.0f); glVertex2f(PanelR, PanelB);	// Bottom Right Of The Texture and Quad
				glTexCoord2f(0, 0.0f); glVertex2f(PanelL, PanelB);	// Bottom Left Of The Texture and Quad		
				glTexCoord2f(0, 1.0f); glVertex2f(PanelL, PanelT);		// Top Left Of The Texture and Quad
				glTexCoord2f(1, 1.0f); glVertex2f(PanelR, PanelT);	// Top Right Of The Texture and Quad
		glEnd();
	
		//get actual data from XPL
		i = XPLMGetDatavf(dxplEGT, fxplEGT, 0, 8);									//consider engine 1 only
		i = XPLMGetDatavf(dxplCHT, fxplCHT, 0, 8);
		i = XPLMGetDatavf(dxplOIL, fxplOIL, 0, 8);
		i = XPLMGetDatavf(dPower, fPower, 0, 8);
		i = XPLMGetDatavf(dRPM, fRPM, 0, 8);

		//load static engine parameters
		fmaxEGT = XPLMGetDataf(dmaxEGT);
		fmaxCHT = XPLMGetDataf(dmaxCHT);
		fmaxOIL = XPLMGetDataf(dmaxOIL);
		fPmax = XPLMGetDataf(dPmax);
	
		//randomize EGT and CHT over cylinders
		for (i = 0; i< 6; i++)
		{
			fEGT[i] = fxplEGT[0] *fCylVariation[i];									//set base EGT per cylinder
			fCHT[i] = fxplCHT[0] *fCylVariation[i];									//set base CHT per cylinder
		}
		
		// draw headlines
		n = 0;
		PaintTexture(CYL1_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		n = 1;
		PaintTexture(CYL2_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		n = 2;
		PaintTexture(CYL3_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		n = 3;
		PaintTexture(CYL4_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		n = 4;
		PaintTexture(CYL5_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		n = 5;
		PaintTexture(CYL6_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
		if (TITOption == 1)
		{
			n = 6;
			XPLMBindTexture2d(gTexture[T_TEXTURE], 0);
			glBegin(GL_QUADS);
					glTexCoord2f(1, 0.0f); glVertex2f(PanelL + BarL + n*BarWidth*2.5 + HdWidth, PanelB +  HeaderB);		// Bottom Right 
					glTexCoord2f(0, 0.0f); glVertex2f(PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB );				// Bottom Left 		
					glTexCoord2f(0, 1.0f); glVertex2f(PanelL + BarL + n*BarWidth*2.5, PanelB +  HdWidth + HeaderB);			// Top Left 
					glTexCoord2f(1, 1.0f); glVertex2f(PanelL + BarL + n*BarWidth*2.5 + HdWidth, PanelB +  HdWidth + HeaderB);	// Top Right 
			glEnd();
		}

		// draw Bars
		for (n = 0; n < 6; n++)														//step through cylinders
		{
		
			iCHT[n] = (fCHT[n] - fmaxCHT/2) / 25;									//18 segments, first missing segment represents 250°F (121°C)
			if (iCHT[n] > 18)  iCHT[n] = 18;

			//display EGT & CHT bargraph
			iEGT[n] = (fEGT[n] - fmaxEGT/2) / (fmaxEGT/2 / 18);						//19 segments, first segment represents BarL% of redline EGT
			if (iEGT[n] > 18)  iEGT[n] = 18;
			if (iEGT[n] > 0) 
			{
				for (i = 0; i <= iEGT[n]; i++)
				{
					if  ((i != iCHT[n]) || (iCHT[n] == 0))							//skip corresponding segment for CHT display
					{
						PaintTexture(LBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5, PanelB + BarB + i*(0.375*BarWidth));
						PaintTexture(RBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5 + BarWidth, PanelB + BarB + i*(0.375*BarWidth));
					}
				}
			}
			if (iCHT[n] >= iEGT[n])													//show CHT segment if at or above EGT
				{
					i = iCHT[n];
					PaintTexture(LBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5, PanelB + BarB + i*(0.375*BarWidth));
					PaintTexture(RBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5 + BarWidth, PanelB + BarB + i*(0.375*BarWidth));
				}
		}

		//display OIL temperature
		if (OILOption == 1)
		{
			iOIL = (fxplOIL[0] - fmaxOIL/2) / (fmaxOIL/2 / 18);						//19 segments, first segment represents BarL% of redline OIL temp
			n = 6;																	//7th bar column counted from left
			if (iOIL > 18)  iOIL = 18;
			if (iOIL > 0) 
			{
				for (i = 0; i <= iOIL; i++)
				{
					PaintTexture(LBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5, PanelB + BarB + i*(0.375*BarWidth));
					PaintTexture(RBAR_TEXTURE, BarWidth, PanelL + BarL + n*BarWidth*2.5 + BarWidth, PanelB + BarB + i*(0.375*BarWidth));
				}
			}
		}

		//draw horsepower digits
		if (HPOption == 1)
		{
			if ((fPower[0] > 0) && (fPmax > 0))
			{
				sprintf(cText0,"%4.0f", 100*fPower[0]/fPmax);
			}
			for  (i=0; i<4; i=i+1)																								
			{
				a = GetTextureID(cText0[i]);
				PaintTexture(a, SymbolWidth2, PanelL + HPL + i*SymbolWidth2*0.6, PanelB + HPB);
			}
			PaintTexture(HP_TEXTURE, SymbolWidth2, PanelL + HPL + 63, PanelB + HPB);
		}
		if (RPMOption == 1)	
		{
			sprintf(cText0,"%4.0f", fRPM[0]);
			for  (i=0; i<4; i=i+1)																								
			{
				a = GetTextureID(cText0[i]);
				PaintTexture(a, SymbolWidth2, PanelL + HPL + i*SymbolWidth2*0.6, PanelB + HPB);
			}
			PaintTexture(RPM_TEXTURE, SymbolWidth2, PanelL + HPL + 63, PanelB + HPB);
		}

		//draw EGT, CHT, OIL and TIT digits
		if ((FSStep >= M_CYL1) && (FSStep <= M_OIL))
		{
			n = FSStep - M_CYL1;
			if (k == 10)														//EGT may cange rapidly, reduce updates for stable display
			{																	//intervall is about every 10 frames
				sprintf(cText1,"%4.0f",fEGT[n]);
				k = 0;
			}
			k++;
			for  (i=0; i<4; i=i+1)												//left digits												
			{
				a = GetTextureID(cText1[i]);
				PaintTexture(a, SymbolWidth, PanelL + DigitL + i*SymbolWidth*0.6, PanelB + DigitB);
			}
			if (FSStep == M_OIL)
			{
				sprintf(cText2,"%4.0f",fxplOIL[0]);
			}
			else
			{
				sprintf(cText2,"%4.0f",fCHT[n]);									
			}
			for  (i=1; i<4; i=i+1)												//right digits
			{
				a = GetTextureID(cText2[i]);
				PaintTexture(a, SymbolWidth, PanelL + DigitL + 2*SymbolWidth + i*SymbolWidth*0.6, PanelB + DigitB);
			}
	
			//draw dot
			if (n < 6)
			{
				if ((FSStep >= M_CYL1) && (FSStep <= M_TIT))
				{
					PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + n*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95); 
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
		if (CoordInRect(x, y, Left+(0.29*(Right-Left)), Top-(0.76*(Top-Bottom)), Left+(0.43*(Right-Left)), Top-(0.84*(Top-Bottom))))	
		{
			HotKeySTEPCallback(0);												// STEP button, need to work with percentage as panel could have been resized
		}
		if (CoordInRect(x, y, Left+(0.60*(Right-Left)), Top-(0.76*(Top-Bottom)), Left+(0.74*(Right-Left)), Top-(0.84*(Top-Bottom))))	
		{
			HotKeyLFCallback(0);												// AUTO button, need to work with percentage as panel could have been resized
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
void	gfeEDM800HotKey(void * refCon)
{
	gfeDisplayPanelWindow = !gfeDisplayPanelWindow;
}

void	HotKeySTEPCallback(void * inRefcon)										//STEP button
{
}

void	HotKeySTEPHoldCallback(void * inRefcon)									//Hold STEP button
{
}

void	HotKeyLFCallback(void * inRefcon)										//AUTO button
{
}

void HotKeyLFHoldCallback(void * inrefCon)									//Hold AUTO button
{
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

	FSMode = M_AUTOSEQ;
	
/*
	if (nTick < 2)																//self test sequence
	{		
		FSMode = M_TEST;
		FSStep = M_TEST_1;														//upper 8888
	}
	else if (nTick < 4)
	{		
		FSMode = M_TEST;
		FSStep = M_TEST_2;														//lower 8888
	}
	else if (nTick < 12)
	{		
		FSMode = M_TEST;
		FSStep = M_TEST_3;														//FS-450
	}
	else if (nTick < 16)
	{		
		FSMode = M_TEST;
		FSStep = M_TEST_4;														//Fuel <unit>
	}
	else if (nTick == 24)														//start init sequence
	{
		FSMode = M_INIT;
		FSStep = M_FILL_N;
	}
	else if (nTick == 1000 + 240)												//60 sec after closing M_INIT start M_AUTOSEQ							
	{
		FSMode = M_AUTOSEQ;
	}
	else if (nTick == 2000 + 2400)												//when 10 min inhibit time elapsed										
	{
		if (nFuelAlarm == 2)													//reactivate alarms
		{
			nFuelAlarm = 1;
			FSMode = M_MANSEQ;
			FSStep = M_REM;
		}
		if (nTimeAlarm == 2)
		{
			nTimeAlarm = 1;
			FSMode = M_MANSEQ;
			FSStep = M_REM;
		}
	}
	else */
	{
		if ((RealMode == 1) && ((nTick % 8) == 0))								// every 2 sec refresh current fuel state
		{
			//XPLMGetDatavf(dFuelFlow,FloatVals, 0, 8);							// get current flow from Dataref (kg/sec)
			//fRemFuel = fRemFuel - (FloatVals[0] * fFuelFactor);	
			RunTime = XPLMGetElapsedTime();
			DeltaTime = RunTime - LastTime;
			LastTime = RunTime;
			fRemFuel = fRemFuel - ((fFlow / 3600) * DeltaTime);					

			/*
			//DEBUG ONLY
			sprintf(s,"%10f",DeltaTime);
			XPLMDebugString(s);
			XPLMDebugString("   ");
			sprintf(sa,"%10f",fRemFuel);
			XPLMDebugString(sa);
			XPLMDebugString("   ");
			sprintf(sb,"%10f",XPLMGetDataf(dFuelTotal) * fFuelFactor);
			XPLMDebugString(sb);
			XPLMDebugString("\n");
			*/

			if ((nTick % 40) == 0)												//every 10 sec save current RemFuel into File
			{			
				WriteINIFile();													//write back variables
			}
		}

		if ((FSMode == M_AUTOSEQ) && (FSMode != M_INIT))						//AUTOSEQ
		{
			if ((nTick % IndexRate) == 0)										//x seconds interval
			{
				FSStep = FSStep + 1;
				if ((FSStep == M_OIL) && (OILOption == 0))						//skip OIL display when not configured
				{
					FSStep++;
				}

				if ((FSStep == M_TIT) && (TITOption == 0))						//skip TIT display when not configured
				{
					FSStep++;
				}
				
				if (nGPSfix)
				{
					if (FSStep == (M_MPG + 1))	FSStep = M_USD;
				}
				else
				{
					if (FSStep > M_OIL)	FSStep = M_CYL1;
				}
			}
		}
	}




	return 0.25;																// Return time interval after that we want to be called again
}																				// if this interval is changed, all nTick based timers will change!

/// Loads all our textures
GLvoid LoadTextures(GLvoid)
{
	if (!LoadGLTexture(PANEL_FILENAME, PANEL_TEXTURE))
		XPLMDebugString("Panel texture PANEL_TEXTURE failed to load\n");
	if (!LoadGLTexture(GAP_FILENAME, GAP_TEXTURE))
		XPLMDebugString("Panel texture GAP_TEXTURE failed to load\n");
	if (!LoadGLTexture(GAP2_FILENAME, GAP2_TEXTURE))
		XPLMDebugString("Panel texture GAP2_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL1_FILENAME, CYL1_TEXTURE))
		XPLMDebugString("Panel texture CYL1_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL2_FILENAME, CYL2_TEXTURE))
		XPLMDebugString("Panel texture CYL2_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL3_FILENAME, CYL3_TEXTURE))
		XPLMDebugString("Panel texture CYL3_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL4_FILENAME, CYL4_TEXTURE))
		XPLMDebugString("Panel texture CYL4_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL5_FILENAME, CYL5_TEXTURE))
		XPLMDebugString("Panel texture CYL5_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL6_FILENAME, CYL6_TEXTURE))
		XPLMDebugString("Panel texture CYL6_TEXTURE failed to load\n");
	if (!LoadGLTexture(T_FILENAME, T_TEXTURE))
		XPLMDebugString("Panel texture T_TEXTURE failed to load\n");
	if (!LoadGLTexture(CYL_DOT_FILENAME, CYL_DOT_TEXTURE))
		XPLMDebugString("Panel texture CYL_DOT_TEXTURE failed to load\n");
	if (!LoadGLTexture(_A_FILENAME, _A_TEXTURE))
		XPLMDebugString("Gauge texture _A_TEXTURE failed to load\n");
	if (!LoadGLTexture(_b_FILENAME, _b_TEXTURE))
		XPLMDebugString("Gauge texture _b_TEXTURE failed to load\n");
	if (!LoadGLTexture(_d_FILENAME, _d_TEXTURE))
		XPLMDebugString("Gauge texture _d_TEXTURE failed to load\n");
	if (!LoadGLTexture(_E_FILENAME, _E_TEXTURE))
		XPLMDebugString("Gauge texture _E_TEXTURE failed to load\n");
	if (!LoadGLTexture(_F_FILENAME, _F_TEXTURE))
		XPLMDebugString("Gauge texture _F_TEXTURE failed to load\n");
	if (!LoadGLTexture(_G_FILENAME, _G_TEXTURE))
		XPLMDebugString("Gauge texture _G_TEXTURE failed to load\n");
	if (!LoadGLTexture(_I_FILENAME, _I_TEXTURE))
		XPLMDebugString("Gauge texture _I_TEXTURE failed to load\n");
	if (!LoadGLTexture(_K_FILENAME, _K_TEXTURE))
		XPLMDebugString("Gauge texture _K_TEXTURE failed to load\n");
	if (!LoadGLTexture(_L_FILENAME, _L_TEXTURE))
		XPLMDebugString("Gauge texture _L_TEXTURE failed to load\n");
	if (!LoadGLTexture(_M_FILENAME, _M_TEXTURE))
		XPLMDebugString("Gauge texture _M_TEXTURE failed to load\n");
	if (!LoadGLTexture(_n_FILENAME, _n_TEXTURE))
		XPLMDebugString("Gauge texture _n_TEXTURE failed to load\n");
	if (!LoadGLTexture(_o_FILENAME, _o_TEXTURE))
		XPLMDebugString("Gauge texture _o_TEXTURE failed to load\n");
	if (!LoadGLTexture(_P_FILENAME, _P_TEXTURE))
		XPLMDebugString("Gauge texture _P_TEXTURE failed to load\n");
	if (!LoadGLTexture(_r_FILENAME, _r_TEXTURE))
		XPLMDebugString("Gauge texture _r_TEXTURE failed to load\n");
	if (!LoadGLTexture(_S_FILENAME, _S_TEXTURE))
		XPLMDebugString("Gauge texture _S_TEXTURE failed to load\n");
	if (!LoadGLTexture(_t_FILENAME, _t_TEXTURE))
		XPLMDebugString("Gauge texture _t_TEXTURE failed to load\n");
	if (!LoadGLTexture(_U_FILENAME, _U_TEXTURE))
		XPLMDebugString("Gauge texture _U_TEXTURE failed to load\n");
	if (!LoadGLTexture(_y_FILENAME, _y_TEXTURE))
		XPLMDebugString("Gauge texture _y_TEXTURE failed to load\n");
	if (!LoadGLTexture(_0_FILENAME, _0_TEXTURE))
		XPLMDebugString("Gauge texture _0_TEXTURE failed to load\n");
	if (!LoadGLTexture(_1_FILENAME, _1_TEXTURE))
		XPLMDebugString("Gauge texture _1_TEXTURE failed to load\n");
	if (!LoadGLTexture(_2_FILENAME, _2_TEXTURE))
		XPLMDebugString("Gauge texture _2_TEXTURE failed to load\n");
	if (!LoadGLTexture(_3_FILENAME, _3_TEXTURE))
		XPLMDebugString("Gauge texture _3_TEXTURE failed to load\n");
	if (!LoadGLTexture(_4_FILENAME, _4_TEXTURE))
		XPLMDebugString("Gauge texture _4_TEXTURE failed to load\n");
	if (!LoadGLTexture(_5_FILENAME, _5_TEXTURE))
		XPLMDebugString("Gauge texture _5_TEXTURE failed to load\n");
	if (!LoadGLTexture(_6_FILENAME, _6_TEXTURE))
		XPLMDebugString("Gauge texture _6_TEXTURE failed to load\n");
	if (!LoadGLTexture(_7_FILENAME, _7_TEXTURE))
		XPLMDebugString("Gauge texture _7_TEXTURE failed to load\n");
	if (!LoadGLTexture(_8_FILENAME, _8_TEXTURE))
		XPLMDebugString("Gauge texture _8_TEXTURE failed to load\n");
	if (!LoadGLTexture(_9_FILENAME, _9_TEXTURE))
		XPLMDebugString("Gauge texture _9_TEXTURE failed to load\n");
	if (!LoadGLTexture(_Q_FILENAME, _Q_TEXTURE))
		XPLMDebugString("Gauge texture _Q_TEXTURE failed to load\n");
	if (!LoadGLTexture(_x_FILENAME, _x_TEXTURE))
		XPLMDebugString("Gauge texture _X_TEXTURE failed to load\n");
	if (!LoadGLTexture(___FILENAME, ___TEXTURE))
		XPLMDebugString("Gauge texture ___TEXTURE failed to load\n");
	if (!LoadGLTexture(_c_FILENAME, _c_TEXTURE))
		XPLMDebugString("Gauge texture _c_TEXTURE failed to load\n");
	if (!LoadGLTexture(LBAR_FILENAME, LBAR_TEXTURE))
		XPLMDebugString("Panel texture LBAR_TEXTURE failed to load\n");
	if (!LoadGLTexture(RBAR_FILENAME, RBAR_TEXTURE))
		XPLMDebugString("Panel texture RBAR_TEXTURE failed to load\n");
	if (!LoadGLTexture(LGAP_FILENAME, LGAP_TEXTURE))
		XPLMDebugString("Panel texture LGAP_TEXTURE failed to load\n");
	if (!LoadGLTexture(RGAP_FILENAME, RGAP_TEXTURE))
		XPLMDebugString("Panel texture RGAP_TEXTURE failed to load\n");
	if (!LoadGLTexture(HP_FILENAME, HP_TEXTURE))
		XPLMDebugString("Panel texture HP_TEXTURE failed to load\n");
	if (!LoadGLTexture(RPM_FILENAME, RPM_TEXTURE))
		XPLMDebugString("Panel texture RPM_TEXTURE failed to load\n");
}

/// Loads one texture
int LoadGLTexture(char *pFileName, int TextureId)
{
	int Status=FALSE;
	char TextureFileName[255];
	#if APL && __MACH__
	char TextureFileName2[255];	
	int Result = 0;
	#endif

	/// Need to get the actual texture path and append the filename to it.
	strcpy(TextureFileName, gPluginDataFile);
	strcat(TextureFileName, pFileName);

	#if APL && __MACH__
	Result = ConvertPath(TextureFileName, TextureFileName2, sizeof(TextureFileName));
	if (Result == 0)
		strcpy(TextureFileName, TextureFileName2);
	else
		XPLMDebugString("gfeEDM800 - Unable to convert path\n");
	#endif

	void *pImageData = 0;
	int sWidth, sHeight;
	IMAGEDATA sImageData;
	/// Get the bitmap from the file
	if (BitmapLoader(TextureFileName, &sImageData))
	{
		Status=TRUE;

		SwapRedBlue(&sImageData);
		pImageData = sImageData.pData;

		/// Do the opengl stuff using XPLM functions for a friendly Xplane existence.
		sWidth=sImageData.Width;
		sHeight=sImageData.Height;
		XPLMGenerateTextureNumbers(&gTexture[TextureId], 1);
		XPLMBindTexture2d(gTexture[TextureId], 0);
	    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, sWidth, sHeight, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	if (pImageData != NULL)
		free(pImageData);

	return Status;
}


//get id of texture corresponding to character
int GetTextureID(char irg)
{
	if (irg == ' ') return ___TEXTURE;
	if (irg == '0') return _0_TEXTURE;
	if (irg == '1') return _1_TEXTURE;
	if (irg == '2') return _2_TEXTURE;
	if (irg == '3') return _3_TEXTURE;
	if (irg == '4') return _4_TEXTURE;
	if (irg == '5') return _5_TEXTURE;
	if (irg == '6') return _6_TEXTURE;
	if (irg == '7') return _7_TEXTURE;
	if (irg == '8') return _8_TEXTURE;
	if (irg == '9') return _9_TEXTURE;
	if (irg == 'A') return _A_TEXTURE;
	if (irg == 'b') return _b_TEXTURE;
	if (irg == 'd') return _d_TEXTURE;
	if (irg == 'E') return _E_TEXTURE;
	if (irg == 'F') return _F_TEXTURE;
	if (irg == 'G') return _G_TEXTURE;
	if (irg == 'I') return _I_TEXTURE;
	if (irg == 'K') return _K_TEXTURE;
	if (irg == 'L') return _L_TEXTURE;
	if (irg == 'M') return _M_TEXTURE;
	if (irg == 'n') return _n_TEXTURE;
	if (irg == 'o') return _o_TEXTURE;
	if (irg == 'P') return _P_TEXTURE;
	if (irg == 'r') return _r_TEXTURE;
	if (irg == 'S') return _S_TEXTURE;
	if (irg == 't') return _t_TEXTURE;
	if (irg == 'U') return _U_TEXTURE;
	if (irg == 'y') return _y_TEXTURE;
	if (irg == 'Q') return _Q_TEXTURE;
	if (irg == '=') return _x_TEXTURE;
	if (irg == '-') return _i_TEXTURE;
	return ___TEXTURE;
}

/*
This function is Windows only
*/
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
	strcat(INIFile, "edm800.ini");
	GetPrivateProfileString("common","FillMain","66",sFillMain,255,INIFile);
	GetPrivateProfileString("common","FillTop","44",sFillTop,255,INIFile);
	GetPrivateProfileString("common","Left","768",sLeft,255,INIFile);
	GetPrivateProfileString("common","Right","1024",sRight,255,INIFile);
	GetPrivateProfileString("common","Bottom","256",sBottom,255,INIFile);
	GetPrivateProfileString("common","Top","512",sTop,255,INIFile);
	GetPrivateProfileString("common","FillMain","0",sFuelUnit,255,INIFile);
	GetPrivateProfileString("common","FuelUnit","0",sFuelUnit,255,INIFile);
	GetPrivateProfileString("common","IndexRate","2",sIndexRate,255,INIFile);
	GetPrivateProfileString("common","RealMode","0",sRealMode,255,INIFile);
	GetPrivateProfileString("common","StartFuel","100",sStartFuel,255,INIFile);
	GetPrivateProfileString("common","FuelAlarmThreshold","10",sFuelThres,255,INIFile);
	GetPrivateProfileString("common","TimeAlarmThreshold","45",sTimeThres,255,INIFile);
	fFillMain = atof(sFillMain);
	fFillTop = atof(sFillTop);
	gaugeX1 = atof(sLeft);
	gaugeX2 = atof(sRight);
	gaugeY1 = atof(sBottom);
	gaugeY2 = atof(sTop);
	FuelUnit = atof(sFuelUnit);
	IndexRate = atof(sIndexRate) * 4;
	RealMode = atoi(sRealMode);
	StartFuel = atof(sStartFuel);
	fFuelThres = atof(sFuelThres);
	iTimeThres = atoi(sTimeThres);
	return TRUE;
}

/*
This function is Windows only
*/
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

	sprintf(sStartFuel,"%f",fRemFuel);

	strcpy(INIFile, gPluginDataFile);
	strcat(INIFile, "edm800.ini");
	WritePrivateProfileString("common","Left",sLeft,INIFile);
	WritePrivateProfileString("common","Right",sRight,INIFile);
	WritePrivateProfileString("common","Bottom",sBottom,INIFile);
	WritePrivateProfileString("common","Top",sTop,INIFile);
	WritePrivateProfileString("common","StartFuel",sStartFuel,INIFile);

	return TRUE;
}

/*
// Used for dragging plugin panel window and mouse clickable spots
*/
int	CoordInRect(float x, float y, float l, float t, float r, float b)
{	return ((x >= l) && (x < r) && (y < t) && (y >= b)); }


int PaintTexture(int texture, float w, float x, float y)
{
	XPLMBindTexture2d(gTexture[texture], 0);
	glBegin(GL_QUADS);
			glTexCoord2f(1, 0.0f); glVertex2f(x + w, y);			// Bottom Right 
			glTexCoord2f(0, 0.0f); glVertex2f(x, y);			// Bottom Left 		
			glTexCoord2f(0, 1.0f); glVertex2f(x, y + w);			// Top Left 
			glTexCoord2f(1, 1.0f); glVertex2f(x + w, y + w);			// Top Right 
	glEnd();
	return TRUE;
}