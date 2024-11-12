/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>

#include "board.h"
#include "fsl_gpio.h"
#include "fsl_lcdc.h"

#include "FreeRTOS.h"
#include "semphr.h"

/*********************
 *      DEFINES
 *********************/

#define LCD_PANEL_CLK 9000000
#define LCD_HSW 2
#define LCD_HFP 8
#define LCD_HBP 43
#define LCD_VSW 10
#define LCD_VFP 4
#define LCD_VBP 12
#define LCD_POL_FLAGS (kLCDC_InvertVsyncPolarity | kLCDC_InvertHsyncPolarity)
#define LCD_INPUT_CLK_FREQ CLOCK_GetLcdClkFreq()

#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define LCD_FB_BYTE_PER_PIXEL 2

#define LCD_BUFFER0_ADDR 0xa0000000
#define LCD_BUFFER1_ADDR (LCD_BUFFER0_ADDR + LCD_WIDTH * LCD_HEIGHT * LCD_FB_BYTE_PER_PIXEL)

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

// 不等待刷新完成而直接完成
#define LCD_IGNORE_WAIT 1

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
#ifndef LCD_IGNORE_WAIT
static volatile bool s_framePending;
static volatile SemaphoreHandle_t s_frameSema;
#endif
/**********************
 *      MACROS
 **********************/
volatile bool disp_flush_enabled = true;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_flush_cb(disp, disp_flush);

    lv_display_set_buffers(disp, (void *)LCD_BUFFER0_ADDR, (void *)LCD_BUFFER1_ADDR, LCD_WIDTH * LCD_HEIGHT * LCD_FB_BYTE_PER_PIXEL, LV_DISPLAY_RENDER_MODE_DIRECT);
}

#ifndef LCD_IGNORE_WAIT
void LCD_IRQHandler(void)
{
 BaseType_t taskAwake = pdFALSE;

 uint32_t intStatus = LCDC_GetEnabledInterruptsPendingStatus(LCD);

 LCDC_ClearInterruptsStatus(LCD, intStatus);

 if (s_framePending)
 {
  if (intStatus & kLCDC_BaseAddrUpdateInterrupt)
  {
   s_framePending = false;

   xSemaphoreGiveFromISR(s_frameSema, &taskAwake);

   portYIELD_FROM_ISR(taskAwake);
  }
 }

 __DSB();
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
   /* Initialize the display. */
   lcdc_config_t lcdConfig;

#ifndef LCD_IGNORE_WAIT
   s_frameSema = xSemaphoreCreateBinary();
   s_framePending = false;
   NVIC_SetPriority(LCD_IRQn,configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
#endif

   /* Route Main clock to LCD. */
   CLOCK_AttachClk(kMAIN_CLK_to_LCD_CLK);
   CLOCK_SetClkDiv(kCLOCK_DivLcdClk, 1, true);

   LCDC_GetDefaultConfig(&lcdConfig);

   lcdConfig.panelClock_Hz = LCD_PANEL_CLK;
   lcdConfig.ppl           = LCD_WIDTH;
   lcdConfig.hsw           = LCD_HSW;
   lcdConfig.hfp           = LCD_HFP;
   lcdConfig.hbp           = LCD_HBP;
   lcdConfig.lpp           = LCD_HEIGHT;
   lcdConfig.vsw           = LCD_VSW;
   lcdConfig.vfp           = LCD_VFP;
   lcdConfig.vbp           = LCD_VBP;
   lcdConfig.polarityFlags = LCD_POL_FLAGS;
   lcdConfig.upperPanelAddr = LCD_BUFFER0_ADDR;
   lcdConfig.lowerPanelAddr = LCD_BUFFER1_ADDR;
   lcdConfig.bpp            = kLCDC_16BPP565;
   lcdConfig.display        = kLCDC_DisplayTFT;
   lcdConfig.swapRedBlue    = true;

   LCDC_Init(LCD, &lcdConfig, LCD_INPUT_CLK_FREQ);

#ifndef LCD_IGNORE_WAIT
   /* Trigger interrupt at start of every vertical back porch. */
   LCDC_EnableInterrupts(LCD, kLCDC_BaseAddrUpdateInterrupt);
   NVIC_EnableIRQ(LCD_IRQn);
#endif

   LCDC_Start(LCD);
   LCDC_PowerUp(LCD);

   /* 背光控制 */
   const gpio_pin_config_t config = {
    kGPIO_DigitalOutput,
    1,
  };

   /* AP5724WG EN引脚 */
   GPIO_PinInit(GPIO, 3, 31, &config);
}

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{
    if(disp_flush_enabled) {
      LCDC_SetPanelAddr(LCD, kLCDC_UpperPanel, (uint32_t)px_map);

#ifndef LCD_IGNORE_WAIT
      s_framePending = true;
      xSemaphoreTake(s_frameSema, portMAX_DELAY);
#endif
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
