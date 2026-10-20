/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#include "fun_log.h"
//#include "ak_common.h"

LOG_PRINT_CB log_print_cb = NULL;

/* print color define */
#define NONE          "\033[m"          //close
#define NONE_N        "\033[m\n"        //close and new_line
#define RED           "\033[0;32;31m"   //red
#define LIGHT_RED     "\033[1;31m"      //light red
#define GREEN         "\033[0;32;32m"   //green
#define LIGHT_GREEN   "\033[1;32m"      //light green
#define BLUE          "\033[0;32;34m"	//blue
#define LIGHT_BLUE    "\033[1;34m"		//light blue
#define DARK_GREY     "\033[1;30m"		//dark grey
#define LIGHT_GREY    "\033[0;37m"		//light grey
#define CYAN          "\033[0;36m"		//syan
#define LIGHT_CYAN    "\033[1;36m"		//light cyan
#define PURPLE        "\033[0;35m"		//purple
#define LIGHT_PURPLE  "\033[1;35m"		//light purple
#define BROWN         "\033[0;33m"		//brown
#define YELLOW        "\033[1;33m"		//yellow
#define WHITE         "\033[1;37m"		//white

#define MAX_TRACE_SIZE          (16)
#define LOG_BUF_LEN             (1024)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


log_app_filter_info log_filter = {0};

static int log_print(int level,char *str_t, long time_usec,char *buf)
{

		/* color select and syslog write judge */
	switch (level) {

	case LOG_APP_FATAL:
		/* error print, red color */
		printf(PURPLE"[%s %03ld] %s"NONE, str_t, time_usec/1000, buf);
		break;
	case LOG_APP_ERROR:
		/* error print, red color */
		printf(LIGHT_RED"[%s %03ld] %s"NONE, str_t, time_usec/1000, buf);
		break;
	case LOG_APP_WARNING:
		/* fatal print, yellow color */
		printf(YELLOW"[%s %03ld] %s"NONE,  str_t, time_usec/1000, buf);
		break;
	case LOG_APP_INFO:
		/* info print, blue color */
		printf(GREEN"[%s %03ld] %s"NONE, str_t, time_usec/1000, buf);
		break;
	case LOG_APP_DEBUG:
		/* debug print, green color */
		printf(WHITE"[%s %03ld] %s"NONE, str_t, time_usec/1000, buf);
		break;
	default:
		break;
	}
	return 0;

}

int log_print_filter(int level,long long ptime,char* buf)
{

	if(ptime > 946684800)
	{
		if(log_filter.StartT != 0 && log_filter.EndT != 0)
		{
			if(ptime < log_filter.StartT || ptime > log_filter.EndT)
			{
				//printf("time:%lld Have been filtered!\n",ptime);
				return -1;
			}
		}
	}
	if((strlen(log_filter.KeyWord) > 0))
	{
		if(strstr(buf,log_filter.KeyWord) == NULL)
		{
			//printf("buf:%s keyword:%s\n",buf,log_filter.KeyWord);
			return -1;
		}
	}

	return 0;
}

int log_print_app(int level, const char *file, const char *fun, int line, const char *fmt, ...)
{
	/*printf("module_id :%d level:%d file:%s fun:%s s_line:%d e_line:%d start_time:%lld end_time:%lld keyword:%s \n",log_filter.module_id,log_filter.level,
		log_filter.file,log_filter.fun,log_filter.start_line,log_filter.end_line,log_filter.start_time,log_filter.end_time,log_filter.keyword);*/
	if (level > log_filter.Level)
	{
		return -1;
	}

	if(log_filter.StartLine >= 0 && log_filter.EndLine > 0)
	{
		if(line < log_filter.StartLine || line > log_filter.EndLine)
		{
			return -1;
		}
	}

    if(log_filter.Function[0] != 0)
	{
		if(strcmp(log_filter.Function,fun) != 0)
		{
			return -1;
		}
	}

	const char* pFileName = strrchr(file,'/');
	if(pFileName != NULL)
	{
		pFileName += 1;
	}
	else
	{
		pFileName = file;
	}

    if(log_filter.File[0] != 0)
	{
		if(strcmp(log_filter.File,pFileName) != 0)
		{
			return -1;
		}
	}

	char buf[LOG_BUF_LEN] = {0};
    int len = sprintf(buf,"[%s %s %d]:",pFileName,fun,line);

    va_list args;
    va_start(args, fmt);
    int actual_len = vsnprintf(buf + len, LOG_BUF_LEN - len - 1, fmt, args);
    va_end(args);

    int total_len = actual_len + len;

	int ret = log_print_filter(level,time((time_t*)NULL),buf);
	if(ret != 0)
	{
		return -1;
	}

	struct timeval tv;
	char str_t[26] = {0};

	gettimeofday(&tv, NULL);
	time_t ptime = tv.tv_sec;
	long time_usec = tv.tv_usec;

    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", localtime(&ptime));

	if(log_print_cb != NULL)
	{
		log_print_cb(level, str_t, time_usec, buf,total_len);
	}

	log_print( level, str_t, time_usec, buf);

	fflush(stdout);
//	fflush(stderr);
	struct timeval tvEnd;
	gettimeofday(&tvEnd, NULL);

	//printf("flushlog=%d_%d_%d_%d_%d  %d_%d_%d_%d_%d\n", tvstart[0].tv_usec/1000, tvstart[1].tv_usec/1000, tvstart[2].tv_usec/1000, tvstart[3].tv_usec/1000, tvstart[4].tv_usec/1000,
	//tvstart[5].tv_usec/1000, tvstart[6].tv_usec/1000, tvstart[7].tv_usec/1000, tvstart[8].tv_usec/1000, tvstart[9].tv_usec/1000);

	return 0;
}
int log_print_app_set_filter(log_app_filter_info *pLogFilterInfo)
{
    if(pLogFilterInfo == NULL)
    {
        return -1;
    }

    if(memcmp(pLogFilterInfo,&log_filter,sizeof(log_app_filter_info)) == 0)
    {
        printf("input param is similar.\n");
        return 0;
    }

    memset(&log_filter,0,sizeof(log_app_filter_info));
    memcpy(&log_filter,pLogFilterInfo,sizeof(log_app_filter_info));

    return 0;
}

int log_print_app_set_cb(LOG_PRINT_CB cb)
{
	log_print_cb = cb;
	return 0;
}

int log_print_app_set_level(int level)
{
	log_app_filter_info LogFilterInfo = {0};
	LogFilterInfo.Level = level;
	log_print_app_set_filter(&LogFilterInfo);
}


/* end of file */
