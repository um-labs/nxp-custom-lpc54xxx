#include "board.h"
#include "fsl_gpio.h"
#include "fsl_lcdc.h"

#include "pin_mux.h"
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

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

#define DEMO_BUFFER0_ADDR 0xa0000000
#define DEMO_BUFFER1_ADDR (DEMO_BUFFER0_ADDR + LCD_WIDTH * LCD_HEIGHT * LCD_FB_BYTE_PER_PIXEL)

// RGB565格式颜色宏定义
#define RGB565(r, g, b) ((uint16_t)((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | ((b) & 0x1F)))

// 常见颜色
#define COLOR_BLACK       RGB565(0, 0, 0)        // 黑色
#define COLOR_WHITE       RGB565(31, 63, 31)     // 白色
#define COLOR_RED         RGB565(31, 0, 0)       // 红色
#define COLOR_GREEN       RGB565(0, 63, 0)       // 绿色
#define COLOR_BLUE        RGB565(0, 0, 31)       // 蓝色
#define COLOR_YELLOW      RGB565(31, 63, 0)      // 黄色
#define COLOR_CYAN        RGB565(0, 63, 31)      // 青色
#define COLOR_MAGENTA     RGB565(31, 0, 31)      // 品红色
#define COLOR_GRAY        RGB565(15, 31, 15)     // 灰色
#define COLOR_ORANGE      RGB565(31, 41, 0)      // 橙色
#define COLOR_PINK        RGB565(31, 52, 28)     // 粉色
#define COLOR_PURPLE      RGB565(15, 0, 15)      // 紫色
#define COLOR_BROWN       RGB565(19, 10, 2)      // 棕色

static volatile bool s_framePending;

void LCD_IRQHandler(void)
{
    uint32_t intStatus = LCDC_GetEnabledInterruptsPendingStatus(LCD);

    LCDC_ClearInterruptsStatus(LCD, intStatus);

    if (s_framePending)
    {
        if (intStatus & kLCDC_BaseAddrUpdateInterrupt)
        {
            s_framePending = false;
        }
    }

    __DSB();
}

static void DEMO_InitLcd(void)
{
    /* Initialize the display. */
    lcdc_config_t lcdConfig;

    /* No frame pending. */
    s_framePending = false;
    NVIC_SetPriority(LCD_IRQn,3);

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
    lcdConfig.upperPanelAddr = DEMO_BUFFER0_ADDR;
    lcdConfig.lowerPanelAddr = DEMO_BUFFER1_ADDR;
    lcdConfig.bpp            = kLCDC_16BPP565;
    lcdConfig.display        = kLCDC_DisplayTFT;
    lcdConfig.swapRedBlue    = true;

    LCDC_Init(LCD, &lcdConfig, LCD_INPUT_CLK_FREQ);

    /* Trigger interrupt at start of every vertical back porch. */
    LCDC_EnableInterrupts(LCD, kLCDC_BaseAddrUpdateInterrupt);
    NVIC_EnableIRQ(LCD_IRQn);

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

void fill_lcd_memory(uint16_t color) {
    // 将指针指向起始地址 0xA0000000
    uint16_t *lcd_ptr = (uint16_t *)DEMO_BUFFER0_ADDR;

    // 遍历每个像素位置并赋值
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        lcd_ptr[i] = color;
    }
}

void InitTask(void *argument)
{
    DEMO_InitLcd();
    for(;;)
    {
        fill_lcd_memory(COLOR_RED);
        vTaskDelay(1000);
        fill_lcd_memory(COLOR_GREEN);
        vTaskDelay(1000);
        fill_lcd_memory(COLOR_BLUE);
        vTaskDelay(1000);
    }
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitSDRAM();

    xTaskCreate(InitTask,"Init",configMINIMAL_STACK_SIZE,NULL,2,NULL);

    vTaskStartScheduler();

    while (1)
    {

    }
}
