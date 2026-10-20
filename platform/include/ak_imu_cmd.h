/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __LINUX_SC7I22_CMD_H__
#define __LINUX_SC7I22_CMD_H__

#include <linux/types.h>
#include <linux/ioctl.h>

#define IOC_MAGIC 'm'

// Command type definitions
// #define _IOC(cmd, nr, type) _IOWR(IOC_MAGIC, nr, type)

// Command groups (each module uses a different command number range)
enum ioctl_commands {
    IOC_SET_CONFIG,
    IOC_SET_QUICK_START_FLAG,
};


#define IOC_SET_CONFIG          _IOW(IOC_MAGIC, 1, struct imu_config*)
#define IOC_SET_QUICK_START_FLAG          _IOW(IOC_MAGIC, 2, uint8_t*)
typedef enum
{
    FUNC_DISABLE = 0x00,        /**< disable function */
    FUNC_ENABLE  = 0x01,        /**< enable function */
}enable_mode_t;

typedef enum
{
    FULL_SCALE_RANGE_125    = 0x4,
    FULL_SCALE_RANGE_250    = 0x3,
    FULL_SCALE_RANGE_500    = 0x2,
    FULL_SCALE_RANGE_1000   = 0x1,
    FULL_SCALE_RANGE_2000   = 0x0,
}gyro_scale_range_t;

typedef enum
{
    FULL_SCALE_RANGE_2G    = 0x0,
    FULL_SCALE_RANGE_4G    = 0x1,
    FULL_SCALE_RANGE_8G   = 0x2,
    FULL_SCALE_RANGE_16G   = 0x3,
}accel_scale_range_t;

typedef enum
{
    ACC_ORD_0_78Hz    = 0x1,
    ACC_ORD_1_5Hz    = 0x2,
    ACC_ORD_3_125Hz   = 0x3,
    ACC_ORD_6_25Hz   = 0x4,
    ACC_ORD_12_5Hz    = 0x5,
    ACC_ORD_25Hz    = 0x6,
    ACC_ORD_50Hz   = 0x7,
    ACC_ORD_100Hz   = 0x8,
    ACC_ORD_200Hz   = 0x9,
    ACC_ORD_400Hz   = 0xa,
    ACC_ORD_800Hz   = 0xb,
    ACC_ORD_1600Hz   = 0xc,
}acc_odr_t;

struct acc_conf {
    accel_scale_range_t         acc_range;  // Full-scale range
    acc_odr_t                   acc_odr;    // Output data rate
};

typedef enum
{
    GYRO_ORD_25Hz    = 0x6,
    GYRO_ORD_50Hz    = 0x7,
    GYRO_ORD_100Hz   = 0x8,
    GYRO_ORD_200Hz   = 0x9,
    GYRO_ORD_400Hz   = 0xa,
    GYRO_ORD_800Hz   = 0xb,
    GYRO_ORD_1600Hz   = 0xc,
}gyro_odr_t;

struct gyro_conf {
    gyro_scale_range_t          gyro_range;
    gyro_odr_t        gyro_odr;
};

/*
DLPF  Ft[Hz]@12.5Hz  Ft[Hz]@25Hz  Ft[Hz]@50Hz  Ft[Hz]@100Hz  Ft[Hz]@200Hz  Ft[Hz]@400Hz
====  ==============  ===========  ===========  ============  ============  ============
 00           0.45         0.90         1.84          3.70          7.42         14.86
 01           1.07         2.15         4.32          8.67         17.37         34.76
 10           1.39         2.79         5.60         11.23         22.49         45.00
 11           1.50         2.99         6.01         12.05         24.13         48.28
*/
typedef enum
{
    LOW_PASS_FILTER_0 = 0x00,
    LOW_PASS_FILTER_1 = 0x01,
    LOW_PASS_FILTER_2 = 0x02,
    LOW_PASS_FILTER_3 = 0x03,

} low_pass_filter_t;

struct low_pass_filter {
    low_pass_filter_t lpf_sel;
    bool              lpf_enable;
};

/*
HPCF  Ft[Hz]@12.5Hz  Ft[Hz]@25Hz  Ft[Hz]@50Hz  Ft[Hz]@100Hz  Ft[Hz]@200Hz  Ft[Hz]@400Hz
====  ==============  ===========  ===========  ============  ============  ============
00         1.000         2.000         4.000         8.000        16.000        32.000
01         0.500         1.000         2.000         4.000         8.000        16.000
10         0.100         0.200         0.400         0.800         1.600         3.200
11         0.025         0.050         0.100         0.200         0.400         0.800
*/
typedef enum
{
    HIGH_PASS_FILTER_0 = 0x00,
    HIGH_PASS_FILTER_1 = 0x01,
    HIGH_PASS_FILTER_2 = 0x02,
    HIGH_PASS_FILTER_3 = 0x03,

} high_pass_filter_t;

struct high_pass_filter {
    high_pass_filter_t hpf_sel;
    bool              hpf_enable;
};


// Sensor configuration parameters
struct imu_config {
    struct acc_conf             acc_config;
    struct gyro_conf            gyro_config;
};

struct imu_data{
    int32_t acc_x;
    int32_t acc_y;
    int32_t acc_z;
    int32_t gyro_x;
    int32_t gyro_y;
    int32_t gyro_z;
    int64_t temp;
    uint64_t timestamp;
};
#endif