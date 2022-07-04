// (c) Meta, Inc. and its affiliates. Confidential and proprietary.


#include <errno.h>
#include <stdlib.h>
#include "globals.h"
#include "cli_cmds.h"
#include "cyhal_gpio.h"
#include "cyhal_i2c.h"
#include "cyhal_pwm.h"
#include "cyhal_uart.h"
#include "FreeRTOS.h"
#include <FreeRTOS_CLI.h>
#include "cy_retarget_io.h"
#include "periphery.h"


extern device_params_t periphery_params;


static void set_uart_clk_gpio3_pins(uint8_t pins_level)
{
    cyhal_gpio_write(MCU_CLK,     pins_level);  // MCU_CLK
    cyhal_gpio_write(GPIO_3,      pins_level);  // GPIO_3
    cyhal_gpio_write(UART_TX_MCU, pins_level);  // UART_TX
    cyhal_gpio_write(UART_RX_MCU, pins_level);  // UART_RX
}


static void set_spi_pins(uint8_t pins_level)
{
    cyhal_gpio_write(SPI_MOSI, pins_level);  // SPI_MOSI
    cyhal_gpio_write(SPI_MISO, pins_level);  // SPI_MISO
    cyhal_gpio_write(SPI_CLK,  pins_level);  // SPI_CLK
    cyhal_gpio_write(SPI_CS_N, pins_level);  // SPI_CS_N
}


static void set_i3c_gpio2_pins(uint8_t pins_level)
{
    cyhal_gpio_write(I3C_SCL_VALI,  pins_level);  // I3C_SCL
    cyhal_gpio_write(I3C_SDA_VALI,  pins_level);  // I3C_SDA
    cyhal_gpio_write(GPIO_2,        pins_level);  // GPIO_2
}


static ret_code_t convert_string_to_pin_number(const char *par1, const char *par2, cyhal_gpio_t *pin_name)
{
    errno = 0;
    char *pEnd = NULL;
    unsigned long port, pin = 0;

    port = strtoul(par1, &pEnd, 10);
    if ((port > 13) || errno)
    {
        return RET_INVALID_PARAMS;
    }

    pin = strtoul(par2, &pEnd, 10);
    if ((pin > 7) || errno)
    {
        return RET_INVALID_PARAMS;
    }

    /*According to CYHAL_GET_GPIO macro in cyhal_psoc6_01_116_bga_ble.h:45,
      gets a pin definition from the provided port and pin numbers*/
    *pin_name = ((((uint8_t)(port)) << 3U) + ((uint8_t)(pin)));

    return RET_SUCCESS;
}


static BaseType_t sensor_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{
    errno = 0;
    uint8_t data = 0;
    const char *par1 = NULL;
    const char *par2 = NULL;
    const uint32_t timeout = 2000;
    BaseType_t par1_len, par2_len = 0;
    cy_rslt_t ret_code = CY_RSLT_SUCCESS;

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par1_len);
    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par2_len);

    if ((strncmp (par1,"IMU",par1_len) == 0) &&
        (strncmp (par2,"id", par2_len) == 0))
    {
        data = 0x0F; // WHO_AM_I register address [datasheet, p.67, 37]
        ret_code = cyhal_i2c_master_write(&periphery_params.I2C_obj, LSM6DS_I2C_ADDR, &data, 1, timeout, false);
        if (ret_code != CY_RSLT_SUCCESS)
        {
            strncpy( pcWriteBuffer,"LSM6DST NAK-ed WHO_AM_I register reading.\r\n", xWriteBufferLen );
            return pdFALSE;
        }

        ret_code = cyhal_i2c_master_read(&periphery_params.I2C_obj, LSM6DS_I2C_ADDR, &data, 1, timeout, false);

        if (ret_code != CY_RSLT_SUCCESS)
        {
            strncpy( pcWriteBuffer,"Unable read WHO_AM_I register from LSM6DST\r\n", xWriteBufferLen );
            return pdFALSE;
        }
        // WHO_AM_I register value = 0x6D [datasheet, p.78]
        (data == 0x6D) ? (strncpy( pcWriteBuffer,"IMU sensor detected successfully. \r\n", xWriteBufferLen )) :
                         (strncpy( pcWriteBuffer,"Unable detect IMU sensor.\r\n", xWriteBufferLen));
    }
    else
    {
        strncpy( pcWriteBuffer,"Wrong sensor command parameters.\r\n", xWriteBufferLen );
    }

    return pdFALSE;
}


BaseType_t gpio_set_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{
    bool pin_value        = 0;
    BaseType_t par_len    = 0;
    cyhal_gpio_t pin_name = NC;
    const char *par1, *par2, *par3 = NULL;

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par_len);
    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par_len);

    if (RET_SUCCESS != convert_string_to_pin_number(par1, par2, &pin_name))
    {
        strncpy(pcWriteBuffer,"Incorrect port or pin number.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    par3 = FreeRTOS_CLIGetParameter(pcCommandString, 3, &par_len);
    if (strncmp(par3, "high", par_len) == 0)
    {
        pin_value = 1;
    }
    else if (strncmp(par3, "low", par_len) == 0)
    {
        pin_value = 0;
    }
    else
    {
        strncpy(pcWriteBuffer,"Error: pins level parameter must be high or low.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    cyhal_gpio_write(pin_name, pin_value);

    strncpy(pcWriteBuffer,"Set pin to desired level\r\n", xWriteBufferLen);

    return pdFALSE;
}


BaseType_t gpio_dir_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{

    BaseType_t par_len = 0;
    cyhal_gpio_t pin_name = NC;
    const char *par1, *par2, *par3 = NULL;
    cyhal_gpio_direction_t  pin_direction  = CYHAL_GPIO_DIR_INPUT;
    cyhal_gpio_drive_mode_t pin_drive_mode = CYHAL_GPIO_DRIVE_NONE;

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par_len);
    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par_len);

    if (RET_SUCCESS != convert_string_to_pin_number(par1, par2, &pin_name))
    {
        strncpy(pcWriteBuffer,"Incorrect port or pin number.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    par3 = FreeRTOS_CLIGetParameter(pcCommandString, 3, &par_len);
    if (strncmp(par3, "in", par_len) == 0)
    {
        pin_direction  = CYHAL_GPIO_DIR_INPUT;
        pin_drive_mode = CYHAL_GPIO_DRIVE_NONE;
    }
    else if (strncmp(par3, "out", par_len) == 0)
    {
        pin_direction  = CYHAL_GPIO_DIR_OUTPUT;
        pin_drive_mode = CYHAL_GPIO_DRIVE_STRONG;
    }
    else
    {
        strncpy(pcWriteBuffer,"Error: pin direction must be in or out.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    /*Free pin before change/set direction on it*/
    cyhal_gpio_free(pin_name);

    if (CY_RSLT_SUCCESS != cyhal_gpio_init(pin_name, pin_direction,
                                           pin_drive_mode, 0))
    {
        strncpy(pcWriteBuffer,"gpio_dir_CLI_command() fails call of cyhal_gpio_init()\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    strncpy(pcWriteBuffer,"Set pin to desired direction.\r\n", xWriteBufferLen);

    return pdFALSE;
}


BaseType_t gpio_get_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{
    BaseType_t par_len      = 0;
    cyhal_gpio_t pin_name   = NC;
    const char *par1, *par2 = NULL;

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par_len);
    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par_len);

    if (RET_SUCCESS != convert_string_to_pin_number(par1, par2, &pin_name))
    {
        strncpy(pcWriteBuffer,"Incorrect port or pin number.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    if (cyhal_gpio_read(pin_name))
    {
        strncpy(pcWriteBuffer,"Pin level is high.\r\n", xWriteBufferLen);
    }
    else
    {
        strncpy(pcWriteBuffer,"Pin level is low.\r\n", xWriteBufferLen);
    }

    return pdFALSE;
}


BaseType_t tbdi_CLI_command( char *pcWriteBuffer,
                            size_t xWriteBufferLen,
                            const char *pcCommandString )
{
    bool pins_level         = 0;
    BaseType_t par_len      = 0;
    const char *par1, *par2 = NULL;

    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par_len);
    if (strncmp(par2, "high", par_len) == 0)
    {
        pins_level = 1;
    }
    else if (strncmp(par2, "low", par_len) == 0)
    {
        pins_level = 0;
    }
    else
    {
        strncpy(pcWriteBuffer,"Error: pins level parameter must be high or low.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par_len);
    if (strncmp(par1, "sa", par_len) == 0)
    {
        cyhal_gpio_write(MCU_LS_MUX_SEL, PIN_LEVEL_LOW);
        cyhal_gpio_write(MCU_HS_MUX_SEL, PIN_LEVEL_LOW);
        set_uart_clk_gpio3_pins(pins_level);
    }
    else if (strncmp(par1, "sb", par_len) == 0)
    {
        cyhal_gpio_write(MCU_LS_MUX_SEL, PIN_LEVEL_LOW);
        cyhal_gpio_write(MCU_HS_MUX_SEL, PIN_LEVEL_HIGH);
        set_uart_clk_gpio3_pins(pins_level);
        set_i3c_gpio2_pins(pins_level);
    }
    else if (strncmp(par1, "sc", par_len) == 0)
    {
        cyhal_gpio_write(MCU_LS_MUX_SEL, PIN_LEVEL_HIGH);
        cyhal_gpio_write(MCU_HS_MUX_SEL, PIN_LEVEL_LOW);
        set_spi_pins(pins_level);
    }
    else if (strncmp(par1, "da", par_len) == 0)
    {
        cyhal_gpio_write(MCU_LS_MUX_SEL, PIN_LEVEL_HIGH);
        cyhal_gpio_write(MCU_HS_MUX_SEL, PIN_LEVEL_HIGH);
        set_spi_pins(pins_level);
        set_i3c_gpio2_pins(pins_level);
    }
    else
    {
        strncpy(pcWriteBuffer,"Error: mux parameter must be sa, sb, sc or da.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    strncpy(pcWriteBuffer,"Signals on tbdi pins set to corresponding levels.\r\n", xWriteBufferLen);

    return pdFALSE;
}


BaseType_t led_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{
    errno = 0;
    char *pEnd = NULL;
    BaseType_t par_len = 0;
    unsigned long duty_cycle = 0;
    const char *par1, *par2  = NULL;

    par2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &par_len);
    duty_cycle = strtoul(par2, &pEnd, 10);
    if ((duty_cycle > 100) || errno)
    {
        strncpy(pcWriteBuffer,"Error: Duty cycle must be in range 0-100.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    par1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &par_len);
    if (strncmp(par1, "red", par_len) == 0)
    {
        cyhal_pwm_set_duty_cycle(&periphery_params.red_led_pwm, duty_cycle, PWM_LED_FREQ_HZ);
    }
    else if (strncmp(par1, "green", par_len) == 0)
    {
        cyhal_pwm_set_duty_cycle(&periphery_params.green_led_pwm, duty_cycle, PWM_LED_FREQ_HZ);
    }
    else if (strncmp(par1, "blue", par_len) == 0)
    {
        cyhal_pwm_set_duty_cycle(&periphery_params.blue_led_pwm, (100 - duty_cycle), PWM_LED_FREQ_HZ);
    }
    else
    {
        strncpy(pcWriteBuffer,"Error: Led parameter must be red green or blue.\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    strncpy(pcWriteBuffer,"PWM set to desired level.\r\n", xWriteBufferLen);

    return pdFALSE;
}


BaseType_t fwversion_CLI_command( char *pcWriteBuffer,size_t xWriteBufferLen, const char *pcCommandString )
{
    char str_out[CLI_OUTPUT_LENGTH_MAX] = {0};

    sprintf(str_out, "Current firmware version: %u.%u.%u \r\n",
                                               MAJOR_FW_VERSION,
                                               MINOR_FW_VERSION,
                                               RELEASE_FW_VERSION);
    strncpy(pcWriteBuffer, str_out, xWriteBufferLen);

    return pdFALSE;
}


static const CLI_Command_Definition_t cmds_arr[] = {
        {
            "sensor",
            "sensor: Read WHO_AM_I register from LSM6DSOXTR IMU chip.\r\n"
            "Usage example: <sensor IMU id> \r\n",
            sensor_CLI_command,
            2
        },
        {
            "gpio_set",
            "gpio_set: Set signal on gpio to high or low level.\r\n"
            "Usage example to set high level on port 5, pin 1: <gpio_set 5 1 1> \r\n",
            gpio_set_CLI_command,
            3
        },
        {
            "gpio_get",
            "gpio_get: Get signal level from gpio.\r\n"
            "Usage example to get signal level on port 5, pin 2: <gpio_get 5 2> \r\n",
            gpio_get_CLI_command,
            2
        },
        {
            "gpio_dir",
            "gpio_dir: Set pin direction to in or out.\r\n"
            "Usage example to initialize port 5 pin 2 direction as input: <gpio_dir 5 2 in> \r\n",
            gpio_dir_CLI_command,
            3
        },
        {
            "tbdi",
            "tbdi: Set pins on tbdi connector to high or low level.\r\n"
            "Usage example: <tbdi sa high>\r\n",
            tbdi_CLI_command,
            2
        },
        {
            "led",
            "led: Set pwm duty cycle on led pins.\r\n"
            "Usage example to set pwm duty cycle to 25 on blue led: <led blue 25>\r\n",
            led_CLI_command,
            2
        },
        {
            "fwversion",
            "fwversion: Prints current firmware version.\r\n"
            "Usage example: <fwversion>",
            fwversion_CLI_command,
            0
        }
};


void task_cli(void* param)
{
    (void)param;

    uint8_t cRxedChar, i  = 0;
    size_t  cInputIndex   = 0;
    size_t clear_line_len = 5;
    BaseType_t xMoreDataToFollow = 0;

    char pcInputString[CLI_INPUT_LENGTH_MAX]   = {0};
    char pcOutputString[CLI_OUTPUT_LENGTH_MAX] = {0};
    /* ESC[2K - ANSI ESC sequence for erase entire line */
    char clear_string[] = {0x1B, 0x5B, 0x32, 0x4B, 0x0D};

    printf("Firmware for manufacture testing purposes only.\r\n"
           "Please enter <help> to see available CLI commands.\r\n");

    for (; i < sizeof(cmds_arr)/sizeof(CLI_Command_Definition_t); i++)
    {
        FreeRTOS_CLIRegisterCommand(&cmds_arr[i]);
    }

    for(;;)
    {
        if (cyhal_uart_getc(&cy_retarget_io_uart_obj, &cRxedChar, 0) == CY_RSLT_SUCCESS)
        {
            switch(cRxedChar)
            {
            case '\r':
                /* A newline character was received, so the input command string is
                complete and can be processed.  Transmit a line separator, just to
                make the output easier to read. */
                printf("\r\n");

                /* The command interpreter is called repeatedly until it returns
                pdFALSE.  See the "Implementing a command" documentation for an
                exaplanation of why this is. */
                do
                {
                    /* Send the command string to the command interpreter.  Any
                    output generated by the command interpreter will be placed in the
                    pcOutputString buffer. */
                    xMoreDataToFollow = FreeRTOS_CLIProcessCommand (
                                        pcInputString,     /* The command string.*/
                                        pcOutputString,    /* The output buffer. */
                                        CLI_OUTPUT_LENGTH_MAX);/* The size of the output buffer. */


                    /* Write the output generated by the command interpreter to the
                    console. */
                    printf( "%s \r\n", pcOutputString);

                } while( xMoreDataToFollow != pdFALSE );

                cInputIndex = 0;
                memset( pcInputString, 0x00, CLI_INPUT_LENGTH_MAX );

                break;

            case '\b':
                if( cInputIndex > 0 )
                {
                    cInputIndex--;
                    pcInputString[ cInputIndex ] = '\0';
                    cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)clear_string,  &clear_line_len);
                    if (cInputIndex)
                    {
                        cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)pcInputString, &cInputIndex);
                    }
                }
                break;

            default:
                if( cInputIndex < CLI_INPUT_LENGTH_MAX )
                {
                    cyhal_uart_putc(&cy_retarget_io_uart_obj, cRxedChar);
                    pcInputString[ cInputIndex ] = cRxedChar;
                    cInputIndex++;
                }
                else
                {
                    printf("Reached command maximum length, press enter to execute the command. \r\n");
                }
            }
        }
    }

}
