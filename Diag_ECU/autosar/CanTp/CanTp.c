/**
 * @file CanTp.c
 * @brief Module vận chuyển CAN (CAN Transport Protocol)
 *
 * @details Cài đặt giao thức CANTP theo chuẩn ISO 15765-2 (AUTOSAR)
 *
 * CanTp chịu trách nhiệm:
 * - Phân mảnh (segmentation) và lắp ráp (reassembly) các gói tin lớn (> 8 byte)
 * - Hổ trợ các loại frame: Single Frame (SF), First Frame (FF), Consecutive Frame (CF), Flow Control (FC)
 * - Quản lý timeout và các thông số truyền nhận
 *
 * Luồng truyền tin nhắn lớn (>= 8 byte):
 *
 *   1. Node TX gửi First Frame (FF): 2 byte PCI (chứa độ dài tổng) + 6 byte dữ liệu đầu
 *   2. Node RX nhận FF, xin buffer từ PduR (StartOfReception), gửi Flow Control (FC)
 *      với Flow Status (CTS/WAIT/OVFLW), BS và STmin
 *   3. Node TX nhận FC (CTS), gửi các Consecutive Frame (CF): 1 byte PCI (SN) + 7 byte dữ liệu,
 *      cách nhau tối thiểu STmin; sau mỗi BS frame thì chờ FC mới (BS = 0: không chờ)
 *   4. Node RX nhận đủ CF theo đúng SN, báo hoàn tất lên PduR (RxIndication)
 *
 * Luồng truyền tin nhắn nhỏ (<= 7 byte):
 *
 *   1. Node TX gửi Single Frame (SF): 1 byte PCI (chứa độ dài) + 7 byte dữ liệu
 *   2. Node RX nhận SF, báo hoàn tất lên PduR (RxIndication)
 *
 * @note các thông số được cấu hình trong CanTp_Cfg.h.
 */

 #include <string.h>

 #include "CanTp.h"
 #include "PduR_CanTp.h"
 #include "CanIf.h"