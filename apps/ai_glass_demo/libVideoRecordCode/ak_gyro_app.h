/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
/**
 * @file ak_gyro_app.h
 * @brief Gyroscope application header file.
 *
 * This file defines the structures and function prototypes for gyroscope data handling,
 * including initialization, data retrieval, and resource release.
 */

#ifndef _AK_GYRO_H_
#define _AK_GYRO_H_

#include <stdint.h>

/** @brief Path to save raw gyroscope data. */
#define GYRO_DATA_PATH "/mnt/gyro_raw_data.csv"

/**
 * @brief Gyroscope raw data structure.
 *
 * Contains the raw byte data from the gyroscope sensor and the timestamp.
 */
typedef struct {
    uint8_t raw_data[12]; /**< Raw data bytes (accelerometer and gyroscope) */
    uint64_t timestamp;   /**< Timestamp of the data */
} gyro_raw_data_t;


/**
 * @brief Gyroscope processed data structure.
 *
 * Contains the parsed accelerometer and gyroscope values along with the timestamp.
 */
typedef struct {

    int16_t acc_x;  /**< Accelerometer X-axis value */
    int16_t acc_y;  /**< Accelerometer Y-axis value */
    int16_t acc_z;  /**< Accelerometer Z-axis value */

    int16_t gyro_x; /**< Gyroscope X-axis value */
    int16_t gyro_y; /**< Gyroscope Y-axis value */
    int16_t gyro_z; /**< Gyroscope Z-axis value */

    uint64_t timestamp; /**< Timestamp of the data */
} gyro_data_t;

/**
 * @brief Initialize the gyroscope module.
 *
 * @param mode Initialization mode.
 *             0: Remote mode.
 *             1: Local/Locate mode.
 * @return int Returns 0 on success, negative value on failure.
 */
int gyro_init(int mode);

/**
 * @brief Deinitialize the gyroscope module.
 *
 * Releases resources associated with the gyroscope module.
 *
 * @return int Returns 0 on success, negative value on failure.
 */
int gyro_deinit(void);

/**
 * @brief Get gyroscope raw data.
 *
 * Retrieves a pointer to the current batch of gyroscope data.
 *
 * @param[out] data Double pointer to store the address of the retrieved data.
 * @return int Returns the count of data items retrieved, or negative value on error.
 */
int gyro_get_raw_data(gyro_data_t **data);

/**
 * @brief Release gyroscope raw data.
 *
 * Frees the memory or releases the buffer associated with the retrieved data.
 *
 * @param[in] data Pointer to the data to be released.
 * @return int Returns 0 on success, negative value on failure.
 */
int gyro_release_raw_data(gyro_data_t *data);




#endif
