/**
 * @file PduR_CanIf.h
 * @brief Đay là interface dành cho CanIf->PduR
 */
#ifndef PDUR_CANIF_H
#define PDUR_CANIF_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"

/**
 * @brief CanIf báo RX PDU lên PduR
 *
 * Luồng:
 * CAN Driver
 *     ↓
 * CanIf_RxIndication()
 *     ↓
 * PduR_CanIfRxIndication()
 *     ↓
 * PduR
 *     ↓
 * COM / CanTp
 *
 * @param CanIfRxPduId
 *        ID của RX PDU được CanIf xác định.
 *
 * @param PduInfoPtr
 *        Thông tin dữ liệu nhận được.
 */
void PduR_CanIfRxIndication(PduIdType CanIfRxPduId, const PduInfoType* PduInfoPtr);

/**
 * @brief CanIf báo Tx hoàn tất lên PduR
 *
 * Luồng
 * CAN Driver
 *     ↓
 * CanIf_TxConfirmation()
 *     ↓
 * PduR_CanIfTxConfirmation()
 *     ↓
 * PduR
 *     ↓
 * COM
 *
 * @param CanIfTxPduId
 *        ID của TX PDU vừa truyền thành công.
 */
void PduR_CanIfTxConfirmation(PduIdType CanIfTxPduId);

/* CanIf → PduR: Lower yêu cầu upper cấp dữ liệu để phát (pull model) */
Std_ReturnType PduR_CanIfTriggerTransmit(PduIdType CanIfTxPduId, PduInfoType* PduInfoPtr);


#ifdef __cplusplus
}
#endif
#endif /*PDUR_CANIF_H*/