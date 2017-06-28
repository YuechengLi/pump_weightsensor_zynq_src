#ifndef GET_TIME_H
#define GET_TIME_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
	char time_year[5];
	char time_month[4];
	char time_day[3];
	char time_hour[3];
	char time_min[3];
	char time_sec[3];
} Current_tm;

//void get_time(Current_tm *);
void get_time(Current_tm * sub_time)
{
	char *allinfo;
	time_t timer;

	timer=time(NULL);
	allinfo=asctime(localtime(&timer));

	strncpy(sub_time->time_year,(allinfo+20*sizeof(char)),4*sizeof(char));

	strncpy(sub_time->time_month,(allinfo+4*sizeof(char)),3*sizeof(char));

	strncpy(sub_time->time_day,(allinfo+8*sizeof(char)),2*sizeof(char));

	strncpy(sub_time->time_hour,(allinfo+11*sizeof(char)),2*sizeof(char));

	strncpy(sub_time->time_min,(allinfo+14*sizeof(char)),2*sizeof(char));

	strncpy(sub_time->time_sec,(allinfo+17*sizeof(char)),2*sizeof(char));
}


#endif
