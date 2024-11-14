#include "board.h"
#include <stdint.h>
#include "fsl_common.h"
#include "fsl_emc.h"
#include "fsl_spifi.h"
#include "fsl_i2c.h"

#include "FreeRTOS.h"
#include "semphr.h"

/* The SDRAM timing. */
#define SDRAM_REFRESHPERIOD_NS (64 * 1000000 / 4096) /* 4096 rows/ 64ms */
#define SDRAM_TRP_NS (18u)
#define SDRAM_TRAS_NS (42u)
#define SDRAM_TSREX_NS (67u)
#define SDRAM_TAPR_NS (18u)
#define SDRAM_TWRDELT_NS (6u)
#define SDRAM_TRC_NS (60u)
#define SDRAM_RFC_NS (60u)
#define SDRAM_XSR_NS (67u)
#define SDRAM_RRD_NS (12u)
#define SDRAM_MRD_NCLK (2u)
#define SDRAM_RAS_NCLK (2u)
#define SDRAM_MODEREG_VALUE (0x23u)
#define SDRAM_DEV_MEMORYMAP (0x09u) /* 128Mbits (8M*16, 4banks, 12 rows, 9 columns)*/

#define SPIFI_READ (0)
#define SPIFI_PROGRAM_PAGE (1)
#define SPIFI_GET_STATUS (2)
#define SPIFI_ERASE_SECTOR (3)
#define SPIFI_WRITE_ENABLE (4)
#define SPIFI_WRITE_REGISTER (5)

/* Clock rate on the CLKIN pin */
const uint32_t ExtClockIn = BOARD_EXTCLKINRATE;
static spifi_command_t spi_read_command = {256, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B};
static SemaphoreHandle_t s_i2cSema;

/* Initialize the external memory. */
void BOARD_InitSDRAM(void)
{
    uint32_t emcFreq;
    emc_basic_config_t basicConfig;
    emc_dynamic_timing_config_t dynTiming;
    emc_dynamic_chip_config_t dynChipConfig;

    emcFreq = CLOCK_GetEmcClkFreq();
    assert(emcFreq != 0); /* Check the clock of emc */
    /* Basic configuration. */
    basicConfig.endian   = kEMC_LittleEndian;
    basicConfig.fbClkSrc = kEMC_IntloopbackEmcclk;
    /* EMC Clock = CPU FREQ/2 here can fit CPU freq from 12M ~ 180M.
     * If you change the divide to 0 and EMC clock is larger than 100M
     * please take refer to emc.dox to adjust EMC clock delay.
     */
    basicConfig.emcClkDiv = 1;
    /* Dynamic memory timing configuration. */
    dynTiming.readConfig            = kEMC_Cmddelay;
    dynTiming.refreshPeriod_Nanosec = SDRAM_REFRESHPERIOD_NS;
    dynTiming.tRp_Ns                = SDRAM_TRP_NS;
    dynTiming.tRas_Ns               = SDRAM_TRAS_NS;
    dynTiming.tSrex_Ns              = SDRAM_TSREX_NS;
    dynTiming.tApr_Ns               = SDRAM_TAPR_NS;
    dynTiming.tWr_Ns                = (1000000000 / emcFreq + SDRAM_TWRDELT_NS); /* one clk + 6ns */
    dynTiming.tDal_Ns               = dynTiming.tWr_Ns + dynTiming.tRp_Ns;
    dynTiming.tRc_Ns                = SDRAM_TRC_NS;
    dynTiming.tRfc_Ns               = SDRAM_RFC_NS;
    dynTiming.tXsr_Ns               = SDRAM_XSR_NS;
    dynTiming.tRrd_Ns               = SDRAM_RRD_NS;
    dynTiming.tMrd_Nclk             = SDRAM_MRD_NCLK;
    /* Dynamic memory chip specific configuration: Chip 0 - MTL48LC8M16A2B4-6A */
    dynChipConfig.chipIndex       = 0;
    dynChipConfig.dynamicDevice   = kEMC_Sdram;
    dynChipConfig.rAS_Nclk        = SDRAM_RAS_NCLK;
    dynChipConfig.sdramModeReg    = SDRAM_MODEREG_VALUE;
    dynChipConfig.sdramExtModeReg = 0; /* it has no use for normal sdram */
    dynChipConfig.devAddrMap      = SDRAM_DEV_MEMORYMAP;
    /* EMC Basic configuration. */
    EMC_Init(EMC, &basicConfig);
    /* EMC Dynamc memory configuration. */
    EMC_DynamicMemInit(EMC, &dynTiming, &dynChipConfig, 1);
}

void BOARD_InitSPIFI(void)
{
 spifi_config_t config = {0};
 uint32_t sourceClockFreq;
 /* Set SPIFI clock source */
 CLOCK_AttachClk(kFRO_HF_to_SPIFI_CLK);
 sourceClockFreq = CLOCK_GetFroHfFreq();

 /* Set the clock divider */
 CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, sourceClockFreq / 96000000, false);

 /* Initialize SPIFI */
 SPIFI_GetDefaultConfig(&config);
 SPIFI_Init(SPIFI0, &config);

 SPIFI_SetMemoryCommand(SPIFI0, &spi_read_command);
}

void BOARD_InitI2C(void)
{
 i2c_master_config_t masterConfig;

 /* attach 12 MHz clock to FLEXCOMM2 (I2C master for touch controller) */
 CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

 I2C_MasterGetDefaultConfig(&masterConfig);

 /* Change the default baudrate configuration */
 masterConfig.baudRate_Bps = 100000U;

 /* Initialize the I2C master peripheral */
 I2C_MasterInit(I2C2, &masterConfig, CLOCK_GetFlexCommClkFreq(2));

 s_i2cSema = xSemaphoreCreateMutex();
}

// I2C2这个板载I2C资源,在每个外设申请到锁后自行继续.
void BOARD_LockI2C(void)
{
 xSemaphoreTake(s_i2cSema, portMAX_DELAY);
}

void BOARD_UnlockI2C(void)
{
 xSemaphoreGive(s_i2cSema);
}