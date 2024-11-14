#include "mma8652fc.h"

#include "fsl_i2c.h"
#include "fsl_pint.h"
#include "fsl_inputmux.h"

#include "board.h"

#include "FreeRTOS.h"
#include "semphr.h"

static float convert_to_g(int8_t msb, int8_t lsb);

status_t MMA_Init()
{
    uint8_t val = 0;

    if (MMA_ReadReg(kMMA8652_WHO_AM_I, &val) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (val != kMMA8652_WHO_AM_I_Device_ID)
    {
        return kStatus_Fail;
    }

    if (MMA_WriteReg(kMMA8652_CTRL_REG1, 0x38) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MMA_WriteReg(kMMA8652_CTRL_REG2, 0x02) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MMA_WriteReg(kMMA8652_CTRL_REG3, 0x00) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    // Data-Ready 中断
    if (MMA_WriteReg(kMMA8652_CTRL_REG4, 0x01) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MMA_WriteReg(kMMA8652_CTRL_REG5, 0x01) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* 测量范围 -8g ~ 8g */
    if (MMA_WriteReg(kMMA8652_XYZ_DATA_CFG, 0x02) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Set the F_MODE, disable FIFO */
    if (MMA_WriteReg(kMMA8652_F_SETUP, 0x00) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MMA_WriteReg(kMMA8652_CTRL_REG1, 0x39) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t MMA_ReadSensorData(mma_data_t *accel)
{
    int8_t val[6] = {0};
    uint8_t ucStatus = 0;

    i2c_master_transfer_t masterXfer;
    status_t result = kStatus_Success;

    do
    {
        if (MMA_ReadReg(kMMA8652_STATUS, &ucStatus) != kStatus_Success)
        {
            return kStatus_Fail;
        }
    } while (!(ucStatus & 0x08));

    masterXfer.slaveAddress   = 0x1D;
    masterXfer.subaddress     = kMMA8652_OUT_X_MSB;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = val;
    masterXfer.dataSize       = 6;
    masterXfer.direction      = kI2C_Read;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    BOARD_LockI2C();
    result = I2C_MasterTransferBlocking(I2C2, &masterXfer);
    BOARD_UnlockI2C();

    /* Get the accelerometer data from the sensor */
    accel->A_x = convert_to_g(val[0], val[1]);
    accel->A_y = convert_to_g(val[2], val[3]);
    accel->A_z = convert_to_g(val[4], val[5]);

    return result;
}

status_t MMA_ReadReg(uint8_t reg, uint8_t *val)
{
    i2c_master_transfer_t masterXfer;
    status_t result = kStatus_Success;

    masterXfer.slaveAddress   = 0x1D;
    masterXfer.subaddress     = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = val;
    masterXfer.dataSize       = 1;
    masterXfer.direction      = kI2C_Read;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    BOARD_LockI2C();
    result = I2C_MasterTransferBlocking(I2C2, &masterXfer);
    BOARD_UnlockI2C();

    return result;
}

status_t MMA_WriteReg(uint8_t reg, uint8_t val)
{
    i2c_master_transfer_t masterXfer;
    status_t result = kStatus_Success;

    masterXfer.slaveAddress   = 0x1D;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &val;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    BOARD_LockI2C();
    result = I2C_MasterTransferBlocking(I2C2, &masterXfer);
    BOARD_UnlockI2C();

    return result;
}

static float convert_to_g(int8_t msb, int8_t lsb) {
    // 将高位和低位组合成 12 位有符号数
    int16_t raw_data = (msb << 4) | (lsb >> 4);

    // 符号扩展，如果最高位为 1，则表示负数
    if (raw_data & 0x0800) {  // 检查第 12 位是否为 1
        raw_data |= 0xF000;   // 扩展符号位
    }

    // 转换为 g 值，每 LSB 对应 8 / 2048 = 0.00390625 g
    return raw_data * 0.00390625;
}