/**
 * @file CanIf_Cfg.c
 * @brief Configuation definitions for CanIf PDU mapping.
 */
#include "CanIf_Cfg.h"

#ifdef EVCU_DIAG_ECU

const CanIf_TxPduCfgType CanIf_TxPduCfg[CANIF_NUM_TX_PDUS] =
{
    {.CanIfTxPduId = CanIfConf_Pdu_DiagRequest, .Hth = (Can_HwHandleType)0u, .CanId = (Can_IdType)0x7E0u, .DlcMax = 8u},
    {.CanIfTxPduId = CanIfConf_Pdu_DiagFunctional, .Hth = (Can_HwHandleType)1u, .CanId = (Can_IdType)0x7DFu, .DlcMax = 8u},
};

const CanIf_RxPduCfgType CanIf_RxPduCfg[CANIF_NUM_RX_PDUS] =
{
    {.CanIfRxPduId = CanIfConf_Pdu_DiagResponse, .Hrh = (Can_HwHandleType)2u, .CanId = (Can_IdType)0x7E8u, .DlcMax = 8u, .Dest = CANIF_RX_DEST_CANTP, .DestPduId = CanTpConf_Pdu_DiagResponse}
};

#else

const CanIf_TxPduCfgType CanIf_TxPduCfg[CANIF_NUM_TX_PDUS] = {
    {.CanIfTxPduId = CanIfConf_Pdu_VehicleCmd, .Hth = (Can_HwHandleType)0u, .CanId = (Can_IdType)0x180u, .DlcMax = 8u},
    {.CanIfTxPduId = CanIfConf_Pdu_BrakeCmd, .Hth = (Can_HwHandleType)1u, .CanId = (Can_IdType)0x280u, .DlcMax = 8u},
    {.CanIfTxPduId = CanIfConf_Pdu_BodyCmd, .Hth = (Can_HwHandleType)2u, .CanId = (Can_IdType)0x380u, .DlcMax = 8u}

};

const CanIf_RxPduCfgType CanIf_RxPduCfg[CANIF_NUM_RX_PDUS] = {
    {.CanIfRxPduId = CanIfConf_Pdu_EngineStatus, .Hrh = (Can_HwHandleType)3u, .CanId = (Can_IdType)0x181u, .DlcMax = 8u, .Dest = CANIF_RX_DEST_PDUR, .DestPduId = CanIfConf_Pdu_EngineStatus}

};

#endif

const CanIf_ConfigType CanIf_Config =
{
    .TxPduCfg = CanIf_TxPduCfg,
    .RxPduCfg = CanIf_RxPduCfg,
    .TxPduCount = CANIF_NUM_TX_PDUS,
    .RxPduCount = CANIF_NUM_RX_PDUS
};
