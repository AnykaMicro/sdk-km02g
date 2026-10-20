/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_EIS_H_
#define _AK_EIS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// C struct definitions
typedef struct {
    double x, y, z;
} CVector3;

typedef struct {
    double w, x, y, z;
} CQuaternion;

typedef struct {
    double roll, pitch, yaw;
} CEulerAngle;

typedef struct {
    double timestamp_ms;
    bool has_gyro;
    CVector3 gyro;
    bool has_accl;
    CVector3 accl;
    bool has_magn;
    CVector3 magn;
} CTimeIMU;

typedef struct {
    int64_t timestamp;
    CQuaternion quaternion;
} CTimeQuatEntry;

typedef struct {
    CTimeQuatEntry* entries;
    int count;
    int capacity;
} CTimeQuat;

// Matrix types and operations
typedef struct {
    double data[3][3];
} Matrix3x3;



// Opaque pointers to C++ objects
typedef void* CSmoother;
typedef void* CIntegrator;

// Vector3 C API
CVector3 vector3_create(double x, double y, double z);
CVector3 vector3_create_zero(void);
double vector3_get_element(const CVector3* v, int idx);
CVector3 vector3_multiply_scalar(const CVector3* v, double scalar);
double vector3_dot(const CVector3* v1, const CVector3* v2);
double vector3_norm(const CVector3* v);
CVector3 vector3_normalized(const CVector3* v);

// Quaternion C API
CQuaternion quaternion_create(double w, double x, double y, double z);
CQuaternion quaternion_create_identity(void);
CQuaternion quaternion_from_euler_angles(double x_angle, double y_angle, double z_angle);
CQuaternion quaternion_from_axis_angle(const CVector3* axis, double angle);
CQuaternion quaternion_negate(const CQuaternion* q);
CQuaternion quaternion_multiply(const CQuaternion* q1, const CQuaternion* q2);
CQuaternion quaternion_inverse(const CQuaternion* q);
double quaternion_dot(const CQuaternion* q1, const CQuaternion* q2);
CQuaternion quaternion_slerp(const CQuaternion* q1, const CQuaternion* q2, double t);
CQuaternion quaternion_normalized(const CQuaternion* q);
void quaternion_normalize(CQuaternion* q);
CVector3 quaternion_rotate_vector(const CQuaternion* q, const CVector3* v);

// EulerAngle C API
CEulerAngle euler_angle_create(double roll, double pitch, double yaw);
CEulerAngle euler_angle_create_zero(void);
void euler_angle_from_quaternion(CEulerAngle* euler, const CQuaternion* q);

// TimeQuat C API
CTimeQuat* time_quat_create(void);
void time_quat_destroy(CTimeQuat* tq);
void time_quat_add_entry(CTimeQuat* tq, int64_t timestamp, const CQuaternion* q);
int time_quat_get_count(const CTimeQuat* tq);
CTimeQuatEntry time_quat_get_entry(const CTimeQuat* tq, int index);
void time_quat_clear(CTimeQuat* tq);

// Integrator C API
CIntegrator integrator_create(void);
void integrator_destroy(CIntegrator integrator);
int integrator_init(CIntegrator integrator,int slide_size);
int integrator_get_quat_vector_size(CIntegrator integrator);
CTimeQuat* integrator_simple_gyro_integrator(CIntegrator integrator,
                                           const CTimeIMU* imu_data,
                                           int data_count,
                                           double duration_ms);
CTimeQuat* integrator_simple_gyro_integrator_ours_break(CIntegrator integrator,
                                                      const CTimeIMU* imu_data,
                                                      int data_count,
                                                      double duration_ms,
                                                      const CQuaternion* orient,
                                                      int64_t prev_timestamp);
void integrator_simple_gyro_integrator_test(CIntegrator integrator,
                                            const CTimeIMU* imu_data,
                                            int data_count,
                                            double sample_time);
void integrator_find_quat_by_ts_restore_tmp_quat(CIntegrator integrator,int64_t timestamp);

// Smoother C API
CSmoother smoother_create(void);
CSmoother smoother_create_with_tc(double tc);
void smoother_destroy(CSmoother smoother);
CTimeQuat* smoother_plain3d(CSmoother smoother, const CTimeQuat* quats, double duration_ms);
CQuaternion cal_quat_diff_by_smoothed_window(CIntegrator integrator, CSmoother smoother);
void smoother_set_time_constant(CSmoother smoother, double tc);
double smoother_get_time_constant(CSmoother smoother);

// Matrix C struct and API
typedef struct {
    double** data;
    int rows;
    int cols;
} CMatrix;

// Matrix C API
CMatrix* matrix_create(int rows, int cols);
CMatrix* matrix_create_zeros(int rows, int cols);
CMatrix* matrix_create_identity(int n);
CMatrix* matrix_from_array(double* data, int rows, int cols);
void matrix_destroy(CMatrix* mat);
void matrix_set(CMatrix* mat, int row, int col, double value);
double matrix_get(const CMatrix* mat, int row, int col);
CMatrix* matrix_transpose(const CMatrix* mat);
CMatrix* matrix_multiply(const CMatrix* A, const CMatrix* B);
CMatrix* matrix_pseudoinverse(const CMatrix* mat);
void matrix_print(const CMatrix* mat, const char* name);
int matrix_get_rows(const CMatrix* mat);
int matrix_get_cols(const CMatrix* mat);
void matrix_copy_data(const CMatrix* mat, double* output);


// Matrix3x3 C API
Matrix3x3 matrix3x3_identity();

Matrix3x3 matrix3x3_multiply(const Matrix3x3* a, const Matrix3x3* b);

// Quaternion to rotation matrix
Matrix3x3 matrix3x3_quaternion_to_rotation(const CQuaternion* q);

// Matrix inverse (simplified, 3x3 only)
Matrix3x3 matrix3x3_inverse(const Matrix3x3* mat);


uint64_t ak_eis_code_mux(uint64_t value);


#define GYRO_FRAME_MUX_FILE "/tmp/gyro_mux_file.bin"

struct _eis_gyro_info
{
	uint64_t timestamp;
	short gyro_x;
	short gyro_y;
	short gyro_z;
}__attribute__ ((__packed__));
typedef struct _eis_gyro_info eis_gyro_info;

int ak_frame_info_store_open();

int ak_frame_info_store_write(long long frame_time, int exp_time);

int ak_frame_info_store_close(char* file_path);


int eis_mux_file(double *pOffsetData, int iOffsetDataLen, const char *gyro_file, const char *bin_file);

// Get version string
const char* ak_eis_get_version();


// Constants
#define C_PI 3.14159265358979323846264338327950288
#define C_DEG2RAD (C_PI / 180.0)
#define C_RAD2DEG (180.0 / C_PI)

#ifdef __cplusplus
}
#endif

#endif // C_WRAPPER_H
