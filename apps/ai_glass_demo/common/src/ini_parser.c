/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include "ini_parser.h"

INI_CONFIG *ini_create(void)
{
    INI_CONFIG *config = (INI_CONFIG *)malloc(sizeof(INI_CONFIG));
    if (config == NULL) {
        return NULL;
    }

    config->items = NULL;
    config->count = 0;
    config->capacity = 0;

    return config;
}

void ini_destroy(INI_CONFIG *config)
{
    if (config != NULL) {
        if (config->items != NULL) {
            free(config->items);
        }
        free(config);
    }
}

// Expand INI configuration capacity
static int ini_resize(INI_CONFIG *config, int new_capacity)
{
    INI_ITEM *new_items = (INI_ITEM *)realloc(config->items, new_capacity * sizeof(INI_ITEM));
    if (new_items == NULL) {
        return -1;
    }

    config->items = new_items;
    config->capacity = new_capacity;
    return 0;
}

// Add INI item
static int ini_add_item(INI_CONFIG *config, const char *section, const char *key, const char *value)
{
    // If capacity is insufficient, expand capacity
    if (config->count >= config->capacity) {
        int new_capacity = (config->capacity == 0) ? 10 : config->capacity * 2;
        if (ini_resize(config, new_capacity) != 0) {
            return -1;
        }
    }

    // Add new item
    INI_ITEM *item = &config->items[config->count];
    strncpy(item->section, section, MAX_SECTION_LEN - 1);
    item->section[MAX_SECTION_LEN - 1] = '\0';

    strncpy(item->key, key, MAX_KEY_LEN - 1);
    item->key[MAX_KEY_LEN - 1] = '\0';

    strncpy(item->value, value, MAX_VALUE_LEN - 1);
    item->value[MAX_VALUE_LEN - 1] = '\0';

    config->count++;
    return 0;
}

// Remove whitespace characters from both ends of the string
static void trim_string(char *str)
{
    // Remove leading whitespace
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')) {
        start++;
    }

    // Remove trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        end--;
    }

    // Rebuild the string
    if (start > str) {
        memmove(str, start, end - start + 1);
    }
    str[end - start + 1] = '\0';
}

// Load INI configuration from file
int ini_load(INI_CONFIG *config, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    char current_section[MAX_SECTION_LEN] = "";
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp) != NULL) {

        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        newline = strchr(line, '\r');
        if (newline != NULL) {
            *newline = '\0';
        }

        trim_string(line);

        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            int section_len = strlen(line) - 2;
            if (section_len > 0 && section_len < MAX_SECTION_LEN) {
                strncpy(current_section, line + 1, section_len);
                current_section[section_len] = '\0';
            }
            continue;
        }

        char *eq_pos = strchr(line, '=');
        if (eq_pos != NULL) {
            *eq_pos = '\0';
            char key[MAX_KEY_LEN];
            strncpy(key, line, MAX_KEY_LEN - 1);
            key[MAX_KEY_LEN - 1] = '\0';
            trim_string(key);

            char value[MAX_VALUE_LEN];
            strncpy(value, eq_pos + 1, MAX_VALUE_LEN - 1);
            value[MAX_VALUE_LEN - 1] = '\0';
            trim_string(value);

            if (ini_add_item(config, current_section, key, value) != 0) {
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    return 0;
}

// Get string value
const char *ini_get_string(INI_CONFIG *config, const char *section, const char *key, const char *default_value)
{
    for (int i = 0; i < config->count; i++) {
        INI_ITEM *item = &config->items[i];
        if (strcmp(item->section, section) == 0 && strcmp(item->key, key) == 0) {
            return item->value;
        }
    }
    return default_value;
}

// Get integer value
int ini_get_int(INI_CONFIG *config, const char *section, const char *key, int default_value)
{
    const char *str_value = ini_get_string(config, section, key, NULL);
    if (str_value == NULL) {
        return default_value;
    }
    return atoi(str_value);
}

// Get double value
double ini_get_double(INI_CONFIG *config, const char *section, const char *key, double default_value)
{
    const char *str_value = ini_get_string(config, section, key, NULL);
    if (str_value == NULL) {
        return default_value;
    }
    return atof(str_value);
}

// Get boolean value
int ini_get_bool(INI_CONFIG *config, const char *section, const char *key, int default_value)
{
    const char *str_value = ini_get_string(config, section, key, NULL);
    if (str_value == NULL) {
        return default_value;
    }

    if (strcmp(str_value, "true") == 0 || strcmp(str_value, "TRUE") == 0 ||
        strcmp(str_value, "yes") == 0 || strcmp(str_value, "YES") == 0 ||
        strcmp(str_value, "1") == 0) {
        return 1;
    }

    if (strcmp(str_value, "false") == 0 || strcmp(str_value, "FALSE") == 0 ||
        strcmp(str_value, "no") == 0 || strcmp(str_value, "NO") == 0 ||
        strcmp(str_value, "0") == 0) {
        return 0;
    }

    return default_value;
}