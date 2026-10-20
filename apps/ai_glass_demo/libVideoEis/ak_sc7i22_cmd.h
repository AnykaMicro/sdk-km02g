/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __LINUX_SC7I22_CMD_H__
#define __LINUX_SC7I22_CMD_H__

#include <linux/types.h>
#include <linux/ioctl.h>

#define IOC_MAGIC 'm'

// Command type definition
// #define _IOC(cmd, nr, type) _IOWR(IOC_MAGIC, nr, type)

/**
 * @file ak_sc7i22_cmd.h
 * @brief SC7I22 Sensor IOCTL Commands and Structures
 *
 * This file defines the IOCTL commands and data structures used for
 * communicating with the SC7I22 accelerometer and gyroscope sensor driver.
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#ifndef __LINUX_SC7I22_CMD_H__
#define __LINUX_SC7I22_CMD_H__

#include <linux/types.h>
#include <linux/ioctl.h>

/** @brief IOCTL Magic Number */
#define IOC_MAGIC 'm'

// Command type definition
// #define _IOC(cmd, nr, type) _IOWR(IOC_MAGIC, nr, type)

/**
 * @brief IOCTL Command Enumeration
 *
 * Groups commands by functional module.
 */
enum ioctl_commands {
    IOC_SET_CONFIG,                 /**< Set sensor configuration */
    IOC_GET_LATEST_TIMESTAMP,       /**< Get latest timestamp */

    // Sensor configuration
    IOC_SET_ACCEL_RANGE,            /**< Set accelerometer range */
    IOC_SET_GYRO_RANGE,             /**< Set gyroscope range */
    IOC_SET_ACCEL_CONFIG,           /**< Configure accelerometer (range, bandwidth, etc.) */
    IOC_SET_GYRO_CONFIG,            /**< Configure gyroscope (range, bandwidth, etc.) */
    IOC_SET_PWR_MGMT,               /**< Configure power management */
    IOC_SET_SLEEP_MODE,             /**< Set sleep mode */
    IOC_SET_HCPF,                   /**< Set High Cutoff Pass Filter */
    IOC_SET_DLPF,                   /**< Set Digital Low Pass Filter */
    IOC_SET_SAMPLE_RATE_DIV,        /**< Set sample rate divider */

    // FIFO control
    IOC_ENABLE_FIFO,                /**< Enable FIFO + configure accelerometer/gyroscope writing to FIFO */

    // Register read/write
    IOC_READ_REGISTER,              /**< Read register */
    IOC_WRITE_REGISTER,             /**< Write register */

    // Sensor data reading
    // IOC_READ_ACCEL_DATA,         // Read real-time accelerometer data
    // IOC_READ_GYRO_DATA,          // Read real-time gyroscope data
    // IOC_READ_TEMP_DATA,          // Read temperature data

    // Calibration and self-test
    // IOC_TRIGGER_SELF_TEST,       // Trigger self-test
    // IOC_SET_OFFSETS,             // Set sensor offset values
    IOC_SET_GYRO_SELF_TEST,         /**< Set gyroscope self-test */
    IOC_SET_ACCEL_SELF_TEST,        /**< Set accelerometer self-test */

    // Interrupt control
    // IOC_SET_INT_PIN_CFG,         // Configure interrupt pin (trigger mode, polarity, etc.)
    IOC_SET_INT_ENABLE,             /**< Enable interrupt source (e.g., FIFO overflow, data ready) */
    // IOC_READ_INT_STATUS,         // Read interrupt status register
};

/** @brief IOCTL: Set Accelerometer Range */
#define IOC_SET_ACCEL_RANGE        _IOW(IOC_MAGIC, 1, uint8_t*)
/** @brief IOCTL: Set Gyroscope Range */
#define IOC_SET_GYRO_RANGE         _IOW(IOC_MAGIC, 2, uint8_t*)
/** @brief IOCTL: Set Accelerometer Configuration */
#define IOC_SET_ACCEL_CONF        _IOW(IOC_MAGIC, 3, uint8_t*)
/** @brief IOCTL: Set Gyroscope Configuration */
#define IOC_SET_GYRO_CONF        _IOW(IOC_MAGIC, 4, uint8_t*)
/** @brief IOCTL: Set Power Management */
#define IOC_SET_PWR_MGMT            _IOW(IOC_MAGIC, 5, uint8_t*)
/** @brief IOCTL: Set High Cutoff Pass Filter */
#define IOC_SET_HCPF                _IOW(IOC_MAGIC, 6, uint8_t*)
/** @brief IOCTL: Set Digital Low Pass Filter */
#define IOC_SET_DLPF                _IOW(IOC_MAGIC, 7, struct low_pass_filter)
/** @brief IOCTL: FIFO Configuration */
#define IOC_FIFO_CONF             _IOW(IOC_MAGIC, 8, uint8_t*)
/** @brief IOCTL: Read Register */
#define IOC_READ_REGISTER           _IOWR(IOC_MAGIC, 9, struct reg_access)
/** @brief IOCTL: Write Register */
#define IOC_WRITE_REGISTER          _IOW(IOC_MAGIC,  10, struct reg_access)
/** @brief IOCTL: Set Interrupt Configuration */
#define IOC_SET_INT_CONF          _IOW(IOC_MAGIC, 11, uint8_t*)
/** @brief IOCTL: Set Sensor Configuration */
#define IOC_SET_CONFIG          _IOW(IOC_MAGIC, 12, struct sensor_config*)
/** @brief IOCTL: Get Latest Timestamp */
#define IOC_GET_LATEST_TIMESTAMP          _IOW(IOC_MAGIC, 13, uint64_t*)

/**
 * @brief Register Access Structure
 */
struct reg_access {
    uint8_t reg_addr; /**< Register address */
    uint8_t value;    /**< Data read/write (32 bits is enough to store 16/8 bit register values) */
};


/**
 * @brief FIFO Configuration Parameters
 */
struct fifo_config {
    uint8_t fifo_en;      /**< FIFO enable bit (e.g., accelerometer + gyroscope) */
    uint16_t watermark;   /**< FIFO watermark (optional) */
};

/**
 * @brief Interrupt Configuration Parameters
 */
struct int_config {
    uint8_t latching_en;  /**< Interrupt level latch enable */
    uint8_t active_high;  /**< Interrupt pin polarity (high/low active) */
    uint8_t int_sources;  /**< Enabled interrupt sources (e.g., FIFO overflow) */
};

/**
 * @brief Sensor Data Structure
 */
union sensor_data {
    struct {
        int16_t x, y, z;  /**< 16-bit raw data */
    } raw;
    struct {
        float x, y, z;    /**< Converted physical quantity (optional) */
    } scaled;
};

/**
 * @brief Function Enable/Disable Mode
 */
typedef enum
{
    FUNC_DISABLE = 0x00,        /**< disable function */
    FUNC_ENABLE  = 0x01,        /**< enable function */
}enable_mode_t;


typedef enum
{
    TEMP_ACC_GYR_OFF,
    TEMP_ACC_GYR_ON,
}pwr_ctrl_t;

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
    accel_scale_range_t         acc_range;  /**< Range */
    acc_odr_t                   acc_odr;    /**< Output frequency */
    uint8_t                     acc_bwp;
    bool                        acc_filter_perf_enable; /**< High performance mode */
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
    uint8_t           gyro_bwp;
    bool              filter_perf_enable;
    bool              noise_perf_enable;
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
struct sensor_config {
    struct acc_conf             acc_config;
    struct gyro_conf            gyro_config;
    struct high_pass_filter     hpf_config;
    struct low_pass_filter      lpf_config;
};

#endif