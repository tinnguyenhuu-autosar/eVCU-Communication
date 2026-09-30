/**
 * @file Can.h
 * @brief AUTOSAR CAN Driver - bxCAN nội bộ STM32F103
 *
 * @details Module MCAL (Microcontroller Abstraction Layer) cung cấp các
 *          hàm để giao tiếp với bộ điều khiển CAN (Controller Area Network) trên vi điều khiển STM32F103.
 *
 *           STM32F103 có bxCAN (Basic Extended CAN) tại địa chỉ
 *          0x40006400 với các đặc điểm:
 *            - Hỗ trợ CAN 2.0A (Standard 11-bit ID) và 2.0B (Extended 29-bit ID)
 *            - 3 mailbox truyền (TX Mailbox 0, 1, 2)
 *            - 2 FIFO nhận (FIFO0, FIFO1) mỗi FIFO 3 cấp
 *            - 14 bộ lọc (Filter Bank) có thể cấu hình
 *            - Tốc độ tối đa 1 Mbps
 *
 *          Chân phần cứng:
 *            PA11 = CAN_RX (nhận dữ liệu từ CAN bus)
 *            PA12 = CAN_TX (truyền dữ liệu lên CAN bus)
 *
 *          API cung cấp (chuẩn AUTOSAR):
 *            Can_Init()               : Cấu hình GPIO + bxCAN peripheral
 *            Can_Write()              : Gửi CAN frame qua bxCAN mailbox
 *            Can_MainFunction_Write() : Kiểm tra TX hoàn tất (polling)
 */

#ifndef CAN_H
#define CAN_H

#include "Can_Cfg.h"

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "Can_GeneralTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Khởi tạo CAN driver
     * @details Thực hiện các bước sau:
     *          1. Baajtclock cho GPIOA, AFIO, CAN1
     *          2. Cấu hình GPIO: PA12=CAN_TX (AF Push-Pull), PA11=CAN_RX (Input Floating)
     *          3. Cấu hình bxCAN peripheral: tốc độ 500kbps, LoopBack mode
     *          4. Cấu hình bộ lọc: Filter 0 chấp nhận tất cả CAN ID
     *          5. Clear các biến trạng thái TX/RX
     *
     * @pre Hệ thống clock (RCC) đã được cấu hình (SystemInit)
     * @post CAN Driver đã sẵn sàng nhận lệnh Can_Write()
     */
    void Can_Init(const Can_ConfigType* ConfigPtr);

    /**
     * @brief gửi 1 CAN PDU qua bxCAN
     * @param Hth Hardware Transmit Handle - chọn slot TX (0..2)
     * @param PduInfo Con trỏ tới cấu trúc Can_PduType chứa:
     *              - id: CAN ID (Standard hoặc Extended)
     *              - length: DLC (0..8 byte)
     *              - sdu: con trỏ tới dữ liệu (0..8 byte)
     *              - swPduHandle: Handle để callback xác nhận
     * @return CAN_OK: nếu đã nạp vào mailbox, đang truyền
     *         CAN_BUSY: nếu mailbox đang bận, chưa nạp được
     *         CAN_NOT_OK: nếu tham số không hợp lệ (id, length, sdu)
     */
    Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType *PduInfo);

    /**
     * @brief Hàm polling kiểm tra TX hoàn tất
     * @details Hàm này nên được gọi định kỳ trong vòng lặp chính (main loop) or OS task.
     *          - Kiểm tra trạng thái mailbox TX:
     *          - Nếu TX gửi thành công => gọi CanIf_TxConfirmation(swPduHandle)
     *          - Nếu TX thất bại => clear trạng thái pending
     *          - Nếu TX đang chờ => không làm gì (chờ tiếp)
     */
    void Can_MainFunction_Write(void);

    /**
     * @brief Hàm polling kiểm tra RX (backup cho ISR)
     * @details Gọi định kỳ trong vòng lặp chính.
     *         - Kiểm tra FIFO RX0, RX1 xem có mess pending không.
     *         - Nếu có => đọc ra và gọi CanIf_RxIndication().
     */
    void Can_MainFunction_Read(void);

#ifdef __cplusplus
}

#endif
#endif /* CAN_H */