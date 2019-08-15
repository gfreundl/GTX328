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
#include <winbase.h>
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

001				copy template from EDM800




next:	


Manual:



Pseudocodes:



functions:



parameterws to save:
FLT_ID			active/inactive Callsign	
FLT_ID			Callsign
PWR_UP ENTRY	
VFR Code		regional VFR Code


Open Issues:


checks on the real thing:


*/

/// Texture stuff
#define MAX_TEXTURES 52

#define PANEL_FILENAME			"Panel.bmp"					//Bezel
#define T_FILENAME				"T.bmp"
#define CYL_DOT_FILENAME		"cyl_dot.bmp"	
#define _A_FILENAME				"_A.bmp"
#define _B_FILENAME				"_B.bmp"
#define _D_FILENAME				"_D.bmp"
#define _E_FILENAME				"_E.bmp"
#define _F_FILENAME				"_F.bmp"
#define _G_FILENAME				"_G.bmp"
#define _I_FILENAME				"_1.bmp"
#define _K_FILENAME				"_K.bmp"					//same as H, X
#define _L_FILENAME				"_L.bmp"
#define _M_FILENAME				"_M.bmp"
#define _N_FILENAME				"_N.bmp"
#define _P_FILENAME				"_P.bmp"
#define _R_FILENAME				"_R.bmp"
#define _S_FILENAME				"_5.bmp"					//same as 5
#define _T_FILENAME				"_T.bmp"
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
#define _Q_FILENAME				"_Q.bmp"					// "?" Question Mark 
#define _x_FILENAME				"_x.bmp"					// "=" Sign
#define ___FILENAME				"__.bmp"					// " " (blank) character
#define _C_FILENAME				"_C.bmp"			
#define _i_FILENAME				"_i.bmp"					// "-" dash 
#define FULL_FILENAME			"_fulldigit.bmp"			// full digit
#define PLUS_FILENAME			"_plus.bmp"					// + sign


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
#define _B_TEXTURE 13
#define _D_TEXTURE 14
#define _E_TEXTURE 15
#define _F_TEXTURE 16
#define _G_TEXTURE 17
#define _I_TEXTURE 18
#define _K_TEXTURE 19
#define _L_TEXTURE 20
#define _M_TEXTURE 21
#define _N_TEXTURE 22
#define _O_TEXTURE 23
#define _P_TEXTURE 24
#define _R_TEXTURE 25
#define _S_TEXTURE 26
#define _T_TEXTURE 27
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
#define _C_TEXTURE 43	
#define _i_TEXTURE 44
#define RBAR_TEXTURE 45	
#define RGAP_TEXTURE 46	
#define LGAP_TEXTURE 47	
#define HP_TEXTURE	48	
#define RPM_TEXTURE	49	
#define FULL_TEXTURE 50
#define PLUS_TEXTURE 51	

//Operational Modes and Menu Items
#define M_TEST			1
#define M_GND			2
#define	M_PA			3
#define M_TIME			4
#define M_ALT			5
#define M_CNTDN			6
#define M_CCNTUP		7

#define M_0				10
#define M_1				11	
#define M_2				12
#define M_3				13


XPLMDataRef	RED = NULL, GREEN = NULL, BLUE = NULL;
XPLMDataRef dFlightTime = NULL;
XPLMDataRef	dBattOn = NULL;
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


int gfeDisplayPanelWindow = 1;				//show panel from start
char gPluginDataFile[255];					//

char Symbol[4];								//content for LED
int FSMode;									//active Operation Mode we're in
int FSStep;									//active step in sequence 
int nTick;									//Counter for Seconds in Flightloop
int nTick2;									//Counter for Seconds in Flightloop
char cText0[4];								//Text in Row 1
char cText1[5];								//Text in Row 1
char cText2[4];								//Text in Row 2
int gaugeX1;								//gauge position bottom left
int gaugeX2;								//gauge position bottom right
int gaugeY1;								//gauge position top left
int gaugeY2;								//gauge position top right
int FirstCall = TRUE;						//first call since plugin start
int BattOn;									//electrical current on Bus 1
int RealMode = 0;							//1: work with fuel added on startup, 0: ignore added fuel, work with XPL fuel state data only
float RunTime;							
float DeltaTime;
float LastTime = 0;
int Code = 0;								//Transponder Code
int Ident = 0;								//Transponder Ident on/off
int Lit = 0;								//Transponder lit
int Mode = 0;								//Transponder Mode
int Fail = 0;								//Transponder Fail
float Baro = 0;								//current actual Baro pressue
float BaroAlt = 0;							//current indicated Baro altitide
int OnGround = 0;							//airplane is on ground

int i;										//free Counter, can be used anywhere
int n;										//Counter
int a;										//Counter
int b;										//reserved Counter
int c;
int k = 1;									//Counter for EGT temp filtering

float fTime = 0;							//seconds since sim startup
int iTime = 0;								//fractions of fTime multiplied with 1000 for timing purposes
float fBat[8];								//bus volts
float fOAT;									//outside air temperature °C


float PanelL, PanelR, PanelB, PanelT;
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



/*
////////////////////////////////////////// PROTOTYPES //////////////////////////////////////////
*/

/// Prototypes for callbacks etc.
GLvoid LoadTextures(GLvoid);

int GetTextureID(char);
int LoadGLTexture(char *pFileName, int TextureId);
int DrawGLScene(float x, float y);
void HotKey(void * refCon);
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
void SelfTest_1();
void SelfTest_2();
void DrawRPM();			
void DrawHeadlines();
void DrawBars();
void DrawLeftDigit();
void DrawRightDigit();
void CalcCLD();
void CalcEGT();
void GradientEGT();
void CalcFL();



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
		char *pFileName = "Resources\\Plugins\\gfeGTX328\\";
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
		gaugeX1 = 768;															//gauge position left
		gaugeX2 = 1024;															//gauge position right
		gaugeY1 = 256;															//gauge position bottom
		gaugeY2 = 512;															//gauge position top
		RealMode = 0;
	}
	
	/// Create window, setup datarefs and register  hotkeys
	ggfePanelDisplayWindow = XPLMCreateWindow(gaugeX1, gaugeY2, gaugeX2, gaugeY1, 1, gfePanelWindowCallback, gfePanelKeyCallback, gfePanelMouseClickCallback, NULL);

	dBattOn = XPLMFindDataRef("sim/cockpit/electrical/battery_on"); 
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

	gHotKey = XPLMRegisterHotKey(XPLM_VK_F5, xplm_UpFlag + xplm_ShiftFlag, "Shift-F5", gfeHotKey, NULL);

	//inital SelfTest Routine
	

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
		if (!BattOn)
		{
			FirstCall = TRUE;													//reset at each battery switch recycle
		}
		if (FirstCall)															//first Call since Plugin Start or Battery Switch on/off
		{
			if (RealMode == 1)
			{
				OpenINIFile();													
			}
			else
			FirstCall = FALSE;

			FSMode = M_TEST;
			nTick = 0;															//reset counter after restart
			b = 0;
			c = 0;
		}
		else
		{
			nTick = nTick;														//just a dummy for debug
		}

		// Need to find out where our window is
		XPLMGetWindowGeometry(ggfePanelDisplayWindow, &PanelWindowLeft, &PanelWindowTop, &PanelWindowRight, &PanelWindowBottom);
		/// Tell Xplane what we are doing
		XPLMSetGraphicsState(0/*Fog*/, 1/*TexUnits*/, 0/*Lighting*/, 0/*AlphaTesting*/, 0/*AlphaBlending*/, 0/*DepthTesting*/, 0/*DepthWriting*/);
		/// Handle day/night
		glColor3f(Red, Green, Blue);
		/// Setup our panel and gauge relative to our window
		PanelL = PanelWindowLeft; PanelR = PanelWindowRight; PanelB = PanelWindowBottom; PanelT = PanelWindowTop;
		// Original Panel Size = 256x256, so in case of resize, scale everything accordingly
		Scaler = (PanelR-PanelL)/256;

		// Row Coordinates
		HdWidth = 16;
		SymbolWidth = 32;
		SymbolWidth2 = 24;
		BarWidth = 8;
		DotWidth = 8;
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
		HeaderB = 175 * Scaler;
		HPL = 94 * Scaler;
		HPB = 190 * Scaler; 

		// Draw Panel
		XPLMBindTexture2d(gTexture[PANEL_TEXTURE], 0);
		glBegin(GL_QUADS);
				glTexCoord2f(1, 0.0f); glVertex2f(PanelR, PanelB);	// Bottom Right Of The Texture and Quad
				glTexCoord2f(0, 0.0f); glVertex2f(PanelL, PanelB);	// Bottom Left Of The Texture and Quad		
				glTexCoord2f(0, 1.0f); glVertex2f(PanelL, PanelT);		// Top Left Of The Texture and Quad
				glTexCoord2f(1, 1.0f); glVertex2f(PanelR, PanelT);	// Top Right Of The Texture and Quad
		glEnd();
	
		//get actual data from XPL
		i = XPLMGetDatavf(dBat, fBat, 0, 8);
		fOAT = XPLMGetDataf(dOAT);
		Code = XPLMGetDatai(dCode);
		Ident = XPLMGetDatai(dIdent);
		Lit = XPLMGetDatai(dCode);
		Mode = XPLMGetDatai(dMode);
		Fail = XPLMGetDatai(dFail);
		Baro = XPLMGetDatai(dBaro);
		BaroAlt = XPLMGetDatai(dBaroAlt);
		OnGround = XPLMGetDatai(dOnGround);


		
		if (OnGround)
		{
			FSMode = M_GND;
		}
		
		CalcFL();

		if (BattOn)
		{

			if (FSMode == M_TEST) 								//Selftest sequence - all symbols on
			{
				SelfTest_1();																		
			}
			else if (FSMode == M_GND)
			{

			}
			
			
			//now draw everything
			if (Ident) { DrawIdent() }


			else if (((FSMode == M_AUTOSEQ) || (FSMode == M_MANSEQ)) && (FSStep == M_TIT))
			{
				DrawRPM();
				DrawHeadlines();
				DrawBars();
				strcpy(cText2, "TIT");
				DrawRightDigit();
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
			HotKeySTEPCallback(0);												// left STEP button, need to work with percentage as panel could have been resized
		}
		if (CoordInRect(x, y, Left+(0.7*(Right-Left)), Top-(0.9*(Top-Bottom)), Left+(0.8*(Right-Left)), Top-(1.0*(Top-Bottom))))	
		{
			HotKeyLFCallback(0);												// right LF button, need to work with percentage as panel could have been resized
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
void	gfeHotKey(void * refCon)
{
	gfeDisplayPanelWindow = !gfeDisplayPanelWindow;
}

void	HotKeySTEPCallback(void * inRefcon)										//STEP button
{
	if (FSMode == M_AUTOSEQ)													//toggle modes
	{
		FSMode = M_MANSEQ;
	}
	else if ((FSMode == M_MANSEQ) && (iLFtap == 1))								//coming from an LF tap
	{
		FSMode = M_AUTOSEQ;
		//FSStep = M_BAT;
		iLFtap = 0;
	}
	else if (FSMode == M_LF)													//coming from LeanFind 
	{
		FSMode = M_AUTOSEQ;
		FSStep = M_BAT;
		iLFtap = 0;
	}
	else
	{
		FSStep++;
		if ((FSStep == M_OIL) && (OILOption == 0))						//skip OIL display when not configured
		{
			FSStep++;
		}

		if ((FSStep == M_TIT) && (TITOption == 0))						//skip TIT display when not configured
		{
			FSStep++;
		}
		if ((FSStep == M_IAT) && (TITOption == 0))						//skip IAT display when not configured
		{
			FSStep++;
		}		
		if ((FSStep == M_CDT) && (TITOption == 0))						//skip CDT display when not configured
		{
			FSStep++;
		}		
		if ((FSStep == M_CRB) && (CRBOption == 0))						//skip CRB display when not configured
		{
			FSStep++;
		}		
		if (FSStep > M_CLD)
		{
			FSStep = M_BAT;
		}	
	}
}

void	HotKeySTEPHoldCallback(void * inRefcon)									//Hold STEP button
{
}

void	HotKeyLFCallback(void * inRefcon)										//LeanFind button
{
	if ((FSMode == M_MANSEQ) && (iLFtap == 0))									//save LF tap for subsequent STEP tap
	{																			//LF followed by STEP changes from MANSEQ back to AUTOSEQ
		iLFtap = 1;
	}
	else if ((FSMode == M_AUTOSEQ) || (FSMode == M_MANSEQ))						//start LeanFind Procedure
	{	
		FSMode = M_LF;		
		FSStep = M_LF_LEANR;
		nTick = 2000;															//init for 2 sec display of "LEAN R"
	}
}

void HotKeyLFHoldCallback(void * inrefCon)										//Hold LeanFind button
{
	if (FSMode == M_AUTOSEQ)
	{
		if (iNRM == 0)
		{
			iNRM = 1;
			for (i = 0; i < 6; i++)													//step through cylinders
			{
				fEGTNRM[i] = fEGT[i];												//save current EGT and OIL
			}
			fOILNRM = fxplOIL[0];
		}
		else
		{
			iNRM = 0;
		}
		

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

	//FSMode = M_AUTOSEQ;
	

	if (nTick < 5)																//start with self test sequence
	{		
		FSMode = M_TEST;
	}
	else if (nTick < 30)
	{		
		FSMode = M_TEST;
	}
	else if (nTick == 30)
	{
	}
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
	if (!LoadGLTexture(_B_FILENAME, _B_TEXTURE))
		XPLMDebugString("Gauge texture _b_TEXTURE failed to load\n");
	if (!LoadGLTexture(_D_FILENAME, _D_TEXTURE))
		XPLMDebugString("Gauge texture _D_TEXTURE failed to load\n");
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
	if (!LoadGLTexture(_N_FILENAME, _N_TEXTURE))
		XPLMDebugString("Gauge texture _N_TEXTURE failed to load\n");
	if (!LoadGLTexture(_P_FILENAME, _P_TEXTURE))
		XPLMDebugString("Gauge texture _P_TEXTURE failed to load\n");
	if (!LoadGLTexture(_R_FILENAME, _R_TEXTURE))
		XPLMDebugString("Gauge texture _R_TEXTURE failed to load\n");
	if (!LoadGLTexture(_S_FILENAME, _S_TEXTURE))
		XPLMDebugString("Gauge texture _S_TEXTURE failed to load\n");
	if (!LoadGLTexture(_T_FILENAME, _T_TEXTURE))
		XPLMDebugString("Gauge texture _T_TEXTURE failed to load\n");
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
	if (!LoadGLTexture(_C_FILENAME, _C_TEXTURE))
		XPLMDebugString("Gauge texture _C_TEXTURE failed to load\n");
	if (!LoadGLTexture(_i_FILENAME, _i_TEXTURE))
		XPLMDebugString("Gauge texture _i_TEXTURE failed to load\n");
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
	if (!LoadGLTexture(FULL_FILENAME, FULL_TEXTURE))
		XPLMDebugString("Panel texture FULL_TEXTURE failed to load\n");
	if (!LoadGLTexture(PLUS_FILENAME, PLUS_TEXTURE))
		XPLMDebugString("Panel texture PLUS_TEXTURE failed to load\n");
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
	if (irg == 'B') return _B_TEXTURE;
	if (irg == 'C') return _C_TEXTURE;
	if (irg == 'D') return _D_TEXTURE;
	if (irg == 'E') return _E_TEXTURE;
	if (irg == 'F') return _F_TEXTURE;
	if (irg == 'G') return _G_TEXTURE;
	if (irg == 'I') return _I_TEXTURE;
	if (irg == 'K') return _K_TEXTURE;
	if (irg == 'L') return _L_TEXTURE;
	if (irg == 'M') return _M_TEXTURE;
	if (irg == 'N') return _N_TEXTURE;
	if (irg == 'O') return _0_TEXTURE;
	if (irg == 'P') return _P_TEXTURE;
	if (irg == 'R') return _R_TEXTURE;
	if (irg == 'S') return _S_TEXTURE;
	if (irg == 'T') return _T_TEXTURE;
	if (irg == 'U') return _U_TEXTURE;
	if (irg == 'y') return _y_TEXTURE;
	if (irg == 'Q') return _Q_TEXTURE;
	if (irg == '=') return _x_TEXTURE;
	if (irg == '-') return _i_TEXTURE;
	if (irg == '+') return PLUS_TEXTURE;
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
			glTexCoord2f(0, 0.0f); glVertex2f(x, y);				// Bottom Left 		
			glTexCoord2f(0, 1.0f); glVertex2f(x, y + w);			// Top Left 
			glTexCoord2f(1, 1.0f); glVertex2f(x + w, y + w);		// Top Right 
	glEnd();
	return TRUE;
}

void SelfTest_1()
{
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
		PaintTexture(T_TEXTURE, HdWidth, PanelL + BarL + n*BarWidth*2.5, PanelB +  HeaderB);
	}

	//draw dots
	for (n=0;n<6; n++)
	{
		PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + n*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95); 
	}

	//draw RPM full digits
	for  (i=0; i<4; i=i+1)																								
	{
		PaintTexture(FULL_TEXTURE, SymbolWidth2, PanelL + HPL + i*SymbolWidth2*0.6, PanelB + HPB);
	}
	PaintTexture(RPM_TEXTURE, SymbolWidth2, PanelL + HPL + 63, PanelB + HPB);

	//draw lower line full digits
	for  (i=0; i<4; i=i+1)												//left digits												
	{
		PaintTexture(FULL_TEXTURE, SymbolWidth, PanelL + DigitL + i*SymbolWidth*0.6, PanelB + DigitB);
	}
	for  (i=1; i<4; i=i+1)												//right digits
	{
		PaintTexture(FULL_TEXTURE, SymbolWidth, PanelL + DigitL + 2*SymbolWidth + i*SymbolWidth*0.6, PanelB + DigitB);
	}
}

void SelfTest_2()
{
	float fTime2;

	//draw lower line "EDM-800"
	strcpy(cText1, "EDM-");
	DrawLeftDigit();
	strcpy(cText2, "800");
	DrawRightDigit();

	// draw Bars
	fTime2 = XPLMGetDataf(dTime);

	if (b < 7)
	{
		if (b == 0)
		{
			PaintTexture(CYL1_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 1)
		{
			PaintTexture(CYL2_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 2)
		{
			PaintTexture(CYL3_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 3)
		{
			PaintTexture(CYL4_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 4)
		{
			PaintTexture(CYL5_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 5)
		{
			PaintTexture(CYL6_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
			PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
		}
		else if (b == 6)
		{
			//if (TITOption == 1)
			{
				PaintTexture(T_TEXTURE, HdWidth, PanelL + BarL + b*BarWidth*2.5, PanelB +  HeaderB);
				PaintTexture(CYL_DOT_TEXTURE, DotWidth, PanelL + BarL + b*BarWidth*2.5 + DotWidth*0.25, PanelB + HeaderB*0.95);
			}
		}

		if (c < 19)
		{
			for (i=0; i<c; i++)
			{
				PaintTexture(LBAR_TEXTURE, BarWidth, PanelL + BarL + b*BarWidth*2.5, PanelB + BarB + i*(0.375*BarWidth));
				PaintTexture(RBAR_TEXTURE, BarWidth, PanelL + BarL + b*BarWidth*2.5 + BarWidth, PanelB + BarB + i*(0.375*BarWidth));
			}
			if ((fTime2 - fTime) > 0.05)										//50ms timing for bargraph testing
			{
				fTime = fTime2;
				c++;
			}
		}
		if (c == 19)
		{
			c = 0;
			b++;
		}
	}
}

void DrawCode()
{
	for  (i=0; i<4; i++)												//left digits												
	{
		if (iCursor > -1)												//if cursor active
		{
			a = GetTextureID(cCode[i] + 10);							//pick inverted texture
		}
		else
		{
			a = GetTextureID(cCode[i]);
		}
		PaintTexture(a, SymbolWidth, PanelL + DigitL + i*SymbolWidth*0.6, PanelB + DigitB);
	}
}

//calculate flight level from current baro setting and indicated altitude
int CalcFL()
{ return (fBaroAlt - (((fBaro * 33.86) - 1013) * 27) / 100)						
}

//draw Init Page

//draw Mode

//draw Code

//draw R

//draw title

//draw FL

//draw time

//draw Ident
{
	PaintTexture(a, SymbolWidth, PanelL + DigitL + i*SymbolWidth*0.6, PanelB + DigitB);
}
