/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_VPSS_ISP_H_
#define _AK_VPSS_ISP_H_

#include "ak_types.h"
#include "ak_common.h"
#include "ak_common_video.h"


#define ISP_MD_DIMENSION_H_MAX      (64)
#define ISP_MD_DIMENSION_V_MAX      (36)

#define PP_MASK_AREA_MAX            (14)

#define BOX_COLOR_NUM               (7)
#define BOX_MAX_NUM                 (50)


typedef enum akISP_DAY_NIGHT_MODE_E {
    MODE_DAY,
    MODE_NIGHT,
    MODE_USR_DEFINE_1,
    MODE_USR_DEFINE_2,
    MODE_USR_DEFINE_3,
    MODE_NUM
} ISP_DAY_NIGHT_MODE_E;

typedef enum akISP_NIGHT_TYPE_E {
    IR_LED_NIGHT,
    WHITE_LED_NIGHT,
    BLACKLIGHT_NIGHT,
    TYPE_NUM
} ISP_NIGHT_TYPE_E;


typedef enum akISP_EFFECT_TYPE_E {
    /* HUE to WDR, value: [-50, 50], 0 means use the value in ISP config file */
    ISP_EFFECT_HUE = 0x00,
    ISP_EFFECT_BRIGHTNESS,
    ISP_EFFECT_SATURATION,
    ISP_EFFECT_CONTRAST,
    ISP_EFFECT_SHARP,
    ISP_EFFECT_HDR,

    ISP_POWER_HZ,		//50 or 60
    ISP_FLIP_MIRROR    //[0, 3]
} ISP_EFFECT_TYPE_E;

typedef enum akISP_GAIN_STAT_E {
    ISP_GAIN_NO_CHANGE_STAT = 0,
    ISP_GAIN_LOW_STAT,
    ISP_GAIN_HIGH_STAT,
    ISP_GAIN_MID_STAT,
} ISP_GAIN_STAT_E;

typedef enum akISP_OP_TYPE_E{
    ISP_OP_TYPE_MANUAL = 0,
    ISP_OP_TYPE_AUTO = 1,
    ISP_OP_TYPE_BUTT,
} ISP_OP_TYPE_E;

typedef enum akISP_AE_STRGY_MODEL_E {
    ISP_AE_EXP_HIGHLIGHT_PRIOR = 0,
    ISP_AE_EXP_LOWLIGHT_PRIOR,
    ISP_AE_STRGY_MODEL_BUFF,
} ISP_AE_STRGY_MODEL_E;

typedef enum akISP_AWB_ALGO_TYPE_E{
    ISP_AWB_ALGO_DEFAULT = 0,
    ISP_AWB_ALGO_AVERAGE_WEIGHT = 1,
    ISP_AWB_ALGO_BLOCK_ASSIST,
    ISP_AWB_ALGO_GW,
    ISP_AWB_ALGO_BUTT,
} ISP_AWB_ALGO_TYPE_E;

typedef enum akISP_DAY_NIGHT_STATE {
    STATE_DAY = 0,
    STATE_NIGHT,
} ISP_DAY_NIGHT_STATE;



typedef struct akISP_MD_INFO_S {
    AK_U16  u16Stat[ISP_MD_DIMENSION_V_MAX][ISP_MD_DIMENSION_H_MAX];
} ISP_MD_INFO_S;

typedef struct akISP_AF_INFO_S {
    AK_U32   u32AfStatics[8][16][5];
    AK_U8    u8AfLocalHighlight[64][128];
} ISP_AF_INFO_S;

typedef struct akISP_OD_INFO_S {
    AK_U32   u32AfStatics[8][16][5];
    AK_U32   u32RgbHist[256];
    AK_U64   u64RgbTotal;
} ISP_OD_INFO_S;


typedef struct akISP_AF_ATTR_S {
    AK_U16  u16AfStatEnable;
    AK_U8   u8AfSpotlightLumTh;	   //[0 ,255]
    AK_U8   u8AfSpotlightEdgeTh;   //[0 ,255]
    AK_U8   u8AfSpotlightEdgeGain; //[0, 15]
    AK_U8   u8AfClipTh;			   //[0, 63]
    AK_U8   u8AfPeakCtrl;		   //[0, 255]
    AK_U8   u8AfRshift;			   //[0, 15]
} ISP_AF_ATTR_S;

typedef struct akISP_FPS_LEVEL_S {
    AK_U32 u32HighFps;
    AK_U32 u32HighFpsExpTime;
    AK_U32 u32HighFpsToLowerFpsGain;

    AK_U32 u32MidFps;                //set 0 means no use middle fps
    AK_U32 u32MidFpsExpTime;         //set 0 means no use middle fps
    AK_U32 u32MidFpsToLowFpsGain;    //set 0 means no use middle fps

    AK_U32 u32LowFps;
    AK_U32 u32LowFpsExpTime;
    AK_U32 u32LowFpsToHigherFpsGain;
} ISP_FPS_LEVEL_S;

typedef struct akISP_RAW_HIST_STAT_INFO_S {
    AK_U32  u32rawGHist[256];
    AK_U64  u32RawGTotal;
} ISP_RAW_HIST_STAT_INFO_S;

typedef struct akISP_RGB_HIST_STAT_INFO_S {
    AK_U32  u32RgbHist[256];
    AK_U64  u32RgbTotal;
} ISP_RGB_HIST_STAT_INFO_S;

typedef struct  akISP_YUV_HIST_STAT_INFO_S {
    AK_U32  u32YHist[256];
    AK_U64  u32YTotal;
} ISP_YUV_HIST_STAT_INFO_S;

typedef struct akISP_WDR_HIST_STAT_INFO_S{
    AK_U32  u32WdrHistShort[256];
    AK_U32	u32WdrHistLong[256];
} ISP_WDR_HIST_STAT_INFO_S;

typedef struct akISP_AE_STAT_INFO_S{
    ISP_RAW_HIST_STAT_INFO_S    stRawHistStatPara;
    ISP_RGB_HIST_STAT_INFO_S    stRgbHistStatPara;
    ISP_YUV_HIST_STAT_INFO_S    stYuvHistStatPara;
    ISP_WDR_HIST_STAT_INFO_S    stRawWdrCombStatPara;
} ISP_AE_STAT_INFO_S;

typedef struct akISP_AE_RUN_INFO_S {
    AK_U8	u8CurrentCalcAvgLumi;
    AK_U8	u8CurrentCalcAvgCompensationLumi;
    AK_U8	u8CurrentTargetLumi;
    AK_U8	u8IsStable;

    AK_U32  u32CurrentAGain;
    AK_U32  u32CurrentDGain;
    AK_U32  u32CurrentIspDGain;
    AK_U32  u32CurrentExpTime;

    AK_U32  u32CurrentAGainStep;
    AK_U32  u32CurrentDGainStep;
    AK_U32  u32CurrentIspDGainStep;
    AK_U32  u32CurrentExpTimeStep;

    AK_U32  u32CurrentRefExpRatio;
    AK_U32  u32CurrentLongExpTime;
    AK_U32  u32CurrentShortExpTime;
    AK_U8   u8CurrentShortAvgLumi;
    AK_U8   u8CurrentLongAvgLumi;

    ISP_AE_STAT_INFO_S stAeStatInfo;
} ISP_AE_RUN_INFO_S;

typedef struct akISP_RAW_G_HIST_ATTR_S{
    AK_U8  u8Enable;
    AK_U8  u8RawRshift;	//[0, 15]
    AK_U8  u8ZoneWeight[8][16];
} ISP_RAW_G_HIST_ATTR_S;

typedef struct akISP_RGB_HIST_ATTR_S{
    AK_U8  u8RgbHistEnable;
    AK_U8  u8ZoneWeight[8][16];
} ISP_RGB_HIST_ATTR_S;

typedef struct akISP_YUV_HIST_ATTR_S{
    AK_U16  u16YuvHistEnable;
    AK_U8   u8ZoneWeight[8][16];
} ISP_YUV_HIST_ATTR_S;

typedef struct akISP_AE_STRGY_ATTR_S {
    AK_U32	u32HistWeight[16];
} ISP_AE_STRGY_ATTR_S;

typedef struct akISP_AE_ROUTE_NODE_ATTR_S {
    AK_U32   u32ExpTime;
    AK_U32   u32Again;
    AK_U32   u32Dgain;
    AK_U32   u32IspDgain;
} ISP_AE_ROUTE_NODE_ATTR_S;

typedef struct akISP_AE_ROUTE_ATTR_S{
    AK_U8   u8AeRouteEn;
    AK_U8   u8AeRouteNodeNum;
    ISP_AE_ROUTE_NODE_ATTR_S  stAeRouteNode[10];
} ISP_AE_ROUTE_ATTR_S;

typedef struct akISP_AE_EX_CONVER_ATTR_S {
    AK_U8    u8AeExConvergenceEn;
    AK_U16   u16ControlZone1;
    AK_U16   u16ControlZone2;
    AK_U16   u16ControlZone3;
    AK_U16   u16ControlZone4;
    AK_U16   u16ControlStep1;
    AK_U16   u16ControlStep2;
    AK_U16   u16ControlStep3;
    AK_U16   u16ControlStep4;
    AK_U16   u16ControlStepMax;
} ISP_AE_EX_CONVER_ATTR_S;

typedef struct akISP_WDR_AE_ATTR_S
{
    AK_U16 u16ExpRatioType;
    AK_U16 u16ExpRatioLS;
    AK_U16 u16ExpRatioSVS;
    AK_U16 u16ExpRatioMax;
    AK_U16 u16ExpRatioMin;
    AK_U8  u8ExpRatioSpeed;
    AK_U8  u8ExpHighlightCtrl;
} ISP_WDR_AE_ATTR_S;


typedef struct akISP_AE_ATTR_S {
    AK_U32  u32ExpTimeMax;			// Max exposure time
    AK_U32  u32AGainMax;			// Max analog gain
    AK_U32  u32DGainMax;				// Max digital gain
    AK_U32  u32IspDGainMax;			// Max ISP digital gain
    AK_U16  u16ExpTimeMin;			// Min exposure time
    AK_U16  u16AGainMin;				// Min analog gain
    AK_U16  u16DGainMin;			// Min digital gain
    AK_U16  u16IspDGainMin;			// Min ISP digital gain
    AK_U16  u16BlacklightRateMax;
    AK_U16  u16BlacklightRateMin;
    AK_U16  u16ExpStep;				// User exposure step
    AK_U8   u8ExpStableRange;		// Stable range
    AK_U8   u8ExpHoldRange;
    AK_U8   u8ExpSpeed;
    AK_U8   u8AntiFlickerTargetLumi;
    AK_U8   u8AntiFlickerStartExp;//[1,100]
    AK_U8   u8ExpLumiFilterPara;
    AK_U8   u8TtargetLumiance[12]; 		// Target luminance
    AK_U8   u8BlacklightCompensationEn;
    AK_U8   u8BlacklightDetectScope;
    ISP_RAW_G_HIST_ATTR_S      stRawGHistAttr;
    ISP_RGB_HIST_ATTR_S        stRgbHistAttr;
    ISP_YUV_HIST_ATTR_S        stYuvHistAttr;
    AK_U16                      u16EnviGainRange[12][2];
    ISP_AE_EX_CONVER_ATTR_S    stAeExConvergencePara;
    ISP_AE_STRGY_MODEL_E       enAeStrgyType;
    ISP_AE_STRGY_ATTR_S        stAeStrgyPara[3];
    ISP_AE_ROUTE_ATTR_S        stExpRoutePara;
    ISP_WDR_AE_ATTR_S          stWdrAePara;
}ISP_AE_ATTR_S;

typedef struct akISP_ME_ATTR_S {
    AK_U32   u32ExpTime;
    AK_U32   u32AGain;
    AK_U32   u32DGain;
    AK_U32   u32IspDGain;
} ISP_ME_ATTR_S;

typedef struct akISP_HDR_S {
    AK_U8   u8HdrEnable;
    AK_U8   u8HdrUvAdjustEnable;
    AK_U16	u16HdrTh1;	  //0-1023
    AK_U16	u16HdrTh2;	  //0-1023
    AK_U16	u16HdrTh3;	  //0-1023
    AK_U16	u16HdrTh4;	  //0-1023
    AK_U16	u16HdrTh5;	  //0-1023

    AK_U16	u16AreaTb1[65];	  // Curve 10bit
    AK_U16	u16AreaTb2[65];	  // Curve 10bit
    AK_U16	u16AreaTb3[65];	  // Curve 10bit
    AK_U16	u16AreaTb4[65];	  // Curve 10bit
    AK_U16	u16AreaTb5[65];	  // Curve 10bit
    AK_U16	u16AreaTb6[65];	  // Curve 10bit

    AK_U16	u16Area1Key[16];
    AK_U16	u16Area2Key[16];
    AK_U16	u16Area3Key[16];
    AK_U16	u16Area4Key[16];
    AK_U16	u16Area5Key[16];
    AK_U16	u16Area6Key[16];

    AK_U16  u16HdrCnoiseSuppressYth1;	 // Chroma noise luminance threshold 1
    AK_U16	u16HdrCnoiseSuppressYth2;	 // Chroma noise luminance threshold 2
    AK_U16  u16HdrC_cnoiseSuppressGain;	 // Chroma suppression gain
    AK_U8   u8HdrCnoiseSuppressSlop;	 // Suppression slope
    AK_U8	u8HdrUvAdjustLevel;
    AK_U8	u8HdrSpatialfiltK;
}ISP_HDR_S;

typedef struct akISP_HDR_ATTR_S
{
    ISP_OP_TYPE_E	enHdrMode;              // Mode: manual or linkage
    ISP_HDR_S		stManualHdr;
    ISP_HDR_S 		stLinkageHdr[12];
} ISP_HDR_ATTR_S;

typedef struct akISP_3DNR_S {
    AK_U8		u8TnrEnable;			//default:1(GUI edit)
    AK_U8		u8TnrRefFrameFormat;	// Reference frame compression format
    AK_U8 		u8YnrDPCStr;	                //[0, 127]
    AK_U8 		u8YnrDPCCap;	                //[0, 127]
    AK_U8 		u8YnrDPCDiffGain[8];	        //[0,7]
    AK_U8 		u8YnrK;			//[0,15]
    AK_U8		u8YnrLumStr[32]; //[0, 15]
    AK_U8		u8YlpK1;			//[0, 15],default:7(GUI edit)
    AK_U8		u8YlpK2;			//[0, 15],default:0(GUI edit)
    AK_U8 		u8YDiffUVWgt;		//[0, 15]
    AK_U8 		u8YDiffEdgeWgt;	//[0, 15]
    AK_U8		u8TYLow1;		//[0,127]
    AK_U8		u8TYLow2;		//[0,127]
    AK_U8		u8TYK1; 		//[0, 127],default:120(GUI edit)
    AK_U8		u8TYK2; 		//[0, 127],default:120(GUI edit)
    AK_U8		u8TYExK; 		//[0, 15]
    AK_U8		u8TYKslop1; 		//[0, 127],default:32(GUI edit)
    AK_U8		u8TYKslop2;		//[0, 127]
    AK_U8		u8TYSrcChooseKTh; 		//[0, 127]
    AK_U8		u8TYUpdateTh;

    AK_U8		u8UvnrK; 		//[0, 15],default:8(GUI edit)
    AK_U8		u8UvlpK1; 		//[0, 15],default:7(GUI edit)
    AK_U8		u8UvlpK2; 		//[0, 15],default:0(GUI edit)
    AK_U8		u8TUvDiffScale;//[0, 15]
    AK_U8		u8TUvKLow1;		//[0, 127]
    AK_U8		u8TUvKLow2;	//[0, 127]
    AK_U8		u8TUvK1; 		//[0, 127],default:120(GUI edit)
    AK_U8		u8TUvK2; 		//[0, 127]
    AK_U8		u8TUvExK; 		//[0, 15],
    AK_U8		u8TUvKslop1; 		//[0, 127],default:32(GUI edit)
    AK_U8		u8TUvKslop2; 		//[0, 127],default:32(GUI edit)
    AK_U8		u8TUvSrcChooseKTh; //[0, 127]

    AK_U8		u8MfactorKTh; 		    //[0, 127]
    AK_U8		u8MfactorSlop; 		//[0, 15]
    AK_U8 		u8MotionStatTh;	        //[0, 15]
    AK_U8		u8SfactorStatTh;	    //[0, 3]
    AK_U8 		u8MotionFilter;			//[0,15]
    AK_U8 		u8MotionAreaExpand;     //[0, 1]
    AK_U8 		u8LumStr[32];	            //[0, 15]
    AK_U16		u16TMfTh1; 	//[0, 8191],default:300(GUI edit)
    AK_U16		u16TMfTh2;	//[0, 8191],default:500(GUI edit)
    AK_U16		u16YnrCalcK;		//[0,65535](GUI edit)
    AK_U16		u16YnrWeightTbl[17];//ynr_strength(GUI edit)
    AK_U16		u16TYTh1;		//[0, 511],default:48(GUI edit)
    AK_U16		u16TYTh2;		//[0, 511],default:16(GUI edit)
    AK_U16		u16YUvTh1;		//[0, 511],default:48(GUI edit)
    AK_U16		u16YUvTh2;		//[0, 511],default:16(GUI edit)
    AK_U16		u16MdTh;			//[0, 65535]
    AK_U16		u16Weight4_7X5;	//[0,8695]
    AK_U16      u16TnrMotionFlagTh;
}ISP_3DNR_S;

typedef struct  akISP_3DNR_ATTR_S {
    AK_U8 	        u8TnrDebugOutput;	//[0,1]
    AK_U8           u8MainTnrEnable;
    ISP_OP_TYPE_E	en3dnrMode;

    ISP_3DNR_S  	stManual3dnr;
    ISP_3DNR_S  	stLinkage3dnr[12];
} ISP_3DNR_ATTR_S;

typedef struct  akISP_MWB_ATTR_S
{
    AK_U16	u16RGain;
    AK_U16	u16GGain;
    AK_U16	u16BGain;
    AK_S16	s16ROffset;
    AK_S16	s16GOffset;
    AK_S16	s16BOffset;
} ISP_MWB_ATTR_S;

typedef struct akISP_AWB_STAT_CALIB_PARA_S{
    AK_U16	 u16GrLow[10];			//gr_low[i]<=gr_high[i]
    AK_U16	 u16GrHigh[10];
    AK_U16	 u16GbLow[10];			//gb_low[i]<=gb_high[i]
    AK_U16	 u16GbHigh[10];
    AK_U16	 u16RbLow[10];		   //rb_low[i]<=rb_high[i]
    AK_U16	 u16RbHigh[10];
} ISP_AWB_STAT_CALIB_PARA_S;

typedef  struct akISP_AWB_ATTR_S {
    AK_U8	                    u8GWeight[16];
    AK_U8	                    u8YLow; 				//y_low<=y_high
    AK_U8	                    u8YHigh;
    AK_U8	                    u8ErrEst;
    AK_U8	                    u8AwbIsoTrackEnable;
    AK_U8	                    u8ColortempEnvi[10];
    ISP_AWB_STAT_CALIB_PARA_S  stAwbStatCalibPara[12];


    // AWB software parameters
    AK_U16	                    u16AutoWbStep;				   // White balance step
    AK_U16	                    u16TotalCntThresh;			  // Pixel count threshold
    AK_U16	                    u16ColortempStableCntThresh; // Stable frame count for env color temp change
    ISP_AWB_ALGO_TYPE_E        enAlgType;
} ISP_AWB_ATTR_S;

typedef  struct  akISP_AWB_RATIO {
    AK_U32  ratio[10];
}ISP_AWB_RATIO;

typedef  struct  akISP_AWB_EX_ATTR {
    AK_U8        awb_ex_ctrl_enable;
    ISP_AWB_RATIO    prefer_rratio[12];
    ISP_AWB_RATIO    prefer_bratio[12];
}ISP_AWB_EX_ATTR;

typedef struct akISP_AWB_BLK_AVG_STAT_INFO_S{
    AK_U32 u32WbRavgStat[16][16];
    AK_U32 u32WbGavgStat[16][16];
    AK_U32 u32WbBavgStat[16][16];
} ISP_AWB_BLK_AVG_STAT_INFO_S;

/* Internal, not displayed */
typedef struct akISP_AWB_BLK_WP_STAT_INFO_S{
    AK_U32 u32WpRStat[16][16];
    AK_U32 u32WpGStat[16][16];
    AK_U32 u32WpBStat[16][16];
    AK_U32 u32WpCntStatics[16][16];
} ISP_AWB_BLK_WP_STAT_INFO_S;

typedef struct akISP_AWB_RUN_INFO_S{
    AK_U16  u16RGain;
    AK_U16  u16GGain;
    AK_U16  u16BGain;
    AK_S16  s16ROffset;
    AK_S16  s16GOffset;
    AK_S16  s16BOffset;
    AK_U8   u8IsStable;
    AK_U8   u8CurrentColortempIndex;	  // Current color temp index (env-dependent)
    AK_U16  u16ColortempStableCnt[10];		   // Stable frame count per color temp
    AK_U16  u16CurrentColortemp;
} ISP_AWB_RUN_INFO_S;


typedef struct akISP_AWB_STAT_INFO_S {
    // AWB statistics
    AK_U32   u32TotalR[10];
    AK_U32   u32TotalG[10];
    AK_U32   u32TotalB[10];
    AK_U32   u32TotalCnt[10];
    // AWB gains from auto white balance algorithm
    ISP_AWB_BLK_AVG_STAT_INFO_S	stWbBlkAvgStat;
    ISP_AWB_BLK_WP_STAT_INFO_S     stWbBlkWpStat;
    ISP_AWB_RUN_INFO_S             stWbRunInfo;
} ISP_AWB_STAT_INFO_S;

typedef struct akISP_SENSOR_REG_INFO_S {
    AK_U16 u16RegAddr;
    AK_U16 u16Value;
} ISP_SENSOR_REG_INFO_S;

typedef struct akISP_LENS_COEF_S
{
    AK_U16 u16CoefB[10];    //[0,255]
    AK_U16 u16CoefC[10];    //[0,1023]
} ISP_LENS_COEF_S;

typedef struct akISP_LSC_ATTR_S
{
    AK_U8               u16LscMode;
    AK_U8               u16LinkageStrength[13];  //[0,1023]
    AK_U16		        u16LscEnable;
    //the reference point of lens correction
    AK_U16		        u16Xref;      //[0,4096]
    AK_U16		        u16Yref;      //[0,4096]
    AK_U16		        u16Lsc_shift;   //[0，15]
    ISP_LENS_COEF_S	stLscRCoef;
    ISP_LENS_COEF_S	stLscGrCoef;
    ISP_LENS_COEF_S	stLscGbCoef;
    ISP_LENS_COEF_S	stLscBCoef;
    //the range of ten segment
    AK_U16		        u16Range[10];   //[0，1023]
} ISP_LSC_ATTR_S;

typedef struct akISP_EX_ZWEIGHT_ATTR_S {
    AK_U8	u8WinWeight[8][16];
    AK_U8	u8ExZoneWeightStr;
} ISP_EX_ZWEIGHT_ATTR_S;

typedef struct akISP_EFFECT_ATTR_S
{
    AK_U8   u8YuvOutEnable;
    AK_U8   u8DarkMarginEn;    // Dark margin enable
    AK_U8   u8YA;      // [0, 255]
    AK_S8   s8YB;     //[-128, 127]
    AK_S16  s16UvA;    //[-256, 255]
    AK_S16  s16UvB;    //[-256, 255]
} ISP_EFFECT_ATTR_S;

typedef struct akISP_RGB_GAMMA_S {
    AK_U16  u16RgbGammaEnable;
    AK_U16  u16RGamma[129];   //10bit
    AK_U16  u16GGamma[129];   //10bit
    AK_U16  u16BGamma [129];  //10bit
    AK_U16  u16RKey[16];
    AK_U16  u16GKey[16];
    AK_U16  u16BKey[16];
} ISP_RGB_GAMMA_S;

typedef struct akISP_RGB_GAMMA_ATTR_S {
    AK_U16	  u16GainTh1;
    AK_U16	  u16GainTh2;
    ISP_RGB_GAMMA_S  stLinkageRgbGamma[3];
} ISP_RGB_GAMMA_ATTR_S;

typedef struct  akISP_CONTRAST_S
{
    AK_U8   u8ContrastEnable;
    AK_U16  u16YContrast;  //[0,511]
    AK_S16  s16YShift;    //[0, 511]
} ISP_CONTRAST_S;

typedef struct  akISP_AUTO_CONTRAST_S
{
    AK_U8   u8ContrastEnable;
    AK_U8   u8ShiftMax;    //[0, 127]
    AK_U16  u16DarkPixelArea; //[0, 511]
    AK_U16   u8DarkPixelRate;  //[1, 256]
} ISP_AUTO_CONTRAST_S;

typedef struct  akISP_CONTRAST_ATTR_S
{
    ISP_OP_TYPE_E			enContrastMode;	// Mode: manual or linkage
    ISP_CONTRAST_S	        stManualContrast;
    ISP_AUTO_CONTRAST_S	stLinkageContrast[12];
} ISP_CONTRAST_ATTR_S;

typedef struct akISP_LCE_S {
    AK_U8	u8LceEnable;
    AK_U8	u8LceUvAdjustEn;
    AK_U8   u8LceUvAdjustLevel;
    AK_U8	u8LceHeStr;
    AK_U8 	u8LceHeStrLow;
    AK_U8   u8LceLumFilterK;//[0,8]
    AK_U8   u8LceSpatFilterK;//[0,8]
    AK_U8   u8LceTempFilterK;//[0,15]
    AK_U8   u8LceStrength[4][8];
} ISP_LCE_S;

typedef struct akISP_LCE_ATTR_S{
    ISP_OP_TYPE_E  enLceMode;
    ISP_LCE_S  	stManualLce;
    ISP_LCE_S  	stLinkageLce[12];
} ISP_LCE_ATTR_S;

typedef struct akISP_Y_GAMMA_ATTR_S
{
    AK_U16  u16YgammaEnable;
    AK_U8   u8YgammaUvAdjustEnable;
    AK_U8   u8YgammaUvAdjustLevel;
    AK_U16  u16Ygamma[129];    //10bit
    AK_U16  u16YgammaKey[16]; // Curve key points
    AK_U16  u16YgammaCnoiseYth1;   // Y gamma chroma noise threshold 1
    AK_U16  u16YgammaCnoiseYth2 ;  // Y gamma chroma noise threshold 2
    AK_U16  u16YgammaCnoiseGain ;	// UV adjustment coefficient
} ISP_Y_GAMMA_ATTR_S;

typedef struct akISP_IR_LED_NIGHT_PARAM_S {
    AK_U32    u32DayToNightLum;	// day to night lum value
    AK_U32    u32NightToDayLum;	// night to day lum value
    AK_U32    u32AwbNightDisMin;
    AK_U32    u32AwbNightDisMax;
    AK_U32    u32GainR;
    AK_S32    s32OffsetR;
    AK_U32    u32GainB;
    AK_S32    s32OffsetB;
    AK_U32    u32GainRB;
    AK_S32    s32OffsetRB;
    AK_U32    u32BlkCheckFlag;
    AK_U32    u32LowLumiTh;
    AK_U32    u32HighLumiTh;
    AK_U32    u32NonInfraredRatioTh;
    AK_U32    u32RevisedDisMaxRatio;
} ISP_IR_LED_NIGHT_PARAM_S;

typedef struct akISP_WHITE_LED_NIGHT_PARAM_S {
    AK_U32    u32DayToNightLum;	// day to night lum value
    AK_U32    u32NightToDayLum;	// night to day lum value
    AK_U32    u32HighlightLumiTh;
    AK_U32    u32HighlightBlkCntMax;
} ISP_WHITE_LED_NIGHT_PARAM_S;

typedef struct akISP_BLACKLIGHT_NIGHT_PARAM_S {
    AK_U32  u32DayToNightAGain;
    AK_U32  u32DayToNightDGain;
    AK_U32  u32DayToNightIspDGain;
    AK_U32  u32DayToNightExpTime;
    AK_U32  u32DayToNightDarkPixelRate;
    AK_U32  u32NightToDayAGain;
    AK_U32  u32NightToDayDGain;
    AK_U32  u32NightToDayIspDGain;
    AK_U32  u32NightToDayExpTime;
    AK_U32  u32NightToDayDarkPixelRate;
} ISP_BLACKLIGHT_NIGHT_PARAM_S;


typedef struct akISP_SOFT_PS_ATTR_S {
    ISP_NIGHT_TYPE_E    enNightType;
    AK_U32              u32DayCheckFrameNum;
    AK_U32              u32NightCheckFrameNum;
    AK_U32              u32ExpStableCheckFrameNum;
    AK_S32              s32ExpStableTimeOutMs;
    AK_U32              u32LockCntTh;
    AK_U32              u32LockTimeMs;

    union
    {
        ISP_IR_LED_NIGHT_PARAM_S        stIrLedNightParm;
        ISP_WHITE_LED_NIGHT_PARAM_S     stWhiteLedNightParm;
        ISP_BLACKLIGHT_NIGHT_PARAM_S    stBlackLightNightParm;
    };
} ISP_SOFT_PS_ATTR_S;

typedef struct akISP_NON_INFRARED_RATIO_ATTR_S {
    AK_U32    u32AwbNightDisMax;
    AK_U32    u32GainR;
    AK_S32    s32OffsetR;
    AK_U32    u32GainB;
    AK_S32    s32OffsetB;
    AK_U32    u32GainRB;
    AK_S32    s32OffsetRB;
    AK_U32    u32LowLumiTh;
    AK_U32    u32HighLumiTh;
    AK_U32    u32RevisedDisMaxRatio;
}ISP_NON_INFRARED_RATIO_ATTR_S;

typedef struct akISP_3D_NR_REF_SIZE_STAT_INFO{
    AK_U32  u32RefSizeStatis[3];/* 0-y used size; 1-u used size; 2-v used size*/
}ISP_3D_NR_REF_SIZE_STAT_INFO;



typedef enum akPP_MASK_TYPE_E {
    PP_MASK_ORIGINAL_COLOR_RECTANGLE = 0,	/* masked with original color rectangle */
    PP_MASK_ORIGINAL_COLOR_TRIANGLE,	    /* masked with original color triangle or diamond*/
    PP_MASK_MOSAIC_VIDEO_RECTANGLE,         /* masked with mosaic video data */
} PP_MASK_TYPE_E;

typedef enum akPP_SHAPE_TYPE_E {
    PP_SHAPE_TYPE_RECTANGLE_HOLLOW = 0,	    /* rectangle hollow */
    PP_SHAPE_TYPE_RECTANGLE_SOLID,	        /* rectangle solid */
    PP_SHAPE_TYPE_TRIANGLE_HOLLOW,			/* triangle or diamond hollow */
    PP_SHAPE_TYPE_TRIANGLE_SOLID,			/* triangle or diamond solid */
} PP_SHAPE_TYPE_E;



typedef struct akPP_MASK_MASIC_ATTR_S{
    AK_U16	u16MosaiSizeHor;
    AK_U16	u16MosaiSizeVec;
} PP_MASK_MASIC_ATTR_S;

typedef struct akPP_MASK_COLOR_INFO_S {
    /* range [0, 0xff]. if 0xff mask video opaquely, then can not see image */
    AK_U8 u8Alpha;

    /* range [0, 0xff] */
    AK_U8 u8RColor;
    /* range [0, 0xff] */
    AK_U8 u8GColor;
    /* range [0, 0xff] */
    AK_U8 u8BColor;

    PP_MASK_MASIC_ATTR_S stMasicAttr;
} PP_MASK_COLOR_INFO_S;

typedef struct akPP_RECTANGLE_AREA_S {
    AK_U16 u16X;
    AK_U16 u16Y;
    AK_U16 u16Width;
    AK_U16 u16Height;
} PP_RECTANGLE_AREA_S;

typedef struct akPP_TRIANGLE_AREA_S {
    AK_U16 u16X;
    AK_U16 u16Y;
    AK_U8  u8HalfWidth;
    AK_U8  u8Bitmap;
} PP_TRIANGLE_AREA_S;



typedef struct akPP_MASK_AREA_S {
    AK_U8                   u8Enable;
    PP_MASK_TYPE_E          enType; //0 color rectangle; 1 color triangle or diamond; 2 mosaic rectangle
    union
    {
        PP_RECTANGLE_AREA_S stRectangleArea;
        PP_TRIANGLE_AREA_S  stTriangleArea;
    };
} PP_MASK_AREA_S;

typedef struct akPP_MASK_AREA_INFO_S {
    PP_MASK_AREA_S stArea[PP_MASK_AREA_MAX];
    AK_U8          u8SyncToSubChn;
} PP_MASK_AREA_INFO_S;

typedef struct akPP_BOX_S {
    AK_U8                   u8Enable;       //[0-1]
    /*0 hollow rectangle; 1 solid rectangle; 2 hollow diamond or triangle line; 3 solid diamond or triangle*/
    PP_SHAPE_TYPE_E         enType;
    AK_U16                  u16ColorId;    //[0-6]
    AK_U16                  u16LineWidth;  //[1-16]

    // -1 draw forever ; 0 not draw; >0 draw frame num
    AK_S32                  s32DrawFrameNum;

    union
    {
        PP_RECTANGLE_AREA_S stRectangleArea;
        PP_TRIANGLE_AREA_S  stTriangleArea;
    };
} PP_BOX_S;

typedef struct akPP_BOX_GROUP_S {
    PP_BOX_S stBox[BOX_MAX_NUM];
} PP_BOX_GROUP_S;

typedef struct akISP_ROI_INFO_S
{
    /** left_top point, horizontal axis */
    AK_S32 left;
    /** left_top point, vertical axis */
    AK_S32 top;
    /** right_bottom point, horizontal axis */
    AK_S32 right;
    /** right_bottom point, vertical axis */
    AK_S32 bottom;
} ISP_ROI_INFO_S;

/**
 * AK_MPI_ISP_GetIspSdkVersion - get isp sdk version
 * return: version string
 * notes:
 */
const char* AK_MPI_ISP_GetIspSdkVersion(AK_VOID);


/********************** md *******************************/

/**
 * AK_MPI_ISP_GetMdStat: get motion detection stat params
 * @s32GrpId[IN]: grp id
 * @pstMd[OUT]: md params
 * return: 0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetMdStat(AK_S32 s32GrpId, ISP_MD_INFO_S *pstMd);

/********************** od *******************************/

/**
 * AK_MPI_ISP_GetOdInfo : get occlusion detection params
 * @s32GrpId[IN]: grp id
 * @pstOd[OUT]: od params
 * return: 0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetOdInfo(AK_S32 s32GrpId, ISP_OD_INFO_S *pstOd);


/********************** af *******************************/

/**
 * AK_MPI_ISP_GetAfStat: get af stat info
 * @s32GrpId[IN]: grp id
 * @pstAfStat[OUT]: af_stat info
 * return: 0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetAfStat(AK_S32 s32GrpId, ISP_AF_INFO_S *pstAfStat);

/**
 * AK_MPI_ISP_SetAfAttr: set af attr
 * @s32GrpId[IN]: grp id
 * @pstAfAttr[IN]: af_attr
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_SetAfAttr(AK_S32 s32GrpId, ISP_AF_ATTR_S *pstAfAttr);

/**
 * AK_MPI_ISP_GetAfAttr: get af attr
 * @s32GrpId[IN]: grp id
 * @pstAfAttr[OUT]: af_attr
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_GetAfAttr(AK_S32 s32GrpId, ISP_AF_ATTR_S *pstAfAttr);


/********************** effect *******************************/

/**
 * AK_MPI_ISP_GetEffect - get isp effect param.
 * @s32GrpId[IN]: grp id
 * @enType[IN]:   effect type name
 * @ps32Value[OUT]: effect value
 * return: 0 - success; otherwise error code
 */
AK_S32 AK_MPI_ISP_GetEffect(AK_S32 s32GrpId, ISP_EFFECT_TYPE_E enType, AK_S32 *ps32Value);

/**
 * AK_MPI_ISP_SetEffect - set isp effect param.
 * @s32GrpId[IN]: grp id
 * @enType[IN]:   effect type name
 * @s32Value[IN]: effect value
 * return: 0 - success; otherwise error code
 */
AK_S32 AK_MPI_ISP_SetEffect(AK_S32 s32GrpId, ISP_EFFECT_TYPE_E enType, AK_S32 s32Value);


/********************** fps ctrl *******************************/

/**
 * AK_MPI_ISP_GetFpsCtrlStat: get fps ctrl stat
 * @s32GrpId[IN]: grp id
 * @penStat[OUT]:gain stat
 * @ps32NeedFps[OUT]:need fps
 * return: 0 success, otherwise error code
 * notes:for switch day high light and low light. use with AK_MPI_ISP_ChangeSensorFps
 */
AK_S32 AK_MPI_ISP_GetFpsCtrlStat(AK_S32 s32GrpId, ISP_GAIN_STAT_E *penStat, AK_S32 *ps32NeedFps);

/**
 * AK_MPI_ISP_ChangeSensorFps: change sensor fps
 * @s32GrpId[IN]: grp id
 * @s32NeedFps[IN]:need fps out by AK_MPI_ISP_GetFpsCtrlStat
 * return:  0 success, otherwise error code
 * notes:for switch day high light and low light. use with AK_MPI_ISP_GetFpsCtrlStat
 */
AK_S32 AK_MPI_ISP_ChangeSensorFps(AK_S32 s32GrpId, AK_S32 s32NeedFps);

/**
 * AK_MPI_ISP_SetFpsLevel: set sensor fps level
 * @s32GrpId[IN]: grp id
 * @pstFpsLevel[IN]:fps_level params
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_SetFpsLevel(AK_S32 s32GrpId, ISP_FPS_LEVEL_S *pstFpsLevel);

/**
 * AK_MPI_ISP_GetFpsLevel: get sensor fps level
 * @s32GrpId[IN]: grp id
 * @pstFpsLevel[OUT]:fps_level params
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetFpsLevel(AK_S32 s32GrpId, ISP_FPS_LEVEL_S *pstFpsLevel);

/********************** ae *******************************/

/**
 * AK_MPI_ISP_GetAeRunInfo: get ae run info
 * @s32GrpId[IN]: grp id
 * @pstAeRunInfo[OUT]: ae run info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAeRunInfo(AK_S32 s32GrpId, ISP_AE_RUN_INFO_S *pstAeRunInfo);


/**
 * AK_MPI_ISP_GetAeAttr: get AE attr
 * @s32GrpId[IN]: grp id
 * @pstAeAttr[OUT]: AE attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAeAttr(AK_S32 s32GrpId, ISP_AE_ATTR_S *pstAeAttr);

/**
 * AK_MPI_ISP_SetAeAttr: set AE attr
 * @s32GrpId[IN]: grp id
 * @pstAeAttr[IN]: AE attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAeAttr(AK_S32 s32GrpId, ISP_AE_ATTR_S *pstAeAttr);

/**
 * AK_MPI_ISP_SetAeConvergenceRate: set AE convergence rate
 * @s32GrpId[IN]: grp id
 * @u32Value[IN]: param of convergence rate
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAeConvergenceRate(AK_S32 s32GrpId, AK_U32 u32Value);

/**
 * AK_MPI_ISP_GetAeConvergenceRate: get AE convergence rate
 * @s32GrpId[IN]: grp id
 * @pu32Value[OUT]: param of convergence rate
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAeConvergenceRate(AK_S32 s32GrpId, AK_U32 *pu32Value);

/**
 * AK_MPI_ISP_CheckAeStable: check ae is stable or not
 * @s32GrpId[IN]: grp id
 * @pu32Stable[OUT]: 1 stable, 0 not stable
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_CheckAeStable(AK_S32 s32GrpId, AK_U32 *pu32Stable);

/*
 * AK_MPI_ISP_SetAeSuspend - set ae suspend flag
 * @s32GrpId[IN]: grp id
 * @u32AeSuspendFlag[IN]: 1 suspend, 0 resume
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAeSuspend(AK_S32 s32GrpId,  AK_U32 u32AeSuspendFlag);

/********************** me *******************************/

/**
 * AK_MPI_ISP_GetMeAttr: get ME attr
 * @s32GrpId[IN]: grp id
 * @pstMeAttr[OUT]: ME attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetMeAttr(AK_S32 s32GrpId, ISP_ME_ATTR_S *pstMeAttr);

/**
 * AK_MPI_ISP_SetMeAttr: set ME attr
 * @s32GrpId[IN]: grp id
 * @pstMeAttr[IN]: ME attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetMeAttr(AK_S32 s32GrpId, ISP_ME_ATTR_S *pstMeAttr);

/**
 * AK_MPI_ISP_GetExpType: get exp type
 * @s32GrpId[IN]: grp id
 * @penExpType[OUT]: exp type
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetExpType(AK_S32 s32GrpId, ISP_OP_TYPE_E *penExpType);

/**
 * AK_MPI_ISP_SetExpType: set exp type
 * @s32GrpId[IN]: grp id
 * @enExpType[IN]: exp type
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetExpType(AK_S32 s32GrpId, ISP_OP_TYPE_E enExpType);


/**
 * AK_MPI_ISP_GetCurLumi: get current lum factor
 * @s32GrpId[IN]: grp id
 * @pu32Lumi[OUT]: lum factor
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_GetCurLumi(AK_S32 s32GrpId, AK_U32 *pu32Lumi);

/********************** anti flicker *******************************/

/**
 * AK_MPI_ISP_SetForceAntiFlickerFlag: set force anti flicker flag
 * @s32GrpId[IN]: grp id
 * @s32ForceFlag[IN]: force anti flicker flag
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetForceAntiFlickerFlag(AK_S32 s32GrpId, AK_S32 s32ForceFlag);

/**
 * AK_MPI_ISP_GetForceAntiFlickerFlag: set force anti flicker flag
 * @s32GrpId[IN]: grp id
 * @s32ForceFlag[OUT]: force anti flicker flag
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetForceAntiFlickerFlag(AK_S32 s32GrpId, AK_S32 *s32ForceFlag);

/**
 * AK_MPI_ISP_SetAntiFlickerStrength: set anti flicker strength
 * @s32GrpId[IN]: grp id
 * @u32Strength[IN]: anti flicker strength [0-100], 0 means disable anti flicker
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAntiFlickerStrength(AK_S32 s32GrpId, AK_U32 u32Strength);


/**
 * AK_MPI_ISP_GetAntiFlickerStrength: get anti flicker strength
 * @s32GrpId[IN]: grp id
 * @pu32Strength[OUT]: anti flicker strength
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAntiFlickerStrength(AK_S32 s32GrpId, AK_U32 *pu32Strength);

/********************** hdr *******************************/

/**
 * AK_MPI_ISP_GetHdrAttr: get hdr attr
 * @s32GrpId[IN]: grp id
 * @pstHdr[OUT]: hdr attr
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_GetHdrAttr(AK_S32 s32GrpId, ISP_HDR_ATTR_S *pstHdr);

/**
 * AK_MPI_ISP_SetHdrAttr: get hdr attr
 * @s32GrpId[IN]: grp id
 * @pstHdr[IN]: hdr attr
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_SetHdrAttr(AK_S32 s32GrpId, ISP_HDR_ATTR_S *pstHdr);


/**
 * AK_MPI_ISP_GetRgbAverage: get rgb average value
 * @s32GrpId[IN]: grp id
 * @u32RAvr[OUT]: r average value
 * @u32GAvr[OUT]: g average value
 * @u32BAvr[OUT]: b average value
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetRgbAverage(AK_S32 s32GrpId,
        AK_U32 *u32RAvr, AK_U32 *u32GAvr, AK_U32 *u32BAvr);

/********************** 3dnr *******************************/

/**
 * AK_MPI_ISP_Get3dnrAttr: get 3D NR attr
 * @s32GrpId[IN]: grp id
 * @pst3dnrAttr[OUT]: 3D NR attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_Get3dnrAttr(AK_S32 s32GrpId, ISP_3DNR_ATTR_S *pst3dnrAttr);

/**
 * AK_MPI_ISP_Set3dnrAttr: set 3D NR attr
 * @s32GrpId[IN]: grp id
 * @pst3dnrAttr[IN]: 3D NR attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_Set3dnrAttr(AK_S32 s32GrpId, ISP_3DNR_ATTR_S *pst3dnrAttr);

/********************** wb *******************************/

/**
 * AK_MPI_ISP_GetWbType: get wb type
 * @s32GrpId[IN]: grp id
 * @penWbType[OUT]: wb type
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetWbType(AK_S32 s32GrpId, ISP_OP_TYPE_E *penWbType);

/**
 * AK_MPI_ISP_SetWbType: set wb type
 * @s32GrpId[IN]: grp id
 * @enWbType[IN]: wb type
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetWbType(AK_S32 s32GrpId, ISP_OP_TYPE_E enWbType);


/**
 * AK_MPI_ISP_GetMwbAttr: get mwb attr
 * @s32GrpId[IN]: grp id
 * @pstMwbAttr[OUT]: mwb attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetMwbAttr(AK_S32 s32GrpId, ISP_MWB_ATTR_S *pstMwbAttr);

/**
 * AK_MPI_ISP_SetMwbAttr: set mwb attr
 * @s32GrpId[IN]: grp id
 * @pstMwbAttr[IN]: mwb attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetMwbAttr(AK_S32 s32GrpId, ISP_MWB_ATTR_S *pstMwbAttr);

/**
 * AK_MPI_ISP_GetAwbAttr: get awb attr
 * @s32GrpId[IN]: grp id
 * @pstAwbAttr[OUT]: awb attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAwbAttr(AK_S32 s32GrpId, ISP_AWB_ATTR_S *pstAwbAttr);

/**
 * AK_MPI_ISP_SetAwbAttr: set awb attr
 * @s32GrpId[IN]: grp id
 * @pstAwbAttr[IN]: awb attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAwbAttr(AK_S32 s32GrpId, ISP_AWB_ATTR_S *pstAwbAttr);

/**
 * AK_MPI_ISP_GetAwbExAttr: get awb ex attr
 * @s32GrpId[IN]: grp id
 * @pstAwbExAttr[OUT]: awb ex attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAwbExAttr(AK_S32 s32GrpId, ISP_AWB_EX_ATTR *pstAwbExAttr);

/**
 * AK_MPI_ISP_SetAwbExAttr: set awb ex attr
 * @s32GrpId[IN]: grp id
 * @pstAwbExAttr[IN]: awb ex attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetAwbExAttr(AK_S32 s32GrpId, ISP_AWB_EX_ATTR *pstAwbExAttr);

/**
 * AK_MPI_ISP_GetAwbStatInfo: get awb stat info
 * @s32GrpId[IN]: grp id
 * @pstAwbStatInfo[OUT]: awb stat info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetAwbStatInfo(AK_S32 s32GrpId, ISP_AWB_STAT_INFO_S *pstAwbStatInfo);


/**
 * AK_MPI_ISP_GetSensorReg: get sensor register info
 * @s32GrpId[IN]: grp id
 * @pstSensorRegInfo[IN/OUT]: sensor register info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetSensorReg(AK_S32 s32GrpId, ISP_SENSOR_REG_INFO_S *pstSensorRegInfo);

/**
 * AK_MPI_ISP_GetLscAttr: get lsc attr
 * @s32GrpId[IN]: grp id
 * @pstLsc[OUT]: lsc attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetLscAttr(AK_S32 s32GrpId, ISP_LSC_ATTR_S *pstLsc);

/**
 * AK_MPI_ISP_SetLscAttr: set lsc attr
 * @s32GrpId[IN]: grp id
 * @pstLsc[IN]: lsc attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetLscAttr(AK_S32 s32GrpId, ISP_LSC_ATTR_S *pstLsc);

/**
 * AK_MPI_ISP_GetExZweightAttr: get ex zweight attr
 * @s32GrpId[IN]: grp id
 * @pstExZweight[OUT]: ex zweight attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetExZweightAttr(AK_S32 s32GrpId, ISP_EX_ZWEIGHT_ATTR_S *pstExZweight);

/**
 * AK_MPI_ISP_SetExZweightAttr: set ex zweight attr
 * @s32GrpId[IN]: grp id
 * @pstExZweight[IN]: ex zweight attr info
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetExZweightAttr(AK_S32 s32GrpId, ISP_EX_ZWEIGHT_ATTR_S *pstExZweight);

/**
 * AK_MPI_ISP_SetYuvEffectAttr: set yuv effect attr
 * @s32GrpId[IN]: grp id
 * @pstYuvEffect[IN]: yuv_effect attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetYuvEffectAttr(AK_S32 s32GrpId, ISP_EFFECT_ATTR_S *pstYuvEffect);

/**
 * AK_MPI_ISP_GetYuvEffectAttr: get yuv effect attr
 * @s32GrpId[IN]: grp id
 * @pstYuvEffect[OUT]: yuv_effect attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetYuvEffectAttr(AK_S32 s32GrpId, ISP_EFFECT_ATTR_S *pstYuvEffect);

/**
 * AK_MPI_ISP_SetRgbGammaAttr: set rgb gamma attr
 * @s32GrpId[IN]: grp id
 * @pstRgbGamma[IN]: rgb_gamma attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetRgbGammaAttr(AK_S32 s32GrpId, ISP_RGB_GAMMA_ATTR_S *pstRgbGamma);

/**
 * AK_MPI_ISP_GetRgbGammaAttr: get rgb gamma attr
 * @s32GrpId[IN]: grp id
 * @pstRgbGamma[OUT]: rgb_gamma attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetRgbGammaAttr(AK_S32 s32GrpId, ISP_RGB_GAMMA_ATTR_S *pstRgbGamma);

/**
 * AK_MPI_ISP_SetContrastAttr: set contrast attr
 * @s32GrpId[IN]: grp id
 * @pstContrast[IN]: contrast attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetContrastAttr(AK_S32 s32GrpId, ISP_CONTRAST_ATTR_S *pstContrast);

/**
 * AK_MPI_ISP_GetContrastAttr: get contrast attr
 * @s32GrpId[IN]: grp id
 * @pstContrast[OUT]: contrast attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetContrastAttr(AK_S32 s32GrpId, ISP_CONTRAST_ATTR_S *pstContrast);

/**
 * AK_MPI_ISP_SetLceAttr: set lce attr
 * @s32GrpId[IN]: grp id
 * @pstLce[IN]: lce attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetLceAttr(AK_S32 s32GrpId, ISP_LCE_ATTR_S *pstLce);

/**
 * AK_MPI_ISP_GetLceAttr: get lce attr
 * @s32GrpId[IN]: grp id
 * @pstLce[OUT]: lce attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetLceAttr(AK_S32 s32GrpId, ISP_LCE_ATTR_S *pstLce);

/**
 * AK_MPI_ISP_SetYgammaAttr: set y_gamma attr
 * @s32GrpId[IN]: grp id
 * @pstYgamma[IN]: y_gamma attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetYgammaAttr(AK_S32 s32GrpId, ISP_Y_GAMMA_ATTR_S *pstYgamma);

/**
 * AK_MPI_ISP_GetYgammaAttr: set y_gamma attr
 * @s32GrpId[IN]: grp id
 * @pstYgamma[OUT]: y_gamma attr
 * return: 0 success, otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetYgammaAttr(AK_S32 s32GrpId, ISP_Y_GAMMA_ATTR_S *pstYgamma);


/**
 * AK_MPI_ISP_SwitchFlipMirror : set flip mirror compare to default value
 * @s32GrpId[IN]: grp id
 * @u32FlipEn[IN]:flip enable or not , 0 disable (default value), 1 enable
 * @u32MirrorEn[IN]:mirror enable or not , 0 disable (default value), 1 enable
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_SwitchFlipMirror(AK_S32 s32GrpId, AK_U32 u32FlipEn, AK_U32 u32MirrorEn);

/**
 * AK_MPI_ISP_GetFlipMirror : get flip mirror compare to default value
 * @s32GrpId[IN]: grp id
 * @pu32FlipEn[OUT]:flip enable or not , 0 disable (equal to default value), \
 * 1 enable (not equal to default value)
 * @pu32MirrorEn[OUT]:mirror enable or not , 0 disable (equal to default value), \
 * 1 enable (not equal to default value)
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetFlipMirror(AK_S32 s32GrpId, AK_U32 *pu32FlipEn, AK_U32 *pu32MirrorEn);

/**
 * AK_MPI_ISP_SetSoftPsParam: set soft ps switch threshold
 * @s32GrpId[IN]: grp id
 * @pstParam[IN]: input param threshold
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_SetSoftPsParam(AK_S32 s32GrpId, ISP_SOFT_PS_ATTR_S *pstParam);


/**
 * AK_MPI_ISP_GetSoftPsLevel: get day or night
 * @s32GrpId[IN]: grp id
 * @s32PreIrLevel[IN]: pre status, 0 day, 1 night
 * @ps32NewIrLevel[OUT]: new status, 0 day, 1 night
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetSoftPsLevel(AK_S32 s32GrpId, AK_S32 s32PreIrLevel, AK_S32 *ps32NewIrLevel);

/**
 * AK_MPI_ISP_WaitExpStable
 * @s32GrpId[IN]: grp id
 * @u32ExpStableCheckFrameNum[IN]: u32ExpStableCheckFrameNum
 * @s32TimeOutMs[IN]: timeout ,-1 block; 0, no block; >0 timeout in ms
 * return: 0 - success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_WaitExpStable(AK_S32 s32GrpId, AK_U32 u32ExpStableCheckFrameNum, AK_S32 s32TimeOutMs);

/**
 * AK_MPI_ISP_GetEv: get ev
 * @s32GrpId[IN]: grp id
 * @pu32Ev[OUT]: ev value
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetEv(AK_S32 s32GrpId, AK_U32 *pu32Ev);

/**
 * AK_MPI_ISP_GetRgbDis: get rgb dis
 * @s32GrpId[IN]: grp id
 * @u32GainR[IN]: gain_r
 * @s32OffsetR[IN]: offset_r
 * @u32GainB[IN]: gain_b
 * @s32OffsetB[IN]: offset_b
 * @pu32Dis[OUT]: dis value
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetRgbDis(AK_S32 s32GrpId, AK_U32 u32GainR, AK_S32 s32OffsetR,
        AK_U32 u32GainB, AK_S32 s32OffsetB, AK_U32 *pu32Dis);

/**
 * AK_MPI_ISP_GetBlkLumi: get block lumi
 * @s32GrpId[IN]: s32GrpId
 * @blk_lumi[OUT]: block lumi[16][16]
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetBlkLumi(AK_S32 s32GrpId, AK_U32 au32BlkLumi[16][16]);

/**
 * AK_MPI_ISP_GetNonInfraredRatio: get non infrared ratio
 * @s32GrpId[IN]: grp id
 * @pstNirAttr[IN]:non_infrared_ratio_attr
 * @pU32NonInfraredRatio[OUT]: non_infrared_ratio
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_GetNonInfraredRatio(AK_S32 s32GrpId,
    ISP_NON_INFRARED_RATIO_ATTR_S *pstNirAttr, AK_U32 *pu32NonInfraredRatio);


/**
 * brief: switch day night mode
 * @s32GrpId[IN]: grp id
 * @enMode[IN]:day or night
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_SwitchMode(AK_S32 s32GrpId, ISP_DAY_NIGHT_MODE_E enMode);

/**
 * AK_MPI_ISP_Get3dnrRefSizeStat: get  3dnr buf used size
 * @s32GrpId[IN]: grp id
 * @pstRefSize[OUT]: 3dnr buf y/u/v used size
 * return: 0 - success; otherwise error code;
 */
AK_S32 AK_MPI_ISP_Get3dnrRefSizeStat(AK_S32 s32GrpId, ISP_3D_NR_REF_SIZE_STAT_INFO *pstRefSize);

/**
 * AK_MPI_PP_MaskSetArea : set main & sub channel mask area
 * @s32GrpId[IN]: grp id
 * @s32ChnId[IN]: s32ChnId
 * @pstArea[IN]: main channel mask area paramters
 * return: 0 success, other error code
 * notes:
 */
AK_S32 AK_MPI_PP_MaskSetArea(AK_S32 s32GrpId, AK_S32 s32ChnId, const PP_MASK_AREA_INFO_S *pstArea);

/**
 * brief: get main & sub channel mask area
 * @s32GrpId[IN]: grp id
 * @s32ChnId[IN]: s32ChnId
 * @pstArea[OUT]: main channel mask area paramters
 * return: 0 success, other error code
 * notes:
 */
AK_S32 AK_MPI_PP_MaskGetArea(AK_S32 s32GrpId, AK_S32 s32ChnId, PP_MASK_AREA_INFO_S *pstArea);

/**
 * AK_MPI_PP_MaskSetColor : set main & sub channel mask color
 * @s32GrpId[IN]: grp id
 * @pstColor[IN]: main & sub channel mask color paramters
 * return: 0 success, other error code
 * notes:
 */
AK_S32 AK_MPI_PP_MaskSetColor(AK_S32 s32GrpId, const PP_MASK_COLOR_INFO_S *pstColor);

/**
 * AK_MPI_PP_MaskGetColor : get main & sub channel mask color
 * @s32GrpId[IN]: grp id
 * @pstColor[OUT]: main & sub channel mask color paramters
 * return: 0 success, other error code
 * notes:
 */
AK_S32 AK_MPI_PP_MaskGetColor(AK_S32 s32GrpId, PP_MASK_COLOR_INFO_S *pstColor);

/**
 * AK_MPI_PP_SetBoxColorTable : set box color table
 * @s32GrpId[IN]: grp id
 * @pu32ColorTable[IN]:color table , format : ARGB (32bit). size : 7.
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_PP_SetBoxColorTable(AK_S32 s32GrpId, AK_U32 *pu32ColorTable);

/**
 * AK_MPI_PP_DrawBox : draw box
 * @s32GrpId[IN]: s32GrpId
 * @s32ChnId[IN]: s32ChnId
 * @pstBoxGroup[IN]:box group attr
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_PP_DrawBox(AK_S32 s32GrpId, AK_S32 s32ChnId, PP_BOX_GROUP_S *pstBoxGroup);

/**
 * AK_MPI_ISP_LightEstimateAe : Adjust the exposure manually to reach the target brightness,using the
 * ROI brightness as reference.
 * @pu8FrameData[IN]: YUV frame data input.
 * @u32PixLine[IN]: The pixel line of the YUV frame.
 * @u64SeqNo[IN]: Frame sequce number.
 * @pstRoiInfo[IN]: The ROI erae
 * @s32TargetLumiAtBackLight[IN]: The target brightness of backlight sence.
 * @s32TargetLumiAtNormalLight[IN]: The target brightness of normall sence.
 * @s32Delta[IN]:The difference between the target brightness and the actual brightness.
 * Exposure adjustment will only occur when the difference between the current brightness and
 * the target brightness meets this value.
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_LightEstimateAe(AK_S32 s32GrpId, AK_U8 *pu8FrameData, AK_U32 u32PixLine, AK_U64 u64SeqNo,
    ISP_ROI_INFO_S *pstRoiInfo, AK_S32 s32TargetLumiAtBackLight, AK_S32 s32TargetLumiAtNormalLight, AK_S32 s32Delta);

/**
 * AK_MPI_ISP_LightEstimateAe : Adjust the exposure manually to reach the target brightness,using the
 * ROI brightness as reference.
 * @pu8FrameData[IN]: YUV frame data input.
 * @u32PixLine[IN]: The pixel line of the YUV frame.
 * @u64SeqNo[IN]: Frame sequce number.
 * @pstRoiInfo[IN]: The ROI erae
 * @s32TargetLumiAtBackLight[IN]: The target brightness of backlight sence.
 * @s32TargetLumiAtNormalLight[IN]: The target brightness of normall sence.
 * @s32Delta[IN]:The difference between the target brightness and the actual brightness.
 * Exposure adjustment will only occur when the difference between the current brightness and
 * the target brightness meets this value.
 * return: 0 success, otherwise error code;
 * notes:
 */
AK_S32 AK_MPI_ISP_WDR_LightEstimateAe(AK_S32 s32GrpId, AK_U8 *pu8FrameData, AK_U32 u32PixLine, AK_U64 u64SeqNo,
    ISP_ROI_INFO_S *pstRoiInfo, AK_S32 s32TargetLumiAtBackLight, AK_S32 s32TargetLumiAtNormalLight, AK_S32 s32Delta);
/**
 * AK_MPI_ISP_SetSensorFps: set sensor fps
 * @s32GrpId[IN]: grp id
 * @s32Fps[IN]:fps to set
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_SetSensorFps(AK_S32 s32GrpId, AK_S32 s32Fps);

/**
 * AK_MPI_ISP_GetSensorFps: get sensor fps
 * @s32GrpId[IN]: grp id
 * @ps32Fps[OUT]:fps
 * return:  0 success, otherwise error code
 * notes:
 */
AK_S32 AK_MPI_ISP_GetSensorFps(AK_S32 s32GrpId, AK_S32 *ps32Fps);

#endif

/* end of file */
