/**
 *
 * @file        bsxlite_interface.h
 *
 * @brief       interface definitons for bsxlite
 *
 *
 */

#ifndef __BSXLITE_INTERFACE_H__
#define __BSXLITE_INTERFACE_H__

/************************************************************************************************************/
/*                                      INCLUDES                                                            */
/************************************************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/************************************************************************************************************/
/*                                          DEBUG                                                           */
/************************************************************************************************************/

/** ENABLE DEBUG (TIMESTAMP VERIFY,...)*/
#define BSXLITE_DEBUG

/************************************************************************************************************/
/*                                          CONSTANT DEFINITIONS                                            */
/************************************************************************************************************/

/*! @brief Provides the enumeration containing return value identifier
 *
 * @{
 */
#define BSXLITE_OK                                   (0)  /** @brief Normal operation was successful without any warning or additional information. */
#define BSXLITE_E_DOSTEPS_TSINTRADIFFOUTOFRANGE     (-5)  /** Difference of time stamps between subsequent doSteps calls is out of range. */
#define BSXLITE_E_FATAL                           (-254)  /** Fatal error. */
#define BSXLITE_I_DOSTEPS_NOOUTPUTSRETURNABLE        (2)  /** all outputs cannot be returned because no memory provided */

/** useful constants for accelerometer/gyro signal scaling during integration*/
/*! conversion factor accel lsb unit -> m/s^2 */
/*! conv_factor = (range/2^(bit_resolution-1))*SCALING_G_TO_MPS2:->(bit_resolution = 16, range = +/-8g, SCALING_G_TO_MPS2 = 9.80665m/s^2)*/
#define ACC_SCALING_ACC_LSB_TO_MPS2 (0.0023942f)

/*!conversion factor gyro lsb unit -> rad/s*/
/*!conv_factor = (Maxdps/2^(bit_resolution-1))*(pi/180):--> bit_resolution = 16, Maxdps = 2000*/
#define GYROSCOPE_SCALING_GYRO_LSB_TO_RADPS (0.001065264436031695f)


/************************************************************************************************************/
/*                                              DATA STRUCTS                                                */
/************************************************************************************************************/
/*! @brief structure definition to hold the version information of the software */
typedef struct
{
    uint8_t version_major;  /**< major version */
    uint8_t version_minor;  /**< minor version */
    uint8_t bugfix_major;   /**< bug-fix version */
} bsxlite_version;

/*! @brief define the module instance */
typedef size_t bsxlite_instance_t;

/*! @brief module return type */
typedef int16_t bsxlite_return_t;

/*! @brief user define structure */
/*! @brief structure definition for the input data and other info from the module */
typedef struct
{
    float x;
    float y;
    float z;
}vector_3d_t;

/*! @brief structure definition for the quaternion output data from the module */
typedef struct
{
    float x;
    float y;
    float z;
    float w;
}quaternion_t;

/*! @brief structure definition for the orientation output data from the module */
typedef struct
{
    float heading;
    float pitch;
    float roll;
    float yaw;
}euler_angles_t;

/*fusion output structure*/
typedef struct
{
    quaternion_t rotation_vector;       /**< rotation quaternion output : quaternion_t */
    euler_angles_t orientation;         /**< euler angles output (rad) :  euler_angles_t */
    uint8_t accel_calibration_status;   /**< accelerometer calibration accuracy status */
    uint8_t gyro_calibration_status;    /**< gyroscope calibration accuracy status */
}bsxlite_out_t;
/************************************************************************************************************/
/*                                          MODULE INTERFACES                                               */
/************************************************************************************************************/

/*! @brief Retrieve the current version of the library
 *
 * @param[in,out]      version_p       Pointer to structure containing the version information
 *
 */
void bsxlite_get_version(bsxlite_version * version_p);

/*! @brief Initializes all the internal parameters required for the library.
 *
 * @param[in,out]   instance        Instance of the library.
 */
bsxlite_return_t bsxlite_init(bsxlite_instance_t *instance_p);

/*!@brief perform signal processing steps of the library for provided signal samples.
 *
 *  @param[in,out]  instance                                             Instance of the library
 *  @param[in]
 *                                  w_time_stamp                         angular rate time stamp in [us] (micro-seconds)
 *                                  acc_input_p                          input acceleration  x y z [struct] to the module in [meter/second^2]
 *                                  gyro_input_p                         input angular rate  x y z [struct] to the module in [radians/second]
 *  @param[out]     bsxlite_out_t:
 *                                  rotation_imu                         returns game rotation quaternion vector[struct]
 *                                  orientation                          returns orientation (Euler angles) in [radians] [struct]
 *                                  accelerometer calibration accuracy status    returns the calibration status for acceleration signal
 *                                  gyroscope calibration accuracy status     returns the calibration status for the gyroscope signal
 *  @return Zero when successful, positive value for information and warnings, otherwise a negative value as error code.
 *  @retval BSXLITE_OK
 *  @retval BSXLITE_E_DOSTEPS_TSINTRADIFFOUTOFRANGE
 *  @retval BSXLITE_I_DOSTEPS_NOOUTPUTSRETURNABLE
 *  @retval BSXLITE_E_FATAL
 */
bsxlite_return_t bsxlite_do_step(const bsxlite_instance_t *instance_p, const int32_t w_time_stamp, const vector_3d_t * accel_in_p, const vector_3d_t * gyro_in_p, bsxlite_out_t * output_data_p);

/*! @brief Sets to default (resets) all the internal parameters to default values
 *
 * @param[in,out]   instance        Instance of the library
 */
bsxlite_return_t bsxlite_set_to_default(const bsxlite_instance_t *instance_p);

#endif /* __BSXLITE_INTERFACE_H__ */
