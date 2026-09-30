/**
 * @file Can_Cfg.c
 */

#include "Can_Cfg.h"
const Can_ConfigType Can_Cfg =
    {
        .Controller =
        {
        .Instance = CAN1,

        .CanMode = CAN_Mode_Normal,

        .Tx =
            {
                .Port = GPIOA,
                .Pin = GPIO_Pin_12,
                .Speed = GPIO_Speed_50MHz,
                .Mode = GPIO_Mode_AF_PP
            },

        .Rx =
            {
                .Port = GPIOA,
                .Pin = GPIO_Pin_11,
                .Speed = GPIO_Speed_50MHz,
                .Mode = GPIO_Mode_IN_FLOATING
            },

        .BitTiming =
            {
                .Prescaler = 4u,
                .BS1 = CAN_BS1_11tq,
                .BS2 = CAN_BS2_6tq,
                .SJW = CAN_SJW_1tq
            },

        .Feature =
            {
                .TTCM = DISABLE,
                .ABOM = ENABLE,
                .AWUM = DISABLE,
                .NART = DISABLE,
                .RFLM = DISABLE,
                .TXFP = DISABLE
            },

        },

        .Filter =
        {
            .Number = 0u,
            .Scale = CAN_FilterScale_32bit,
            .Mode = CAN_FilterMode_IdMask,
            .IdHigh = 0x0000,
            .IdLow = 0x0000,
            .MaskIdHigh = 0x0000,
            .MaskIdLow = 0x0000,
            .Activation = ENABLE
        }

    };
