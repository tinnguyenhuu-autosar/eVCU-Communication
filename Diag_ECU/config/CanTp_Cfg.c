#include "CanTp_Cfg.h"
#include "CanIf_Cfg.h"

const CanTp_TxNSduCfgType CanTp_TxSduCfg[CANTP_NUM_TX_SDUS] =
{
    {
        .CanTpTxSduId = CanTpConf_CanTpTxNSdu_DiagTx,

        /*
         * Diagnostic request:
         * CAN ID = 0x7E8
         */
        .CanIfTxPduId = CanIfConf_Pdu_DiagResponse,

        .PduRTxSduId = 0u,

        .N_As = 1000u,
        .N_Bs = 1000u,
        .N_Cs = 90u
    }
};


const CanTp_RxNSduCfgType CanTp_RxSduCfg[CANTP_NUM_RX_SDUS] =
{
    {
        .CanTpRxSduId = CanTpConf_CanTpRxNSdu_DiagRx,

        /*
         * Flow Control:
         * eVCU -> Diagnostic ECU
         * CAN ID = 0x7E0
         */
        .CanIfTxFcPduId = CanIfConf_Pdu_DiagResponse,

        .PduRRxSduId = 0u,

        .N_Ar = 1000u,
        .N_Br = 1000u,
        .N_Cr = 1000u,

        .BlockSize = 8u,
        .STmin = 20u
    }
};