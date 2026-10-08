#include "Can.h"
#include "CanIf.h"
#include "PduR.h"
#include "CanTp.h"
#include "Dcm.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

#include <string.h>

extern const Can_ConfigType Can_Cfg;
extern const CanIf_ConfigType CanIf_Config;
extern const PduR_PBConfigType PduR_ConfigPB;

static void Log_Init(void)
{
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_USART1 |
        RCC_APB2Periph_GPIOA,
        ENABLE
    );

    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin   = GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &gpio);

    USART_InitTypeDef usart;

    usart.USART_BaudRate            = 115200;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Tx;

    USART_Init(USART1, &usart);

    USART_Cmd(USART1, ENABLE);
}

void Log_Print(const char* str)
{
    if (str == NULL)
    {
        return;
    }

    while (*str != '\0')
    {
        while (USART_GetFlagStatus(
            USART1,
            USART_FLAG_TXE) == RESET)
        {
        }

        USART_SendData(
            USART1,
            (uint16)(uint8)(*str)
        );

        str++;
    }
}

int main(void)
{
    Log_Init();
    /* BSW initialization */
    Can_Init(&Can_Cfg);
    CanIf_Init(&CanIf_Config);
    PduR_Init(&PduR_ConfigPB);
    CanTp_Init();
    Dcm_Init();

    while (1)
    {
        /* CAN MainFunction - 1 ms task */
        Can_MainFunction_Read();
        Can_MainFunction_Write();

        /* CAN TP MainFunction - 5 ms task */
        CanTp_MainFunction();
    }

    return 0;
}