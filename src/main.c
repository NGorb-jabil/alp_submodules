// (c) Meta, Inc. and its affiliates. Confidential and proprietary.


#include "FreeRTOS.h"
#include "task.h"
#include "periphery.h"


int main(void)
{
    if (RET_SUCCESS != init_peripherials())
    {
        CY_ASSERT(0);
    }

    if (RET_SUCCESS != init_tasks())
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    vTaskStartScheduler();

/********************** Should never get here ***************************/
    for (;;)
    {
        /* Halt the CPU if RTOS scheduler exits */
        CY_ASSERT(0);
    }
}
