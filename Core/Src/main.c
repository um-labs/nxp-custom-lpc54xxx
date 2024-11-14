#include "board.h"
#include "fsl_gpio.h"

#include "pin_mux.h"
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "mma8652fc.h"

volatile UBaseType_t uxHighWaterMark;
mma_data_t sensorData = {0};

extern int tflm_main(void);
void InitTask(void *argument)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    MMA_Init();

    for(;;)
    {
        // 获取当前任务栈的剩余空间
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        lv_task_handler();
        vTaskDelay(1);
        MMA_ReadSensorData(&sensorData);
    }
}

void vApplicationTickHook( void )
{
    lv_tick_inc(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 进入死循环或者其他处理方式
    for (;;) {
    }
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitSDRAM();
    BOARD_InitSPIFI();
    BOARD_InitI2C();

    tflm_main();

    xTaskCreate(InitTask,"Init",1024,NULL,tskIDLE_PRIORITY + 2,NULL);

    vTaskStartScheduler();

    while (1)
    {

    }
}
