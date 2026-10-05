/**
 * @file CanIf.h
 * @brief AUTOSAR CAN Interface (CanIf) - Header
 * @details Tầng ECU Abstraction trừ tượng có CAN Driver (MCAL) cho
 * các module phía trên (PduR/COM).
 */
#ifndef CANIF_H
#define CANIF_H

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "Can_GeneralTypes.h"

#include "CanIf_Cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Khởi tạo CanIf module
     * @details Set cờ CanIf_Initialized = TRUE.
     *          Production code sẽ load cấu hình, quản lý state controller.
     */
    void CanIf_Init(const CanIf_ConfigType* ConfigPtr);

    /**
     * @brief PduR gọi: yêu cầu gửi I-PDU qua CAN bus
     * @param CanIfTxPduId ID PDU phía CanIf (từ bảng route PduR)
     * @param PduInfoPtr Dữ liệu PDU (payload + length)
     * @return E_OK: CAN Driver chấp nhận, E_NOT_OK: lỗi
     *
     * @details Quy trình:
     *          1. Tra bảng CanIf_TxPduCfg -> tìm Can ID, HTH, DLC
     *          2. Dựng Can_PduType từ PduInfoType + config
     *          3. Gọi Can_Write(HTH, &canPdu) xuống CAN Driver
     */
    Std_ReturnType CanIf_Transmit(PduIdType CanIfTxPduId, const PduInfoType *PduInfoPtr);

    /**
     * @brief Callback từ CAN Driver: TX đã hoàn tất
     * @param CanTxPduId Handle đã truyền (swPduHandle trong Can_PduType)
     * @details CanIf chuyển tiếp callback lên PduR -> COM để thông báo
     *          I-PDU đã gửi thành công.
     */
    void CanIf_TxConfirmation(PduIdType CanTxPduId);

    /**
     * @brief Callback từ CAN Driver: RX đã hoàn tất
     * @param canId CAN ID của Frame nhận được
     * @param PduInfoPtr Thông tin Frame (payload + length)
     * @details Tra CAN ID để routing: Diagnostic -> CanTp, COM -> PduR
     */
    void CanIf_RxIndication(Can_IdType CanId, const PduInfoType *PduInfoPtr);

#ifdef __cplusplus
}
#endif

#endif /*CANIF_H*/