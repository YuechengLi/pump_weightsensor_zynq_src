	#include <stdio.h>	
	#include <stdlib.h>
	#include <string.h>
    	#include <fcntl.h>

	#include "global_api.h"

	void cmd_proc(char *file_name, int argc, char *argv[])
	{
	    extern char *optarg;
	    int optchar;

		  if(argc==1) {
		    usage(argv[0]);
		  }

		  while((optchar = getopt(argc, argv, "abc:g:e")) != EOF) {
		    switch((char)optchar) {
		    case 'g':
		      /** Global Parameters File **/
		      strcpy(file_name, optarg);
		      break;
		    default:
		      /** Print Usage if Invalid Command Line Arguments **/
		      usage(argv[0]);
		      break;
		    }
		  }

	}


	void usage(char *temp)
	{
	  fprintf(stderr,"Usage: %s -g<model_control_file>\n",temp);
	  fprintf(stderr,"\t<model_control_file> is a file that contains all needed model\n\t\tparameters. If not defined, default values will be used.\n");
	}

	void initialize_global(option_struct  *global_options) {
		global_options -> reboot_interval_mins = 0.5;
		global_options -> RESOLUTION=0;
		global_options -> CONTRAST=0;
		global_options -> BRIGHTNESS=0;
		global_options -> SATURATION=0;		
		global_options -> LIGHT_MODE = 1;
		global_options -> LIGHT_SENSOR_ON=1;
		global_options -> WIRELESS_ON=0;
		global_options -> INERTIAL_ON=0;
		global_options -> BAROMETER_ON=0;
		global_options -> PROXIMITY_ON=0;
		//global_options -> inertialNum=3;
		global_options -> MOTION_6AXIS = 0;
		global_options -> XADC_ON = 0;
		global_options -> IMAGE_ENCRYPTION=0;
		global_options -> CAMERA_ON=1;
		global_options -> CAMERA_FRAMERATE=1;
		global_options -> FOUR_CAMERA_MODE=0;
		global_options -> JPG_EN=1;
		global_options -> COMP_Q=2;
		global_options -> SRC_EN=0;
		global_options -> SLEEP=0;
		global_options -> SLEEP_SEC=2;
		global_options -> RTC_UPDATE = 0;
		strcpy(global_options -> RTC_TIME, "000000000000.00");
	}

	void get_param(option_struct * options, char *filename)
	{
		FILE *RDconfig;
		char cmdstr[MAXSTRING];
		char optstr[MAXSTRING];

		if (strcmp(filename,CONFIG_FILE_NAME)==0)
		{
			RDconfig=fopen(filename,"r");
			printf("Use global control in the configuration file!\n");

	  		while(!feof(RDconfig)) 
			{
				fgets(cmdstr,MAXSTRING,RDconfig);
		    		if(cmdstr[0]!='#' && cmdstr[0]!='\n' && cmdstr[0]!='\0') 
				{
		
		      		      sscanf(cmdstr,"%s",optstr);

				    	/*******************************
					 Get Model Global Parameters
					*****************************/
				     if(strcasecmp("REBOOT_INTERVAL_MINS",optstr)==0) {
					sscanf(cmdstr,"%*s %f",&options->reboot_interval_mins);
				      }
				     if(strcasecmp("CAMERA_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->CAMERA_ON);
				      }
				      if(strcasecmp("CAMERA_FRAMERATE",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->CAMERA_FRAMERATE);
				      }
				      if(strcasecmp("FOUR_CAMERA_MODE",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->FOUR_CAMERA_MODE);
				      }
				      if(strcasecmp("RESOLUTION",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->RESOLUTION);
				      }
				      if(strcasecmp("CONTRAST",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->CONTRAST);
				      }
				      if(strcasecmp("BRIGHTNESS",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->BRIGHTNESS);
				      }
				      if(strcasecmp("SATURATION",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->SATURATION);
				      }
				      if(strcasecmp("LIGHT_MODE",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->LIGHT_MODE);
				      }
					if(strcasecmp("JPEG_EN",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->JPG_EN);
				      }					
					if(strcasecmp("COMP_QUALITY",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->COMP_Q);
				      }			
					if(strcasecmp("SourceImage_EN",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->SRC_EN);
				      }
					if(strcasecmp("IMAGE_ENCRYPTION",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->IMAGE_ENCRYPTION);
				      }
					 if(strcasecmp("WIRELESS_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->WIRELESS_ON);
				      }
	 				if(strcasecmp("INERTIAL_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->INERTIAL_ON);
					printf("INERTIAL_ON is %d\n",options->INERTIAL_ON);
				      }					
	 				if(strcasecmp("MOTION_6AXIS",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->MOTION_6AXIS);
					printf("MOTION_6AXIS is %d\n",options->MOTION_6AXIS);
				      }	
	 				if(strcasecmp("XADC_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->XADC_ON);
					printf("XADC_ON is %d\n",options->XADC_ON);
				      }	

		                        /*if(strcasecmp("inertialNum",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->inertialNum);
				      }*/
					if(strcasecmp("BAROMETER_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->BAROMETER_ON);
					printf("BAROMETER_ON is %d\n",options->BAROMETER_ON);
				      }
					if(strcasecmp("PROXIMITY_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->PROXIMITY_ON);
				      }
					 if(strcasecmp("LIGHT_ON",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->LIGHT_SENSOR_ON);
				      }
					 if(strcasecmp("SLEEP",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->SLEEP);
				      }
					 if(strcasecmp("SLEEP_SEC",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->SLEEP_SEC);
				      }
				      if(strcasecmp("RTC_UPDATE",optstr)==0) {
					sscanf(cmdstr,"%*s %d",&options->RTC_UPDATE);
				      }
				      if(strcasecmp("RTC_TIME",optstr)==0) {
					sscanf(cmdstr,"%*s %s", options->RTC_TIME);			
				      }

				}
			}
			fclose(RDconfig);
		}
	#if 1
		printf("System auto reboot interval is %f min(s)\n",options->reboot_interval_mins);
		printf("CAMERA_ON is %d\n",options->CAMERA_ON);
		printf("CAMERA_FRAMERATE is %d\n",options->CAMERA_FRAMERATE);
		printf("FOUR_CAMERA_MODE is %d\n",options->FOUR_CAMERA_MODE);
		printf("resolution is %d\n",options->RESOLUTION);
		printf("CONTRAST is %d\n",options->CONTRAST);
		printf("BRIGHTNESS is %d\n",options->BRIGHTNESS);
		printf("SATURATION is %d\n",options->SATURATION);
		printf("LIGHT_MODE is %d\n",options->LIGHT_MODE);

		printf("JPEG_EN is %d\n",options->JPG_EN);
		printf("COMP_QUALITY is %d\n",options->COMP_Q);
		printf("SourceImage_EN is %d\n",options->SRC_EN);
		printf("IMAGE_ENCRYPTION is %d\n",options->IMAGE_ENCRYPTION);

		printf("WIRELESS_ON is %d\n",options->WIRELESS_ON);
		printf("INERTIAL_ON is %d\n",options->INERTIAL_ON);
		printf("BAROMETER_ON is %d\n",options->BAROMETER_ON);
		printf("MOTION_6AXIS is %d\n",options->MOTION_6AXIS);
		printf("XADC_ON is %d\n",options->XADC_ON);
		//printf("inertialNum is %d\n",options->inertialNum);
		printf("PROXIMITY_ON is %d\n",options->PROXIMITY_ON);
		printf("LIGHT_SENSOR_ON is %d\n",options->LIGHT_SENSOR_ON);
		printf("SLEEP is %d\n",options->SLEEP);
		printf("SLEEP TIME is %d\n",options->SLEEP_SEC);
		printf("RTC_UPDATE is %d\n",options->RTC_UPDATE);
		printf("RTC_TIME is %s\n",options->RTC_TIME);
	#endif
	}
