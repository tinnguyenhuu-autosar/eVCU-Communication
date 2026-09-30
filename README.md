# Student_ECU

Project bài tập AUTOSAR Classic dành cho học viên, gồm các ECU giao tiếp với **eVCU (Electronic Vehicle Control Unit)** thông qua CAN.

## 1. Tổng quan

`Student_ECU` là workspace chứa các ECU do học viên phát triển để kiểm thử giao tiếp với eVCU.

Mô hình tổng quát:

```text
                         CAN Bus
                            │
             ┌──────────────┴──────────────┐
             │                             │
     ┌───────▼───────┐             ┌───────▼───────┐
     │ Diagnostic ECU│             │    COM ECU     │
     │               │             │                │
     │ UDS           │             │ AUTOSAR COM    │
     │ CAN TP + DCM  │             │ PduR/CanIf/CAN │
     └───────┬───────┘             └───────┬────────┘
             │                             │
             └──────────────┬──────────────┘
                            │
                      ┌─────▼─────┐
                      │   eVCU    │
                      │  ECU đối  │
                      │   tác     │
                      └───────────┘
```

eVCU là node ECU đối tác được cung cấp sẵn. Học viên tập trung phát triển:

- `Com_ECU`: giao tiếp signal bằng AUTOSAR COM.
- `Diag_ECU`: giao tiếp chẩn đoán bằng UDS trên CAN TP/DCM.

---

## 2. Mục tiêu học tập

Project tập trung vào các thành phần AUTOSAR Classic:

- CAN Driver
- CanIf
- PduR
- COM
- CAN Transport Protocol (CAN TP)
- DCM
- UDS
- PDU routing
- Signal packing/unpacking
- CAN Tx/Rx
- Cấu hình AUTOSAR dạng `*_Cfg.c/.h`
- Build firmware cho STM32F103C8T6

Mục tiêu chính là hiểu luồng dữ liệu từ application xuống CAN hardware và ngược lại.

---

## 3. Cấu trúc thư mục

```text
Student_ECU/
│
├── autosar/
│   ├── Can/
│   │   ├── Can.c
│   │   └── Can.h
│   │
│   ├── Canif/
│   │   ├── CanIf.c
│   │   └── CanIf.h
│   │
│   ├── PduR/
│   │   ├── PduR.c
│   │   └── PduR.h
│   │
│   └── include/
│       ├── Std_Types.h
│       ├── ComStack_Types.h
│       └── ...
│
├── config/
│   ├── Can_Cfg.c
│   ├── Can_Cfg.h
│   ├── CanIf_Cfg.c
│   ├── CanIf_Cfg.h
│   ├── PduR_Cfg.c
│   └── PduR_Cfg.h
│
├── spl/
│   ├── inc/
│   └── src/
│
├── bsp/
│   └── cmsis/
│
├── Com_ECU/
│   ├── main.c
│   ├── platformio.ini
│   ├── syscalls.c
│   ├── autosar/
│   │   └── Com/
│   │       ├── Com.c
│   │       └── Com.h
│   └── config/
│       ├── Com_Cfg.c
│       └── Com_Cfg.h
│
└── Diag_ECU/
    ├── Makefile
    └── autosar/
        ├── CanTp/
        └── Dcm/
```

> Cấu trúc thực tế có thể được mở rộng khi thêm module AUTOSAR mới.

---

# 4. Com_ECU

`Com_ECU` là ECU dùng để học và kiểm thử **AUTOSAR COM communication**.

Stack chính:

```text
Application
    │
    ▼
   COM
    │
    ▼
   PduR
    │
    ▼
  CanIf
    │
    ▼
   CAN
    │
    ▼
 CAN Bus
```

### Các chức năng hiện tại

`Com_ECU` xử lý các nhóm signal:

- `VehicleCmd`
- `BrakeCmd`
- `BodyCmd`
- `EngineStatus`

Ví dụ `EngineStatus` được tạo trong application và gửi thông qua:

```c
Com_SendSignal()
Com_TriggerIPDUSend()
```

Sau đó dữ liệu đi qua:

```text
COM
 ↓
PduR
 ↓
CanIf
 ↓
CAN
```

### EngineStatus

Các signal hiện được sử dụng gồm:

```text
Engine_RPM
Engine_Temp
Engine_TorqueActual
Engine_State
Engine_Alive
Engine_CRC
```

`Engine_Alive` được dùng làm alive counter.

`Engine_CRC` được tính từ các dữ liệu của `EngineStatus` trong application hiện tại.

---

# 5. Diag_ECU

`Diag_ECU` dành cho phần chẩn đoán AUTOSAR.

Stack:

```text
Application
    │
    ▼
   DCM
    │
    ▼
  CAN TP
    │
    ▼
   PduR
    │
    ▼
  CanIf
    │
    ▼
   CAN
```

Các module chính:

- CAN
- CanIf
- PduR
- CanTp
- DCM

Mục tiêu là thực hiện giao tiếp:

```text
Diagnostic ECU
      │
      │ UDS
      ▼
    CAN TP
      │
      ▼
    eVCU
```

Các service UDS cụ thể được triển khai phụ thuộc vào cấu hình của `Diag_ECU` và đề bài tương ứng.

---

# 6. CAN Communication

Project sử dụng CAN của STM32F103.

Cấu hình CAN hiện tại của project:

```text
CAN Clock : 36 MHz
Prescaler : 4
BS1       : 11 TQ
BS2       : 6 TQ
SJW       : 1 TQ
Bitrate   : 500 kbit/s
```

Tính toán:

```text
Total TQ = 1 + 11 + 6
         = 18 TQ

1 TQ = 4 / 36 MHz
     ≈ 111.1 ns

1 bit = 18 × 111.1 ns
      ≈ 2 µs

Bitrate ≈ 500 kbit/s
```

---

# 7. CAN ID

Các CAN ID được cấu hình trong project/config và phải thống nhất giữa các ECU.

Một số ID đang được sử dụng trong COM communication:

| PDU | CAN ID | Hướng |
|---|---:|---|
| `VehicleCmd` | `0x180` | eVCU → COM ECU |
| `BrakeCmd` | `0x280` | eVCU → COM ECU |
| `BodyCmd` | `0x380` | eVCU → COM ECU |
| `EngineStatus` | theo cấu hình `CanIf_Cfg` | COM ECU → eVCU |

**Không nên tự thay đổi CAN ID ở một ECU mà không cập nhật ECU đối tác.**

---

# 8. Build Com_ECU bằng PlatformIO

`Com_ECU` hiện được chuyển sang PlatformIO.

Đặc điểm:

```text
PlatformIO
    │
    ├── GCC ARM
    │
    ├── STM32 SPL
    │
    └── AUTOSAR source
```

Project **không sử dụng STM32Cube/HAL framework**.

Trong `platformio.ini`:

```ini
framework =
```

PlatformIO chỉ được dùng để quản lý build/toolchain và upload firmware.

## Build

Mở terminal tại:

```text
Student_ECU/Com_ECU
```

chạy:

```bash
pio run
```

Build thành công sẽ tạo:

```text
.pio/build/bluepill_f103c8/firmware.elf
.pio/build/bluepill_f103c8/firmware.bin
```

Kiểm tra clean build:

```bash
pio run -t clean
pio run
```

---

# 9. Flash bằng ST-Link

Nếu đã kết nối ST-Link với STM32F103C8T6:

```text
ST-Link SWDIO → PA13
ST-Link SWCLK → PA14
ST-Link GND   → GND
```

Upload:

```bash
pio run -t upload
```

PlatformIO sử dụng:

```ini
upload_protocol = stlink
```

để thực hiện upload.

---

# 10. Kết quả build hiện tại của Com_ECU

Build hiện tại đã thành công với:

```text
RAM:
1272 bytes / 20480 bytes
≈ 6.2%

Flash:
6064 bytes / 65536 bytes
≈ 9.3%
```

Firmware được tạo:

```text
firmware.elf
firmware.bin
```

---

# 11. Luồng COM Rx

Ví dụ khi `Com_ECU` nhận `VehicleCmd`:

```text
CAN Frame
   │
   ▼
CAN Driver
   │
   ▼
CanIf_RxIndication()
   │
   ▼
PduR_CanIfRxIndication()
   │
   ▼
COM
   │
   ▼
Com_ReceiveSignal()
   │
   ▼
Application
```

Application có thể đọc:

```c
Com_ReceiveSignal(
    ComSig_Vehicle_Throttle,
    &throttleReq
);
```

Tương tự với các signal:

```text
Vehicle_Start
Vehicle_TorqueLimit
Vehicle_Alive
Vehicle_CRC

Brake_BrakeReq
Brake_RegenReq
Brake_Alive
Brake_CRC

Body_HeadLamp
Body_TurnL
Body_TurnR
Body_DoorLock
```

---

# 12. Luồng COM Tx

Ví dụ `Com_ECU` gửi `EngineStatus`:

```text
Application
   │
   ├── Com_SendSignal()
   │
   ▼
  COM
   │
   ▼
PduR_ComTransmit()
   │
   ▼
 CanIf_Transmit()
   │
   ▼
 CAN Driver
   │
   ▼
CAN Bus
```

Application hiện tạo dữ liệu:

```text
RPM
Temperature
TorqueActual
EngineState
Alive Counter
CRC
```

sau đó trigger I-PDU:

```c
Com_TriggerIPDUSend(
    ComConf_ComIPdu_EngineStatus
);
```

---

# 13. PduR

PduR là module định tuyến PDU giữa các AUTOSAR communication modules.

Ví dụ COM Tx:

```text
COM
 │
 ▼
PduR
 │
 ▼
CanIf
```

Trong cấu hình hiện tại, `EngineStatus` được route:

```text
ComConf_ComIPdu_EngineStatus
        │
        ▼
CanIfConf_Pdu_EngineStatus
        │
        ▼
CanIf
```

PduR cũng được thiết kế để có thể mở rộng cho:

```text
COM
CAN TP
DCM
```

Phần CAN TP/DCM được bật khi build ECU chẩn đoán tương ứng.

---

# 14. STM32 SPL

Project sử dụng **STM32 Standard Peripheral Library (SPL)** thay vì STM32Cube HAL.

Các peripheral SPL hiện được sử dụng cho CAN:

```text
stm32f10x_can.c
stm32f10x_gpio.c
stm32f10x_rcc.c
misc.c
system_stm32f10x.c
```

Compile flag quan trọng:

```text
STM32F10X_MD
USE_STDPERIPH_DRIVER
```

Target:

```text
STM32F103C8T6
Cortex-M3
72 MHz
64 KB Flash
20 KB RAM
```

---

# 15. Công cụ phát triển

Các công cụ chính:

| Công cụ | Mục đích |
|---|---|
| VS Code | IDE |
| PlatformIO | Build/upload Com_ECU |
| ARM GNU Toolchain | Compile/link ARM Cortex-M |
| Git | Version control |
| ST-Link | Debug/flash STM32 |
| STM32 SPL | MCU peripheral layer |
| Renode | Mô phỏng/debug ECU khi cần |

---

# 16. Git

Repository chứa toàn bộ workspace:

```text
Student_ECU/
```

Có thể kiểm tra:

```bash
git status
```

Commit:

```bash
git add .
git commit -m "Update Student ECU"
```

Push:

```bash
git push
```

Không commit các file build sinh ra nếu `.gitignore` đã loại chúng:

```text
.pio/
build/
```

---

# 17. Quy trình phát triển

Quy trình đề xuất:

```text
1. Sửa code AUTOSAR/application
           ↓
2. Build
           ↓
3. Kiểm tra compiler/linker
           ↓
4. Flash bằng ST-Link
           ↓
5. Kiểm thử CAN
           ↓
6. Kiểm tra communication với eVCU
           ↓
7. Debug
```

Với `Com_ECU`:

```bash
cd Com_ECU
pio run
pio run -t upload
```

---

# 18. Các warning hiện tại

Build `Com_ECU` hiện có một số warning không làm build thất bại:

### `Can.c`

```text
unused variable 'rxMsg'
unused variable 'pduInfo'
```

### `CanIf.c`

```text
unused variable 'CanPdu'
```

### `PduR_Cfg.c`

```text
excess elements in array initializer
```

Warning cuối liên quan đến số lượng phần tử của:

```text
PduR_CanIfTrigTxRoutes
```

Cần kiểm tra lại khi hoàn thiện cấu hình PduR/COM.

---

# 19. Trạng thái hiện tại

### Com_ECU

- [x] AUTOSAR COM
- [x] PduR
- [x] CanIf
- [x] CAN
- [x] STM32 SPL
- [x] PlatformIO build
- [x] `firmware.elf`
- [x] `firmware.bin`
- [ ] Kiểm thử CAN thực tế với eVCU
- [ ] Hoàn thiện/kiểm tra PduR trigger TX warning
- [ ] Xác nhận toàn bộ signal mapping với ECU đối tác

### Diag_ECU

- [x] CAN
- [x] CanIf
- [x] PduR
- [x] CAN TP
- [x] DCM
- [ ] Hoàn thiện build/flash
- [ ] Kiểm thử UDS với eVCU

---

# 20. Lưu ý

Đây là project học tập AUTOSAR Classic trên STM32F103C8T6.

Các module trong project được tổ chức theo kiến trúc AUTOSAR để phục vụ việc học:

```text
Application
    ↓
COM / DCM
    ↓
PduR
    ↓
CanIf / CanTp
    ↓
CAN
    ↓
STM32 CAN Peripheral
```

Project không nhằm triển khai một AUTOSAR production stack đầy đủ theo toàn bộ specification của AUTOSAR Classic.

---

## License / Academic Use

Project được sử dụng cho mục đích học tập, thực hành AUTOSAR Classic, CAN communication và embedded software development.
