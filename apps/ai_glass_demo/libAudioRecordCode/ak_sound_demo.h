/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_SUOUND_DEMO_H__
#define __AK_SUOUND_DEMO_H__

/**
 * @brief Starts the audio recording.
 *
 * @param audio_save_file_name Path to save the recorded audio file.
 * @param record_sec Maximum duration of the recording in seconds.
 * @param record_min_sec Minimum duration of the recording in seconds.
 * @return int 0 on success, negative error code on failure.
 */
int ak_sound_demo_start_record(char *audio_save_file_name, unsigned int record_sec, unsigned int record_min_sec);

/**
 * @brief Gets the number of times audio has been recorded.
 *
 * @return int Number of recording sessions.
 */
int ak_sound_demo_get_already_record_times(void);

/**
 * @brief Checks if the audio recording has finished.
 *
 * @return int 1 if finished, 0 if still recording.
 */
int ak_sound_demo_is_recording_finished(void);

/**
 * @brief Stops the audio recording.
 *
 * @return int 0 on success, negative error code on failure.
 */
int ak_sound_demo_stop_record(void);

/**
 * @brief Initializes the audio recording demo environment.
 *
 * @return int 0 on success, negative error code on failure.
 */
int ak_sound_demo_init(void);

/**
 * @brief Destroys the audio recording demo environment.
 *
 * @return int 0 on success, negative error code on failure.
 */
int ak_sound_demo_destroy(void);

#endif
