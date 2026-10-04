/**
 * @file CanIf_Cfg.h
 * @brief Configuration definitions for CAN Interface PDU mapping.
 */


#ifndef CANIF_CFG_H
#define CANIF_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "Can_GeneralTypes.h"

#define CANIF_NUM_TX_PDUS (3u)
#define CANIF_NUM_RX_PDUS (1u)

enum{
    /* COM / eVCU*/
    CanIfConf_Pdu_VehicleCmd        = 0u,
    CanIfConf_Pdu_BrakeCmd          = 1u,
    CanIfConf_Pdu_BodyCmd           = 2u,

    CanIfConf_Pdu_EngineStatus      = 3u,

    /* Diag ECU*/
    CanIfConf_Pdu_DiagRequest       = 4u,
    CanIfConf_Pdu_DiagFunctional    = 5u,
    CanIfConf_Pdu_DiagResponse      = 6u
};

typedef struct
{
    PduIdType           CanIfTxPduId;
    Can_HwHandleType    Hth;
    Can_IdType          CanId;
    uint8               DlcMax;
} CanIf_TxPduCfgType;

typedef enum
{
    CANIF_RX_DEST_PDUR = 0u,
    CANIF_RX_DEST_CANTP = 1u
} CanIf_RxDestType;

typedef struct
{
    PduIdType           CanIfRxPduId;
    Can_HwHandleType    Hrh;
    Can_IdType          CanId;
    uint8               DlcMax;
    CanIf_RxDestType    Dest;
    PduIdType           DestPduId;
} CanIf_RxPduCfgType;

/** CanIf Configuration
 *
 * Config được truyền vào CanIf_Init()
 * CanIf.c không tự truy cập vào bảng cấu hình*/
typedef struct
{
    const CanIf_TxPduCfgType* TxPduCfg;
    const CanIf_RxPduCfgType* RxPduCfg;

    uint16 TxPduCount;
    uint16 RxPduCount;
} CanIf_ConfigType;

extern const CanIf_ConfigType CanIf_Config;

#ifdef __cplusplus
}
#endif
#endif /*CANIF_CFG_H*/