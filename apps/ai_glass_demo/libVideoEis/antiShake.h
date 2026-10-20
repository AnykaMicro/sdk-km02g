/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file antiShake.h
 * @brief Anti-Shake / EIS (Electronic Image Stabilization) Definitions
 *
 * This file contains definitions, macros, and structures related to
 * the IPU (Image Processing Unit) based anti-shake functionality.
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#ifndef _ANTISHAKE_H_
#define _ANTISHAKE_H_

#include "ak_ipu.h"

/** @brief PI divided by 180 (Degrees to Radians conversion factor) */
#define M_PI180 0.0174532925199432957692 // pi/180

/** @brief IPU Command Flag: Pre-Sync */
#define IPU_CMD_FLAG_PRE_SYNC 8
/** @brief IPU Command Flag: Post-Sync */
#define IPU_CMD_FLAG_POST_SYNC 9
/** @brief IPU Command Flag: Response */
#define IPU_CMD_FLAG_RESPONSE 10
/** @brief IPU Command Flag: Interrupt */
#define IPU_CMD_FLAG_INTERRUPT 11

/** @brief Transformation Block Size */
#define TRANS_BLOCK 256 //64 //32

/**
 * @brief IPU Command Codes Enumeration
 */
typedef enum
{
    IPU_CMD_NULL = 0X00,        /**< Null command */
    IPU_CMD_DRYRUN = 0X01,      /**< Dry run command */

    IPU_CMD_DROP = 0X10,        /**< Drop command */
    IPU_CMD_REGISTRY = 0X12,    /**< Registry command */
    IPU_CMD_PALETTE = 0X14,     /**< Palette command */

    IPU_CMD_COPY = 0X20,        /**< Copy command */
    IPU_CMD_FILL = 0X24,        /**< Fill command */

    IPU_CMD_BLEND1 = 0X30,      /**< Blend mode 1 */
    IPU_CMD_BLEND2 = 0X31,      /**< Blend mode 2 */
    IPU_CMD_BLEND3 = 0X32,      /**< Blend mode 3 */
    IPU_CMD_BLEND4 = 0X33,      /**< Blend mode 4 */
    IPU_CMD_BLEND5 = 0X34,      /**< Blend mode 5 */
    IPU_CMD_BLEND6 = 0X35,      /**< Blend mode 6 */

    IPU_CMD_SCALE1 = 0X40,      /**< Scale mode 1 */
    IPU_CMD_TRANS4 = 0X50,      /**< Transform mode 4 */

    IPU_CMD_RETURN = 0XF0,      /**< Return command */
    IPU_CMD_BRANCH = 0XF1,      /**< Branch command */
    IPU_CMD_STOP = 0XF9,        /**< Stop command */

} E_IPU_CMD_CODE;

/**
 * @brief Step values (double precision) for transformation
 */
typedef struct
{
    double row_step1;   /**< Row step 1 */
    double row_step2;   /**< Row step 2 */
    double row_step3;   /**< Row step 3 */
    double row_step4;   /**< Row step 4 */
    double col_step1;   /**< Column step 1 */
    double col_step2;   /**< Column step 2 */
    double col_step3;   /**< Column step 3 */
    double col_step4;   /**< Column step 4 */
} T_STEP_DOUBLE;

/**
 * @brief Coordinates (double precision)
 */
typedef struct
{
    double col;         /**< Column coordinate */
    double row;         /**< Row coordinate */

} T_COORD_DOUBLE;

/**
 * @brief Coordinates (32-bit integer)
 */
typedef struct
{
    int32_t col;        /**< Column coordinate */
    int32_t row;        /**< Row coordinate */

} T_COORD_S32;

/**
 * @brief IPU Hardware Response Header
 */
typedef struct
{
    uint8_t code;           /**< Response code */
    uint8_t flags;          /**< Flags */
    uint8_t result;         /**< Result status */
    uint8_t quu_id;         /**< Queue ID */
    uint16_t trunk_cmd_id;  /**< Trunk command ID */
    uint16_t branch_cmd_id; /**< Branch command ID */

} T_IPU_HW_RESP_HEAD;

/**
 * @brief IPU Hardware Response Function Data
 */
typedef struct
{
    T_IPU_HW_RESP_HEAD header;  /**< Header */
    uint64_t cache_hit_count;   /**< Cache hit count (36 bit) */
    uint64_t cache_miss_count;  /**< Cache miss count (36 bit) */
    uint64_t elapsed_time;      /**< Elapsed time (40 bit) */
} T_IPU_HW_RESP_FUNCTION;

/**
 * @brief Step values (mixed integer types)
 */
typedef struct
{
    int32_t row_step1;          /**< Row step 1 */
    int16_t row_step2;          /**< Row step 2 */
    int16_t row_step3;          /**< Row step 3 */
    int32_t row_step4;          /**< Row step 4 */
    int32_t col_step1;          /**< Column step 1 */
    int16_t col_step2;          /**< Column step 2 */
    int16_t col_step3;          /**< Column step 3 */
    int32_t col_step4;          /**< Column step 4 */

    //debug test, xuezhh, 20241127
    double row_step1_double;    /**< Row step 1 (double) for debug */
    double row_step2_double;    /**< Row step 2 (double) for debug */
    double row_step3_double;    /**< Row step 3 (double) for debug */
    double row_step4_double;    /**< Row step 4 (double) for debug */

    double col_step1_double;
    double col_step2_double;
    double col_step3_double;
    double col_step4_double;


    double row0;
    double row1;
    double row2;
    double row3;
    double col0;
    double col1;
    double col2;
    double col3;
} T_STEP_TRANS4;

typedef struct
{
    int16_t w;
    int16_t h;
} T_RES_S16;

typedef struct
{
    int16_t col;
    int16_t row;
} T_COORD_S16;

typedef struct
{
    uint8_t     b;  /**< U */
    uint8_t     g;  /**< Y */
    uint8_t     r;  /**< V */
    uint8_t     a;  /**< alpha */
}T_IPU_PIXEL;

typedef struct TAG_IPU_PLANE {
    uint8_t* base;
    uint32_t        offset;
    int32_t         pitch_px;   /**< no used for index color or plane[3] */
    int32_t         pitch_ln;
} T_IPU_PLANE;

typedef struct TAG_IPU_RES {
    int32_t w; /**< base 1 */
    int32_t h; /**< base 1 */
} T_IPU_RES;

typedef enum {
    IPU_MIPMAP_AVG = 0,
    IPU_MIPMAP_SHARP = 1
} E_IPU_MIPMAP;


typedef enum
{
    IPU_PADDING_REPEAT = 0,
    IPU_PADDING_BLACK = 1,
    IPU_PADDING_WHITE = 2,

}E_IPU_PADDING_COLOR;

typedef enum
{
    IPU_FORMAT_FULLPLANE_GREY = 0X01,
    IPU_FORMAT_FULLPLANE_RGB = 0X02,
    IPU_FORMAT_FULLPLANE_YV24 = 0X03,
    IPU_FORMAT_FULLPLANE_YV16 = 0X04,
    IPU_FORMAT_FULLPLANE_YV12 = 0X05, /**< YUV420P */

    IPU_FORMAT_SEMIPLANE_NV24 = 0X10,
    IPU_FORMAT_SEMIPLANE_NV42 = 0X11,
    IPU_FORMAT_SEMIPLANE_NV16 = 0X12,
    IPU_FORMAT_SEMIPLANE_NV61 = 0X13,
    IPU_FORMAT_SEMIPLANE_NV12 = 0X14,
    IPU_FORMAT_SEMIPLANE_NV21 = 0X15, /**< YUV420SP */

    IPU_FORMAT_PACKED_ARGB32 = 0X20,
    IPU_FORMAT_PACKED_ABGR32 = 0X21,
    IPU_FORMAT_PACKED_BGRA32 = 0X22,
    IPU_FORMAT_PACKED_RGBA32 = 0X23,
    IPU_FORMAT_PACKED_RGB24 = 0X24,
    IPU_FORMAT_PACKED_BGR24 = 0X25,
    IPU_FORMAT_PACKED_RGB565 = 0X26,
    IPU_FORMAT_PACKED_BGR565 = 0X27,
    IPU_FORMAT_PACKED_RGB555 = 0X28,
    IPU_FORMAT_PACKED_BGR555 = 0X29,

    IPU_FORMAT_PACKED_ARGB1555 = 0X2A, /**< output */
    IPU_FORMAT_PACKED_ABGR1555 = 0X2B, /**< output */

    IPU_FORMAT_PACKED_AYUV = 0X30,
    IPU_FORMAT_PACKED_AVUY = 0X31,
    IPU_FORMAT_PACKED_VUYA = 0X32,
    IPU_FORMAT_PACKED_YUVA = 0X33,
    IPU_FORMAT_PACKED_YUV = 0X34,
    IPU_FORMAT_PACKED_VUY = 0X35,
    IPU_FORMAT_PACKED_IYU2 = 0X36,
    IPU_FORMAT_PACKED_YUYV = 0X37,
    IPU_FORMAT_PACKED_YVYU = 0X38,
    IPU_FORMAT_PACKED_UYVY = 0X39,
    IPU_FORMAT_PACKED_VYUY = 0X3A,
    IPU_FORMAT_PACKED_IYU1 = 0X3B,

    IPU_FORMAT_INDEX_1BIT = 0X60, /**< input */
    IPU_FORMAT_INDEX_2BITS = 0X61, /**< input */
    IPU_FORMAT_INDEX_4BITS = 0X62, /**< input */
    IPU_FORMAT_INDEX_8BITS = 0X63, /**< input */

}E_IPU_FORMAT;

typedef enum
{
    IPU_COLOR_UNKNOWN = 0,
    IPU_COLOR_RGB = 1,
    IPU_COLOR_YUV601_PC = 2,
    IPU_COLOR_YUV601_TV = 3,
    IPU_COLOR_YUV709_PC = 4,
    IPU_COLOR_YUV709_TV = 5,
    IPU_COLOR_YUV2020_PC = 6,
    IPU_COLOR_YUV2020_TV = 7,

}E_IPU_COLOR_TYPE;

typedef enum
{
    IPU_ALPHA_CH_NONE = 0, /**< 255 */
    IPU_ALPHA_CH_PACKED = 1,
    IPU_ALPHA_CH_TRANS_IDX = 2, /**< input */
    IPU_ALPHA_CH_PLANE_1BIT = 4,
    IPU_ALPHA_CH_PLANE_2BIT = 5,
    IPU_ALPHA_CH_PLANE_4BIT = 6,
    IPU_ALPHA_CH_PLANE_8BIT = 7,

}E_IPU_ALPHA_CH;

typedef struct TAG_IPU_IMAGE {

    E_IPU_FORMAT            format;
    E_IPU_COLOR_TYPE        pre_color;
    E_IPU_COLOR_TYPE        pst_color;
    E_IPU_ALPHA_CH          alpha_ch;
    bool                    alpha_inv;
    uint8_t                 alpha_global;
    IPU_DITHER_E			color_dither;
    IPU_DITHER_E			alpha_dither;
    E_IPU_PADDING_COLOR     pad_color;
    uint8_t                 trans_index;
    IPU_DIRECTION_E         pst_rotate;
    E_IPU_MIPMAP            mip_algo;
    uint8_t                 mip_horizon;
    uint8_t                 mip_vertial;
    T_IPU_RES               pre_res;
    T_IPU_PLANE             plane[4];
    T_IPU_PIXEL* palette;
    uint8_t                 colors_used;

} T_IPU_IMAGE;

typedef struct
{
    uint8_t pre_sync;  /**< [8]  | 1 bit | "0" */
    uint8_t post_sync; /**< [9]  | 1 bit | "1" */
    uint8_t response;  /**< [10] | 1 bit */
    uint8_t interrupt; /**< [11] | 1 bit */

} T_IPU_HW_FLAG;


typedef struct
{
    uint8_t code;
    uint8_t flags;
    uint16_t cmd_id;

} T_IPU_HW_CMD_HEAD;


typedef struct
{
    T_IPU_HW_CMD_HEAD header; /**< 0X50 */

    uint8_t dst_id;
    uint8_t src_id;
    uint8_t blk_w; /**< -1 */
    uint8_t blk_h; /**< -1 */

    int16_t dst_col;
    int16_t dst_row;

    int16_t weit_stepW2;
    int16_t weit_stepW3;
    uint8_t weit_stepW1[3]; /**< 24 bit */
    uint8_t weit_stepW4[3]; /**< 24 bit */

    int16_t weit_stepH2;
    int16_t weit_stepH3;
    uint8_t weit_stepH1[3];
    uint8_t weit_stepH4[3];

} T_IPU_HW_CMD_TRANS4;


typedef struct
{
    uint16_t w; /**< -1 */
    uint16_t h; /**< -1 */
} T_IPU_HW_RES;

typedef struct
{
    T_IPU_IMAGE* pImgOut;
    T_IPU_IMAGE* pImgIn;
    T_COORD_S16 out_pos;
    T_RES_S16 out_res;
    T_STEP_TRANS4 weit_step;
} T_IPU_CMD_TRANS4;

typedef struct
{
    uint64_t* pImgOut;
    uint64_t* pImgIn;
    T_COORD_S16 out_pos;
    T_RES_S16 out_res;
    T_STEP_TRANS4 weit_step;
} T_IPU_CMD_TRANS4_2;

typedef enum {
    IPU_OK,
    IPU_ERROR,
    IPU_PAR_ERR,              //parameter error
    IPU_ALLOC_ERR,              // memory malloc error
    IPU_INVALID_DATA_ERR,       // invalid data in memory
    IPU_FILE_ERR,
    IPU_COORD_ERR
} E_IPU_STATUS;

typedef enum
{
    DISTRECT_NONE = 0,
    DISTRECT_FISHEYE = 1,
    DISTRECT_NORMAL = 2,
    DISTRECT_HARSH = 3,
    DISTRECT_CYLINDER = 4,
    DISTRECT_TRAN4_NEW = 5,
} E_DISTRECT_FUNCTION;

typedef enum
{
    ROTATE_0 = 0,
    ROTATE_90 = 1,
    ROTATE_180 = 2,
    ROTATE_270 = 3
}E_ROTATE_TYPE;

typedef struct
{
    // Cylindrical projection disabled
    bool cylindrical_enable; // Enable, set to false for disabled
    int cylindrical_direction; // Cylindrical surface direction: 0: horizontal; 1: vertical
    int width;
    int height;
    double f_length_pixel;
    //int horizontal_img_length; // Sensor horizontal frame width
    //double horizontal_fov_angle; // Sensor horizontal FOV angle

    // Perspective transform enabled
    bool perspective_transform_enable; // Enable, set to true for enabled
    double perspective_tran4_mat[3][3]; // Main start putting these 9 values

    // Camera calibration disabled
    bool camera_calibrate_enable; // Enable, set to false for disabled
    double camera_m1[3][3];
    double camera_m2[3][3];
    double dist_coeffs[8];

}T_LEAST_TRANS4_NEW;

typedef struct
{
    // Input and output parameters
    // The following parameters must be set
    int32_t block_w; /**< 32 | 64 block size */
    int32_t block_h;
    int32_t src_w; /**< Source image resolution */
    int32_t src_h;

    int32_t padding_x; /**< Not used yet */
    int32_t padding_y; /**< Not used yet */

    int32_t start_x; /**< Top left coordinate start position, generally set to 0, 0 */
    int32_t start_y;
    int32_t dst_w; /**< Destination image resolution */
    int32_t dst_h;


    int32_t move_x; /**< Displacement, ignore, set to 0 */
    int32_t move_y;

    // Sub-function parameters
    // The following parameters do not need to be set when not enabled
    E_DISTRECT_FUNCTION dist_function; /**< 0 for disabled */
    int32_t trans4_w;
    int32_t trans4_h;

    bool crop_enable;
    int32_t crop_x;
    int32_t crop_y;
    int32_t crop_w;
    int32_t crop_h;

    bool scale_enable;
    int32_t scale_w;
    int32_t scale_h;

    bool rotate_enable;
    E_ROTATE_TYPE rotate;
    //int rotate;
    int32_t rotate_w;
    int32_t rotate_h;

    T_LEAST_TRANS4_NEW trans4_factor;

    int32_t block_cnt;/**< Calculated */

}T_TRANS4_PAR;

//Refactor trans4 function
//Input, geometric transformation, crop, scale, rotate, padding output
//for IS
void trans4_coordinate_change_2( T_IPU_CMD_TRANS4* pcmd, T_TRANS4_PAR* trans4_para );
T_IPU_HW_CMD_TRANS4 set_IPU_HW_CMD_TRANS4( T_IPU_HW_FLAG* flag, uint16_t cmd_id, uint8_t dst_id, uint8_t src_id, T_IPU_CMD_TRANS4* cmd4);
uint32_t get_branch_quu_trans4_distrect_gradual_joint_yv12_new(
    //int32_t joint_w,
    //int32_t joint_h,
    //int32_t blend_length,
    T_TRANS4_PAR* trans4_para_in[2],
    uint8_t src_img_id,
    uint8_t dst_img_id,
    uint8_t cmd_id_input,
    //E_JOINT_TYPE joint_type,
    //E_INPUT_BUFFER_TYPE input_locate,
    //uint8_t src_yv12_64[],
    //uint32_t TEMP_BUFFER_30M, // 0X8E000000
    //uint32_t TEMP_BUFFER_2M, // 0X8FF9A000
    //uint8_t cmd_quu_buffer[], //Jump instruction does not need to be generated
    uint8_t branch_quu_buffer[] //Generated distortion correction instruction pointer
    //char* ipu_quu_jont_c_txt,
    //char* ipu_trans_branch_name *
    );
int init_trans4_par( T_TRANS4_PAR *p_trans4_par, int src_w, int src_h, int dst_w, int dst_h );
T_COORD_DOUBLE get_trans4_new_coord( T_LEAST_TRANS4_NEW trans4_factor, T_COORD_DOUBLE* std_pos);
int32_t quantify_s32(double x, uint8_t bit);
int16_t quantify_s16(double x, uint8_t bit);
#endif