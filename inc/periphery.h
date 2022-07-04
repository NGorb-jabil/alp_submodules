// (c) Meta, Inc. and its affiliates. Confidential and proprietary.


#ifndef PERIPHERY_H_
#define PERIPHERY_H_


#include "globals.h"
#include "cyhal_i2c.h"
#include "cyhal_pwm.h"


#define I2C_FREQ            (400000UL)
#define DEBUG_UART_BAUDRATE (115200)


typedef struct device_params
{
    cyhal_i2c_t I2C_obj;
    cyhal_pwm_t red_led_pwm;
    cyhal_pwm_t blue_led_pwm;
    cyhal_pwm_t green_led_pwm;
}device_params_t;


ret_code_t init_peripherials(void);
ret_code_t init_tasks(void);

#endif /* PERIPHERY_H_ */
