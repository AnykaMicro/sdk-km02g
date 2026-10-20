/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

//#include "rtsp_cmd.h"
#include "ak_rtsp_demo.h"

#if !defined(WIN32)
    #define __PACKED__        __attribute__ ((__packed__))
#else
    #define __PACKED__
#endif


/**
 * @brief Initialize the RTSP server.
 *
 * @param callback Callback function for RTSP events.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_init(rtsp_event_callback_t callback);

/**
 * @brief Deinitialize the RTSP server.
 *
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_deinit(void);

/**
 * @brief RTSP error codes.
 */
typedef enum
{
    E_RTSP_CONNECT_LOST     = -1,   /**< Abnormal disconnection */
    E_RTSP_NO_VIDEO_PARAM   = 0,    /**< No video parameters */
    E_RTSP_NO_AUDIO_PARAM   = 1,    /**< No audio parameters */
    E_RTSP_NO_VIDEA_DATA    = 2,    /**< No video data */
    E_RTSP_NO_AUDIO_DATA    = 3,    /**< No audio data */
    E_RTSP_NO_PLAY_CB       = 4,    /**< No play callback */
    E_RTSP_NO_TEARDOWN_CB   = 5,    /**< No teardown callback */
} rtsp_error_type_e;

/**
 * @brief RTSP network address type.
 */
typedef enum
{
    RTSP_NET_ADDR_IPV4 = 0, /**< IPv4 address */
    RTSP_NET_ADDR_IPV6 = 1, /**< IPv6 address */
    RTSP_PARAM_TYPE_MAX,    /**< Maximum parameter type */
} rtsp_param_type_e;

/**
 * @brief RTSP audio codec type.
 */
typedef enum
{
    RTSP_AUDIO_TYPE_G726 = 0,       /**< G.726 audio */
    RTSP_AUDIO_TYPE_AAC = 4,        /**< AAC audio */
    RTSP_AUDIO_TYPE_PCM_ALAW = 17,  /**< PCM A-law */
    RTSP_AUDIO_TYPE_PCM_ULAW = 18,  /**< PCM U-law */
} rtsp_audio_type;

/**
 * @brief RTSP video codec type.
 */
typedef enum
{
    RTSP_VIDEO_H264 = 0,    /**< H.264 video */
    RTSP_VIDEO_MJPEG,       /**< MJPEG video */
    RTSP_VIDEO_H265,        /**< H.265 video */
    RTSP_VIDEO_MAX,         /**< Maximum video type */
} rtsp_video_type;

/**
 * @brief RTSP video encoding parameters.
 */
struct _rtsp_video_enc_param
{
    rtsp_video_type eCodeType; /**< Video codec type */
    int             iWidth;    /**< Video width */
    int             iHeight;   /**< Video height */
    int             iFps;      /**< Frame rate */
    int             iBps;      /**< Bit rate */
} __PACKED__;

typedef struct _rtsp_video_enc_param rtsp_video_enc_param;

/**
 * @brief RTSP audio encoding parameters.
 */
struct _rtsp_audio_enc_param
{
    rtsp_audio_type eCodeType;   /**< Audio codec type */
    int             iChannelNum; /**< Channel number */
    int             iSampleBits; /**< Sample bits */
    int             iSampleRate; /**< Sample rate */
} __PACKED__;

typedef struct _rtsp_audio_enc_param rtsp_audio_enc_param;

/**
 * @brief RTSP frame information.
 */
struct _rtsp_frame_info
{
    int iCodeType;            /**< Codec type */
    int iFrameType;           /**< Frame type (0: P frame, 1: I frame) */
    unsigned int uiDataLen;   /**< Data length */
    unsigned int uiSeqNo;     /**< Sequence number */
    unsigned long long u64Time; /**< Timestamp */
    int iWidth;               /**< Width */
    int iHeight;              /**< Height */
    int iSampleRate;          /**< Sample rate */
    int iSampleBits;          /**< Sample bits */
    int iChannelNum;          /**< Channel number */
    unsigned char* pData;     /**< Pointer to data */
} __PACKED__;

typedef struct _rtsp_frame_info rtsp_frame_info;

/**
 * @brief RTSP initialization parameters.
 */
struct _rtsp_init_param
{
    unsigned int iServerPort;        /**< Local TCP listening port (default 554) */
    char cUserName[64];              /**< Username */
    char cPassword[64];              /**< Password */
    char cRecordUrlPrefix[16];       /**< Real-time stream URL prefix */
    char cLiveStearmUrlPrefix[16];   /**< Video recording URL prefix */
} __PACKED__;

typedef struct _rtsp_init_param rtsp_init_param;

/**
 * @brief RTSP device parameters.
 */
struct _rtsp_dev_param
{
    char cAddrIPv4[16]; /**< IPv4 address */
    char cAddrIPv6[64]; /**< IPv6 address */
} __PACKED__;

typedef struct _rtsp_dev_param rtsp_dev_param;

/**
 * @brief Get RTSP server device parameters.
 *
 * @param pInitParam Pointer to store initialization parameters.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_dev_param(rtsp_init_param* pInitParam);

/**
 * @brief Set RTSP server initialization parameters (username, password, etc.).
 *
 * @param pInitParam Pointer to initialization parameters.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_init_param(rtsp_init_param* pInitParam);

/**
 * @brief Callback function type for getting error codes.
 * @param eErrorCode The error code.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_ERROR_CODE_CALLBACK)(rtsp_error_type_e eErrorCode);

/**
 * @brief Set the error code callback.
 *
 * @param funGetErrorCodeCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_error_code_callback(RTSP_SERVER_GET_ERROR_CODE_CALLBACK funGetErrorCodeCb);

/**
 * @brief Callback function type for getting device parameters.
 * @param eParamType Parameter type.
 * @param pDevParam Pointer to store device parameters.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_DEV_PARAM_CALLBACK)(rtsp_param_type_e eParamType, rtsp_dev_param* pDevParam);

/**
 * @brief Set the device parameter callback.
 *
 * @param funGetDevParamCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_dev_param_callback(RTSP_SERVER_GET_DEV_PARAM_CALLBACK funGetDevParamCb);

/**
 * @brief Callback function type for getting video parameters.
 * @param pSourceToken Source token string.
 * @param pParam Pointer to store video parameters.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_VIDEO_PARAM_CALLBACK)(const char* pSourceToken, rtsp_video_enc_param* pParam);

/**
 * @brief Set the video parameter callback.
 *
 * @param funGetVideoParamCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_video_param_callback(RTSP_SERVER_GET_VIDEO_PARAM_CALLBACK funGetVideoParamCb);

/**
 * @brief Callback function type for getting audio parameters.
 * @param pSourceToken Source token string.
 * @param pParam Pointer to store audio parameters.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_AUDIO_PARAM_CALLBACK)(const char* pSourceToken, rtsp_audio_enc_param* pParam);

/**
 * @brief Set the audio parameter callback.
 *
 * @param funGetAudioParamCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_audio_param_callback(RTSP_SERVER_GET_AUDIO_PARAM_CALLBACK funGetAudioParamCb);

/**
 * @brief Callback function type for accepting a client.
 *
 * @param pClientIP Client IP address.
 * @param iClientPort Client port.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_ACCEPT_CLIENT_CALLBACK)(char* pClientIP, int iClientPort);

/**
 * @brief Set the accept client callback.
 *
 * @param funGetClientInfoCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_accept_client_callback(RTSP_SERVER_ACCEPT_CLIENT_CALLBACK funGetClientInfoCb);

/**
 * @brief Callback function type for playing a stream.
 *
 * The received play callback needs to force an I-frame to avoid slow image output.
 *
 * @param pUrl Client streaming URL.
 * @return int Streaming handle on success, negative value on failure.
 */
typedef int (*RTSP_SERVER_PLAY_CALLBACK)(char* pUrl);

/**
 * @brief Set the play callback.
 *
 * @param funPlayCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_play_callback(RTSP_SERVER_PLAY_CALLBACK funPlayCb);

/**
 * @brief Callback function type for tearing down a stream.
 *
 * @param iHandle Streaming handle returned by the play callback.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_TEARDOWN_CALLBACK)(int iHandle);

/**
 * @brief Set the teardown callback.
 *
 * @param funTeardownCb The callback function.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_teardown_callback(RTSP_SERVER_TEARDOWN_CALLBACK funTeardownCb);

/**
 * @brief Callback function type for getting video data.
 * @param iHandle Streaming handle.
 * @param pVideo Pointer to video frame info.
 * @param pUserData User data pointer.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_VIDEO_CALLBACK)(int iHandle, rtsp_frame_info* pVideo, void* pUserData);

/**
 * @brief Set the video data callback.
 *
 * @param funGetVideoCb The callback function.
 * @param pUserData User data pointer.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_video_callback(RTSP_SERVER_GET_VIDEO_CALLBACK funGetVideoCb, void* pUserData);

/**
 * @brief Callback function type for getting audio data.
 * @param iHandle Streaming handle.
 * @param pAudio Pointer to audio frame info.
 * @param pUserData User data pointer.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_AUDIO_CALLBACK)(int iHandle, rtsp_frame_info* pAudio, void* pUserData);

/**
 * @brief Set the audio data callback.
 *
 * @param funGetAudioCb The callback function.
 * @param pUserData User data pointer.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_audio_callback(RTSP_SERVER_GET_AUDIO_CALLBACK funGetAudioCb, void* pUserData);

/**
 * @brief Callback function type for getting media data (generic).
 * @param iHandle Streaming handle.
 * @param pMedia Pointer to media frame info.
 * @param pUserData User data pointer.
 * @return int Callback result.
 */
typedef int (*RTSP_SERVER_GET_MEDIA_CALLBACK)(int iHandle, rtsp_frame_info* pMedia, void* pUserData);

/**
 * @brief Set the media data callback.
 *
 * @param funGetMediaCb The callback function.
 * @param pUserData User data pointer.
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_set_media_callback(RTSP_SERVER_GET_MEDIA_CALLBACK funGetMediaCb, void* pUserData);

/**
 * @brief Starts the RTSP service.
 *
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_start(void);

/**
 * @brief Stops the RTSP service.
 *
 * @return int 0 on success, negative error code on failure.
 */
int rtsp_server_stop(void);


#endif
