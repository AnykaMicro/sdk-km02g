/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_COMMON_H_
#define _AK_COMMON_H_

#include <stdio.h>
#include <time.h>
#include "ak_types.h"

#define AK_SUCCESS          0
#define AK_FAILED           -1

/* os_adapter */
#define MODULE_NAME_MEMORY             "MEMORY"
#define	MODULE_NAME_THREAD             "THREAD"
#define	MODULE_NAME_LOG                "LOG"
#define	MODULE_NAME_COMMON             "COMMON"
#define	MODULE_NAME_IPCSRV             "IPCSRV"
#define	MODULE_NAME_AI                 "AI"
#define	MODULE_NAME_AO                 "AO"
#define	MODULE_NAME_VI                 "VI"
#define	MODULE_NAME_VO                 "VO"
#define	MODULE_NAME_TIMER              "TIMER"
#define	MODULE_NAME_VPSS               "VPSS"
#define	MODULE_NAME_DRV                "DRV"
#define MODULE_NAME_TDE                "TDE"
#define MODULE_NAME_UUID			   "UUID"
#define MODULE_NAME_OSAL               "OSAL"

/* mpi */
#define	MODULE_NAME_VENC               "VENC"
#define	MODULE_NAME_VDEC               "VDEC"
#define	MODULE_NAME_AENC               "AENC"
#define	MODULE_NAME_ADEC               "ADEC"
#define	MODULE_NAME_OSD                "OSD"
#define MODULE_NAME_MUX                "MUX"
#define MODULE_NAME_DEMUX              "DEMUX"
#define MODULE_NAME_SVP                "SVP"
#define MODULE_NAME_FR                 "FR"
#define MODULE_NAME_IPU                 "IPU"

/* app */
#define	MODULE_NAME_DVR                "DVR"
#define	MODULE_NAME_MD                 "MD"
#define	MODULE_NAME_GUI                "GUI"
#define	MODULE_NAME_AED                "AED"
#define MODULE_NAME_AUDIO_TOOL         "AUDIO_TOOL"
#define MODULE_NAME_VQE                "VQE"
#define MODULE_NAME_AUDIO_EFFECT       "AUDIO_EFFECT"

#define	MODULE_NAME_APP_VIDEO          "APP_VIDEO"
#define	MODULE_NAME_APP_AUDIO          "APP_AUDIO"

#define MODULE_NAME_DVR_IDX            "DVR_IDX"
#define MODULE_NAME_NET                "NET"
#define MODULE_NAME_APP_MEM            "APP_MEM"
#define MODULE_NAME_APP_AUDIO          "APP_AUDIO"
#define	MODULE_NAME_NPU			       "NPU"

/*atc*/
#define MODULE_NAME_ATC_ECHO		   "ECHO"
#define MODULE_NAME_ATC_ADEC		   "SD_ADEC"
#define MODULE_NAME_ATC_AENC		   "SD_AENC"
#define	MODULE_NAME_AKV_VENC		   "AKV_VENC"
#define	MODULE_NAME_AKV_VDEC		   "AKV_VDEC"
#define	MODULE_NAME_AKV_MUX			   "AKV_MUX"
#define	MODULE_NAME_AKV_DEMUX		   "AKV_DEMUX"



/* end */
#define MODULE_NAME_APP                "Application"
#define	MODULE_NAME_ALL                "ALL"

/**********************Module ID*******************/
/* 注意：新增模块ID只能在尾部添加，不可在中间插入 */
/**************************************************/
typedef enum akMODULE_ID_E {
    MODULE_ID_MEMORY                 = 0x0,
    MODULE_ID_THREAD,
    MODULE_ID_LOG,
    MODULE_ID_COMMON,
    MODULE_ID_IPCSRV,
    MODULE_ID_AI,
    MODULE_ID_AO,
    MODULE_ID_VI,
    MODULE_ID_VO,
    MODULE_ID_TIMER,
    MODULE_ID_VPSS,
    MODULE_ID_DRV,
    MODULE_ID_TDE,
    MODULE_ID_VENC,
    MODULE_ID_VDEC,
    MODULE_ID_AENC,// 15
    MODULE_ID_ADEC,
    MODULE_ID_OSD,
    MODULE_ID_MUX,
    MODULE_ID_DEMUX,
    MODULE_ID_DVR,
    MODULE_ID_MD,
    MODULE_ID_GUI,
    MODULE_ID_AED,
    MODULE_ID_AUDIO_TOOL,

    MODULE_ID_VQE,

    MODULE_ID_APP_VIDEO,
    MODULE_ID_DVR_IDX,
    MODULE_ID_NET,
    MODULE_ID_OSAL,
    MODULE_ID_APP_MEM,
    MODULE_ID_APP_AUDIO,
    MODULE_ID_UUID,
    MODULE_ID_ATC_ECHO,
    MODULE_ID_ATC_ADEC,
    MODULE_ID_ATC_AENC,
    MODULE_ID_AKV_VENC,
    MODULE_ID_AKV_VDEC,
    MODULE_ID_AKV_MUX,
    MODULE_ID_AKV_DEMUX,
    MODULE_ID_SVP,
    MODULE_ID_NPU,
    MODULE_ID_FR,
    MODULE_ID_AUDIO_EFFECT,
    MODULE_ID_IPU,
    /* 新增模块ID，请在该行之上添加 */
    MODULE_ID_APP                     = 0xFE,
    MODULE_ID_ALL                     = 0xFF,
}MODULE_ID_E;

typedef enum akERROR_TYPE_E {
    ERROR_TYPE_NO_ERROR            = 0,
    ERROR_TYPE_POINTER_NULL        = 0x100,
    ERROR_TYPE_INVALID_ARG,
    ERROR_TYPE_MALLOC_FAILED,
    ERROR_TYPE_PMEM_MALLOC_FAILED,
    ERROR_TYPE_FUNC_NOT_SUPPORT,		//5
    ERROR_TYPE_NOT_INIT,
    ERROR_TYPE_CALL_ORDER_WRONG,
    ERROR_TYPE_DEV_OPEN_FAILED,
    ERROR_TYPE_DEV_INIT_FAILED,
    ERROR_TYPE_DEV_CTRL_FAILED,			//10
    ERROR_TYPE_DEV_READ_FAILED,
    ERROR_TYPE_DEV_WRITE_FAILED,
    ERROR_TYPE_DEV_CLOSE_FAILED,
    ERROR_TYPE_FILE_OPEN_FAILED,
    ERROR_TYPE_FCNTL_FAILED,
    ERROR_TYPE_FILE_READ_FAILED,		//15
    ERROR_TYPE_FILE_WRITE_FAILED,
    ERROR_TYPE_IOCTL_FAILED,
    ERROR_TYPE_CLOSE_FAILED,
    ERROR_TYPE_GET_V4L2_PTR_FAILED,
    ERROR_TYPE_INVALID_USER,
    ERROR_TYPE_THREAD_CREATE_FAILED,	//20
    ERROR_TYPE_POPEN_FAILED,
    ERROR_TYPE_MEDIA_LIB_FAILED,
    ERROR_TYPE_NO_DATA,
    ERROR_TYPE_EBUSY,
    ERROR_TYPE_NO_ENOUGH_RESOURCE,
    ERROR_TYPE_NO_LEFT_SPACE,

    /* for other module*/
    ERROR_TYPE_NUM
}ERROR_TYPE_E;

typedef enum akSDK_RUN_MODE_E {
    SDK_RUN_NORMAL,
    SDK_RUN_DEBUG
}SDK_RUN_MODE_E;

/* audio/video data frame */
typedef struct akFRAME_S {
    AK_U8 *pu8Data;	//frame data
    AK_U32 u32Len;		//frame len in bytes
    AK_U64 u64Ts;	//timestamp(ms)
    AK_U64 u64SeqNo;	//current frame sequence no.
}FRAME_S;

typedef struct akTIME_VAL_S {
    AK_U64 u64Sec;     /* seconds */
    AK_U64 u64Usec;    /* microseconds */
}TIME_VAL_S;

typedef struct akDATE_S {
    AK_S32 s32Year; 		//4 number
    AK_S32 s32Month;		//1-12
    AK_S32 s32Day; 		//1-31
    AK_S32 s32Hour; 		//0-23
    AK_S32 s32Minute; 	//0-59
    AK_S32 s32Second; 	//0-59
    AK_S32 s32Timezone; 	//local time zone -12-12
}DATE_S;

typedef struct akSDK_RUN_CONFIG_S {
    AK_S32 s32MemTraceFlag;
    AK_S32 s32DmaMemTraceFlag;
    // isp tool server flag, 0 : disable, 1 : enable
    AK_S32 s32IspToolServerFlag;
} SDK_RUN_CONFIG_S;

/**
 * AK_BASE_CommonGetVersion - get common module version
 * return: version string
 */
const AK_CHAR* AK_BASE_CommonGetVersion(AK_VOID);

/**
 * AK_BASE_SleepMs - sleep certain time
 * @ms[IN]: milli-seconds
 * return: none
 */
AK_VOID AK_BASE_SleepMs(const AK_S32 s32Ms);

/**
 * AK_BASE_GetOstime - get OS time since cpu boot
 * @tv[OUT]: time value since startup
 * return: AK_VOID
 */
AK_VOID AK_BASE_GetOstime(TIME_VAL_S *pstTv);

/**
 * AK_BASE_GetOsTimeStamp - get OS time stamp
 * return: time stamp
 */
time_t AK_BASE_GetOsTimeStamp(AK_VOID);

/**
 * AK_BASE_GetLocalDate - get local date time
 * @date[OUT]: get date time
 * return: AK_SUCCESS or error_code
 * notes:
 */
AK_S32 AK_BASE_GetLocalDate(DATE_S *pstDate);

/**
 * AK_BASE_SetLocalDate - set local date time
 * @date[IN]: set date time
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_SetLocalDate(const DATE_S *pstDate);

/**
 * AK_BASE_SetTimezone - set timezone only
 * @s32Timezone[IN]: timezone value, range [-12, 12]
 * return: AK_SUCCESS or error_code
 * notes: This function only sets the timezone, does not modify system time
 */
AK_S32 AK_BASE_SetTimezone(AK_S32 s32Timezone);

/**
 * AK_BASE_SetSystemTime - set system time only
 * @pstDate[IN]: set date time (timezone field is ignored)
 * return: AK_SUCCESS or error_code
 * notes: This function only sets the system time, does not modify timezone
 */
AK_S32 AK_BASE_SetSystemTime(const DATE_S *pstDate);

/**
 * AK_BASE_DiffMsTime - 计算2个时间戳之间的差值
 * @cur_time[IN]: 当前时间戳
 * @pre_time[IN]: 过往时间戳
 * return: 以毫秒为单位的整数
 * notes: 1.当前时间戳如果小于过往时间戳,返回负值.
          2.获取到最大时间差值在64bit和32bit有不同,
            在64bit环境下返回值范围: -9223372036854775808 ~ 9223372036854775807
            在32bit环境下返回值范围: -2147483648 ~ 2147483647
            假如时间戳的差值大于上述范围则产生溢出,返回最小或最大时间戳值
 */
long AK_BASE_DiffMsTime(const TIME_VAL_S *pstCurTime,
                    const TIME_VAL_S *pstPreTime);

/**
 * AK_BASE_SecondsToString - transfer total seconds to readable time string
 * @secs: passed total seconds from 1970-01-01 00:00:00
 * return: readable time string after transferred
 * notes: string format: yyyy-MM-dd HH:mm:ss, ex: 2016-04-06 10:06:06
 *      time_str min len 20 bytes.
 * IMPORTANT: use ONLY ONCE after call this function.
 */
AK_CHAR* AK_BASE_SecondsToString(time_t secs);

/**
 * AK_BASE_SecondsToDate - transfer seconds to date time value
 * @seconds[IN]: seconds from 1970-01-01 00:00:00
 * @date[OUT]: date time value
 * return: 0 success; error code when failed
 * notes: seconds from 1970-01-01 00:00:00 +0000(UTC)
 */
AK_S32 AK_BASE_SecondsToDate(time_t seconds, DATE_S *pstDate);


/**
 * AK_BASE_DateToSeconds - transfer date time value to seconds
 * @date[IN]: date time value
 * return: seconds after transferred; failed -1
 * notes: seconds from 1970-01-01 00:00:00 +0000(UTC)
 */
time_t AK_BASE_DateToSeconds(const DATE_S *pstDate);

/**
 * AK_BASE_DateToString - transfer date time value to time string
 * @date[IN]: date time value
 * @str[OUT]: date time string after transfer OK
 * return: return AK_SUCCESS or error_code
 * notes: 1. string format: yyyyMMdd-HHmmss, ex: 20160406-100606
 *		2. MAKE SURE str has enough space outside
 */
AK_S32 AK_BASE_DateToString(const DATE_S *pstDate, AK_CHAR *pcStr);

/**
 * AK_BASE_StringToDate - transfer date time string to date time value
 * @time_str[IN]: time string
 * @date[OUT]: date time value after transfer OK
 * return: return AK_SUCCESS or error_code
 * notes: string format: yyyyMMdd-HHmmss, ex: 20160406-100606
 */
AK_S32 AK_BASE_StringToDate(const AK_CHAR *pcTimeStr, DATE_S *pstDate);

/**
 * AK_BASE_Open - open file
 * @pathname[IN]: path
 * @flags[IN]: flag
 * return: return AK_SUCCESS or error_code
 * notes:
 */
AK_S32 AK_BASE_Open(const AK_CHAR *pcPathName, AK_S32 s32Flags);

/**
 * AK_BASE_SdkInit - SDK init
 * @pstConfig[IN]: sdk run pstConfig
 * return: return AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_SdkInit(SDK_RUN_CONFIG_S *pstConfig);

/**
 * AK_BASE_SdkExit - SDK exit
 * @AK_VOID: AK_VOID
 * return: return AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_SdkExit(AK_VOID);

#endif

/* end of file */
