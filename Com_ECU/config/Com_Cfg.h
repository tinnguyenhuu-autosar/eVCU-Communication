/**
 * @file Com_Cfg.h
 * @brief AUTOSAR COM - Configuration for COM over CAN
 *
 * @details
 * I-PDU:
 *   - VehicleCommand : CAN ID 0x180, eVCU -> COM ECU
 *   - EngineStatus   : CAN ID 0x181, COM ECU -> eVCU
 *
 * Signal:
 *   - VehicleCommand: ThrottleReq, EngineStartReq, TorqueLimit,
 *                     Alive, CRC
 *   - EngineStatus  : Engine_RPM, Engine_Temp, Engine_TorqueActual,
 *                     Engine_State, Alive, CRC
 */
#ifndef COM_CFG_H
#define COM_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "Std_types.h"
#include "Comstack_Types.h"

/*Số lượng */
#define COM_NUM_IPDUS       (4u)
#define COM_NUM_SIGNALS     (19u)
#define COM_MAX_IPDU_LEN    (8u)

    /* I-PDU Direction*/
    typedef enum
    {
        COM_PDU_DIR_TX = 0u,
        COM_PDU_DIR_RX = 1u
    } Com_PduDirection_e;

    /* Signal Type*/
    typedef enum
    {
        COM_SIGTYPE_UINT8 = 0u,
        COM_SIGTYPE_UINT16 = 1u,
        COM_SIGTYPE_BOOLEAN = 2u
    } Com_SignalType_e;

    typedef uint16 Com_SignalIdType; /*Signal ID type (0..COM_NUM_SIGNALS-1)*/

    /*I-PDU IDs (CAN)*/
    enum
    {
        /* eVCU => COM ECU*/
        ComConf_ComIPdu_VehicleCmd = 0u, /*0x180 DLC=8*/
        ComConf_ComIPdu_BrakeCmd = 1u,  /*0x280 DLC=8*/
        ComConf_ComIPdu_BodyCmd = 2u,   /*0x380 DLC=8*/

        /* COM ECU => eVCU*/
        ComConf_ComIPdu_EngineStatus = 3u, /*CAN 0x181 DLC=8*/
    };

    /*Signal IDs*/
    enum
    {
        /* VehicleCmd (CAN 0x180) DLC=8 -Rx*/
        ComSig_Vehicle_Throttle    = 0u,
        ComSig_Vehicle_Start       = 1u,
        ComSig_Vehicle_TorqueLimit = 2u,
        ComSig_Vehicle_Alive       = 3u,
        ComSig_Vehicle_CRC         = 4u,

        /*EngineStatus (CAN 0x181) DLC=8 - Tx*/
        ComSig_Engine_RPM = 5u,
        ComSig_Engine_Temp = 6u,
        ComSig_Engine_TorqueActual = 7u,
        ComSig_Engine_State = 8u,
        ComSig_Engine_Alive = 9u,
        ComSig_Engine_CRC = 10u,

        /*BrakeCmd (CAN 0x280) DLC=8 -Rx*/
        ComSig_Brake_BrakeReq = 11u,
        ComSig_Brake_RegenReq = 12u,
        ComSig_Brake_Alive = 13u,
        ComSig_Brake_CRC = 14u,

        /*BodyCmd (CAN 0x380) DLC=8 -Rx*/
        ComSig_Body_HeadLamp = 15u,
        ComSig_Body_TurnL = 16u,
        ComSig_Body_TurnR = 17u,
        ComSig_Body_DoorLock = 18u,

    };

    /* I-PDU Configuration*/
    typedef struct
    {
        PduIdType           PduId;
        PduLengthType       Length;
        Com_PduDirection_e  direction;
    } Com_IPduCfgType;

    /* Signal Configuration*/
    typedef struct
    {
        PduIdType           PduId;
        uint16              byteIndex;
        uint8               bitOffset;
        uint8               bitLength;
        Com_SignalType_e    type;
        Com_PduDirection_e  direction;
    } Com_SignalCfgType;

    /* Configuration Tables*/
    typedef struct
    {
        const Com_IPduCfgType* IPduCfg;
        uint16 NumIPdus;

        const Com_SignalCfgType* SignalCfg;
        uint16 Numsignals;
    } Com_ConfigType;

    extern const Com_ConfigType Com_Config;

#ifdef __cplusplus
}
#endif

#endif /*COM_CFG_H*/