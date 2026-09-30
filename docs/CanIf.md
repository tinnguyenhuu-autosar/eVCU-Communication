# AUTOSAR CAN Interface (CanIf)

## prv_find_txpdu – Tìm index TX PDU trong bảng cấu hình
-Tra bảng CanIf_TxPduCfg[] để tìm entry có CanIfTxPduId khớp với id truyền vào.
-@param  id   PDU ID cần tìm
@return index (0..N-1) nếu tìm thấy, -1 nếu không thấy

## prv_find_rxpdu – Tìm index RX PDU trong bảng cấu hình
-Tra bảng CanIf_RxPduCfg[] để tìm entry có CanIfRxPduId khớp với id truyền vào.
-@param  id   PDU ID cần tìm
@return index (0..N-1) nếu tìm thấy, -1 nếu không thấy

## CanIf_Init Khởi tạo CanIf module
Đánh dấu CanIf đã được khởi tạo.

## CanIf_Transmit
Quy trình chi tiết:

   1. Kiểm tra CanIf đã init chưa, param hợp lệ không
   2. Tra bảng CanIf_TxPduCfg[] → tìm config cho PDU ID:
      ┌─────────────┬─────┬──────────┬────────┐
      │CanIfTxPduId │ HTH │  CanId   │ DlcMax │
      ├─────────────┼─────┼──────────┼────────┤
      │ 0 (Engine)  │ 0   │  0x180   │  5     │
      │ 1 (Brake)   │ 1   │  0x280   │  3     │
      │ 2 (Body)    │ 2   │  0x380   │  4     │
      └─────────────┴─────┴──────────┴────────┘
   3. Kiểm tra DLC không vượt quá DlcMax
   4. Dựng Can_PduType:
      - swPduHandle = CanIfTxPduId (để callback ngược)
      - id   = CanId từ config
      - length = SduLength từ PduInfo
      - sdu    = SduDataPtr từ PduInfo
   5. Gọi Can_Write(HTH, &canPdu)

## CanIf_TxConfirmation - CAN Driver báo TX hoàn tất
Khi CAN Driver (Can_MainFunction_Write) phát hiện mailbox
đã truyền xong, nó gọi callback này.

 CanIf chuyển tiếp lên PduR:
   CanIf_TxConfirmation()
    → PduR_CanIfTxConfirmation()
    → Com_TxConfirmation()

## CanIf_RxIndication - Can Driver báo có RX frame
Nhận CAN frame từ CAN Driver, lưu CAN ID gần nhất để debug, tìm PDU tương ứng trong CanIf Cfg và chuyển lên PduR.

**LUỒNG ĐI**
CAN Driver
    │
    │ CanIf_RxIndication(CanId, PduInfo)
    ↓
┌─────────────────────────┐
│        CanIf            │
│                         │
│  1. Check Init          │
│  2. Check PduInfo       │
│  3. Lưu CAN ID debug    │
│  4. Tìm Config theo ID  │
│  5. Check DLC           │
└───────────┬─────────────┘
            │
            ↓
       PduR_CanIfRxIndication()
            │
            ↓
           PduR

**VÍ DỤ**
CAN ID = 0x180
       ↓
CanIf_FindRxPdu(0x180)
       ↓
CanIf_RxPduCfg[0]
       ↓
CanIfRxPduId = VehicleCmd
       ↓
PduR_CanIfRxIndication(VehicleCmd, PduInfo)