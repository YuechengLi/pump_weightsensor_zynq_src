/*
 * global_def.h
 */

#ifndef GLOBAL_API_H_
#define GLOBAL_API_H_

#define MAXSTRING 100

#define CONFIG_FILE_NAME "config"


typedef struct {
	float reboot_interval_mins;
	int CAMERA_ON;
	int CAMERA_FRAMERATE;
	int FOUR_CAMERA_MODE;
	int RESOLUTION;
	int CONTRAST;
	int BRIGHTNESS;
	int SATURATION;
	int LIGHT_MODE;
	int JPG_EN;
        int COMP_Q;
	int SRC_EN;	
	int IMAGE_ENCRYPTION;
	int WIRELESS_ON;
        int INERTIAL_ON;
        int inertialNum;
	int MOTION_6AXIS;
	int XADC_ON;
	int BAROMETER_ON;
	int PROXIMITY_ON;
	int LIGHT_SENSOR_ON;
	int SLEEP;
	int SLEEP_SEC;
	int RTC_UPDATE;
	char RTC_TIME[MAXSTRING];
 
} option_struct;


void usage(char *temp);
void cmd_proc(char *file_name, int argc, char *argv[]);
void initialize_global(option_struct *);
void get_param(option_struct *, char *);

#endif /* GLOBAL_DEF_H_ */
