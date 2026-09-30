/**********************************************************
 * @file    Com_Cfg.c
 * @brief   AUTOSAR COM - Configuration tables
 * @details
 * CAN I-PDUs:
 *   - VehicleCmd   : CAN ID 0x180
 *   - BrakeCmd     : CAN ID 0x280
 *   - BodyCmd      : CAN ID 0x380
 *   - EngineStatus : CAN ID 0x181
 *
 * Routing:
 *
 *   RX:
 *      CAN -> CanIf -> PduR -> COM
 *
 *   TX:
 *      COM -> PduR -> CanIf -> CAN
 **********************************************************/
#include "Com_cfg.h"

/* I-PDU Configuration*/
const Com_IPduCfgType Com_IPduCfg[COM_NUM_IPDUS] =
{
    {.PduId = ComConf_ComIPdu_VehicleCmd, .Length = 8u, .direction = COM_PDU_DIR_RX},
    {.PduId = ComConf_ComIPdu_BrakeCmd, .Length = 8u, .direction = COM_PDU_DIR_RX},
    {.PduId = ComConf_ComIPdu_BodyCmd, .Length = 8u, .direction = COM_PDU_DIR_RX},

    {.PduId = ComConf_ComIPdu_EngineStatus, .Length = 8u, .direction = COM_PDU_DIR_TX}
};

/* Signal Configuration*/
const Com_SignalCfgType Com_SignalCfg[COM_NUM_SIGNALS] =
{
    /* ===== VehicleCmd (CAN 0x180) ===== */

    [ComSig_Vehicle_Throttle] = {.PduId = ComConf_ComIPdu_VehicleCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Vehicle_Start] = {.PduId = ComConf_ComIPdu_VehicleCmd, .byteIndex = 1u, .bitOffset = 0u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_RX},
    [ComSig_Vehicle_TorqueLimit] = {.PduId = ComConf_ComIPdu_VehicleCmd, .byteIndex = 2u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Vehicle_Alive] = {.PduId = ComConf_ComIPdu_VehicleCmd, .byteIndex = 3u, .bitOffset = 0u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Vehicle_CRC] = {.PduId = ComConf_ComIPdu_VehicleCmd, .byteIndex = 3u, .bitOffset = 4u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},

    /* ===== BrakeCmd (CAN 0x280) ===== */
    [ComSig_Brake_BrakeReq] = {.PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Brake_RegenReq] = {.PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 1u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Brake_Alive] = {.PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 2u, .bitOffset = 0u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},
    [ComSig_Brake_CRC] = {.PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 2u, .bitOffset = 4u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_RX},

    /* ===== BodyCmd (CAN 0x380) ===== */
    [ComSig_Body_HeadLamp] = {.PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_RX},
    [ComSig_Body_TurnL] = {.PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 1u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_RX},
    [ComSig_Body_TurnR] = {.PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 2u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_RX},
    [ComSig_Body_DoorLock] = {.PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 3u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_RX},

/* ===== EngineStatus (CAN 0x181) ===== */

    [ComSig_Engine_RPM] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 16u, .type = COM_SIGTYPE_UINT16, .direction = COM_PDU_DIR_TX},
    [ComSig_Engine_Temp] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 2u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_TX},
    [ComSig_Engine_TorqueActual] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 3u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_TX},
    [ComSig_Engine_State] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 4u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_TX},
    [ComSig_Engine_Alive] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 5u, .bitOffset = 0u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_TX},
    [ComSig_Engine_CRC] = {.PduId = ComConf_ComIPdu_EngineStatus, .byteIndex = 5u, .bitOffset = 4u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8, .direction = COM_PDU_DIR_TX}

};

const Com_ConfigType Com_Config =
{
    .IPduCfg = Com_IPduCfg,
    .NumIPdus = COM_NUM_IPDUS,

    .SignalCfg = Com_SignalCfg,
    .Numsignals = COM_NUM_SIGNALS
};