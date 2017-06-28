/*
 * global_def.h
 *
 *  Created on: April 4, 2011
 *      Author: Yaofeng Yue
 */

#ifndef GLOBAL_DEF_H_
#define GLOBAL_DEF_H_

//#define MAXSTRING 100

char *jpgfileTail=".JPG";
char *srcfileTail = ".bmp";
char *hwclock_rtc="hwclock -w";

#define PATH "/home/linaro/weightData/"
//"/mnt/Data/"
#define CONFIG_FILE_NAME "config"

//#define FOLDER_NAME "lcn_obesity"

#define MAX_PIC 40000
#define FOLDER_PIC 4000

#define PHY_ADDR_CFG      0x0E0f0000
#define PHY_ADDR_SRC      0x0C000000
#define PHY_ADDR_jpgSIZE  0x0E000000
#define PHY_ADDR_JPG      0x0F000000
#define PHY_ADDR_xadc     0x0fd00000
#define PHY_ADDR_baro     0x0fe00000
#define PHY_ADDR_motion   0x0ff00000
#define PHY_ADDR_loadcell     0x0c000000

struct MPU9150_DATA
{
	float ACC_X;
	float ACC_Y;
	float ACC_Z;
	float TEMP;
	float GYRO_X;
	float GYRO_Y;
	float GYRO_Z;
	float MAG_X;
	float MAG_Y;
	float MAG_Z;
	char ASA_X;
	char ASA_Y;
	char ASA_Z;
	float ACTUAL_ACC_X;
	float ACTUAL_ACC_Y;
	float ACTUAL_ACC_Z;

	};


#define MAX(x,y) (((x)>(y))?(x):(y))
#define MIN(x,y) (((x)>(y))?(y):(x))


int poll_int(int fd);
int LOADCELL_save(char *imFile, unsigned char loadcell_fs, unsigned int *start_loadcell, char header);

#endif /* GLOBAL_DEF_H_ */

