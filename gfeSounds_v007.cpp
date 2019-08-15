#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <time.h>

#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMPlugin.h"

//OpenAL
#include <al.h>
#include <alc.h>


/*
001				creates an authentic fuel pump sound for piston engine planes, original sound from SR22
				sound includes startup and shutdown sequence 
002				start external program when X-plane has loaded
				used for BFF CL driver
003				renamed to gfeSounds, 64bit port
				migrated previous Button2Dataref functions here
				rocker and momentary switches, King avionics, autopilot
004				requires XPLM200 preprocessor definition
				custom command radio master volume up/down
				custom command BFF driver load
				changed generator from integer to array
				combined HSI and NAV1 OBS input to accomodate RealityXP GNS430
005				drive flaps fully up/down when handle pushed up/down more than 1 second
				CommandRefs for all rocker switches
006				COM1 and COM2 volume control with analog potentiometers		
				fixed fuel pump runnning on reload
				Elev Trim switch: power selectively for trim unavailable, no effect on AP, needs AP reprog
007				corrected typo in OBS, search functions parses only name, not description
				Call of BFF external looks for aircraft ICAO type and selects appropriate BFF Config
				

Open Issues
ADF Timer stops after button press <- not verified !
adjust throttle scaling ? <- to what ?
rename CommandRefs to copy description, as search functions parses only name, not description


*/

#define XPLM200		

#define A_FUELPUMP_1	"gtx_timeexp_m.wav"								//pump winding up
#define A_FUELPUMP_2	"gtx_timeexp_m.wav"								//continous run
#define A_FUELPUMP_3	"gtx_timeexp_m.wav"								//pump winding down

XPLMDataRef d_joystick_axis_reverse = NULL;
XPLMDataRef d_joystick_axis_assignments = NULL;
XPLMDataRef d_joystick_axis_values = NULL;
XPLMDataRef d_joystick_axis_minimum = NULL;
XPLMDataRef d_joystick_axis_maximum = NULL;
XPLMDataRef d_joystick_button_values = NULL;
XPLMDataRef d_fuel_pump = NULL;
XPLMDataRef d_battery_on = NULL;
XPLMDataRef d_landing_lights_on = NULL;
XPLMDataRef d_generator_on = NULL;
XPLMDataRef d_avionics_on = NULL;
XPLMDataRef d_taxi_light_on = NULL;
XPLMDataRef d_beacon_lights_on = NULL;
XPLMDataRef d_nav_lights_on = NULL;
XPLMDataRef d_strobe_lights_on = NULL;
XPLMDataRef d_ice_pitot_heat_on_pilot = NULL;
XPLMDataRef d_carb_heat_ratio = NULL;
XPLMDataRef d_parking_brake_ratio = NULL;
XPLMDataRef d_alternate_static_air_ratio = NULL;
XPLMDataRef d_audio_marker_enabled = NULL;
XPLMDataRef d_audio_dme_enabled = NULL;
XPLMDataRef d_DME_slave_source = NULL;
XPLMDataRef d_adf1_right_is_selected = NULL;
XPLMDataRef d_dme_power = NULL;
XPLMDataRef d_DME_mode = NULL;
XPLMDataRef d_adf1_standby_frequency_hz = NULL;
XPLMDataRef d_dme_frequency_hz = NULL;
XPLMDataRef d_nav2_power = NULL;
XPLMDataRef d_com2_power = NULL;
XPLMDataRef d_nav1_obs_deg_mag_pilot = NULL;
XPLMDataRef d_hsi_obs_deg_mag_pilot = NULL;
XPLMDataRef d_heading_dial_deg_mag_pilot = NULL;
XPLMDataRef d_nav2_obs_deg_mag_pilot = NULL;
XPLMDataRef d_adf1_card_heading_deg_mag_pilot= NULL;
XPLMDataRef d_annunciator_test_timeout = NULL;
XPLMDataRef d_annunciator_test_pressed = NULL;
XPLMDataRef d_fire_mode = NULL;
XPLMDataRef d_flight_director_mode = NULL;
XPLMDataRef d_adf1_power = NULL;
XPLMDataRef d_timer_is_running_sec = NULL;
XPLMDataRef d_timer_elapsed_time_sec = NULL;
XPLMDataRef d_adf2_power = NULL;
XPLMDataRef d_dg_drift_ele_deg = NULL;
XPLMDataRef d_left_brake_ratio = NULL;
XPLMDataRef d_right_brake_ratio = NULL;
XPLMDataRef d_radio_volume_ratio = NULL;
XPLMDataRef d_flap_ratio = NULL;
XPLMDataRef d_audio_volume_com1 = NULL;
XPLMDataRef d_audio_volume_com2 = NULL;
XPLMDataRef d_acf_ICAO = NULL;

XPLMCommandRef c_servos_on = NULL;
XPLMCommandRef c_servos_fdir_off = NULL;
XPLMCommandRef gfe_elev_trim = NULL;
XPLMCommandRef gfe_radios_vol_up = NULL;
XPLMCommandRef gfe_radios_vol_dn = NULL;
XPLMCommandRef gfe_adf_100_10_up = NULL;
XPLMCommandRef gfe_adf_100_10_down = NULL;
XPLMCommandRef gfe_adf_1_up = NULL;
XPLMCommandRef gfe_adf_1_down = NULL;
XPLMCommandRef gfe_hsi_obs_1_up = NULL;
XPLMCommandRef gfe_hsi_obs_1_down = NULL;
XPLMCommandRef gfe_hsi_obs_10_up = NULL;
XPLMCommandRef gfe_hsi_obs_10_down = NULL;
XPLMCommandRef gfe_hsi_hdg_1_up = NULL;
XPLMCommandRef gfe_hsi_hdg_1_down = NULL;
XPLMCommandRef gfe_hsi_hdg_10_up = NULL;
XPLMCommandRef gfe_hsi_hdg_10_down = NULL;
XPLMCommandRef gfe_obs2_1_up = NULL;
XPLMCommandRef gfe_obs2_1_down = NULL;
XPLMCommandRef gfe_obs2_10_up = NULL;
XPLMCommandRef gfe_obs2_10_down = NULL;
XPLMCommandRef gfe_rmi_1_up = NULL;
XPLMCommandRef gfe_rmi_1_down = NULL;
XPLMCommandRef gfe_rmi_10_up = NULL;
XPLMCommandRef gfe_rmi_10_down = NULL;
XPLMCommandRef gfe_ext_bff = NULL;
XPLMCommandRef gfe_flap_ratio_up = NULL;
XPLMCommandRef gfe_flap_ratio_down = NULL;
XPLMCommandRef gfe_land_light = NULL;
XPLMCommandRef gfe_battery = NULL;
XPLMCommandRef gfe_avionics = NULL;
XPLMCommandRef gfe_taxi_light = NULL;
XPLMCommandRef gfe_beacon_lights = NULL;
XPLMCommandRef gfe_nav_lights = NULL;
XPLMCommandRef gfe_strobe_lights = NULL;
XPLMCommandRef gfe_ice_pitot_heat_on_pilot = NULL;
XPLMCommandRef gfe_fuel_pump = NULL;
XPLMCommandRef gfe_generator_on = NULL;
XPLMCommandRef gfe_carb_heat_ratio = NULL;
XPLMCommandRef gfe_audio_marker_enabled = NULL;
XPLMCommandRef gfe_audio_dme_enabled = NULL;
XPLMCommandRef gfe_annunciator_test_timeout = NULL;
XPLMCommandRef gfe_dme_power = NULL;
XPLMCommandRef gfe_DME_slave_source = NULL;
XPLMCommandRef gfe_DME_mode_0 = NULL;
XPLMCommandRef gfe_DME_mode_1 = NULL;
XPLMCommandRef gfe_DME_mode_2 = NULL;
XPLMCommandRef gfe_KAP_test = NULL;
XPLMCommandRef gfe_KAP_FD = NULL;

int OLDjoystick_button_values[1600] = { 0 };
int joystick_button_values[1600] = { 0 };
float joystick_axis_values[500] = { 0.0 };
int fuel_pump;
int battery_on;
int landing_lights_on;
int generator_on;
int avionics_on;
int taxi_light_on;
int beacon_lights_on;
int nav_lights_on;
int strobe_lights_on;
int ice_pitot_heat_on_pilot;
int carb_heat_ratio;
int parking_brake_ratio;
int alternate_static_air_ratio;
int audio_marker_enabled;
int audio_dme_enabled;
int DME_slave_source;
int adf1_right_is_selected;
int dme_power;
int DME_mode;
int adf1_standby_frequency_hz;
int dme_frequency_hz;
int nav2_power;
int com2_power;
int nav1_obs_deg_mag_pilot;
int heading_dial_deg_mag_pilot;
int nav2_obs_deg_mag_pilot;
int adf1_card_heading_deg_mag_pilot = NULL;
int annunciator_test_timeout;
int fire_mode;
int flight_director_mode;
int fdtest = 0;																	//FD test mode
int adf1_power;
int timer_is_running_sec;
int timer_elapsed_time_sec;
int adf2_power;
int dg_drift_ele_deg;
int flaps_timer = 0;

int FuelPump[8] = {0};
int FuelPumpOld[8] = {0};
int i = 0;
int n = 0;
char gPluginDataFile[255];														//file path and name
DWORD timestamp;																//timer for KAP150 test sequence

static ALuint			snd_src[3];												// Sample source and buffer - this is one "sound" we play.
static ALuint			snd_buffer[3];
static float			pitch		= 1.0f;										// Start with 1.0 pitch - no pitch shift.
ALCdevice *my_dev = NULL;														// We make our own device and context to play sound through.
ALCcontext *my_ctx= NULL;
ALCcontext *old_ctx = NULL;														// have to save old AL context
char sAudioDevice[255];															// read device name from INI File

static float init_sound(float elapsed, float elapsed_sim, int counter, void * ref);
float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon);    
float call_ext_FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon);    
int gfe_elev_trim_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_radios_vol_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_radios_vol_dn_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_adf_100_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_adf_100_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_adf_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_adf_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_obs_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_obs_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_obs_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_obs_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_hdg_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_hdg_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_hdg_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_hsi_hdg_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_obs2_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_obs2_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_obs2_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_obs2_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_rmi_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_rmi_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_rmi_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_rmi_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_ext_bff_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_flap_ratio_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_flap_ratio_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_land_light_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_battery_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_avionics_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_taxi_light_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_beacon_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_nav_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_strobe_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_ice_pitot_heat_on_pilot_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_fuel_pump_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_generator_on_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_carb_heat_ratio_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_audio_marker_enabled_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_audio_dme_enabled_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_annunciator_test_timeout_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_dme_power_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_DME_slave_source_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_DME_mode_0_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_DME_mode_1_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_DME_mode_2_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
int gfe_KAP_test_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);

/*--------------------------------  End of Declarations --------------------------------*/

PLUGIN_API int XPluginStart(char * name, char * sig, char * desc)
{
	char *pFileName = "Resources\\plugins\\";

	/// Setup texture and ini file locations
	XPLMGetSystemPath(gPluginDataFile);
	strcat(gPluginDataFile, pFileName);

	strcpy(name,"gfe Sounds & Switches");
	strcpy(sig,"gfe.sound_switches");
	strcpy(desc,"Fuel pump sound, King avionics and other switches");
	
	d_joystick_axis_reverse = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
	d_joystick_axis_assignments = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
	d_joystick_axis_minimum = XPLMFindDataRef("sim/joystick/joystick_axis_minimum");
	d_joystick_axis_maximum = XPLMFindDataRef("sim/joystick/joystick_axis_maximum");
	d_joystick_axis_values = XPLMFindDataRef("sim/joystick/joystick_axis_values");
	d_joystick_button_values = XPLMFindDataRef("sim/joystick/joystick_button_values");
	d_battery_on = XPLMFindDataRef("sim/cockpit/electrical/battery_on");
	d_fuel_pump = XPLMFindDataRef("sim/cockpit2/engine/actuators/fuel_pump_on");
	d_landing_lights_on = XPLMFindDataRef("sim/cockpit/electrical/landing_lights_on");
	d_generator_on = XPLMFindDataRef("sim/cockpit/electrical/generator_on");
	d_avionics_on = XPLMFindDataRef("sim/cockpit/electrical/avionics_on");
	d_taxi_light_on = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
	d_beacon_lights_on = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
	d_nav_lights_on = XPLMFindDataRef("sim/cockpit/electrical/nav_lights_on");
	d_strobe_lights_on = XPLMFindDataRef("sim/cockpit/electrical/strobe_lights_on");
	d_ice_pitot_heat_on_pilot = XPLMFindDataRef("sim/cockpit2/ice/ice_pitot_heat_on_pilot");
	d_carb_heat_ratio = XPLMFindDataRef("sim/cockpit2/engine/actuators/carb_heat_ratio");
	d_parking_brake_ratio = XPLMFindDataRef("sim/cockpit2/controls/parking_brake_ratio");
	d_alternate_static_air_ratio = XPLMFindDataRef("sim/cockpit2/switches/alternate_static_air_ratio");
	d_audio_marker_enabled = XPLMFindDataRef("sim/cockpit2/radios/actuators/audio_marker_enabled");
	d_audio_dme_enabled = XPLMFindDataRef("sim/cockpit2/radios/actuators/audio_dme_enabled");
	d_DME_slave_source = XPLMFindDataRef("sim/cockpit2/radios/actuators/DME_slave_source");
	d_adf1_right_is_selected = XPLMFindDataRef("sim/cockpit2/radios/actuators/adf1_right_is_selected");
	d_dme_power = XPLMFindDataRef("sim/cockpit2/radios/actuators/dme_power");
	d_DME_mode = XPLMFindDataRef("sim/cockpit2/radios/actuators/DME_mode");
	d_adf1_standby_frequency_hz = XPLMFindDataRef("sim/cockpit2/radios/actuators/adf1_standby_frequency_hz");
	d_dme_frequency_hz = XPLMFindDataRef("sim/cockpit2/radios/actuators/dme_frequency_hz");
	d_nav2_power = XPLMFindDataRef("sim/cockpit2/radios/actuators/nav2_power");
	d_com2_power = XPLMFindDataRef("sim/cockpit2/radios/actuators/com2_power");
	d_nav1_obs_deg_mag_pilot = XPLMFindDataRef("sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot");
	d_hsi_obs_deg_mag_pilot = XPLMFindDataRef("sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot");
	d_heading_dial_deg_mag_pilot = XPLMFindDataRef("sim/cockpit2/autopilot/heading_dial_deg_mag_pilot");
	d_nav2_obs_deg_mag_pilot = XPLMFindDataRef("sim/cockpit2/radios/actuators/nav2_obs_deg_mag_pilot");
	d_adf1_card_heading_deg_mag_pilot = XPLMFindDataRef("sim/cockpit2/radios/actuators/adf1_card_heading_deg_mag_pilot");
	d_annunciator_test_timeout = XPLMFindDataRef("sim/cockpit/warnings/annunciator_test_timeout");
	d_annunciator_test_pressed = XPLMFindDataRef("sim/cockpit/warnings/d_annunciator_test_pressed");
	d_fire_mode = XPLMFindDataRef("sim/cockpit2/weapons/fire_mode");
	d_flight_director_mode = XPLMFindDataRef("sim/cockpit2/autopilot/flight_director_mode");
	d_adf1_power = XPLMFindDataRef("sim/cockpit2/radios/actuators/adf1_power");
	d_timer_is_running_sec = XPLMFindDataRef("sim/time/timer_is_running_sec");
	d_timer_elapsed_time_sec = XPLMFindDataRef("sim/time/timer_elapsed_time_sec");
	d_adf2_power = XPLMFindDataRef("sim/cockpit2/radios/actuators/adf2_power");
	d_dg_drift_ele_deg = XPLMFindDataRef("sim/cockpit/gyros/dg_drift_ele_deg");
	d_left_brake_ratio = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
	d_right_brake_ratio = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
	d_radio_volume_ratio = XPLMFindDataRef("sim/operation/sound/radio_volume_ratio");
	d_flap_ratio = XPLMFindDataRef("sim/cockpit2/controls/flap_ratio");
	d_audio_volume_com1 = XPLMFindDataRef("sim/cockpit2/radios/actuators/audio_volume_com1");
	d_audio_volume_com2 = XPLMFindDataRef("sim/cockpit2/radios/actuators/audio_volume_com2");
	d_acf_ICAO = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
	c_servos_fdir_off = XPLMFindCommand("sim / autopilot / servos_fdir_off");
	c_servos_on = XPLMFindCommand("sim/autopilot/servos_on");

	// Create custom commands
	gfe_elev_trim = XPLMCreateCommand("gfe/Autopilot/ElevTrim", "Elev Trim");
	gfe_radios_vol_up = XPLMCreateCommand("gfe/Avionics/VolUp", "Radios Volume Up");
	gfe_radios_vol_dn = XPLMCreateCommand("gfe/Avionics/VolDn", "Radios Volume Down");
	gfe_adf_100_10_up = XPLMCreateCommand("gfe/Avionics/ADF_100_10_up", "ADF coarse up");
	gfe_adf_100_10_down = XPLMCreateCommand("gfe/Avionics/100_10_down", "ADF coarse down");
	gfe_adf_1_up = XPLMCreateCommand("gfe/Avionics/ADF_1_up", "ADF fine up");
	gfe_adf_1_down = XPLMCreateCommand("gfe/Avionics/ADF_1_down", "ADF fine down");
	gfe_hsi_obs_1_up = XPLMCreateCommand("gfe/Avionics/HSI_OBS_1_up", "HSI up");
	gfe_hsi_obs_1_down = XPLMCreateCommand("gfe/Avionics/HSI_OBS_1_down", "HSI down");
	gfe_hsi_obs_10_up = XPLMCreateCommand("gfe/Avionics/HSI_OBS_10_up", "HSI tens up");
	gfe_hsi_obs_10_down = XPLMCreateCommand("gfe/Avionics/HSI_OBS_10_down", "HSI tens down");
	gfe_hsi_hdg_1_up = XPLMCreateCommand("gfe/Avionics/HSI_HDG_1_up", "HDG up");
	gfe_hsi_hdg_1_down = XPLMCreateCommand("gfe/Avionics/HSI_HDG_1_down", "HDG down");
	gfe_hsi_hdg_10_up = XPLMCreateCommand("gfe/Avionics/HSI_HDG_10_up", "HDG tens up");
	gfe_hsi_hdg_10_down = XPLMCreateCommand("gfe/Avionics/HSI_HDG_10_down", "HDG tens down");
	gfe_obs2_1_up = XPLMCreateCommand("gfe/Avionics/OBS2_1_up", "OBS2 up");
	gfe_obs2_1_down = XPLMCreateCommand("gfe/Avionics/OBS2_1_down", "OBS2 down");
	gfe_obs2_10_up = XPLMCreateCommand("gfe/Avionics/OBS2_10_up", "OBS2 tens up");
	gfe_obs2_10_down = XPLMCreateCommand("gfe/Avionics/OBS2_10_down", "OBS2 tens down");
	gfe_rmi_1_up = XPLMCreateCommand("gfe/Avionics/RMI_1_up", "RMI up");
	gfe_rmi_1_down = XPLMCreateCommand("gfe/Avionics/RMI_1_down", "RMI down");
	gfe_rmi_10_up = XPLMCreateCommand("gfe/Avionics/RMI_10_up", "RMI tens up");
	gfe_rmi_10_down = XPLMCreateCommand("gfe/Avionics/RMI_10_down", "RMI tens down");
	gfe_ext_bff = XPLMCreateCommand("gfe/Externals/bff", "Load external programm BFF");
	gfe_flap_ratio_up = XPLMCreateCommand("gfe/Controls/flaps_up", "Set flaps up");
	gfe_flap_ratio_down = XPLMCreateCommand("gfe/Controls/flaps_down", "Set flaps down");
	gfe_land_light = XPLMCreateCommand("gfe/Electrical/land_light", "Landing Light");
	gfe_battery = XPLMCreateCommand("gfe/Electrical/Battery", "Battery");
	gfe_avionics = XPLMCreateCommand("gfe/Electrical/Avionics", "Avionics");
	gfe_taxi_light = XPLMCreateCommand("gfe/Electrical/taxi_light", "Taxi Light");
	gfe_beacon_lights = XPLMCreateCommand("gfe/Electrical/beacon_lights", "Beacon Lights");
	gfe_nav_lights = XPLMCreateCommand("gfe/Electrical/nav_lights", "Nav Lights");
	gfe_strobe_lights = XPLMCreateCommand("gfe/Electrical/strobe_lights", "Strobe Lights");
	gfe_ice_pitot_heat_on_pilot = XPLMCreateCommand("gfe/Electrical/ice_pitot_heat_on_pilot", "Pitot Heat");
	gfe_fuel_pump = XPLMCreateCommand("gfe/Engine/fuel_pump", "Fuel Pump");
	gfe_generator_on = XPLMCreateCommand("gfe/Electrical/generator_on", "Generator");
	gfe_carb_heat_ratio = XPLMCreateCommand("gfe/Engine/carb_heat_ratio", "Carburetor Heat");
	gfe_audio_marker_enabled = XPLMCreateCommand("gfe/Avionics/audio_marker_enabled", "Marker Mute");
	gfe_audio_dme_enabled = XPLMCreateCommand("gfe/Avionics/audio_dme_enabled", "DME Audio");
	gfe_annunciator_test_timeout = XPLMCreateCommand("gfe/Electrical/annunciator_test_timeout", "Annunciator Test");
	gfe_dme_power = XPLMCreateCommand("gfe/Avionics/dme_power", "DME Power");
	gfe_DME_slave_source = XPLMCreateCommand("gfe/Avionics/DME_slave_source", "DME Source");
	gfe_DME_mode_0 = XPLMCreateCommand("gfe/Avionics/DME_mode_0", "DME Mode RMT");
	gfe_DME_mode_1 = XPLMCreateCommand("gfe/Avionics/DME_mode_1", "DME Mode FREQ");
	gfe_DME_mode_2 = XPLMCreateCommand("gfe/Avionics/DME_mode_2", "DME Mode GS/T");
	gfe_KAP_test = XPLMCreateCommand("gfe/Autopilot/KAP_test", "KAP 150 Test");

	// Register custom commands
	XPLMRegisterCommandHandler(gfe_elev_trim, gfe_elev_trim_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_radios_vol_up, gfe_radios_vol_up_Proc, 1, (void *)0);          
	XPLMRegisterCommandHandler(gfe_radios_vol_dn, gfe_radios_vol_dn_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_adf_100_10_up, gfe_adf_100_10_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_adf_100_10_down, gfe_adf_100_10_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_adf_1_up, gfe_adf_1_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_adf_1_down, gfe_adf_1_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_obs_1_up, gfe_hsi_obs_1_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_obs_1_down, gfe_hsi_obs_1_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_obs_10_up, gfe_hsi_obs_10_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_obs_10_down, gfe_hsi_obs_10_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_hdg_1_up, gfe_hsi_hdg_1_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_hdg_1_down, gfe_hsi_hdg_1_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_hdg_10_up, gfe_hsi_hdg_10_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_hsi_hdg_10_down, gfe_hsi_hdg_10_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_obs2_1_up, gfe_obs2_1_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_obs2_1_down, gfe_obs2_1_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_obs2_10_up, gfe_obs2_10_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_obs2_10_down, gfe_obs2_10_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_rmi_1_up, gfe_rmi_1_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_rmi_1_down, gfe_rmi_1_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_rmi_10_up, gfe_rmi_10_up_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_rmi_10_down, gfe_rmi_10_down_Proc, 1, (void *)0);
	XPLMRegisterCommandHandler(gfe_ext_bff, gfe_ext_bff_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_flap_ratio_up, gfe_flap_ratio_up_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_flap_ratio_down, gfe_flap_ratio_down_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_land_light, gfe_land_light_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_battery, gfe_battery_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_avionics, gfe_avionics_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_taxi_light, gfe_taxi_light_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_beacon_lights, gfe_beacon_lights_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_nav_lights, gfe_nav_lights_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_strobe_lights, gfe_strobe_lights_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_ice_pitot_heat_on_pilot, gfe_ice_pitot_heat_on_pilot_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_fuel_pump, gfe_fuel_pump_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_generator_on, gfe_generator_on_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_carb_heat_ratio, gfe_carb_heat_ratio_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_audio_marker_enabled, gfe_audio_marker_enabled_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_audio_dme_enabled, gfe_audio_dme_enabled_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_annunciator_test_timeout, gfe_annunciator_test_timeout_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_dme_power, gfe_dme_power_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_DME_slave_source, gfe_DME_slave_source_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_DME_mode_0, gfe_DME_mode_0_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_DME_mode_1, gfe_DME_mode_1_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_DME_mode_2, gfe_DME_mode_2_Proc, 1, (void*)0);
	XPLMRegisterCommandHandler(gfe_KAP_test, gfe_KAP_test_Proc, 1, (void*)0);

	// Launch external program once XPL has fully loaded	
	// Do deferred  initialization. See http://www.xsquawkbox.net/xpsdk/mediawiki/DeferredInitialization
	//XPLMRegisterFlightLoopCallback(call_ext_FlightLoopCallback,-1.0,NULL);	

	// Do deferred sound initialization. See http://www.xsquawkbox.net/xpsdk/mediawiki/DeferredInitialization
	XPLMRegisterFlightLoopCallback(init_sound, -1.0, NULL);

	//Register FlightLoop Callback for every 250msec
	XPLMRegisterFlightLoopCallback(
		MyFlightLoopCallback,	/* Callback */
		0.10,					/* Interval */
		NULL);					/* refcon not used. */

	//defaults for cold & dark start
	XPLMSetDatai(d_battery_on, 0);												//default to cold & dark
	XPLMSetDatai(d_adf2_power, 0);												//defaut ADF to non-timer mode
	XPLMSetDatai(d_fire_mode, 3);												//on Battery on, bring KAP150 in pre-test mode
	int a[8] = {0,0,0,0,0,0,0,0};												//temp array for XPLMSetDatavi, int a[8]={0} no good
	XPLMSetDatavi(d_fuel_pump, a, 0, 8);										//reset fuel pump oon start

	return 1;
}

PLUGIN_API void XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
	//XPLMUnregisterFlightLoopCallback(call_ext_FlightLoopCallback, NULL);
	XPLMUnregisterCommandHandler(gfe_elev_trim, gfe_elev_trim_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_radios_vol_up, gfe_radios_vol_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_radios_vol_dn, gfe_radios_vol_dn_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_adf_100_10_up, gfe_adf_100_10_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_adf_100_10_down, gfe_adf_100_10_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_adf_1_up, gfe_adf_1_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_adf_1_down, gfe_adf_1_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_obs_1_up, gfe_hsi_obs_1_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_obs_1_down, gfe_hsi_obs_1_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_obs_10_up, gfe_hsi_obs_10_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_obs_10_down, gfe_hsi_obs_10_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_hdg_1_up, gfe_hsi_hdg_1_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_hdg_1_down, gfe_hsi_hdg_1_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_hdg_10_up, gfe_hsi_hdg_10_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_hsi_hdg_10_down, gfe_hsi_hdg_10_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_obs2_1_up, gfe_obs2_1_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_obs2_1_down, gfe_obs2_1_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_obs2_10_up, gfe_obs2_10_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_obs2_10_down, gfe_obs2_10_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_rmi_1_up, gfe_rmi_1_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_rmi_1_down, gfe_rmi_1_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_rmi_10_up, gfe_rmi_10_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_rmi_10_down, gfe_rmi_10_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_ext_bff, gfe_ext_bff_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_flap_ratio_up, gfe_flap_ratio_up_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_flap_ratio_down, gfe_flap_ratio_down_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_land_light, gfe_land_light_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_battery, gfe_battery_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_avionics, gfe_avionics_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_taxi_light, gfe_taxi_light_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_beacon_lights, gfe_beacon_lights_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_nav_lights, gfe_nav_lights_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_strobe_lights, gfe_strobe_lights_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_ice_pitot_heat_on_pilot, gfe_ice_pitot_heat_on_pilot_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_fuel_pump, gfe_fuel_pump_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_generator_on, gfe_generator_on_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_carb_heat_ratio, gfe_carb_heat_ratio_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_audio_marker_enabled, gfe_audio_marker_enabled_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_audio_dme_enabled, gfe_audio_dme_enabled_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_annunciator_test_timeout, gfe_annunciator_test_timeout_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_dme_power, gfe_dme_power_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_DME_slave_source, gfe_DME_slave_source_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_DME_mode_0, gfe_DME_mode_0_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_DME_mode_1, gfe_DME_mode_1_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_DME_mode_2, gfe_DME_mode_2_Proc, 0, 0);
	XPLMUnregisterCommandHandler(gfe_KAP_test, gfe_KAP_test_Proc, 0, 0);

	// Cleanup: nuke our context if we have it.  This is hacky and bad - we should really destroy
	// our buffers and sources.  I have _no_ idea if OpenAL will leak memory.
	if(alcGetCurrentContext() != NULL)
	{
		//printf("0x%08x: deleting snd %d\n", XPLMGetMyID(),snd_buffer);
		if(snd_src[0])		alDeleteSources(1,&snd_src[0]);
		if(snd_buffer[0]) alDeleteBuffers(1,&snd_buffer[0]);
	}
	if(my_ctx) 
	{
		//printf("0x%08x: deleting my context 0x%08x\n", XPLMGetMyID(),my_ctx);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(my_ctx);
	}
	if(my_dev) alcCloseDevice(my_dev);	
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					long			inMessage,
					void *			inParam)
{
}


//Main flight loop
float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{
	static int nTick;
	static int firstLoop;
																				
	XPLMSetDataf(d_parking_brake_ratio, 0.0);									//unset parking brake

	//read all joystick axis vaules and assigments
	i = XPLMGetDatavf(d_joystick_axis_values, joystick_axis_values, 0, 500);
	//edit response function to exponential parameter
	//exponential function: y = x/n * pow(n,x) (with joystick value x = 0..1 and factor n = 1..20 and result 0..1)
	//joystick_axis_values[6] = joystick_axis_values[6] / 20 * pow(20, joystick_axis_values[6]);
	//joystick_axis_values[7] = joystick_axis_values[7] / 20 * pow(20, joystick_axis_values[7]);
	//XPLMSetDataf(d_left_brake_ratio, joystick_axis_values[6]);
	//XPLMSetDataf(d_right_brake_ratio, joystick_axis_values[7]);
	for (i = 0; i < 1600; i++)													//save last joystick state 
	{																			//for detecting long press
		OLDjoystick_button_values[i] = joystick_button_values[i];				
	}
	//read all joystick button values
	i = XPLMGetDatavi(d_joystick_button_values, joystick_button_values, 0, 1600);

	//KAP150 Test sequence
	//still need to make this Elev Trim switch dependent
	if (TRUE)
	{
		if ((fire_mode == 2) && (GetTickCount() - timestamp > 2000))
		{
			XPLMSetDatai(d_fire_mode, 1);											//all lights on
			fire_mode = 1;
		}
		if ((fire_mode == 1) && (GetTickCount() - timestamp > 5000))
		{
			XPLMSetDatai(d_fire_mode, 0);											//flash TRIM and AP
			fire_mode = 0;
			XPLMSetDatai(d_flight_director_mode, 2);
			fdtest = 1;																//test cycle finished
			/*no success in trying to play sound, servo activation gets persistent*/
			/*XPLMCommandButtonPress(xplm_joy_ott_dis);
			XPLMCommandButtonRelease(xplm_joy_ott_dis);*/
			/*XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_on"));
			XPLMCommandOnce(c_servos_fdir_off);
			XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_toggle")); */
		}
		if ((fire_mode == 0) && (GetTickCount() - timestamp > 5050) && (fdtest))
		{
			XPLMSetDatai(d_flight_director_mode, 0);
			fdtest = 0;
		}
	}

	//ADF start timer on powering up, reset on power off
	if (XPLMGetDatai(d_adf1_power) > 1 ) 
	{ 
		XPLMSetDatai(d_timer_is_running_sec, 1);
	}
	else
	{
		XPLMSetDatai(d_timer_is_running_sec, 0);
		XPLMSetDatai(d_adf2_power, 0);
	}
	//ET mode with fake dataref, display timer when d_adf2_power=1
	if ((joystick_button_values[483] == 1) && (OLDjoystick_button_values[483] == 0))
	{
		XPLMSetDatai(d_adf2_power, !XPLMGetDatai(d_adf2_power));
	}
	//reset timer
	if ((joystick_button_values[480] == 1) && (OLDjoystick_button_values[480] == 0))
	{
		XPLMSetDataf(d_timer_elapsed_time_sec, 0.0);
	}

	//KA51 slaved gyro
	if (joystick_button_values[1137] == 1)
	{
		XPLMSetDataf(d_dg_drift_ele_deg, 0.0);
	}
	if (joystick_button_values[1146] == 1) 
	{
		XPLMSetDataf(d_dg_drift_ele_deg, XPLMGetDataf(d_dg_drift_ele_deg) - 0.2);
	}
	if (joystick_button_values[1136] == 1)
	{
		XPLMSetDataf(d_dg_drift_ele_deg, XPLMGetDataf(d_dg_drift_ele_deg) + 0.2);
	}

	//play pump sound when pump runs
	i = XPLMGetDatavi(d_fuel_pump, FuelPump, 0, 8);
	for (i=0;i<2;i++)
	{
		if ((FuelPump[i] == 1) && (FuelPumpOld[i] == 0))		//
		{
			alSourcef(snd_src[0],AL_PITCH,pitch);				//play start sound
			alSourcePlay(snd_src[0]);
			FuelPumpOld[i] = 1;									//"pump running" flag
			firstLoop = 1;
		}
		else if ((FuelPump[i] == 1) && (FuelPumpOld[i] == 1))	//pump sound is already running
		{
			if ((nTick % 20 == 0) || (firstLoop))				//loop initially after startup and then every second
			{
				alSourceStop(snd_src[1]);						
				alSourcef(snd_src[1],AL_PITCH,pitch);			//play continous sound
				alSourcePlay(snd_src[1]);
				firstLoop = 0;
			}
		}
		else if ((FuelPump[i] == 0) && (FuelPumpOld[i] == 1))	//pump has been shut off but sound is still on
		{
			alSourceStop(snd_src[1]);
			alSourcef(snd_src[2],AL_PITCH,pitch);				//play shutoff sound
			alSourcePlay(snd_src[2]);
			FuelPumpOld[i] = 0;
		}
	}

	//adjust COM1 and COM2 radio volume
	XPLMSetDataf(d_audio_volume_com1, joystick_axis_values[226]);
	XPLMSetDataf(d_audio_volume_com2, joystick_axis_values[76]);

	nTick++;

	return 0.10;														// Return time interval after that we want to be called again
}

/*--------------------------------  Custom Commands --------------------------------*/
int gfe_elev_trim_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		//XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_on"));
	}
	else
	{
		XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_fdir_off"));
		XPLMSetDatai(d_fire_mode, 3);												//bring KAP150 back in pre-test mode
	}
	return 0;
}

int gfe_radios_vol_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_radio_volume_ratio) <= 0.95)
		{
			XPLMSetDataf(d_radio_volume_ratio, XPLMGetDataf(d_radio_volume_ratio) + 0.05);
		}
	}
	return 0;
}

int gfe_radios_vol_dn_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_radio_volume_ratio) >= 0.05)
		{
			XPLMSetDataf(d_radio_volume_ratio, XPLMGetDataf(d_radio_volume_ratio) - 0.05);
		}
	}
	return 0;
}

int gfe_adf_100_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDatai(d_adf1_standby_frequency_hz) < 406)
		{
			XPLMSetDatai(d_adf1_standby_frequency_hz, XPLMGetDatai(d_adf1_standby_frequency_hz) + 10);
		}
	}
	return 0;
}

int gfe_adf_100_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDatai(d_adf1_standby_frequency_hz) > 209)
		{
			XPLMSetDatai(d_adf1_standby_frequency_hz, XPLMGetDatai(d_adf1_standby_frequency_hz) - 10);
		}
	}
	return 0;
}

int gfe_adf_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDatai(d_adf1_standby_frequency_hz) < 415)
		{
			XPLMSetDatai(d_adf1_standby_frequency_hz, XPLMGetDatai(d_adf1_standby_frequency_hz) + 1);
		}
	}
	return 0;
}

int gfe_adf_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDatai(d_adf1_standby_frequency_hz) > 200)
		{
			XPLMSetDatai(d_adf1_standby_frequency_hz, XPLMGetDatai(d_adf1_standby_frequency_hz) - 1);
		}
	}
	return 0;
}

int gfe_hsi_obs_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if ((XPLMGetDataf(d_nav1_obs_deg_mag_pilot) < 360.0) || (XPLMGetDataf(d_hsi_obs_deg_mag_pilot) < 360.0))
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, XPLMGetDataf(d_nav1_obs_deg_mag_pilot) + 1.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, XPLMGetDataf(d_hsi_obs_deg_mag_pilot) + 1.0);
		}
		else
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, 0.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_hsi_obs_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if ((XPLMGetDataf(d_nav1_obs_deg_mag_pilot) > 1.0) || (XPLMGetDataf(d_hsi_obs_deg_mag_pilot) > 1.0))
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, XPLMGetDataf(d_nav1_obs_deg_mag_pilot) - 1.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, XPLMGetDataf(d_hsi_obs_deg_mag_pilot) - 1.0);
		}
		else
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, 360.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_hsi_obs_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if ((XPLMGetDataf(d_nav1_obs_deg_mag_pilot) < 355.0) || (XPLMGetDataf(d_hsi_obs_deg_mag_pilot) < 355.0))
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, XPLMGetDataf(d_nav1_obs_deg_mag_pilot) + 5.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, XPLMGetDataf(d_hsi_obs_deg_mag_pilot) + 5.0);
		}
		else
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, 0.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_hsi_obs_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if ((XPLMGetDataf(d_nav1_obs_deg_mag_pilot) >  5.0) || (XPLMGetDataf(d_hsi_obs_deg_mag_pilot) >  5.0))
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, XPLMGetDataf(d_nav1_obs_deg_mag_pilot) - 5.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, XPLMGetDataf(d_hsi_obs_deg_mag_pilot) - 5.0);
		}
		else
		{
			XPLMSetDataf(d_nav1_obs_deg_mag_pilot, 360.0);
			XPLMSetDataf(d_hsi_obs_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_hsi_hdg_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_heading_dial_deg_mag_pilot) < 360.0)
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, XPLMGetDataf(d_heading_dial_deg_mag_pilot) + 1.0);
		}
		else
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_hsi_hdg_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_heading_dial_deg_mag_pilot) > 1.0)
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, XPLMGetDataf(d_heading_dial_deg_mag_pilot) - 1.0);
		}
		else
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_hsi_hdg_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_heading_dial_deg_mag_pilot) < 355.0)
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, XPLMGetDataf(d_heading_dial_deg_mag_pilot) + 5.0);
		}
		else
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_hsi_hdg_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_heading_dial_deg_mag_pilot) >  5.0)
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, XPLMGetDataf(d_heading_dial_deg_mag_pilot) - 5.0);
		}
		else
		{
			XPLMSetDataf(d_heading_dial_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_obs2_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_nav2_obs_deg_mag_pilot) < 360.0)
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, XPLMGetDataf(d_nav2_obs_deg_mag_pilot) + 1.0);
		}
		else
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_obs2_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_nav2_obs_deg_mag_pilot) > 1.0)
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, XPLMGetDataf(d_nav2_obs_deg_mag_pilot) - 1.0);
		}
		else
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_obs2_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_nav2_obs_deg_mag_pilot) < 355.0)
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, XPLMGetDataf(d_nav2_obs_deg_mag_pilot) + 5.0);
		}
		else
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_obs2_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_nav2_obs_deg_mag_pilot) >  5.0)
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, XPLMGetDataf(d_nav2_obs_deg_mag_pilot) - 5.0);
		}
		else
		{
			XPLMSetDataf(d_nav2_obs_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_rmi_1_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) < 360.0)
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) + 1.0);
		}
		else
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_rmi_1_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) > 1.0)
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) - 1.0);
		}
		else
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int gfe_rmi_10_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) < 355.0)
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) + 5.0);
		}
		else
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, 0.0);
		}
	}
	return 0;
}

int gfe_rmi_10_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandEnd)
	{
		if (XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) >  5.0)
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, XPLMGetDataf(d_adf1_card_heading_deg_mag_pilot) - 5.0);
		}
		else
		{
			XPLMSetDataf(d_adf1_card_heading_deg_mag_pilot, 360.0);
		}
	}
	return 0;
}

int	gfe_ext_bff_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	char acf_ICAO[40];
	int ByteVals[40];
	XPLMGetDatab(d_acf_ICAO, ByteVals, 0, 40);
	strcpy(acf_ICAO, (char*)&ByteVals);
	char str[255];

	strcpy(str, "\"C:\\BFF_Control_Loader_v1_300_Release2\\Configuration Manager\\Aircraft Config Files\\");
	strcat(str, acf_ICAO);
	strcat(str, ".cfg\"");
	if (inPhase == xplm_CommandEnd)
	{
		int nRet = (int)ShellExecute(NULL, "open",
			"\"C:\\BFF_Control_Loader_v1_300_Release2\\BFF_Control_LoaderX_v1_300.exe\"", str ,	NULL, SW_SHOWNORMAL);
		if (nRet <= 32) 
		{
			DWORD dw = GetLastError();
			char szMsg[250];
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM,
				0, dw, 0,
				szMsg, sizeof(szMsg),
				NULL
			);
			XPLMDebugString(szMsg);
		}
	}
	return 0;
}

int gfe_flap_ratio_up_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin)
	{
		flaps_timer = time(0);
		XPLMCommandButtonPress(xplm_joy_flapsup);								//drive flaps one notch
		XPLMCommandButtonRelease(xplm_joy_flapsup);
	}
	else if (inPhase == xplm_CommandContinue)
	{
		if (time(0) > (flaps_timer + 1))										//if presssed longer than 1 second
		{
			XPLMCommandButtonPress(xplm_joy_flapsup);							//drive one more notch
		}
	}
	else
	{
		XPLMCommandButtonRelease(xplm_joy_flapsup);
		flaps_timer = 0;
	}
	return 0;
}

int gfe_flap_ratio_down_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin)
	{
		flaps_timer = time(0);
		XPLMCommandButtonPress(xplm_joy_flapsdn);								//drive flaps one notch
		XPLMCommandButtonRelease(xplm_joy_flapsdn);
	}
	else if (inPhase == xplm_CommandContinue)
	{
		if (time(0) > (flaps_timer + 1))										//if presssed longer than 1 second
		{
			XPLMCommandButtonPress(xplm_joy_flapsdn);							//drive one more notch
		}
	}
	else
	{
		XPLMCommandButtonRelease(xplm_joy_flapsdn);
		flaps_timer = 0;
	}
	return 0;
}

int gfe_land_light_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_landing_lights_on, 1);
	}
	else  
	{
		XPLMSetDatai(d_landing_lights_on, 0);
	}
	return 0;
}

int gfe_battery_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin)
	{
		XPLMSetDatai(d_fire_mode, 3);											//on Battery on, bring KAP150 in pre-test mode
	}
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_battery_on, 1);
	}
	else
	{
		XPLMSetDatai(d_battery_on, 0);
	}
	return 0;
}

int gfe_avionics_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_avionics_on, 1);
	}
	else
	{
		XPLMSetDatai(d_avionics_on, 0);
	}
	return 0;
}

int gfe_taxi_light_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_taxi_light_on, 1);
	}
	else
	{
		XPLMSetDatai(d_taxi_light_on, 0);
	}
	return 0;
}

int gfe_beacon_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_beacon_lights_on, 1);
	}
	else
	{
		XPLMSetDatai(d_beacon_lights_on, 0);
	}
	return 0;
}

int gfe_nav_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_nav_lights_on, 1);
	}
	else
	{
		XPLMSetDatai(d_nav_lights_on, 0);
	}
	return 0;
}

int gfe_strobe_lights_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_strobe_lights_on, 1);
	}
	else
	{
		XPLMSetDatai(d_strobe_lights_on, 0);
	}
	return 0;
}

int gfe_ice_pitot_heat_on_pilot_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_ice_pitot_heat_on_pilot, 1);
	}
	else
	{
		XPLMSetDatai(d_ice_pitot_heat_on_pilot, 0);
	}
	return 0;
}

int gfe_fuel_pump_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	int a[8] = { 0 };															//temp array for XPLMSetDatavi
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		a[0] = 1;
	}
	else
	{
		a[0] = 0;
	}
	XPLMSetDatavi(d_fuel_pump, a, 0, 8);
	return 0;
}

int gfe_generator_on_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	int a[8] = { 0 };															//temp array for XPLMSetDatavi
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		a[0] = 1;
	}
	else
	{
		a[0] = 0;
	}
	XPLMSetDatavi(d_generator_on, a, 0, 8);
	return 0;
}

int gfe_carb_heat_ratio_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	float f[8] = { 0.0 };														//temp array for XPLMSetDatavf
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		f[0] = 1.0;
	}
	else
	{
		f[0] = 0.0;
	}
	XPLMSetDatavf(d_carb_heat_ratio, f, 0, 8);
	return 0;
}

int gfe_audio_marker_enabled_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin)
	{
		XPLMSetDatai(d_audio_marker_enabled, !XPLMGetDatai(d_audio_marker_enabled));
	}
	return 0;
}

int gfe_audio_dme_enabled_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin) 
	{
		XPLMSetDatai(d_audio_dme_enabled, !XPLMGetDatai(d_audio_dme_enabled));
	}
	return 0;
}

int gfe_annunciator_test_timeout_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if (inPhase == xplm_CommandBegin) 
	{
		XPLMSetDataf(d_annunciator_test_timeout, 4.0);							//annunciator test delay
	}
	return 0;
}

int gfe_dme_power_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_dme_power, 1);
	}
	else
	{
		XPLMSetDatai(d_dme_power, 0);
	}
	return 0;
}

int gfe_DME_slave_source_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_DME_slave_source, 0);
	}
	else
	{
		XPLMSetDatai(d_DME_slave_source, 1);
	}
	return 0;
}

int gfe_DME_mode_0_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_DME_mode, 0);
	}
	return 0;
}

int gfe_DME_mode_1_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_DME_mode, 1);
	}
	return 0;
}

int gfe_DME_mode_2_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_DME_mode, 2);
	}
	return 0;
}

int gfe_KAP_test_Proc(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
	if ((inPhase == xplm_CommandBegin) || (inPhase == xplm_CommandContinue))
	{
		XPLMSetDatai(d_fire_mode, 2);
		fire_mode = 2;
		timestamp = GetTickCount();
	}
	return 0;
}

float 	call_ext_FlightLoopCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
								   int                  inCounter,    
                                   void *               inRefcon)
{
	char sa[255];														//for test BFF launch
	char sb[255];

	strcpy(sa, "\"C:\\BFF_Control_Loader_v1_300_Release2\\BFF_Control_LoaderX_v1_300.exe\"");
	strcpy(sb, "\"C:\\BFF_Control_Loader_v1_300_Release2\\Configuration Manager\\Aircraft Config Files\\Cessna172-Xplane.cfg\"");
	ShellExecute(NULL, NULL, sa, sb, NULL, SW_MINIMIZE); 

	return 0.0f;
}



/*--------------------------------  OpenAL stuff --------------------------------*/
/**************************************************************************************************************
 * WAVE FILE LOADING
**************************************************************************************************************/

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
	strcpy(wavfile[0], A_FUELPUMP_1);
	strcpy(wavfile[1], A_FUELPUMP_2);
	strcpy(wavfile[2], A_FUELPUMP_3);

	CHECK_ERR();
	
	// We have to save the old context and restore it later, so that we don't interfere with X-Plane and other plugins.
	old_ctx = alcGetCurrentContext();

	if (old_ctx)
	{
		//printf("0x%08x: I found someone else's context 0x%08x.\n",XPLMGetMyID(), old_ctx);
		XPLMDebugString("gfeSounds: found someone else's context.\n");
	}

	// Enumerate Audio Devices
	s = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	//try open specific device, if unavailable, revert to default device
	my_dev = alcOpenDevice("Lautsprecher (Realtek High Definition Audio)");
	//my_dev = alcOpenDevice(sAudioDevice);
	if (my_dev == NULL)
	{
		my_dev = alcOpenDevice(NULL);
	}
	if (my_dev == NULL)
	{
		XPLMDebugString("gfeSounds: Could not open the default OpenAL device.\n");
		return 0;		
	}	
	my_ctx = alcCreateContext(my_dev, NULL);
	if(my_ctx == NULL)
	{
		if(old_ctx)
			alcMakeContextCurrent(old_ctx);
		alcCloseDevice(my_dev);
		my_dev = NULL;
		XPLMDebugString("gfeSounds: Could not create a context.\n");
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
	
	ALfloat	zero[3] = {0,0,0} ;
	strcpy(filepath, gPluginDataFile);

	for (i=0;i<3;i++)
	{
		strcpy(buf, filepath);
		strcat(buf, wavfile[i]);
	
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
		//zero[0] = -1.0;
		//zero[1] = 1.0;
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


