#ifndef CANTP_CFG_H
#define CANTP_CFG_H

#include "Std_Types.h"
#include "ComStack_Types.h"

/* Tổng số cấu hình N-SDU truyền và nhận */
#define CANTP_NUM_TX_SDUS    (1u)
#define CANTP_NUM_RX_SDUS    (1u)

/* ID N-SDU cho CanTp */
#define CanTpConf_CanTpTxNSdu_DiagTx    (0u)
#define CanTpConf_CanTpRxNSdu_DiagRx    (0u)

/* CanTp Protocol / Implementation Configuration */
#define CANTP_MAX_PAYLOAD_LENGTH    (512u)

#define CANTP_CAN_FRAME_LENGTH      (8u)
#define CANTP_SF_DATA_LENGTH        (7u)
#define CANTP_FF_DATA_LENGTH        (6u)
#define CANTP_CF_DATA_LENGTH        (7u)

#define CANTP_MAIN_FUNCTION_PERIOD_MS   (5u)

/* =========================================================
 * CanTp TX N-SDU configuration
 * ========================================================= */

typedef struct
{
    PduIdType CanTpTxSduId;       /* ID của CanTp (0, 1...) */
    PduIdType CanIfTxPduId;       /* PDU ID để gọi xuống CanIf_Transmit */
    PduIdType PduRTxSduId;        /* PDU ID báo tx confirm lên PduR */
    uint16    N_As;               /* Timeout truyền frame: ví dụ 1000 ms */
    uint16    N_Bs;               /* Timeout đợi FC: ví dụ 1000 ms */
    uint16    N_Cs;               /* Timeout truyền CF: ví dụ 1000 ms */
} CanTp_TxNSduCfgType;


/* =========================================================
 * CanTp RX N-SDU configuration
 * ========================================================= */

typedef struct
{
    PduIdType CanTpRxSduId;       /* ID rx sdu của CanTp */
    PduIdType CanIfTxFcPduId;     /* PDU ID dùng để gửi Flow Control qua CanIf */
    PduIdType PduRRxSduId;        /* PDU ID báo rx indication lên PduR */
    uint16    N_Ar;               /* Timeout truyền FC */
    uint16    N_Br;               /* Timeout đợi STmin rx */
    uint16    N_Cr;               /* Timeout đợi CF tiếp theo */
    uint8     BlockSize;          /* BS rx */
    uint8     STmin;              /* STmin rx */
} CanTp_RxNSduCfgType;


/* =========================================================
 * Configuration tables
 * ========================================================= */

extern const CanTp_TxNSduCfgType CanTp_TxSduCfg[CANTP_NUM_TX_SDUS];

extern const CanTp_RxNSduCfgType CanTp_RxSduCfg[CANTP_NUM_RX_SDUS];


#endif /* CANTP_CFG_H */