# AUTOSAR CAN DRIVER - bxCAN nội bộ STM32F103

## Định nghĩa phần cứng

- CAN_PERIPH: Peripheral bxCAN (CAN1)
- CAN_GPIO_PORT: Cổng GPIO chứa chân CAN_TX/CAN_RX
- CAN_RX_PIN: PA11 = CAN_RX
- CAN_TX_PIN: PA12 = CAN_TX

## Biến trạng thái nội bộ

- CAN_TX_SLOTS: Số lượng mailbox TX (3)
- s_inited: Cờ cho biết Can_Init() đã được gọi
- s_swPduHandle: Mảng lưu swPduHandle của từng mailbox TX
- s_txPending: Mảng cờ cho biết mailbox TX đang bận truyền
- s_txMailbox: Mailbox ID do CAN_Transmit() trả về

## prv_CAN_GPIO_Init – Cấu hình chân GPIO cho CAN

- Bật clock cho GPIOA, AFIO (Alternate Function), CAN1.
- Cấu hình:
- PA12 (CAN_TX): Alternate Function Push-Pull, 50MHz
- PA11 (CAN_RX): Input Floating (nhận tín hiệu từ bus)

### Bước 1: Bật clock cho các peripheral cần thiết

- - GPIOA : Port chứa chân CAN (PA11, PA12)
- - AFIO : Cho phép dùng Alternate Function
- - CAN1 : Peripheral bxCAN (trên bus APB1)

### Bước 2 Cấu hình PA12 (CAN_TX) – Alternate Function Push-Pull

- Chế độ AF_PP cho phép peripheral bxCAN điều khiển chân này
- để tạo tín hiệu CAN dominant/recessive trên bus

### Bước 3: Cấu hình PA11 (CAN_RX) – Input Floating

- Chân RX nối với CAN transceiver, nhận tín hiệu từ bus
- Chế độ Floating cho phép đọc mức logic từ transceiver

## prv_CAN_Periph_Init – Cấu hình bxCAN peripheral

- Gồm 2 phần:
- A. Cấu hình CAN (CAN_Init): bit timing, mode, tính năng
- B. Cấu hình Filter (CAN_FilterInit): bộ lọc nhận
- **Tính toán bit timing cho 500 kbps**:
- APB1 clock = 36MHz
- Perscaler=4
- TQ = 4/36MHz = 111.11ns
- 1 bit = Sync + BS1 + BS2
-        = 1 + 11 + 6 = 18 TQ
-        = 18*111.11ns = 2µs
- Bit rate = 1/2µs = 500kbps
- Sample point = (Sync+BS1)/1bit = (1+11)/18 = 66.67%
- SJW = 1
-
- static void prv_CAN_Periph_Init(void)
- reset Can peripheral về trạng thái mặc định
  **A. cấu hình CAN**
  **Bit timing**:
  Prescaler = 4 (chia clock: 36MHz/4=9MHz chu kỳ 1/9MHz=111.1 ns == 1TQ)
  SJW = 1 (Synchronization Jump Width) Nó cho phép dịch chuyển vị trí đồng bộ một vài TQ.
  BS1 Bit Segment 1 = 11TQ
  BS2 Bit Segment 2 = 6TQ
  **Mode hoạt động**: Mạng thật hoặc Renode Hub yêu cầu Normal Mode
  **Các tính năng bổ sung**:
  **CAN_TTCM** **Time Triggered Communication**, nghĩa là CAN hỗ trợ truyền theo mốc thời gian đồng bộ giữa các node.

=> Không sử dụng vì project không yêu cầu Time Triggered Communication
**CAN_ABOM** **Auto Bus-Off Management**, chức năng tự phục hồi nếu CAN Controller vào trạng thái Bus-Off.

ABOM = ENABLE

↓

Controller tự recovery

**CAN_AWUM** **Automatic Wake-Up**
Project không sử dụng Sleep Mode.

↓

Không cần Auto Wake-Up.

↓

DISABLE.

**CAN_NART** **No Automatic Retransmission**
ENABLE = Không tự gửi lại
DISABLE = Cho phép tự gửi lại
=> DISABLE để hardware tự retransmit nếu mất ACK

**CAN_RFLM** **Receive FIFO Locked Mode**
Nếu **ENABLE** -> FIFO bị đầy -> Frame mới sẽ bị bỏ.
Nếu **DISABLE** -> FIFO bị đầy -> Frame cũ nhất sẽ bị ghi đè.
=> DISABLE

**CAN_TXFP** **Transmit FIFO Piority**, quyết định thứ tự ưu tiên trên Mailbox chờ truyền.
**DISABLE** CAN ưu tiên ID nhỏ => gửi trước.
**ENABLE** CAN gửi theo thứ tự Mailbox/FIFO (không xét theo ID).
Không có yêu cầu đối với cơ chế ưu tiên -> để mặc định **DISABLE**.

**Khởi tạo bxCAN**

**B. Cấu hình Filter**

- Filter 0: nhận tất cả ID (mask = 0)
- -Mode: IdMask (so sánh ID với Mask)
- -Scale: 32-bit (lọc cả Standard ID và Extended ID)
- -Mask: 0x0000 (Mask = 0, nhận tất cả ID)
- -FIFO: Gán vào FIFO0
- -Kích hoạt: ENABLE

**Can_Init - API công khai: Khởi tạo CAN Driver**
Bước 1 cấu hình GPIO (PA11/12)
Bước 2 cấu hình bxCAN peripheral (bit timing, filter)
Bước 3 Clear trạng thái TX cho tất cả các slot
Bước 4 bật CAN RX FIFO interrupt
Bước 5 Cấu hình NVIC cho CAN1 RX0 (IRQ 20 = USB_LP_CAN1_RX0)
Bước 6 đánh dấu driver đã sẵn sàng

**Can_Write - API công khai: Gửi 1 PDU CAN**

- Luồng xử lý:
- 1.  Kiểm tra điều kiện tiên quyết (init, param không NULL)
- 2.  Kiểm tra slot TX tương ứng có rảnh không
- 3.  Chuyển đổi Can_PduType (AUTOSAR) → CanTxMsg (SPL)
- 4.  Gọi CAN_Transmit() của SPL để nạp vào mailbox
- 5.  Lưu thông tin để callback khi TX hoàn tất
-
- Mapping kiểu dữ liệu:
- ┌────────────────────┬──────────────────────┐
- │ Can_PduType (AUTOSAR) │ CanTxMsg (SPL) │
- ├────────────────────┼──────────────────────┤
- │ id ≤ 0x7FF │ StdId + CAN_Id_Standard│
- │ id > 0x7FF │ ExtId + CAN_Id_Extended│
- │ length │ DLC │
- │ sdu[i] │ Data[i] │
- └────────────────────┴──────────────────────┘

**Bước 1**: Kiểm tra điều kiện tiên quyết
**Bước 2**: giới hạn chỉ số slot (bxCAN có 3 slot mailbox: 0 1 2 )
**Bước 3**: Chuyển đổi Can_PduType → CanTxMsg (SPL)
CanTxMsg là cấu trúc mà hàm CAN_Transmit() của SPL yêu cầu
**3a** DLC (Data Length Code): giới hạn tối da 8byte
**3b** CAN ID: phân biệt Standard (11bit) và Extended (29bit)
Standard ID: < 0x7FF (11 bit)
_ → txMsg.IDE = CAN_Id_Standard
_ → txMsg.StdId = giá trị ID
_
Extended ID: > 0x7FF (29 bit)
_ → txMsg.IDE = CAN_Id_Extended \* → txMsg.ExtId = giá trị ID (29 bit thấp)
**3c** RTR: Data Frame (Not Remote Frame)
**3d** copy dữ liệu payload

**Bước 4**: Gọi SPL API để nạp frame vào mailbox bxCAN
_ CAN_Transmit() trả về:
_ 0, 1, 2 : Số mailbox đã sử dụng (thành công) \* CAN_TxStatus_NoMailBox (4) : Tất cả 3 mailbox đều bận

**Bước 5**: lưu thông tin cho callback TxConfirmation

- Khi Can_MainFunction_Write() phát hiện TX xong, sẽ dùng
  s_swPduHandle[slot] để báo ngược lên CanIf.

**Can_MainFunction_Write - Polling kiểm tra TX hoàn tất**

- Được gọi định kỳ trong vòng lặp chính (mỗi 5-10ms).

- Quy trình cho mỗi slot TX đang pending:
- 1. Gọi CAN_TransmitStatus() để hỏi trạng thái mailbox
- 2. Nếu CAN_TxStatus_Ok:
-      → TX thành công, gọi CanIf_TxConfirmation(handle)
-      → CanIf chuyển tiếp lên PduR → COM
- 3. Nếu CAN_TxStatus_Failed:
-      → TX thất bại, clear pending (upper layer có thể retry)
- 4. Nếu CAN_TxStatus_Pending:
-      → Đang truyền, chờ lần polling tiếp theo

**Can_MainFunction_Read - Polling kiểm tra RX**
Trích xuất message từ FIFO0 và FIFO1, sau đó báo lên CanIf_RxIndication
Kiểm tra FIFO0
Kiểm tra FIFO1

**USB_LP_CAN1_RX0_IRQHandler – ISR nhận CAN frame từ FIFO0**
Renode STMCAN model kích hoạt interrupt khi frame đến FIFO0.
ISR này đọc frame và chuyển lên CanIf_RxIndication.

=> Project này ko yêu cầu interrup mode -> để vào trường (weak).
