// (c) Meta, Inc. and its affiliates. Confidential and proprietary.


#include "FreeRTOS.h"
#include "task.h"
#include "cybsp.h"
#include "periphery.h"
#include "cyhal_uart.h"
#include "cyhal_gpio.h"
#include "cy_retarget_io.h"


device_params_t periphery_params = {0};


extern void task_cli(void* param);


static ret_code_t init_pins(void)
{
    cy_rslt_t ret_code = CY_RSLT_SUCCESS;

#ifdef SVB_BOARD
/*1. Initialize PWM pins*/
    ret_code = cyhal_pwm_init(&periphery_params.red_led_pwm, PWM_LED_RED, NULL);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_set_duty_cycle(&periphery_params.red_led_pwm, 0, PWM_LED_FREQ_HZ);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_start(&periphery_params.red_led_pwm);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_init(&periphery_params.green_led_pwm, PWM_LED_BLUE, NULL);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_set_duty_cycle(&periphery_params.green_led_pwm, 0, PWM_LED_FREQ_HZ);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_start(&periphery_params.green_led_pwm);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_init(&periphery_params.blue_led_pwm, PWM_LED_BLUE, NULL);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_set_duty_cycle(&periphery_params.blue_led_pwm, 0, PWM_LED_FREQ_HZ);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_start(&periphery_params.blue_led_pwm);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

/*2. Initialize multiplexers enable, selection pins*/
    ret_code = cyhal_gpio_init(MCU_LS_MUX_EN, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_HIGH);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(MCU_HS_MUX_OE_N, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(MCU_HS_MUX_SEL, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(MCU_LS_MUX_SEL, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

/*3. Initialize pins, which used in tbdi test command.
 * Only for testing purposes in first firmware version.*/
    ret_code = cyhal_gpio_init(MCU_CLK, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(GPIO_3, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(UART_TX_MCU, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(UART_RX_MCU, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);


    ret_code = cyhal_gpio_init(SPI_MOSI, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(SPI_MISO, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(SPI_CLK, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(SPI_CS_N, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);


    ret_code = cyhal_gpio_init(I3C_SCL_VALI, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(I3C_SDA_VALI, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_gpio_init(GPIO_2, CYHAL_GPIO_DIR_OUTPUT,
                               CYHAL_GPIO_DRIVE_STRONG, PIN_LEVEL_LOW);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

#else /*SVB_BOARD*/
    ret_code = cyhal_pwm_init(&periphery_params.blue_led_pwm, DEV_KIT_BLUE_LED, NULL);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_set_duty_cycle(&periphery_params.blue_led_pwm, 100, PWM_LED_FREQ_HZ);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);

    ret_code = cyhal_pwm_start(&periphery_params.blue_led_pwm);
    CHECK_AND_RETURN(ret_code, CY_RSLT_SUCCESS, RET_FAILS);
#endif

    return RET_SUCCESS;
}


/*TODO:
  1. Change io_target to uart functions for SVB.
  2. Add CY_RTOS_AWARE define to avoid concurrency in printf() function
  file:///home/nick/mtw/mtb_shared/retarget-io/release-v1.3.0/docs/html/index.html
*/
static ret_code_t init_uart(void)
{
    size_t clear_screen_len = 8;
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    char clear_screen[] = {0x1B, 0x5B, 0x32, 0x4A, 0x1B, 0x5B, 0x3B, 0x48};

    if (CY_RSLT_SUCCESS != cy_retarget_io_init(DEV_KIT_DEBUG_UART_TX,
                                               DEV_KIT_DEBUG_UART_RX,
                                               DEBUG_UART_BAUDRATE))
    {
        return RET_FAILS;
    }

    cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)clear_screen,  &clear_screen_len);

    return RET_SUCCESS;
}


static ret_code_t init_i2c()
{
    cy_rslt_t result;
    cyhal_i2c_cfg_t mI2C_cfg;

    mI2C_cfg.is_slave = false;
    mI2C_cfg.address = 0;
    mI2C_cfg.frequencyhal_hz = I2C_FREQ;

    result = cyhal_i2c_init(&periphery_params.I2C_obj, I3C_SDA_VALI, I3C_SCL_VALI, NULL);
    CHECK_AND_RETURN(result, CY_RSLT_SUCCESS, RET_FAILS);

    result = cyhal_i2c_configure(&periphery_params.I2C_obj, &mI2C_cfg);
    CHECK_AND_RETURN(result, CY_RSLT_SUCCESS, RET_FAILS);

    return RET_SUCCESS;
}

ret_code_t init_peripherials()
{
    ret_code_t ret = RET_FAILS;

    if (CY_RSLT_SUCCESS != cybsp_init())
    {
        return RET_FAILS;
    }

    ret = init_uart();
    CHECK_AND_RETURN(ret, RET_SUCCESS, RET_FAILS);

    ret = init_pins();
    CHECK_AND_RETURN(ret, RET_SUCCESS, RET_FAILS);

    ret = init_i2c();
    CHECK_AND_RETURN(ret, RET_SUCCESS, RET_FAILS);

    return ret;
}


ret_code_t init_tasks()
{
    BaseType_t ret = pdPASS;

    ret = xTaskCreate(task_cli, "CLI Task", TASK_CLI_STACK_SIZE,
                NULL, TASK_CLI_PRIORITY, NULL);
    CHECK_AND_RETURN(ret, pdPASS, RET_FAILS);

    return RET_SUCCESS;
}

