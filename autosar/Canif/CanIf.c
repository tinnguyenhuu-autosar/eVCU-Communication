/**
 * @file CanIf.c
 * @brief AUTOSAR CAN Interface (CanIf) - Triển khai TX path
 *
 * @details Tầng ECU Abstraction kết nối PduR (phía trên) với
 *          CAN Driver (phía dưới). Chịu trách nhiệm:
 *          1. Chuyển đổi dữ liệu:
 *              PduInfoType (AUTOSAR chung) -> Can_PduType (CAN riêng)
 *
 *          2. Tra bảng cấu hình:
 *              CanIf_TxPduCfg[] map CanIfTxPduId -> CAN ID + HTH + DLc
 *
 *          3. Callback ngược:
 *              Can Driver báo TX xong -> CanIf -> PduR -> COM
 */

#include "CanIf.h"
#include "Can.h"
#include "PduR_CanIf.h"

#ifdef CANIF_USE_CANTP
#include "CanTp.h"
#endif

#include <stdint.h>
#include <string.h>

static const CanIf_ConfigType* CanIf_ConfigPtr = NULL;
static boolean s_inited = FALSE;

/**
 * prv_find_txpdu - Tìm index TX PDU trong bảng cấu hình
 */
static const CanIf_TxPduCfgType* prv_find_txpdu(PduIdType id)
{
    for (uint16 i = 0u; i < CANIF_NUM_TX_PDUS; ++i)
        {
            if (CanIf_ConfigPtr->TxPduCfg[i].CanIfTxPduId == id)
            {
                return &CanIf_ConfigPtr->TxPduCfg[i];
            }

        }
    return NULL;
}

/**
 * prv_find_rxpdu - Tìm index RX PDU trong bảng cấu hình
 */
static const CanIf_RxPduCfgType* prv_find_rxpdu(Can_IdType CanId)
{
    for (uint16 i = 0u; i < CANIF_NUM_RX_PDUS; ++i)
        {
            if (CanIf_ConfigPtr->RxPduCfg[i].CanId == CanId)
            {
                return &CanIf_ConfigPtr->RxPduCfg[i];
            }

        }
    return NULL;
}

/**
 * CanIf_Init
 */
 void CanIf_Init(const CanIf_ConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL)
    {
        CanIf_ConfigPtr = NULL;
        s_inited = FALSE;
        return;
    }

    CanIf_ConfigPtr = ConfigPtr;
    s_inited = TRUE;

}

/**
 * CanIf_Transmit - Pdu gọi: gửi I-PDU qua Can bus
 */
Std_ReturnType CanIf_Transmit(PduIdType CanIfTxPduId, const PduInfoType* PduInfoPtr)
{
    const CanIf_TxPduCfgType* cfg;
    Can_PduType CanPdu;

    /*Bước 1: Kiểm tra điều kiện*/
    if(s_inited == FALSE)
    {
    return E_NOT_OK;
    }

    /*Kiểm tra PDU*/
    if((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return E_NOT_OK;
    }

    /*Bước 2: Tra bảng TX PDU config*/
    cfg = prv_find_txpdu(CanIfTxPduId);

    if(cfg == NULL)
    {
        return E_NOT_OK;
    }

    /*Bước 3: Kiểm tra DLC không vượt config*/
    if(PduInfoPtr->SduLength > cfg->DlcMax)
    {
        return E_NOT_OK;
    }

    /*Bước 4: Dựng Can_PduType cho CAN driver*/
    Can_PduType frame;
    frame.swPduHandle   = CanIfTxPduId;
    frame.length        = (uint8)PduInfoPtr->SduLength;
    frame.id            = cfg->CanId;
    frame.sdu           = (uint8*)PduInfoPtr->SduDataPtr;

    /*Bước 5: Gọi xuống Can Driver (MCAL)*/
    Can_ReturnType rc = Can_Write(cfg->Hth, &frame);
    return (rc == CAN_OK) ? E_OK : E_NOT_OK;
}

/**
 * CanIf_TxConfirmation
 */
void CanIf_TxConfirmation(PduIdType CanTxPduId)
{
    if((s_inited == FALSE) || (prv_find_txpdu(CanTxPduId) == NULL)) {return;}

    #ifdef CANIF_USE_CANTP
    /**
     * Diagnostic TX confirmation.
     */
    if((CanTxPduId == CanIfConf_Pdu_DiagRequest) || (CanTxPduId == CanIfConf_Pdu_DiagFunctional))
    {
        CanTp_TxConfirmation(CanTxPduId);
        return;
    }
    #endif

    /**
     * Normal COM PDU confirmation.
     */
    PduR_CanIfTxConfirmation(CanTxPduId);
}

/**
 * CanIf_TriggerTransmit – CAN Driver yêu cầu dữ liệu (pull)
 * Trong một số cấu hình AUTOSAR, CAN Driver không nhận data
 * trực tiếp từ Can_Write() mà "kéo" (pull) data khi cần.
 *
 * Luồng: CAN Driver → CanIf → PduR → COM (copy shadow buffer)
*/
Std_ReturnType CanIf_TriggerTransmit(PduIdType CanTxPduId, PduInfoType* PduInfoPtr)
{
    return PduR_CanIfTriggerTransmit(CanTxPduId, PduInfoPtr);
}

/**
 * CanIf_RxIndication
 */
#define CANIF_RX_DEBUG_BUFFER_SIZE 10u

static volatile Can_IdType CanIf_LastRxIds[CANIF_RX_DEBUG_BUFFER_SIZE] = {0};
static volatile uint32 CanIf_RxIdIndex = 0u;

void CanIf_RxIndication(Can_IdType CanId, const PduInfoType* PduInfoPtr)
{
    const CanIf_RxPduCfgType* cfg;
    /*Kiểm tra CanIf đã được khởi tạo chưa*/
    if(s_inited == FALSE) return;

    /*Kiểm tra PDU pointer*/
    if((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL)) return;

    /*Lưu CAN ID nhận được để debug*/
    CanIf_LastRxIds[CanIf_RxIdIndex % CANIF_RX_DEBUG_BUFFER_SIZE] = CanId;
    CanIf_RxIdIndex++;

    /*Tìm RX PDU configuation theo CAN ID*/
    cfg = prv_find_rxpdu(CanId);
    if(cfg == NULL) return;

    /*Kiểm tra chiều dài dữ liệu*/
    if(PduInfoPtr->SduLength > cfg->DlcMax)
    {
        return;
    }

    if(cfg->Dest == CANIF_RX_DEST_CANTP)
    {
    #ifdef CANIF_USE_CANTP
        extern void CanTp_RxIndication(PduIdType CanTpRxPduId, const PduInfoType* PduInfoPtr);

        CanTp_RxIndication(cfg->DestPduId, PduInfoPtr);
    #endif
    }
    else
    {
        PduR_CanIfRxIndication(cfg->DestPduId, PduInfoPtr);
    }

}


