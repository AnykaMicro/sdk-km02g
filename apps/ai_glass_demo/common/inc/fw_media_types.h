/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _FW_TYPES_H_
#define _FW_TYPES_H_

#if !defined(WIN32)
	#define __PACKED__        __attribute__ ((__packed__))
#else
	#define __PACKED__
#endif

#define MAX_CHN 3               /**< Number of channels */
#define MAX_RECT_BOX_NUM 10     /**< Max number of draw boxes */

#define MAX_CROP_JPG_SAVE_HEIGHT 	224	/**< jpg crop save height */
#define MAX_CROP_JPG_SAVE_WIDTH 	224	/**< jpg crop save width */

/**
 * @brief Framework error types.
 */
typedef enum
{
	FW_ERROR_CHANNEL_NOEXIST         = -1,  /**< Channel does not exist */
	FW_ERROR_HANDLE_NOEXIST          = -2,  /**< Handle does not exist */
	FW_ERROR_NO_DATA                 = -3,  /**< No data */
	FW_ERROR_REACH_MAX               = -4,  /**< Reached maximum */
	FW_ERROR_NULL_POINTER_PARAM      = -5,  /**< Null pointer parameter error */
	FW_ERROR_TIMEOUT      			 = -6,  /**< Timeout */
	FW_ERROR_OUT_OF_MEM      		 = -7,  /**< Out of memory */
	FW_ERROR_BUF_TOO_SMALL      	 = -8,  /**< Buffer too small */
	FW_ERROR_DATA_OVERWRITE      	 = -10, /**< Data overwrite */
	FW_ERROR_READ_MEDIA_END      	 = -11, /**< dec media end flag */
} FW_error_type;

/**
 * @brief Camera effects.
 */
typedef enum
{
	FW_CAMERA_EFFECT_HUE = 0x00,    /**< Hue */
    FW_CAMERA_EFFECT_BRIGHTNESS,    /**< Brightness */
    FW_CAMERA_EFFECT_SATURATION,    /**< Saturation */
    FW_CAMERA_EFFECT_CONTRAST,      /**< Contrast */
    FW_CAMERA_EFFECT_SHARP,         /**< Sharpness */
    FW_CAMERA_EFFECT_WDR            /**< Wide Dynamic Range */
} fw_camera_effect;


/**
 * @brief Media types.
 */
typedef enum
{
    FW_MEDIA_TYPE_VIDEO    = 0, /**< Video */
    FW_MEDIA_TYPE_AUDIO    = 1, /**< Audio */
	FW_MEDIA_TYPE_SUBTITLE = 2  /**< Subtitle */
} fw_media_type;

/**
 * @brief Video codec types.
 */
typedef enum
{
	FW_CODEC_TYPE_INVALID_VIDEO = -1,   /**< Invalid video codec */
    FW_CODEC_TYPE_H264  = 0,            /**< H.264 */
	FW_CODEC_TYPE_MJPEG = 1,            /**< MJPEG */
	FW_CODEC_TYPE_H265  = 2,            /**< H.265 */

	FW_CODEC_TYPE_NV12 = 0x100,         /**< YUV420SP (NV12) */
	FW_CODEC_TYPE_RGB24  = 0x101,       /**< RGB24 */

    FW_CODEC_TYPE_YV12  = 0x102,        /**< YUV420P (YV12) */

} fw_video_codec_type;

/**
 * @brief Audio codec types.
 */
typedef enum
{
	FW_CODEC_TYPE_INVALID_AUDIO = -1,   /**< Invalid audio codec */
	FW_CODEC_TYPE_G726  = 0,            /**< G.726 */
	FW_CODEC_TYPE_MP3   = 2,            /**< MP3 */
	FW_CODEC_TYPE_AAC   = 4,            /**< AAC */
	FW_CODEC_TYPE_PCM   = 6,            /**< PCM */
	FW_CODEC_TYPE_G711A = 17,           /**< G.711 A-law */
	FW_CODEC_TYPE_G711U = 18,           /**< G.711 u-law */
} fw_audio_codec_type;

/**
 * @brief Stream state.
 */
typedef enum
{
	STREAM_STATE_INIT,      /**< Initialized */
	STREAM_STATE_READY,     /**< Ready */
	STREAM_STATE_PLAYING,   /**< Playing */
	STREAM_STATE_PAUSED,    /**< Paused */
	STREAM_STATE_STOPPED    /**< Stopped */
} fw_stream_state;


/**
 * @brief Frame information structure.
 */
struct _fw_frame_info
{
	int iCodeType;          /**< Codec type */
	int iFrameType;         /**< 0: P frame, 1: I frame */
	unsigned int uiDataLen; /**< Data length */
	unsigned int uiSeqNo;   /**< Sequence number */
	unsigned long long u64Time; /**< Timestamp */

	int iWidth;             /**< Video width */
	int iHeight;            /**< Video height */

	int iSampleRate;        /**< Audio sample rate */
	int iSampleBits;        /**< Audio sample bits */
	int iChannelNum;        /**< Audio channel number */

	unsigned char *pData;   /**< Pointer to data */
} __PACKED__;

typedef struct _fw_frame_info fw_frame_info;

/**
 * @brief Pixel frame structure.
 */
typedef struct
{
	#define AK_NUM_DATA_POINTERS 4

    unsigned char* data[AK_NUM_DATA_POINTERS];  /**< Data pointers */
    int linesize[AK_NUM_DATA_POINTERS];         /**< Line sizes */
	int data_count;                             /**< Data count */

    /**
     * Presentation timestamp in time_base units (time when frame should be shown to user).
     */
    long long pts;
	long long seq_no;                           /**< Sequence number */
    /**
     * Duration of the frame, in the same units as pts. 0 if unknown.
     */
    long long duration;

	int width;          /**< Width */
	int height;         /**< Height */

	void *reserved;     /**< Reserved */
} fw_frame_pixel;


/**
 * @brief PCM frame structure.
 */
typedef struct
{
    unsigned char* data;    /**< Data pointer */
    int data_len;           /**< Data length */

    long long pts;          /**< Timestamp */
	long long seq_no;       /**< Sequence number */

	int iSampleRate;        /**< Sample rate */
	int iSampleBits;        /**< Sample bits */
	int iChannelNum;        /**< Channel number */

	void *reserved;         /**< Reserved */
} fw_frame_pcm;


/**
 * @brief Media video information structure.
 */
typedef struct
{
    int iVideoType; /**< Encode/decode type */
    int iFps;       /**< Frame rate */
    int iWidth;     /**< Width */
    int iHeight;    /**< Height */
} TMediaVideoInfo;

/**
 * @brief Media audio information structure.
 */
typedef struct
{
    int iAudioType;     /**< Encode/decode type */
    int iSampleRate;    /**< Sample rate */
    int iSampleBits;    /**< Sample bits */
    int iChannelNum;    /**< Channel number */
} TMediaAudioInfo;

/**
 * @brief Network video frame info structure.
 */
struct _fw_net_frame_info_v
{
	int iCodeType;          /**< Codec type */
	int iFrameType;         /**< 0: P frame, 1: I frame */
	unsigned int uiDataLen; /**< Data length */
	unsigned int uiSeqNo;   /**< Sequence number */
	unsigned long long u64Time; /**< Timestamp */


	int iWidth;             /**< Width */
	int iHeight;            /**< Height */
} __PACKED__;
typedef struct _fw_net_frame_info_v fw_net_frame_info_v;

/**
 * @brief Network audio frame info structure.
 */
struct _fw_net_frame_info_a
{
	int iCodeType;          /**< Codec type */
	unsigned int uiDataLen; /**< Data length */
	unsigned int uiSeqNo;   /**< Sequence number */
	unsigned long long u64Time; /**< Timestamp */

	int iSampleRate;        /**< Sample rate */
	int sample_bits;        /**< Sample bits */
	int channel_num;        /**< Channel number */
} __PACKED__;
typedef struct _fw_net_frame_info_a fw_net_frame_info_a;

/**
 * @brief DMA frame info structure.
 */
struct _fw_dma_frame_info
{
	fw_frame_info frame_info;   /**< Frame info */
	unsigned long phyaddr;      /**< Physical address */
	void *pDmaAddr;             /**< DMA address */
} __PACKED__;
typedef struct _fw_dma_frame_info fw_dma_frame_info;

/**
 * @brief Video encoding parameters.
 */
typedef struct
{
	int code_type;      /**< Codec type: H264, H265 */
	int	width;          /**< Width */
	int	height;         /**< Height */
	int framerate;      /**< Frame rate */
	int bitrate;        /**< Bitrate */
	int goplen;         /**< GOP length */
	int bitrate_mode;   /**< Bitrate control mode */
} fw_video_enc_param;

/**
 * @brief Extended video encoding parameters.
 */
typedef struct
{
    int             iSmartMode;         /**< 0: disable smart, 1: mode of LTR, 2: mode of changing GOP length */
    unsigned short  uiSmartGopLen;      /**< Smart GOP length */
    unsigned short  uiSmartQuality; 	/**< Smart quality */
    unsigned short  uiSmartStaticValue; /**< Smart static value */
    unsigned short  uiMaxKBps;          /**< Max KBps */
    int             GdrMode;            /**< GDR mode */
    int             iProfile;           /**< Profile */
    int	            iBRMode;            /**< Bitrate mode */
    unsigned short  uiInitQp;           /**< Initial QP */
    unsigned short  uiMinQp;            /**< Min QP */
    unsigned short  uiMaxQp;            /**< Max QP */
    unsigned short  uiJpegQLevel;       /**< JPEG quality level */
    int             iChromaMode;        /**< Chroma mode */
    int             iEncOutType;        /**< Encoder output type */
    unsigned int    uiMaxPictureSize;   /**< Max picture size */
    int             iEncLevel;          /**< Encoder level */
} fw_video_enc_param_ex;

/**
 * @brief Audio encoding parameters.
 */
typedef struct
{
	int bEnable;        /**< 0: Disable 1: Enable */
	int iCodeType;      /**< g711a, g711mu, aac */
	int iChannelNum;    /**< Number of channels */
	int iSampleBits;    /**< Sample bits */
	int iSampleRate;    /**< Sample rate */
} fw_audio_enc_param;

/**
 * @brief Live stream parameters.
 */
typedef struct
{
	int iAudioOutVolume;    /**< Audio output volume */
	int iYuvFps;            /**< YUV FPS */
	int iEchoEnable;        /**< Echo cancellation enable */
	int iVadEnable;         /**< VAD enable */
	int iVadValue;          /**< VAD value */
	int iCryEnable;         /**< Cry detection enable */
	int iCryValue;          /**< Cry detection value */
} fw_ls_param;

/**
 * @brief Rectangle information (normalized).
 */
typedef struct
{
	double dLeft;       /**< Left (0-1) */
	double dTop;        /**< Top (0-1) */
	double dWidth;      /**< Width (0-1) */
	double dHeight;     /**< Height (0-1) */
	int iLineWidth;     /**< Line width */
	unsigned int uiColor; /**< Color */
} fw_rect_info;

/**
 * @brief RGBA color structure.
 */
typedef struct
{
	unsigned r; /**< Red */
	unsigned g; /**< Green */
	unsigned b; /**< Blue */
	unsigned a; /**< Alpha */
} fw_rgba;

/**
 * @brief OSD rectangle array.
 */
typedef struct
{
	fw_rect_info RectBox[MAX_RECT_BOX_NUM]; /**< Rectangle boxes */
	int iRectNum;                           /**< Number of rectangles */
} osd_rect_array;

/**
 * @brief Captured picture info.
 */
typedef struct
{
	int  iType;         /**< Type */
	char *pBuf;         /**< Buffer */
	int  iBufLen;       /**< Buffer length */
	int  iDataLen;      /**< Data length */
} fw_cap_picture_info;

/**
 * @brief Captured rectangle info.
 */
typedef struct
{
	// Resolution normalization [0,1]
	double dLeft;       /**< Left */
	double dTop;        /**< Top */
	double dWidth;      /**< Width */
	double dHeight;     /**< Height */

	unsigned int uiDstW;    /**< Destination width */
	unsigned int uiDstH;    /**< Destination height */
	unsigned int iDataLen;  /**< Data length */
	unsigned int uiScore;   /**< Score */
} fw_cap_rect_info;

/**
 * @brief Captured picture cut info.
 */
typedef struct
{
	int  	iType;      /**< 0: jpg, 1: yuv420sp */
	char 	*pBuf;      /**< Buffer */
	int 	iBufLen;    /**< Buffer length */
	int 	iDataLen;	/**< Pic data length <= iBufLen */
	int 	iPicNum;    /**< Picture number */
	fw_cap_rect_info stCapInfo[MAX_RECT_BOX_NUM]; /**< Capture info array */
} fw_cap_picture_cut_info;

/**
 * @brief JPEG encoding parameters.
 */
typedef struct
{
	int iChannel;       /**< Channel number, internal use */

	int bEnable;        /**< 0: Disable 1: Enable */
	int	iWidth;         /**< Width */
	int	iHeight;        /**< Height */
} fw_jpg_enc_param;

/**
 * @brief Electronic zoom parameters.
 */
typedef struct
{
	fw_rect_info	stCrop; /**< Crop area (normalized [0,1]) */
	int				iChnID;	/**< VI Channel ID */
	int				iStep; 	/**< How many frames to complete the action */
} fw_electronic_zoom;

/**
 * @brief Stream callback function type.
 */
typedef int (*FW_STREAM_CB)(int nHandle, fw_frame_info *pframe, void* pDataCbUser);

/**
 * @brief Message callback function type.
 */
typedef int (*FW_MSG_CB)(int nHandle, int nMsgType,  char *pMsg,  int iMsgLen, void* pMsgCbUser);

/**
 * @brief DMA frame callback function type.
 */
typedef int (*DMA_FRAME_CALLBACK)(int camera, int channel, fw_dma_frame_info* dma_frame, void* user);

/**
 * @brief Live stream registered client structure.
 */
typedef struct
{
    char name[16];          /**< Client name */
	int nHandle;            /**< Handle */
	FW_STREAM_CB pDataCb;   /**< Data callback */
	void* pDataCbUser;      /**< User data for data callback */
	FW_MSG_CB pMsgCb;       /**< Message callback */
	void* pMsgCbUser;       /**< User data for message callback */
} FW_ls_reg_stream_client_t;



#endif
