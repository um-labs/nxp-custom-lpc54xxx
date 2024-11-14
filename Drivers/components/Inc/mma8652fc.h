#ifndef _MMA8652FC_H_
#define _MMA8652FC_H_

#include "fsl_common.h"

#define WHO_AM_I_REG kMMA8652_WHO_AM_I
#define MMA8652FC_ACCEL_RESOLUTION_BITS 12

/*!
 * @brief Register definitions for the MMA8652.
 */
enum _MMA8652_constants
{
    kMMA8652_STATUS             = 0x00,
    kMMA8652_OUT_X_MSB          = 0x01,
    kMMA8652_OUT_X_LSB          = 0x02,
    kMMA8652_OUT_Y_MSB          = 0x03,
    kMMA8652_OUT_Y_LSB          = 0x04,
    kMMA8652_OUT_Z_MSB          = 0x05,
    kMMA8652_OUT_Z_LSB          = 0x06,
    kMMA8652_F_SETUP            = 0x09,
    kMMA8652_TRIG_CFG           = 0x0a,
    kMMA8652_SYSMOD             = 0x0b,
    kMMA8652_INT_SOURCE         = 0x0c,
    kMMA8652_WHO_AM_I           = 0x0d,
    kMMA8652_WHO_AM_I_Device_ID = 0x4a,
    kMMA8652_XYZ_DATA_CFG       = 0x0e,
    kMMA8652_CTRL_REG1          = 0x2a,
    kMMA8652_CTRL_REG2          = 0x2b,
    kMMA8652_CTRL_REG3          = 0x2c,
    kMMA8652_CTRL_REG4          = 0x2d,
    kMMA8652_CTRL_REG5          = 0x2e
};

typedef struct _mma_data
{
    // 转换结果
    float A_x;
    float A_y;
    float A_z;
} mma_data_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t MMA_Init();

status_t MMA_ReadSensorData(mma_data_t *accel);
status_t MMA_ReadReg(uint8_t reg, uint8_t *val);
status_t MMA_WriteReg(uint8_t reg, uint8_t val);

static inline uint8_t MMA_GetResolutionBits(void)
{
    return MMA8652FC_ACCEL_RESOLUTION_BITS;
}

#if defined(__cplusplus)
}
#endif

#endif /* _MMA8652FC_H_ */
