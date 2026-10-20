/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SECTION_LEN 50
#define MAX_KEY_LEN 50
#define MAX_VALUE_LEN 100
#define MAX_LINE_LEN 200

// INI configuration item structure
typedef struct {
    char section[MAX_SECTION_LEN];  /**< Section name */
    char key[MAX_KEY_LEN];          /**< Key name */
    char value[MAX_VALUE_LEN];      /**< Key value */
} INI_ITEM;

// INI configuration structure
typedef struct {
    INI_ITEM *items;    /**< Array of INI items */
    int count;          /**< Current number of items */
    int capacity;       /**< Allocated capacity */
} INI_CONFIG;

/**
 * @brief Initialize INI configuration
 * @return Pointer to INI_CONFIG structure, or NULL on failure
 */
INI_CONFIG *ini_create(void);

/**
 * @brief Destroy INI configuration and free memory
 * @param config Pointer to INI_CONFIG structure
 */
void ini_destroy(INI_CONFIG *config);

/**
 * @brief Load INI configuration from file
 * @param config Pointer to INI_CONFIG structure
 * @param filename Path to INI file
 * @return 0 on success, -1 on failure
 */
int ini_load(INI_CONFIG *config, const char *filename);

/**
 * @brief Get string value from INI configuration
 * @param config Pointer to INI_CONFIG structure
 * @param section Section name
 * @param key Key name
 * @param default_value Default value if key not found
 * @return String value or default_value
 */
const char *ini_get_string(INI_CONFIG *config, const char *section, const char *key, const char *default_value);

/**
 * @brief Get integer value from INI configuration
 * @param config Pointer to INI_CONFIG structure
 * @param section Section name
 * @param key Key name
 * @param default_value Default value if key not found
 * @return Integer value or default_value
 */
int ini_get_int(INI_CONFIG *config, const char *section, const char *key, int default_value);

/**
 * @brief Get boolean value from INI configuration
 * @param config Pointer to INI_CONFIG structure
 * @param section Section name
 * @param key Key name
 * @param default_value Default value if key not found
 * @return Boolean value (0/1) or default_value
 */
int ini_get_bool(INI_CONFIG *config, const char *section, const char *key, int default_value);

/**
 * @brief Get double value from INI configuration
 * @param config Pointer to INI_CONFIG structure
 * @param section Section name
 * @param key Key name
 * @param default_value Default value if key not found
 * @return double value or default_value
 */
double ini_get_double(INI_CONFIG *config, const char *section, const char *key, double default_value);

#endif // INI_PARSER_H
