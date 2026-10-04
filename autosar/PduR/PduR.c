/**
 * @file PduR.c
 * @brief AUTOSAR PDU Router - COM <-> CanIf
 *
 * @details PduR định tuyến I-PDU giữa Upper Layer (COM) và Lower Layer
 *          CanIf dựa trên bảng route tĩnh.
 * TX (COM gửi xuống):
 *      COM → PduR_ComTransmit() → CanIf_Transmit()
 *
 * RX (callback lower báo lên):
 *      CanIf → PduR_CanIfTxConfirmation() → Com_TxConfirmation()
 */

#include "PduR.h"
#include "PduR_Com.h"
#include "PduR_CanIf.h"

#include "Std_Types.h"
#include "ComStack_Types.h"

#include <stddef.h>
#include <stdint.h>

/**
 *  EXTERN: Lower layers
 */
extern Std_ReturnType CanIf_Transmit(PduIdType CanIfTxPduId, const PduInfoType* PduInfoPtr);

/**
 * EXTERN: Upper layer (COM) callback
 */
extern void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType* PduInfoPtr);
extern void Com_TxConfirmation(PduIdType ComTxPduId);
extern Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, PduInfoType* PduInfoPtr);

/**
 * Trạng thái nội bộ
 */
static PduR_StateType s_State = PDUR_UNINIT;
static const PduR_PBConfigType* s_Cfg = NULL;
static boolean s_RoutingEnabled = FALSE;

/* DET hook (stub) */
#define PDUR_DET_REPORT(apiid, errcode)   ((void)0)

/* API IDs */
#define PDUR_API_INIT          ((uint8)0x00)
#define PDUR_API_COM_TRANSMIT  ((uint8)0x10)
#define PDUR_API_CANIF_RX_IND  ((uint8)0x20)
#define PDUR_API_CANIF_TX_CONF ((uint8)0x21)
#define PDUR_API_CANIF_TRIG_TX ((uint8)0x22)


/* =========================================================
 * Helper: tìm route COM TX
 * =======================================================*/
static int32 prv_find_com_tx_route(PduIdType src)
{
    const PduR_Route1to1Type* tbl = (const PduR_Route1to1Type*)s_Cfg->ComTxRoutingTable;
    for (uint16 i = 0; i < PDUR_NUM_COM_TX_ROUTES; ++i)
    {
        if (tbl[i].SrcPduId == src)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

/* =========================================================
 * Helper: tìm callback route
 * =======================================================*/
static inline int32_t prv_find_callback_route(const PduR_CallbackRouteType* tbl, uint16 n, PduIdType src)
{
    for (uint16 i = 0; i < n; ++i)
    {
        if (tbl[i].SrcPduId == src)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

/* =========================================================
 * Lifecycle
 * =======================================================*/
void PduR_Init(const PduR_PBConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL)
    {
        s_State = PDUR_UNINIT;
        s_Cfg = NULL;
        s_RoutingEnabled = FALSE;
        return;
    }

    s_Cfg = ConfigPtr;
    s_State = PDUR_ONLINE;
    s_RoutingEnabled = TRUE;
}

/* =========================================================
 * Enable / Disable routing
 * =======================================================*/
Std_ReturnType PduR_EnableRouting(PduR_RoutingPathGroupIdType id)
{
    (void)id;

    if (s_State == PDUR_UNINIT)
    {
        return E_NOT_OK;
    }

    s_RoutingEnabled = TRUE;

    return E_OK;
}

Std_ReturnType PduR_DisableRouting(PduR_RoutingPathGroupIdType id)
{
    (void)id;

    if (s_State == PDUR_UNINIT)
    {
        return E_NOT_OK;
    }

    s_RoutingEnabled = FALSE;

    return E_OK;
}

PduR_StateType PduR_GetState(void)
{
    return s_State;
}

/* =========================================================
 * COM -> PduR -> CanIf
 * =======================================================*/

#ifdef PDUR_USE_COM

 Std_ReturnType PduR_ComTransmit(PduIdType ComTxPduId, const PduInfoType* PduInfoPtr)
{

    if (s_State == PDUR_UNINIT)
    {
        PDUR_DET_REPORT(PDUR_API_COM_TRANSMIT,PDUR_E_UNINIT);
        return E_NOT_OK;
    }

    if (!s_RoutingEnabled)
    {
        return E_NOT_OK;
    }

    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return E_NOT_OK;
    }

    if ((s_Cfg == NULL) || (s_Cfg->ComTxRoutingTable == NULL))
    {
        return E_NOT_OK;
    }

    int32_t idx = prv_find_com_tx_route(ComTxPduId);

    if (idx < 0)
    {
        PDUR_DET_REPORT(PDUR_API_COM_TRANSMIT, PDUR_E_PDU_ID_INVALID);

        return E_NOT_OK;
    }

    const PduR_Route1to1Type* tbl = (const PduR_Route1to1Type*)s_Cfg->ComTxRoutingTable;
    const PduIdType           dstId = tbl[idx].DstPduId;
    const PduR_DestModuleType dest  = tbl[idx].DestModule;

    /* Dispatch theo DestModule */
    switch (dest)
    {
        case PDUR_DEST_CANIF:
            return CanIf_Transmit(dstId, PduInfoPtr);

        default:
            return E_NOT_OK;
    }
}


/* =========================================================
 * CanIf -> PduR -> COM : RX
 * =======================================================*/

 void PduR_CanIfRxIndication(PduIdType CanIfRxPduId, const PduInfoType* PduInfoPtr)
{

    if (s_State == PDUR_UNINIT || !s_RoutingEnabled)
    {
        return;
    }

    if (PduInfoPtr == NULL)
    {
        return;
    }

    if ((s_Cfg == NULL) || (s_Cfg->CanIfRxRoutingTable == NULL))
    {
        return;
    }

    /* Tra bảng route RX*/
    const PduR_CallbackRouteType* tbl = (const PduR_CallbackRouteType*)s_Cfg->CanIfRxRoutingTable;

    int32_t idx = prv_find_callback_route(tbl, PDUR_NUM_CANIF_RX_ROUTES, CanIfRxPduId);

    if (idx >= 0)
    {
        Com_RxIndication(tbl[idx].DstPduId, PduInfoPtr);
    }
}

/* =========================================================
 * CanIf -> PduR -> COM : TX Confirmation
 * =======================================================*/


 void PduR_CanIfTxConfirmation(PduIdType CanIfTxPduId)
{

    if (s_State == PDUR_UNINIT || !s_RoutingEnabled)
    {
        return;
    }

    if ((s_Cfg == NULL) || (s_Cfg->CanIfTxConfRoutingTable == NULL))
    {
        return;
    }

    int32_t idx = prv_find_callback_route(PduR_CanIfTxConfRoutes, PDUR_NUM_CANIF_TXCONF_ROUTES, CanIfTxPduId);

    if (idx >= 0)
    {
        Com_TxConfirmation(PduR_CanIfTxConfRoutes[idx].DstPduId);
    }
}

/* =========================================================
 * CanIf -> PduR -> COM : TriggerTransmit
 * =======================================================*/


 Std_ReturnType PduR_CanIfTriggerTransmit(PduIdType CanIfTxPduId, PduInfoType* PduInfoPtr)
{
    if (s_State == PDUR_UNINIT || !s_RoutingEnabled)
    {
        return E_NOT_OK;
    }

    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return E_NOT_OK;
    }

    if ((s_Cfg == NULL) || (s_Cfg->CanIfTrigTxRoutingTable == NULL))
    {
        return E_NOT_OK;
    }

    int32_t idx = prv_find_callback_route(PduR_CanIfTrigTxRoutes, PDUR_NUM_CANIF_TRIGTX_ROUTES, CanIfTxPduId);

    if (idx < 0)
    {
        return E_NOT_OK;
    }

    return Com_TriggerTransmit(
        PduR_CanIfTrigTxRoutes[idx].DstPduId, PduInfoPtr);
}
#endif

#ifdef PDUR_USE_CANTP
/* =========================================================
 * CanTp → PduR → (COM / Dcm)
 * =======================================================*/
#include "PduR_CanTp.h"
#include "../Dcm/Dcm.h"

void PduR_CanTpRxIndication(PduIdType RxPduId, NotifResultType Result)
{
    /* Định tuyến tới bộ định tuyến đích thực tế. Ở đây demo gọi Dcm */
    Dcm_TpRxIndication(RxPduId, Result);
}

BufReq_ReturnType PduR_CanTpStartOfReception(PduIdType id, const PduInfoType* info,
PduLengthType TpSduLength, PduLengthType* bufferSizePtr)
{
    /* Định tuyến gọi thẳng sang Dcm_StartOfReception */
    return Dcm_StartOfReception(id, info, TpSduLength, bufferSizePtr);
}

BufReq_ReturnType PduR_CanTpCopyRxData(PduIdType id, const PduInfoType* info,
PduLengthType* bufferSizePtr)
{
    /* Định tuyến gọi sang Dcm_CopyRxData */
    return Dcm_CopyRxData(id, info, bufferSizePtr);
}

BufReq_ReturnType PduR_CanTpCopyTxData(PduIdType id, const PduInfoType* info,
PduLengthType* availableDataPtr)
{
    /* Định tuyến gọi sang Dcm_CopyTxData */
    return Dcm_CopyTxData(id, info, availableDataPtr);
}

void PduR_CanTpTxConfirmation(PduIdType TxPduId, NotifResultType Result)
{
    /* Định tuyến sang Dcm_TpTxConfirmation */
    Dcm_TxConfirmation(TxPduId, Result);
}

Std_ReturnType PduR_DcmTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr)
{
    if(PduInfoPtr == NULL || PduInfoPtr->SduDataPtr == NULL)
    {
        return E_NOT_OK;
    }

    return CanTp_Transmit(TxPduId, PduInfoPtr);
}

#endif