	#include <stdio.h>	
	#include <stdlib.h>
	#include <string.h>
    	#include <fcntl.h>
    	#include <sys/mman.h>//mmap head file

	#include "jpg_header.h"
	#include "get_time.h"
	#include "global_def.h"
	#include "global_api.h"
	#include "bmp_ops.h"

	float gravityX=0;
	float gravityY=0;
	float gravityZ=0;

	char first_loop = 1;

	char *Serial1_DEV= "/dev/ttyPS1";
	char *INT_DEV= "/dev/PL";
	char *MEM_DEV= "/dev/mem";

//	unsigned int MAP_SIZE_SRC = 2*m_image_x*m_image_y;
	unsigned int MAP_SIZE_JPG = 1024*1024;//1MB buffer for jpg image


    	int main (int argc, char* argv[])
    	{
	       FILE *fp=NULL;
		unsigned int num_loops=0;
		unsigned int start_loops=0;
		unsigned int reboot_mini=1;
		int i,j;
	       int fd_pl_int,fd_mem, fd_serial1;
	       unsigned char *start_src, *start_jpg;
	       short *start_motion;
	       unsigned int *start_baro;
		unsigned short *start_xadc;
	       unsigned int *start_jpg_size, *start_cfg, *tmp_int, configuration_int;
	       unsigned int jpg_size;
		unsigned char config_pl[8]={0x12,0x34,0,0,0,0,0,0};		

		unsigned char gpio_jb8=0;

		char sys_cmd[MAXSTRING];
		char moDayDir[MAXSTRING];
		char hourDir[MAXSTRING];
		char hourDir_baro[MAXSTRING];
		char tm[MAXSTRING];
		char cmd[MAXSTRING];

		char FOLDER_NAME[MAXSTRING];
		char deviceID[5] = "0000";
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char imFile[MAXSTRING];
		char yrMonDayHrMinSec[MAXSTRING];
		char baroFile_old[MAXSTRING];
		char xadcFile_new[MAXSTRING];
		char baroFile_new[MAXSTRING];
		char motionFile_new[MAXSTRING];
		char testFile_new[MAXSTRING];


		char config_file[MAXSTRING];
		option_struct  *global_options;
		
		char rtc_cmd[MAXSTRING];
		char *deviceIDfile="/mnt/system/DeviceID";//"/root/DeviceID";//
		FILE *idFid;

		unsigned int cfg_tmp;
		int int_num_cur;
		int int_num_new;
		char src_en;
		char four_cam_mode;
		char jpg_en;
		char baro_en;
		char motion_en;
		char xadc_en;
		char motion_3axis;

		unsigned char baro_fs = 2*16;//fixed,16 samples/second
		unsigned short motion_fs = 2*80;//fixed,80 samples/second
		unsigned int  xadc_fs = 2*960;

		int image_width;
		int image_height;
		char quality_sl;
		char resolution_sl;

		int src_size;
		int baro_size;
		int motion_size;
		int xadc_size;

		idFid=fopen(deviceIDfile,"r");
		if (idFid!=NULL)
		{
			fgets(deviceID, 4, idFid);
			fclose(idFid);
		}
		printf("/**************************************************************************/\n");
		printf("DeviceID = %s\n", deviceID);

		//************************************************** Get configuration infos *************************************************************************//
		if (( global_options = (option_struct *) malloc( sizeof(option_struct) ) ) == NULL) {
				printf("ERROR ALLOCATING global_options\n");
				exit(1);
		}
		/* Initialize global options */
		initialize_global(global_options);
		/* Parse the command options */
		cmd_proc(config_file,argc,argv);
		/* Read the configuration file to set the global options */
		get_param(global_options, config_file);

		//*************************************************** Set configuration directives *******************************************************************//
		if (global_options -> RTC_UPDATE)
		{
			sprintf(rtc_cmd,"%s%c%s","date",' ', global_options->RTC_TIME);
			system(rtc_cmd);
			system(hwclock_rtc);
		}

		//capture source image
		src_en = global_options->SRC_EN;
		//capture jpg image
		jpg_en = global_options->JPG_EN;
		
		baro_en = global_options->BAROMETER_ON;
		motion_en = global_options->INERTIAL_ON;
		motion_3axis = 1;//global_options->MOTION_3AXIS;
		xadc_en = global_options->XADC_ON;

		//printf("inertial_ON is %d\n",motion_en);
		//printf("baro_ON is %d\n",baro_en);

		//motion_sp = global_options->inertialNum;
		//motion_fs =  (20*(motion_sp+1));

		//compression quality
		quality_sl = global_options->COMP_Q;

		resolution_sl = global_options->RESOLUTION;
		if(!resolution_sl)
		{
			image_width = 1280;
			image_height = 960;
		}
		else
		{
			image_width = 640;
			image_height = 480;			
		}

		//four camera loop mode
		four_cam_mode = global_options->FOUR_CAMERA_MODE;
		if(four_cam_mode)
		{
			//resolution_sl = 1;//vga

			image_width = image_width;//1280;//640;
			image_height = 4*image_height;//4*960;	//4*480;	//stitching		
		}

		src_size = (2*image_width*image_height);
		xadc_size = (2*xadc_fs);
		baro_size = (8*baro_fs);//8 bytes for each sample
		motion_size = (24*motion_fs);//24 bytes for each sample

		//loops before reboot
		if(global_options->reboot_interval_mins==0)
			reboot_mini = 1;//1min, for test
		else if(global_options->reboot_interval_mins==1)
			reboot_mini = 5;//5 mins
		else if(global_options->reboot_interval_mins==2)
			reboot_mini = 10;//10 mins
		else if(global_options->reboot_interval_mins==3)
			reboot_mini = 20;//20 mins
		else if(global_options->reboot_interval_mins==4)
			reboot_mini = 30;//30 mins
		else if(global_options->reboot_interval_mins==5)
			reboot_mini = 60;//60 mins
		else
			reboot_mini = 5;//5min, default

		if(four_cam_mode&(!resolution_sl))//0.5fps
			num_loops = (unsigned int)(0.5*60*(reboot_mini));
		else
			num_loops = (unsigned int)(60*(reboot_mini));

		//*******************************************************Open device********************************************************************//
		////open interruption
		fd_pl_int = open(INT_DEV,O_RDWR);
		if(fd_pl_int<0)
		{			
			perror("cannot open zbutton int\n");
			return 1;
		}

		////Open mem device
		//open /dev/mem with read and write mode
		fd_mem = open (MEM_DEV, O_RDWR|O_SYNC);//O_RDONLY);//
		if (fd_mem < 0)
		{
			printf("cannot open /dev/mem.");
			return -1;
		}

		//******************************************************** MMAP physical memory ***************************************************************//
		////map physical memory for configuration 						
/*		start_cfg = (unsigned int *)mmap(NULL, 4, PROT_WRITE|MAP_LOCKED, MAP_PRIVATE, fd_mem, PHY_ADDR_CFG);
		if (start_cfg == 0)
		{
			printf("NULL pointer!\n");
			return -1;
		}*/

/*		fd_serial1 = open(Serial1_DEV, O_RDWR);
		if(fd_serial1<0)
		{			
			perror("cannot open serial port1!\n");
			return 1;
		}
				
		config_pl[2] = (unsigned char)((((unsigned int)motion_en)<<6)+(((unsigned int)baro_en)<<5)+(((unsigned int)quality_sl)<<3)+(((unsigned int)jpg_en)<<2)+(((unsigned int)resolution_sl)<<1)+(unsigned int)src_en);
		config_pl[3] = 0x7E;

		for(i=0;i<8;i++)
			printf("%x\n",config_pl[i]);

*/

/*		for(j=0;j<1000;j++)
		{
			for(i=0;i<8;i++)
			printf("%d",config_pl[i]);
		}
*/
		//////write 1MB to flush data out of cache
//		tmp_int = start_cfg;
		//for(i=0;i<256*1024;i++)	
		//{	
//			*tmp_int = 0x550000+cfg_tmp;
//			printf("configuration=%d\n",*tmp_int);
			//tmp_int++;
		//}

		///////////////////
		//__clear_cache(start_cfg,start_cfg+4);//disable cache
		//fflush(stdout);//flush out cache
		//////////////////

//		configuration_int = 0x550000+cfg_tmp;

//		printf("configuration=%d\n",configuration_int);
//		munmap(start_cfg, 4);
		
			////map physical memory for jpg file
			if(jpg_en)
			{			
			
				start_jpg_size = (unsigned int *)mmap(NULL, 4, PROT_READ|MAP_LOCKED, MAP_PRIVATE, fd_mem, PHY_ADDR_jpgSIZE);//0x0e000000);//MAP_PRIVATE
				if (start_jpg_size == 0)
				{
					printf("NULL pointer!\n");
				}

				start_jpg = (unsigned char *)mmap(NULL, MAP_SIZE_JPG, PROT_READ|MAP_LOCKED, MAP_SHARED, fd_mem, PHY_ADDR_JPG);//0x0f000000);
				//start = mmap(NULL, file_size_jpg, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x0f000000);
				if (start_jpg == 0)
				{
					printf("NULL pointer!\n");
				}
			}
			//map physical memory for source image
			if(src_en)
			{			
				start_src = (unsigned char *)mmap(NULL, src_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_SRC);//0x0c000000);//MAP_PRIVATE,MAP_SHARED
				if (start_src == 0)
				{
					printf("NULL pointer!\n");
				}
			}
			//map physical memory for xadc data
			if(xadc_en)
			{			
				start_xadc = (unsigned short *)mmap(NULL, xadc_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_xadc);
				if (start_xadc == 0)
				{
					printf("NULL pointer!\n");
				}
			}
			//map physical memory for barometer data
			if(baro_en)
			{			
				start_baro = (unsigned int *)mmap(NULL, baro_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_baro);
				if (start_baro == 0)
				{
					printf("NULL pointer!\n");
				}
			}
			//map physical memory for motion data
			if(motion_en)
			{			
				start_motion = (short *)mmap(NULL, motion_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_motion);
				if (start_motion == 0)
				{
					printf("NULL pointer!\n");
				}
			}
		



		//***************************************************************************************************************************//

		printf("/**************************************************************************/\n");

		int_num_cur=0;
		int_num_new=0;
		int_num_cur = poll_int(fd_pl_int);

		while(num_loops>1)//
		{
			num_loops--;
			start_loops++;


			////poll interruption from PL
			do
			{
				int_num_new = poll_int(fd_pl_int);
				for(i=0;i<100;i++)
					i=i;
			}while(int_num_cur==int_num_new);	      


			//************************************************* Path generation **************************************************************************//
			get_time(&time_struct);

			//printf("store data to %s\n",PATH); //PATH="/sdcard/"
			strcpy(moDayDir,PATH);
			//printf("moDayDir is %s\n",moDayDir);

			sprintf(FOLDER_NAME,"%s%s%c%s%c%02d","ID",deviceID, '_',time_struct.time_month,'.',atoi(time_struct.time_day));	 //month.day

			strcat(moDayDir,FOLDER_NAME);
			//printf("folder is %s\n",FOLDER_NAME);
			//printf("moDayDir is %s\n",moDayDir); 
			strcpy(sys_cmd,"mkdir -p ");
			strcat(sys_cmd,moDayDir);
			system(sys_cmd);

			sprintf(moDayDir,"%s%c",moDayDir,'/'); //moDayDir: sdcard/month.day/

			get_time(&time_struct); //update time

			/// create a folder named as the current hour 
			sprintf(hourDir,"%s%d%c",moDayDir,atoi(time_struct.time_hour),'/');// moDayDir: sdard/month.day/; folderHead: hour
			//printf("hourDir is %s\n",hourDir);
			strcpy(cmd,"mkdir -p ");
			strcat(cmd,hourDir);
			system(cmd);
			//******************************************************* end ********************************************************************//

			get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".dat");

			//******************************************************* save src  ********************************************************************//
			if(src_en)//read source file
			{

/*				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".dat");*/
				sprintf(imFile,"%s%s%c%s%s",hourDir,deviceID,'_',yrMonDayHrMinSec,srcfileTail);

				src_save(imFile, start_src, image_width, image_height);
			}
		
			//******************************************************* save jpg  ********************************************************************//
			if(jpg_en)//read jpg file
			{
				jpg_size = *start_jpg_size;
				//memcpy(jpg_size,start_jpg_size,4);
				
				//printf("jpg size = %d bytes, %d\n", jpg_size[0],jpg_size[0]%128);
				printf("jpg size = %d bytes, %d\n", jpg_size,jpg_size%128);
				
				//file name	
/*				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".jpg");//".dat");//*/
				sprintf(imFile,"%s%s%c%s%s",hourDir,deviceID,'_',yrMonDayHrMinSec,jpgfileTail);

				jpg_save(imFile, start_jpg, jpg_size, quality_sl, image_width, image_height);				
			}

/*			if( (!strcmp(time_struct.time_min,"00"))&&(!strcmp(time_struct.time_sec,"00")) )
				first_loop = 1;
			else
				first_loop = 0;*/

			//******************************************************* save barometer  ********************************************************************//
			if(xadc_en)
			{				
				sprintf(xadcFile_new,"%s%s%c%s%s",hourDir,deviceID,'_',"xadc", ".txt");

				XADC_save(xadcFile_new, xadc_fs,start_xadc, first_loop);	
			}
	
			//******************************************************* save barometer  ********************************************************************//
			if(baro_en)
			{				
				//sprintf(baroFile_new,"%s%s%c%s%s%s",hourDir_baro,deviceID,'_',"baro_",yrMonDayHrMinSec,".txt");

				sprintf(baroFile_new,"%s%s%c%s%s",hourDir,deviceID,'_',"baro", ".txt");

				BARO_save(&gpio_jb8, baroFile_new, baro_fs,start_baro, first_loop);	
				//memcpy(baroFile_old, baroFile_new, MAXSTRING);
			}
			//******************************************************* save motion  ********************************************************************//
			if(motion_en)
			{				
				//sprintf(motionFile_new,"%s%s%c%s%s%s",hourDir,deviceID,'_',"motion_",yrMonDayHrMinSec, ".txt");
				sprintf(motionFile_new,"%s%s%c%s%s",hourDir,deviceID,'_',"gyroscope", ".txt");
				

				//MOTION_save_sixaxis(motionFile_new, motion_fs, start_motion, first_loop);
				MOTION_save(motionFile_new, motion_fs, start_motion, first_loop);		
			}			
			
			first_loop = 0;

			int_num_cur = int_num_new;


			/////controlled by gpio 
			if((start_loops>10)&&(gpio_jb8==128))
			{

				if(jpg_en)
				{
					munmap(start_jpg_size, 4); 
					munmap(start_jpg, MAP_SIZE_JPG); //destroy map memory
				}
				if(src_en)
					munmap(start_src, baro_size); //destroy map memory
				if(baro_en)
					munmap(start_baro, baro_size); //destroy map memory
				if(motion_en)
					munmap(start_motion, motion_size); //destroy map memory

				close(fd_mem);  //close file

				close(fd_pl_int);

				strcpy(cmd,"reboot");//poweroff
				system(cmd);
			}
						
		}
		
		//////////////////////////////////////////////////////////////////////////
		if(jpg_en)
		{
			munmap(start_jpg_size, 4); 
			munmap(start_jpg, MAP_SIZE_JPG); //destroy map memory
		}
		if(src_en)
			munmap(start_src, src_size); //destroy map memory
		if(xadc_en)
			munmap(start_xadc, xadc_size); //destroy map memory
		if(baro_en)
			munmap(start_baro, baro_size); //destroy map memory
		if(motion_en)
			munmap(start_motion, motion_size); //destroy map memory

		close(fd_mem);  //close file

		close(fd_pl_int);

		strcpy(cmd,"reboot");//poweroff
		system(cmd);

	       return 0;
    	}

	int poll_int(int fd)
	{
	
		int int_num;
		char buff;

		buff = read(fd, &int_num, sizeof(int));
		
		return int_num;

	}

	int jpg_save(char *imFile, unsigned char *start_jpgstream, unsigned int jpg_size, unsigned char quality_sl, int image_width, int image_height)
	{
		unsigned char image_size[4];
		FILE *fp=NULL;

		//create file
		fp=fopen(imFile, "wb");
		//fp=fopen("/mnt/zButton/pic.jpg", "wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		//jpg header
		////fwrite(HEADER, 623, sizeof(unsigned char), fp);
		fwrite(&header_part1[quality_sl], 163, sizeof(unsigned char), fp);//header
		image_size[0] = (uint8)(image_height>>8);
		image_size[1] = (uint8)(image_height & 0xFF);
		image_size[2] = (uint8)(image_width>>8);
		image_size[3] = (uint8)(image_width & 0xFF);
		fwrite(image_size, 4, sizeof(unsigned char), fp);//image size
		fwrite(&header_part2[quality_sl], 456, sizeof(unsigned char), fp);//header

		//jpg stream
		fwrite(start_jpgstream, jpg_size, 1, fp);//

		///clear buffer
		//memset(start,0x00,file_size);

		fflush(fp);

		//close file
		fclose(fp);
		printf("jpg file is saved!\n");

		return 0;
	}

	int src_save(char * imFile, unsigned char *start_src, int image_width, int image_height)
	{
		FILE *fp=NULL;
		unsigned char *RGB;

		/*fp=fopen(imFile, "wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		//source image data
		fwrite(start_src, src_size, 1, fp);

		fflush(fp);
		fclose(fp);*/

		RGB = (unsigned char *)(malloc(sizeof(unsigned char) * image_height * image_width * 3 ));
		YCBCR422_RGB(start_src, RGB, image_width, image_height);
		BMP_save(imFile, RGB, image_width, image_height);

		printf("source image is saved!\n");

		return 0;
	}

	int XADC_save(char *imFile, unsigned int xadc_fs, unsigned short *start_xadc, char header)
	{
		FILE *fp=NULL;
		int i;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char	time_print[50];
		char HrMinSec[MAXSTRING];
		unsigned short xadc_data;

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "time(hhmmss), pressure(hectopascal), temperature(Celsius Degree)\n");

		get_time(&time_struct);
		sprintf(time_print,"%d%c%d%c%d%c%d",atoi(time_struct.time_day),'.',atoi(time_struct.time_hour),'.',atoi(time_struct.time_min),'.',atoi(time_struct.time_sec));
//sprintf(HrMinSec,"%s%s%s",time_struct.time_hour,time_struct.time_min, time_struct.time_sec);		
		for(i = 0;i < xadc_fs; i++)
		{			
			xadc_data = (unsigned short)(start_xadc[i]);

			fprintf(fp, "%u,%s\n", xadc_data, time_print);	

			if(i==0)printf("%u\n", xadc_data);				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);

		return 0;
	}

	int BARO_save(unsigned char *gpio, char *imFile, unsigned char baro_fs, unsigned int *start_baro, char header)
	{
		FILE *fp=NULL;
		float temperature;
		unsigned int pressure;
		int i;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char	time_print[50];
		char HrMinSec[MAXSTRING];

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "time(hhmmss), pressure(hectopascal), temperature(Celsius Degree)\n");

		get_time(&time_struct);
		sprintf(time_print,"%d%c%d%c%d%c%d",atoi(time_struct.time_day),'.',atoi(time_struct.time_hour),'.',atoi(time_struct.time_min),'.',atoi(time_struct.time_sec));
//sprintf(HrMinSec,"%s%s%s",time_struct.time_hour,time_struct.time_min, time_struct.time_sec);		
		for(i = 0;i < baro_fs; i++)
		{			
			pressure = (unsigned int)(start_baro[2*i]);
			temperature = ((float)(start_baro[2*i+1]))/10.0;
			*gpio =  (unsigned char)((start_baro[2*i+1]>>24)&0xff);

			fprintf(fp, "%u,%5.2f\n,%s\n", pressure, temperature, time_print);	

			if(i==0)printf("%u,%5.2f\n", pressure, temperature);				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}

	int MOTION_save_sixaxis(char *imFile, unsigned short motion_fs, short *start_motion, char header)
	{
		FILE *fp=NULL;
		int i;
		char tmp_55;
		struct MPU9150_DATA MPU9150_DATA;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char	time_print[50];
		char HrMinSec[MAXSTRING];
		char LB, HB;
		float T_coe;
		float Dominator_x,Dominator_y,Dominator_z;
		unsigned short taildata;

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "ACC_X, ACC_Y, ACC_Z, TEMP, GYRO_X, GYRO_Y, GYRO_Z, time(hhmmss)\n");

		get_time(&time_struct);
		sprintf(time_print,"%d%c%d%c%d%c%d",atoi(time_struct.time_day),'.',atoi(time_struct.time_hour),'.',atoi(time_struct.time_min),'.',atoi(time_struct.time_sec));
		//sprintf(HrMinSec,"%s%s%s",time_struct.time_hour,time_struct.time_min, time_struct.time_sec);		
		for(i = 0;i < motion_fs; i++)
		{
//			MPU9150_DATA.ACC_X = *(start_motion+3);
//			MPU9150_DATA.ACC_Y = *(start_motion+2);
//			MPU9150_DATA.ACC_Z = *(start_motion+1);
//			MPU9150_DATA.TEMP = ((float)(*(start_motion))/340.0+35);
//			MPU9150_DATA.GYRO_X = *(start_motion+7);
//			MPU9150_DATA.GYRO_Y = *(start_motion+6);
//			MPU9150_DATA.GYRO_Z = *(start_motion+5);
//			MPU9150_DATA.MAG_X = *(start_motion+4);
//			MPU9150_DATA.MAG_Y = *(start_motion+11);
//			MPU9150_DATA.MAG_Z = *(start_motion+10);
//			MPU9150_DATA.ASA_X = (char)(((*(start_motion+9))>>8)&0xff);
//			MPU9150_DATA.ASA_Y = (char)((*(start_motion+9))&0xff);
//			MPU9150_DATA.ASA_Z = (char)(((*(start_motion+8))>>8)&0xff);

			MPU9150_DATA.ACC_X = 2.0*9.81*((float)(*(start_motion+3))/32768.0);
			MPU9150_DATA.ACC_Y = 2.0*9.81*((float)(*(start_motion+2))/32768.0);
			MPU9150_DATA.ACC_Z = 2.0*9.81*((float)(*(start_motion+1))/32768.0);
			MPU9150_DATA.TEMP = ((float)(*(start_motion))/340.0+35);
			MPU9150_DATA.GYRO_X = 250.0*((float)(*(start_motion+7))/32768.0);//*(start_motion+7);
			MPU9150_DATA.GYRO_Y = 250.0*((float)(*(start_motion+6))/32768.0);//*(start_motion+6);
			MPU9150_DATA.GYRO_Z = 250.0*((float)(*(start_motion+5))/32768.0);//*(start_motion+5);
			taildata = (unsigned short)(*(start_motion+4));

			start_motion = start_motion + 8;			
	
	
			fprintf(fp, "%9f,%9f,%9f,%9f,%9f,%9f,%9f,%d\n,%s\n", MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, taildata, time_print);

				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}

	int MOTION_save(char *imFile, unsigned short motion_fs, short *start_motion, char header)
	{
		FILE *fp=NULL;
		int i;
		char tmp_55;
		struct MPU9150_DATA MPU9150_DATA;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char	time_print[50];
		char HrMinSec[MAXSTRING];
		char LB, HB;
		float T_coe;
		float Dominator_x,Dominator_y,Dominator_z;




		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "time(hhmmss), ACC_X,ACC_Y,ACC_Z,TEMP,GYRO_X,GYRO_Y,GYRO_Z,MAG_X,MAG_Y,MAG_Z,ACTUAL_ACC_X,ACTUAL_ACC_Y,ACTUAL_ACC_Z\n");

		get_time(&time_struct);
		sprintf(time_print,"%d%c%d%c%d%c%d",atoi(time_struct.time_day),'.',atoi(time_struct.time_hour),'.',atoi(time_struct.time_min),'.',atoi(time_struct.time_sec));
		//sprintf(HrMinSec,"%s%s%s",time_struct.time_hour,time_struct.time_min, time_struct.time_sec);		
		for(i = 0;i < motion_fs; i++)
		{
//			MPU9150_DATA.ACC_X = *(start_motion+3);
//			MPU9150_DATA.ACC_Y = *(start_motion+2);
//			MPU9150_DATA.ACC_Z = *(start_motion+1);
//			MPU9150_DATA.TEMP = ((float)(*(start_motion))/340.0+35);
//			MPU9150_DATA.GYRO_X = *(start_motion+7);
//			MPU9150_DATA.GYRO_Y = *(start_motion+6);
//			MPU9150_DATA.GYRO_Z = *(start_motion+5);
//			MPU9150_DATA.MAG_X = *(start_motion+4);
//			MPU9150_DATA.MAG_Y = *(start_motion+11);
//			MPU9150_DATA.MAG_Z = *(start_motion+10);
//			MPU9150_DATA.ASA_X = (char)(((*(start_motion+9))>>8)&0xff);
//			MPU9150_DATA.ASA_Y = (char)((*(start_motion+9))&0xff);
//			MPU9150_DATA.ASA_Z = (char)(((*(start_motion+8))>>8)&0xff);

			MPU9150_DATA.ACC_X = 4.0*9.81*((float)(*(start_motion+3))/32768.0);
			MPU9150_DATA.ACC_Y = 4.0*9.81*((float)(*(start_motion+2))/32768.0);
			MPU9150_DATA.ACC_Z = 4.0*9.81*((float)(*(start_motion+1))/32768.0);
			MPU9150_DATA.TEMP = ((float)(*(start_motion))/340.0+35);
			MPU9150_DATA.GYRO_X = 250.0*((float)(*(start_motion+7))/32768.0);//*(start_motion+7);
			MPU9150_DATA.GYRO_Y = 250.0*((float)(*(start_motion+6))/32768.0);//*(start_motion+6);
			MPU9150_DATA.GYRO_Z = 250.0*((float)(*(start_motion+5))/32768.0);//*(start_motion+5);
			MPU9150_DATA.ASA_X = (char)(((*(start_motion+9))>>8)&0xff);
			MPU9150_DATA.ASA_Y = (char)((*(start_motion+9))&0xff);
			MPU9150_DATA.ASA_Z = (char)(((*(start_motion+8))>>8)&0xff);
			
			//magnetometer
			LB = (*(start_motion+4))&0xff;
			HB = ((*(start_motion+4))>>8)&0xff;
			MPU9150_DATA.MAG_X = (LB<<8)+HB;
			Dominator_x = 4095.0;
			if (MPU9150_DATA.MAG_X>4095)
			{
				MPU9150_DATA.MAG_X = MPU9150_DATA.MAG_X-65536;
				Dominator_x = 4096.0;
			}
			LB = (*(start_motion+11))&0xff;
			HB = ((*(start_motion+11))>>8)&0xff;
			MPU9150_DATA.MAG_Y = (LB<<8)+HB;
			Dominator_y = 4095.0;
			if (MPU9150_DATA.MAG_Y>4095)
			{
				MPU9150_DATA.MAG_Y = MPU9150_DATA.MAG_Y-65536;
				Dominator_y = 4096.0;
			}
			LB = (*(start_motion+10))&0xff;
			HB = ((*(start_motion+10))>>8)&0xff;
			MPU9150_DATA.MAG_Z = (LB<<8)+HB;
			Dominator_z = 4095.0;	
			if (MPU9150_DATA.MAG_Z>4095)
			{
				MPU9150_DATA.MAG_Z = MPU9150_DATA.MAG_Z-65536;	
				Dominator_z = 4096.0;
			}	
			MPU9150_DATA.MAG_X = 1229.0*((float)(MPU9150_DATA.MAG_X)*(0.5*((float)MPU9150_DATA.ASA_X-128.0)/128.0+1.0))/Dominator_x;//*(start_motion+4);
			MPU9150_DATA.MAG_Y = 1229.0*((float)(MPU9150_DATA.MAG_Y)*(0.5*((float)MPU9150_DATA.ASA_Y-128.0)/128.0+1.0))/Dominator_y;//*(start_motion+11);
			MPU9150_DATA.MAG_Z = 1229.0*((float)(MPU9150_DATA.MAG_Z)*(0.5*((float)MPU9150_DATA.ASA_Z-128.0)/128.0+1.0))/Dominator_z;//*(start_motion+10);
			////tmp_55 = (char)((*(start_motion+8))&0xff);
			
			start_motion = start_motion + 12;			
	
			///// calculate gravity 
			getGravity(&MPU9150_DATA);

			//fprintf(fp, "%s: %d,%d,%d,%5.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", HrMinSec, MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, MPU9150_DATA.MAG_X, MPU9150_DATA.MAG_Y, MPU9150_DATA.MAG_Z, MPU9150_DATA.ASA_X, MPU9150_DATA.ASA_Y, MPU9150_DATA.ASA_Z);	
			fprintf(fp, "%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f,%9f\n,%s\n", MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, MPU9150_DATA.MAG_X, MPU9150_DATA.MAG_Y, MPU9150_DATA.MAG_Z, MPU9150_DATA.ACTUAL_ACC_X, MPU9150_DATA.ACTUAL_ACC_Y, MPU9150_DATA.ACTUAL_ACC_Z, time_print);

			//if(i==0)printf("%s: %d,%d,%d,%5.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", HrMinSec, MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, MPU9150_DATA.MAG_X, MPU9150_DATA.MAG_Y, MPU9150_DATA.MAG_Z, MPU9150_DATA.ASA_X, MPU9150_DATA.ASA_Y, MPU9150_DATA.ASA_Z);
				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}


/* calculate gravity */
void getGravity(struct MPU9150_DATA *MPU9150_DATA)
{
	float alpha = 0.8;
	gravityX = gravityX * alpha + (1.0 - alpha) * (MPU9150_DATA->ACC_X);
	gravityY = gravityY * alpha + (1.0 - alpha) * (MPU9150_DATA->ACC_Y);
	gravityZ = gravityZ * alpha + (1.0 - alpha) * (MPU9150_DATA->ACC_Z);
	
	(MPU9150_DATA->ACTUAL_ACC_X) =  (MPU9150_DATA->ACC_X) - gravityX;
	(MPU9150_DATA->ACTUAL_ACC_Y) =  (MPU9150_DATA->ACC_Y) - gravityY;
	(MPU9150_DATA->ACTUAL_ACC_Z) =  (MPU9150_DATA->ACC_Z) - gravityZ;
}
