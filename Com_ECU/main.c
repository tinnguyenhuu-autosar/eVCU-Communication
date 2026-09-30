/**********************************************************
 * @file    main.c
 * @brief   COM_ECU Application - AUTOSAR Classic COM
 *
 * @details
 * COM_ECU thực hiện:
 *
 *   RX:
 *     VehicleCommand : CAN 0x180
 *     BrakeCommand   : CAN 0x280
 *     BodyCommand    : CAN 0x380
 *
 *   TX:
 *     EngineStatus   : CAN 0x181
 *
 * Communication flow:
 *
 *   RX:
 *     CAN -> CanIf -> PduR -> COM -> Application
 *
 *   TX:
 *     Application -> COM -> PduR -> CanIf -> CAN
 **********************************************************/

/* ===== AUTOSAR BSW Includes ===== */
#include "Std_Types.h"
#include "ComStack_Types.h"

#include "Can.h"
#include "CanIf.h"

#include "PduR.h"
#include "PduR_Cfg.h"

#include "Com.h"
#include "Com_Cfg.h"


/* ===========================================================
 * delay_ms
 * -----------------------------------------------------------
 * Simple Delay.
 * ===========================================================*/

static void delay_ms(volatile uint32 ms)
{
    while (ms--)
    {
        for (volatile uint32 i = 0u; i < 7200u; i++)
        {
            __asm__("nop");
        }
    }
}


/* ===========================================================
 * BSW_Init
 * -----------------------------------------------------------
 * Khởi tạo Communication Stack theo thứ tự:
 *
 *   1. CAN Driver
 *   2. CanIf
 *   3. PduR
 *   4. COM
 * ===========================================================*/

static void BSW_Init(void)
{
    /* 1. MCAL - CAN Driver */
    Can_Init(&Can_Cfg);

    /* 2. ECU Abstraction - CAN Interface */
    CanIf_Init(&CanIf_Config);

    /* 3. Service - PDU Router */
    PduR_Init(&PduR_ConfigPB);

    /* 4. Service - COM */
    Com_Init(&Com_Config);
}


/* ===========================================================
 * Demo_CAN_VehicleCmd
 * -----------------------------------------------------------
 * VehicleCommand
 *
 * CAN ID : 0x180
 * DLC    : 8
 *
 * eVCU -> COM_ECU
 *
 * Byte 0 : ThrottleReq
 * Byte 1 : EngineStartReq bit 0
 * Byte 2 : TorqueLimit
 * Byte 3 : Alive [3:0]
 *          CRC   [7:4]
 * ===========================================================*/

static void Demo_CAN_VehicleCmd(void)
{
    uint8 throttleReq;
    boolean engineStartReq;
    uint8 torqueLimit;
    uint8 alive;
    uint8 crc;

    /* ThrottleReq */
    (void)Com_ReceiveSignal(ComSig_Vehicle_Throttle, &throttleReq);

    /* EngineStartReq */
    (void)Com_ReceiveSignal(ComSig_Vehicle_Start, &engineStartReq);

    /* TorqueLimit */
    (void)Com_ReceiveSignal(ComSig_Vehicle_TorqueLimit, &torqueLimit);

    /* Alive */
    (void)Com_ReceiveSignal(ComSig_Vehicle_Alive, &alive);

    /* CRC */
    (void)Com_ReceiveSignal(ComSig_Vehicle_CRC, &crc);

    /*
     * TODO:
     * Sử dụng các giá trị trên trong application
     * điều khiển động cơ nếu cần.
     */

    (void)throttleReq;
    (void)engineStartReq;
    (void)torqueLimit;
    (void)alive;
    (void)crc;
}


/* ===========================================================
 * Demo_CAN_BrakeCmd
 * -----------------------------------------------------------
 * BrakeCommand
 *
 * CAN ID : 0x280
 * DLC    : 8
 *
 * eVCU -> COM_ECU
 *
 * Byte 0 : BrakeReq
 * Byte 1 : RegenReq
 * Byte 2 : Alive [3:0]
 *          CRC   [7:4]
 *
 * Optional PDU
 * ===========================================================*/

static void Demo_CAN_BrakeCmd(void)
{
    uint8 brakeReq;
    uint8 regenReq;
    uint8 alive;
    uint8 crc;

    /* BrakeReq */
    (void)Com_ReceiveSignal(ComSig_Brake_BrakeReq, &brakeReq);

    /* RegenReq */
    (void)Com_ReceiveSignal(ComSig_Brake_RegenReq, &regenReq);

    /* Alive */
    (void)Com_ReceiveSignal(ComSig_Brake_Alive, &alive);

    /* CRC */
    (void)Com_ReceiveSignal(ComSig_Brake_CRC, &crc);

    /*
     * TODO:
     * Sử dụng BrakeReq / RegenReq trong application
     * nếu cần.
     */

    (void)brakeReq;
    (void)regenReq;
    (void)alive;
    (void)crc;
}


/* ===========================================================
 * Demo_CAN_BodyCmd
 * -----------------------------------------------------------
 * BodyCommand
 *
 * CAN ID : 0x380
 * DLC    : 8
 *
 * eVCU -> COM_ECU
 *
 * Byte 0:
 *   bit 0 : HeadLamp
 *   bit 1 : TurnL
 *   bit 2 : TurnR
 *   bit 3 : DoorLock
 *
 * Optional PDU
 * ===========================================================*/

static void Demo_CAN_BodyCmd(void)
{
    boolean headLamp;
    boolean turnL;
    boolean turnR;
    boolean doorLock;

    /* HeadLamp */
    (void)Com_ReceiveSignal(ComSig_Body_HeadLamp, &headLamp);

    /* Turn Left */
    (void)Com_ReceiveSignal(ComSig_Body_TurnL, &turnL);

    /* Turn Right */
    (void)Com_ReceiveSignal(ComSig_Body_TurnR, &turnR);

    /* Door Lock */
    (void)Com_ReceiveSignal(ComSig_Body_DoorLock, &doorLock);

    /*
     * TODO:
     * Sử dụng các trạng thái Body trong application
     * nếu cần.
     */

    (void)headLamp;
    (void)turnL;
    (void)turnR;
    (void)doorLock;
}


/* ===========================================================
 * Demo_CAN_EngineStatus
 * -----------------------------------------------------------
 * EngineStatus
 *
 * CAN ID : 0x181
 * DLC    : 8
 *
 * COM_ECU -> eVCU
 *
 * Byte 0-1 : Engine_RPM
 * Byte 2   : Engine_Temp
 * Byte 3   : Engine_TorqueActual
 * Byte 4   : Engine_State
 * Byte 5   : Alive [3:0]
 *            CRC   [7:4]
 * Byte 6-7 : Reserved
 *
 * Flow:
 *
 *   Com_SendSignal()
 *       ->
 *   Com_TriggerIPDUSend()
 *       ->
 *   PduR
 *       ->
 *   CanIf
 *       ->
 *   CAN
 * ===========================================================*/

static void Demo_CAN_EngineStatus(void)
{
    uint16 engineRpm;
    uint8 engineTemp;
    uint8 engineTorqueActual;
    uint8 engineState;

    static uint8 alive = 0u;
    uint8 crc;

    /*
     * Giá trị demo.
     *
     * Đây là dữ liệu mẫu để kiểm tra communication path.
     */

    engineRpm = 1500u;
    engineTemp = 90u;
    engineTorqueActual = 100u;
    engineState = 1u;

    /*
     * Alive Counter:
     * 4-bit, giá trị 0..15.
     */
    alive = (uint8)((alive + 1u) & 0x0Fu);

    /*
     * Com_Cfg hiện chỉ quy định CRC là signal
     * 4-bit tại byte 5 bit [4..7].
     *
     * Chưa có thuật toán CRC cụ thể trong các file
     * configuration được cung cấp.
     *
     * Giữ cách tính XOR đơn giản giống main.c mẫu.
     */
    crc = (uint8)(
        (engineRpm & 0xFFu) ^
        ((engineRpm >> 8u) & 0xFFu) ^
        engineTemp ^
        engineTorqueActual ^
        engineState ^
        alive
    );

    crc &= 0x0Fu;


    /* ===== Send EngineStatus signals ===== */

    (void)Com_SendSignal(ComSig_Engine_RPM, &engineRpm);

    (void)Com_SendSignal(ComSig_Engine_Temp, &engineTemp);

    (void)Com_SendSignal(ComSig_Engine_TorqueActual, &engineTorqueActual);

    (void)Com_SendSignal(ComSig_Engine_State, &engineState);

    (void)Com_SendSignal(ComSig_Engine_Alive, &alive);

    (void)Com_SendSignal(ComSig_Engine_CRC, &crc);


    /*
     * Trigger EngineStatus I-PDU
     * xuống PduR -> CanIf -> CAN.
     */
    (void)Com_TriggerIPDUSend(ComConf_ComIPdu_EngineStatus);
}


/* ===========================================================
 * main
 * ===========================================================*/

int main(void)
{
    /*
     * Khởi tạo AUTOSAR Communication Stack
     */
    BSW_Init();


    /*
     * Main loop
     */
    while (1)
    {
        /*
         * ---------------------------------------------------
         * CAN RX
         * ---------------------------------------------------
         *
         * Poll CAN Driver.
         *
         * Khi có CAN frame:
         *
         * Can_MainFunction_Read()
         *      ->
         * CanIf_RxIndication()
         *      ->
         * PduR
         *      ->
         * Com_RxIndication()
         */
        Can_MainFunction_Read();


        /*
         * ---------------------------------------------------
         * Application RX
         * ---------------------------------------------------
         */

        /* Core requirement */
        Demo_CAN_VehicleCmd();

        /* Optional */
        Demo_CAN_BrakeCmd();

        /* Optional */
        Demo_CAN_BodyCmd();


        /*
         * ---------------------------------------------------
         * Application TX
         * ---------------------------------------------------
         */

        Demo_CAN_EngineStatus();


        /*
         * ---------------------------------------------------
         * CAN TX confirmation
         * ---------------------------------------------------
         *
         * Can Driver kiểm tra TX mailbox.
         */
        Can_MainFunction_Write();


        /*
         * Delay giữa các chu kỳ.
         */
        delay_ms(100u);
    }


    return 0;
}