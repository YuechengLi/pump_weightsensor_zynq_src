	#include <stdio.h>	
	#include <stdlib.h>
	#include <string.h>
    	#include <fcntl.h>
    	#include <sys/mman.h>//mmap head file


	#include "get_time.h"
	#include "global_def.h"
	#include "global_api.h"


	

	char first_loop = 1;

	char *INT_DEV= "/dev/PL_int";
	char *MEM_DEV= "/dev/mem";


    	int main (int argc, char* argv[])
    	{
	       FILE *fp=NULL;
		unsigned int num_loops=0;
		unsigned int start_loops=0;
		unsigned int reboot_mini=1;
		int i,j;
	       int fd_pl_int,fd_mem, fd_serial1;
	       
		unsigned int *start_loadcell;
		
	       unsigned int configuration_int;


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

		char loadcellFile_new[MAXSTRING];



		char config_file[MAXSTRING];
		option_struct  *global_options;
		
		char rtc_cmd[MAXSTRING];

		char hr_pre=0;

		unsigned int cfg_tmp;
		int int_num_cur;
		int int_num_new;

		int loadcell_size;
		loadcell_size = (4*4*80);

		//strcpy(cmd,"mkdir -p ");
		//strcat(cmd,PATH);
		//system(cmd);

		
		/***************************************************************************/	
		//set timezone
		//system("rm /etc/localtime");
		//system("ln -s /usr/share/zoneinfo/US/Eastern /etc/localtime");
	
		//get time from internet
		//system("ntpdate -s ntp.ubuntu.com");
		get_time(&time_struct);
		if(atoi(time_struct.time_year)<2016)
		{
			printf("/*****Fail to update time from internet, check network!********/\n");
			printf("/*******************System will reboot!******************/\n");
			//system("reboot");
			return;
		} 

		
		printf("\n   System time was updated successfully!\n");
		system("date");


		/**************************************************************************/
	//	strcpy(sys_cmd,"sudo insmod /home/linaro/bed/PL-int.ko ");
	//	system(sys_cmd);

		//*******************************************************Open device********************************************************************//
		////open interruption
		fd_pl_int = open(INT_DEV,O_RDWR);
		if(fd_pl_int<0)
		{			
			perror("cannot open PL_int\n");
			return 1;
		}

		////Open mem device
		//open /dev/mem with read and write mode
		fd_mem = open (MEM_DEV, O_RDWR|O_SYNC);//O_RDONLY);//
		if (fd_mem < 0)
		{
			printf("cannot open /dev/mem.\n");
			return -1;
		}

		//******************************************************** MMAP physical memory ***************************************************************//
		////map physical memory for configuration 						
		start_loadcell = (unsigned int *)mmap(NULL, loadcell_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_loadcell);
		if (start_loadcell == 0)
		{
			printf("NULL pointer!\n");
		}

		//***************************************************************************************************************************//

		printf("/**************************************************************************/\n");

		int_num_cur=0;
		int_num_new=0;
		int_num_cur = poll_int(fd_pl_int);

		num_loops=3600*2;
		while(1)//num_loops)//
		{
			num_loops--;
			//start_loops++;

			//printf("1\n");

			////poll interruption from PL
			do
			{
				int_num_new = poll_int(fd_pl_int);
				for(i=0;i<100;i++)
					i=i;
			}while(int_num_cur==int_num_new);	      

			//printf("2\n");

			for(i=0;i<100;i++){;}
			

			//printf("loop: %d\n",num_loops);
			//***************************/home/linaro/********************** Path generation **************************************************************************//
			get_time(&time_struct);

			//printf("3\n");

			//create file for each hour
			if(hr_pre!=atoi(time_struct.time_hour))
			{
				//printf("store data to %s\n",PATH); 
				strcpy(moDayDir,PATH);
				//printf("moDayDir is %s\n",moDayDir);

		//		sprintf(FOLDER_NAME,"%s%c%02d",time_struct.time_month,'.',atoi(time_struct.time_day));	 //month.day

		//		strcat(moDayDir, FOLDER_NAME);
				//printf("folder is %s\n",FOLDER_NAME);
				//printf("moDayDir is %s\n",moDayDir); 
				strcpy(sys_cmd,"mkdir -p ");
				strcat(sys_cmd,moDayDir);
				system(sys_cmd);

		//		sprintf(moDayDir,"%s%c",moDayDir,'/'); //moDayDir: sdcard/month.day/

				//printf("4\n");
	
			/// create a folder named as the current hour 
		//		get_time(&time_struct); //update time
		//		sprintf(hourDir,"%s%d%c",moDayDir,atoi(time_struct.time_hour),'/');// moDayDir: sdard/month.day/; folderHead: hour
				//printf("hourDir is %s\n",hourDir);
		//		strcpy(cmd,"mkdir -p ");
		//		strcat(cmd,hourDir);
		//		system(cmd);

				hr_pre=atoi(time_struct.time_hour);
			}

			//printf("5\n");			
			//printf("path generated!\n");
			//******************************************************* end ********************************************************************//

			//get_time(&time_struct);
			//	sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".dat");

	
			//******************************************************* save barometer  ********************************************************************//				
				sprintf(loadcellFile_new,"%s%s%s%02d%c%02d%s",moDayDir,"loadcell_", time_struct.time_month, atoi(time_struct.time_day),'_',atoi(time_struct.time_hour), ".txt");
				//sprintf(loadcellFile_new,"%s%c%s%s",moDayDir,'/',"loadcell", ".txt");
				printf("%s\n",loadcellFile_new);

				//printf("6\n");

				LOADCELL_save(loadcellFile_new, 80, start_loadcell, first_loop);	
				//memcpy(baroFile_old, baroFile_new, MAXSTRING);
			
			//printf("file saved!\n");

			first_loop = 0;

			int_num_cur = int_num_new;
						
		}
		
		//////////////////////////////////////////////////////////////////////////
		

		munmap(start_loadcell, loadcell_size); //destroy map memory

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
	
	//	printf("7\n");
	
		return int_num;

	}

	int LOADCELL_save(char *imFile, unsigned char loadcell_fs, unsigned int *start_loadcell, char header)
	{
		FILE *fp=NULL;
		float sensor_a, sensor_b, sensor_c, sensor_d;
		int i;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char	time_print[50];
		char HrMinSec[MAXSTRING];

		float scale = -10080.0;

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		//if(header)
			//fprintf(fp, "time(hhmmss), sensor_A, sensor_B, sensor_C, sensor_D\n");

		//printf("8\n");

		get_time(&time_struct);
		sprintf(time_print,"%02d%02d%02d",atoi(time_struct.time_hour),atoi(time_struct.time_min),atoi(time_struct.time_sec));
//sprintf(HrMinSec,"%s%s%s",time_struct.time_hour,time_struct.time_min, time_struct.time_sec);		

		//printf("9\n");

		for(i = 0;i < loadcell_fs; i++)
		{			
			sensor_a = MAX((float)(((int)(start_loadcell[4*i+0]<<8))>>8)/scale,0);
			sensor_b = MAX((float)(((int)(start_loadcell[4*i+1]<<8))>>8)/scale,0);
			sensor_c = MAX((float)(((int)(start_loadcell[4*i+2]<<8))>>8)/scale,0);
			sensor_d = MAX((float)(((int)(start_loadcell[4*i+3]<<8))>>8)/scale,0);
			
			if(sensor_a>500) sensor_a = 0;
			if(sensor_b>500) sensor_b = 0;
			if(sensor_c>500) sensor_c = 0;
			if(sensor_d>500) sensor_d = 0;

			fprintf(fp, "%s,%5.2f,%5.2f,%5.2f,%5.2f,\n", time_print, sensor_a, sensor_b, sensor_c, sensor_d);					
		}
		printf("%s,%5.2f,%5.2f,%5.2f,%5.2f,\n", time_print, sensor_a, sensor_b, sensor_c, sensor_d);	
		//printf("data saved!\n");

		//flush data
//		fflush(fp);

		//close file
		fclose(fp);

		return 0;
	}

