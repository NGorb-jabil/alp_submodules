// (c) Meta, Inc. and its affiliates. Confidential and proprietary.


#ifndef GLOBALS_H_
#define GLOBALS_H_

/*Macro used to distinguish SVB and Dev kit. Comment it if CY8CKIT-062-BLE
 * dev kit using.*/
#define     SVB_BOARD

#define     MAJOR_FW_VERSION   (1U)
#define     MINOR_FW_VERSION   (0U)
#define     RELEASE_FW_VERSION (0U)

/*EVK board pins, for testing without SVB*/
#define DEV_KIT_DEBUG_UART_RX (P5_0)
#define DEV_KIT_DEBUG_UART_TX (P5_1)
#define DEV_KIT_I2C_SCL       (P6_0)
#define DEV_KIT_I2C_SDA       (P6_1)
#define DEV_KIT_BLUE_LED      (P11_1)

/*MCU pins names according to SVB board schematic*/
#define     I3C_SCL_VALI    (P0_2)
#define     I3C_SDA_VALI    (P0_3)
#define     MCU_CLK         (P0_5)
#define     UART_RX_FTDI    (P1_0)
#define     UART_TX_FTDI    (P1_1)
#define     UART_TX_MCU     (P6_0)
#define     UART_RX_MCU     (P6_1)
#define     PWM_LED_RED     (P6_4)
#define     PWM_LED_GREEN   (P5_0)
#define     CAMERA_I2C_SCL  (P8_0)
#define     CAMERA_I2C_SDA  (P8_1)
#define     CAMERA_SYNC_MUX (P8_2)
#define     PWM_LED_BLUE    (P8_6)
#define     GPIO_3          (P10_1)
#define     GPIO_2          (P10_2)
#define     SPI_MOSI        (P11_0)
#define     SPI_MISO        (P11_1)
#define     SPI_CLK         (P11_2)
#define     SPI_CS_N        (P11_3)
#define     MCU_HS_MUX_OE_N (P11_4)
#define     MCU_HS_MUX_SEL  (P11_5)
#define     MCU_LS_MUX_EN   (P11_6)
#define     MCU_LS_MUX_SEL  (P11_7)

#define     PIN_LEVEL_LOW   (0UL)
#define     PIN_LEVEL_HIGH  (1UL)

/* Tasks priorities. configMAX_PRIORITIES is defined in the FreeRTOSConfig.h
 * Higher priority numbers denote high priority tasks,configMAX_PRIORITIES=7*/
#define TASK_CLI_PRIORITY       (configMAX_PRIORITIES - 1)

#define TASK_CLI_STACK_SIZE (512u)

#define LSM6DS_I2C_ADDR     (0x6A)
#define PWM_LED_FREQ_HZ     (1000000u)  /* in Hz */

/**
* @def    CHECK_AND_RETURN
* @brief  Macro for function return code checking.
*/
#define CHECK_AND_RETURN(rc, expected, return_code) \
                        do {                        \
                            if (rc != expected) {   \
                                return return_code; \
                            }                       \
                        } while(0)


typedef enum ret_code
{
    RET_SUCCESS = 0,
    RET_INVALID_PARAMS,
    RET_FAILS
} ret_code_t;


#endif /* GLOBALS_H_ */
