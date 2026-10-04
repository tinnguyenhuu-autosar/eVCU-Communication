/**
 * @file PduR_Cfg.h
 * @brief Configuration of PDU Router
 * @details Định nghĩa bảng routing giữa các module.
 *
 * RX:
 *      CanIf => PduR => COM
 *
 * TX:
 *      COM => PduR => CanIf
 *
 * Diagnostic:
 *      CanIf => PduR => CanTp
 */
#ifndef PDUR_CFG_H
#define PDUR_CFG_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "PduR_Type.h"

/* Destination Module Type*/
typedef enum {
    PDUR_DEST_CANIF = 0u,
    PDUR_DEST_CANTP = 1u
} PduR_DestModuleType;

/* Route entry 1:1 với Destination Module*/
typedef struct {
    PduIdType           SrcPduId;       /*PDU ID nguồn (COM side)*/
    PduIdType           DstPduId;       /*PDU ID đích (CanIf side)*/
    PduR_DestModuleType DestModule;     /*Module đích: CANIF*/
} PduR_Route1to1Type;

/* CanIf callback routes*/
typedef struct{
    PduIdType SrcPduId;
    PduIdType DstPduId;
} PduR_CallbackRouteType;

/* Số lượng routes*/
#ifdef PDUR_USE_COM

#define PDUR_NUM_COM_TX_ROUTES          (1u)
#define PDUR_NUM_CANIF_RX_ROUTES        (3u)
#define PDUR_NUM_CANIF_TXCONF_ROUTES    (1u)
#define PDUR_NUM_CANIF_TRIGTX_ROUTES    (3u)
#define PDUR_NUM_DCM_ROUTES             (0u)

#else

#define PDUR_NUM_COM_TX_ROUTES          (0u)
#define PDUR_NUM_CANIF_RX_ROUTES        (0u)
#define PDUR_NUM_CANIF_TXCONF_ROUTES    (0u)
#define PDUR_NUM_CANIF_TRIGTX_ROUTES    (0u)
#define PDUR_NUM_DCM_ROUTES             (1u)

#endif

/* Route Table*/

/* COM => PduR => CanIf*/
#if PDUR_NUM_COM_TX_ROUTES > 0
extern const PduR_Route1to1Type PduR_ComTxRoutes[PDUR_NUM_COM_TX_ROUTES];
#endif

/* CanIf => PduR => COM*/
#if PDUR_NUM_CANIF_RX_ROUTES > 0
extern const PduR_CallbackRouteType PduR_CanIfRxRoutes[PDUR_NUM_CANIF_RX_ROUTES];
#endif

/* CanIf => PduR => COM (TX Confirmation)*/
#if PDUR_NUM_CANIF_TXCONF_ROUTES > 0
extern const PduR_CallbackRouteType PduR_CanIfTxConfRoutes[PDUR_NUM_CANIF_TXCONF_ROUTES];
#endif

/* CanIf => PduR => COM (Trigger Transmit)*/
#if PDUR_NUM_CANIF_TRIGTX_ROUTES > 0
extern const PduR_CallbackRouteType PduR_CanIfTrigTxRoutes[PDUR_NUM_CANIF_TRIGTX_ROUTES];
#endif

/** @brief Bảng cấu hình post-build của PduR */
extern const PduR_PBConfigType PduR_ConfigPB;

#ifdef __cplusplus
}
#endif

#endif /*PDUR_CFG_H*/