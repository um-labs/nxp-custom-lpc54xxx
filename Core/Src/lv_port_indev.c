/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"

#include "board.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"

#include "ft5406.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_init(void);
static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_indev_t * indev_touchpad;
static ft5406_handle_t touchHandle;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
  status_t status;
  gpio_pin_config_t pin_config = {
   kGPIO_DigitalOutput,
   0,
 };

  CLOCK_EnableClock(kCLOCK_Gpio2);

  /* 复位引脚 */
  GPIO_PinInit(GPIO, 2, 27, &pin_config);
  GPIO_PinWrite(GPIO, 2, 27, 1);

  /* Initialize touch panel controller */
  status = FT5406_Init(&touchHandle);
  if (status != kStatus_Success)
  {
   assert(0);
  }
}

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
 touch_event_t touch_event;
 static int touch_x = 0;
 static int touch_y = 0;

 data->state = LV_INDEV_STATE_RELEASED;

 if (kStatus_Success == FT5406_GetSingleTouch(&touchHandle, &touch_event, &touch_x, &touch_y))
 {
  if ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact))
  {
   data->state = LV_INDEV_STATE_PRESSED;
  }
 }

 /*Set the last pressed coordinates*/
 data->point.x = touch_y;
 data->point.y = touch_x;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
