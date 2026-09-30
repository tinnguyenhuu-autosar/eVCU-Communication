/**
 * @file    Com.h
 * @brief   AUTOSAR COM – API rút gọn cho truyền tín hiệu
 * @details COM thực hiện:
 *          - Quản lý Signal theo Com_Cfg
 *          - Pack/Unpack Signal vào I-PDU buffer
 *          - Truyền I-PDU xuống PduR
 *          - Nhận I-PDU từ PduR và unpack Signal
 */
#ifndef COM_H
#define COM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "Std_Types.h"      /* Std_ReturnType, boolean, E_OK/E_NOT_OK, uint8,... */
#include "ComStack_Types.h" /* PduIdType, PduInfoType */
#include "Com_Cfg.h"        /* Com_SignalIdType, thông số cấu hình, symbolic IDs */

/*
 * 0) Return codes cho dịch vụ COM (kiểu trả về uint8)
 *    Tên & ngữ nghĩa theo SWS COM. Giá trị số bên dưới được
 *    dùng phổ biến trong nhiều stack (giữ tương thích).
 */
#ifndef COM_SERVICE_NOT_AVAILABLE
/** Dịch vụ không khả dụng (ví dụ I-PDU group đang stopped, điều kiện không thỏa). */
#define COM_SERVICE_NOT_AVAILABLE ((uint8)0x80u)
#endif

#ifndef COM_BUSY
/** Tài nguyên bận (ví dụ TP-buffer đang khóa cho dữ liệu lớn). */
#define COM_BUSY ((uint8)0x81u)
#endif
    /* E_OK (0x00) / E_NOT_OK (0x01) đã có trong Std_Types.h. */

    /* 1) Lifecycle */

    /**
     * @brief   Khởi tạo COM module.
     * @details Thiết lập/clear các shadow buffers & biến nội bộ.
     *          Phải được gọi trước khi dùng mọi API COM khác.
     * @note    Việc bật truyền nhận (start I-PDU groups) do hệ thống quản lý.
     */
    void Com_Init(const Com_ConfigType* ConfigPtr);

    /*
     * 2) TX API
     */

    /**
     * @brief   Ghi một Signal vào shadow buffer của I-PDU theo cấu hình.
     *
     * @param   SignalId       ID tín hiệu (index trong Com_SignalCfg).
     * @param   SignalDataPtr  Con trỏ dữ liệu nguồn (đúng kiểu của Signal).
     *
     * @return  uint8
     *          - E_OK: dịch vụ chấp nhận, đã pack vào shadow buffer.
     *          - COM_SERVICE_NOT_AVAILABLE: dịch vụ không khả dụng
     *            (ID/hướng/bộ đệm không hợp lệ, I-PDU không TX, v.v.).
     *          - COM_BUSY: tài nguyên bận (ví dụ TP-buffer), tùy hệ thống.
     *
     * @note    Khi cấu hình TransferProperty=TRIGGERED với TxMode=DIRECT/MIXED,
     *          hệ thống có thể kích phát gửi ngay (tối đa ở main function kế tiếp),
     *          tùy MDT/TMS. Trong demo, có thể chủ động gọi Com_TriggerIPDUSend().
     */
    /**
     * @brief   Ghi một Signal vào shadow buffer của I-PDU theo cấu hình.
     * ...
     */
    uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);

    /**
     * @brief   Đọc một Signal từ shadow buffer của I-PDU.
     *
     * @param   SignalId       ID tín hiệu (index trong Com_SignalCfg).
     * @param   SignalDataPtr  Con trỏ bộ nhớ để lưu dữ liệu đọc ra.
     *
     * @return  uint8
     *          - E_OK: đã đọc thành công.
     *          - COM_SERVICE_NOT_AVAILABLE: ID/hướng không hợp lệ.
     */
    uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void *SignalDataPtr);

    /**
     * @brief   Kích phát gửi I-PDU (TX) ngay xuống PduR.
     *
     * @param   PduId  ID I-PDU (TX) cần gửi.
     *
     * @return  Std_ReturnType
     *          - E_OK: đã gọi xuống PduR thành công.
     *          - E_NOT_OK: I-PDU không hợp lệ/chưa sẵn sàng (ví dụ group stopped).
     *
     * @details Chữ ký & mã trả về theo SWS: API này dùng Std_ReturnType.
     */
    Std_ReturnType Com_TriggerIPDUSend(PduIdType PduId);

    /*
     * 3) (Tối giản) Liên kết PduR
     */
    /* Callback/callout để PduR/CanIf gọi ngược lên COM */
    /**
    * @brief PduR thông báo COM đã nhận được một I-PDU.
    *
    * PduR gọi API này sau khi nhận PDU từ CanIf.
    *
    * @param ComRxPduId  ID của I-PDU RX.
    * @param PduInfoPtr  Thông tin PDU nhận được.
    */
    void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType *PduInfoPtr);

    /**
    * @brief PduR thông báo TX hoàn tất.
    *
    * @param ComTxPduId ID của I-PDU TX.
    */
    void Com_TxConfirmation(PduIdType ComTxPduId);

    /**
    * @brief PduR yêu cầu COM cung cấp dữ liệu TX.
    *
    * Dùng trong trường hợp TriggerTransmit.
    *
    * @param ComTxPduId  ID của I-PDU TX.
    * @param PduInfoPtr  Buffer chứa dữ liệu cần truyền.
    *
    * @return
    *         E_OK     Dữ liệu được cung cấp.
    *         E_NOT_OK Không thể cung cấp dữ liệu.
    */
    Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, PduInfoType *PduInfoPtr);

    /**
    * @brief Main function xử lý transmission của COM.
    *
    * Có thể dùng để xử lý các I-PDU có transmission mode
    * yêu cầu polling/timing.
    */
    void Com_MainFunctionTx(void);


    /**
    * @brief Main function xử lý reception của COM.
    */
    void Com_MainFunctionRx(void);


#ifdef __cplusplus
}
#endif

#endif /* COM_H */
