/**
 * @file Can_Cfg.h
 * @brief
 */
#ifndef CAN_CFG_H
#define CAN_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"

    typedef struct
    /*Cấu hình pin Gpio */
    {
        GPIO_TypeDef *Port;
        uint16_t Pin;
        GPIOSpeed_TypeDef Speed;
        GPIOMode_TypeDef Mode;
    } Can_GpioPinCfgType;

    typedef struct
    {
        /*Cấu hình Bit timing*/
        uint16_t Prescaler;
        uint8_t SJW;
        uint8_t BS1;
        uint8_t BS2;
    } Can_BitTimingCfgType;

    typedef struct
    {
        /*Cấu hình chức năng CAN*/
        FunctionalState TTCM;
        FunctionalState ABOM;
        FunctionalState AWUM;
        FunctionalState NART;
        FunctionalState RFLM;
        FunctionalState TXFP;
    } Can_FeatureCfgType;


    typedef struct
    /*Cấu hình phần cứng Can Controller*/
    {
        CAN_TypeDef *Instance;

        Can_GpioPinCfgType Tx;
        Can_GpioPinCfgType Rx;

        Can_BitTimingCfgType BitTiming;
        uint8_t CanMode;
        Can_FeatureCfgType Feature;
    } Can_ControllerCfgType;


    typedef struct
    {
        /*Cấu hình Filter*/
        uint8_t Number;
        uint8_t Scale;
        uint8_t Mode;
        uint16_t IdHigh;
        uint16_t IdLow;
        uint16_t MaskIdHigh;
        uint16_t MaskIdLow;
        FunctionalState Activation;
    } Can_FilterCfgType;

        typedef struct
    {
        Can_ControllerCfgType Controller;
        Can_FilterCfgType Filter;
    } Can_ConfigType;

    extern const Can_ConfigType Can_Cfg;


#ifdef __cplusplus
}
#endif

#endif /*CAN_CFG_H*/