/**
 * @file PduR_Cfg.c
 * @brief Bảng route COM <=> CanIf
 * @details
 * Routing:
 *
 *      COM => PduR => CanIf
 *      CanIf => PduR => COM
 */
#include <stddef.h>

#include "PduR_Cfg.h"
#include "Com_Cfg.h"
#include "CanIf_Cfg.h"

/* COM TX Routes: COM => PduR => CanIf*/
const PduR_Route1to1Type PduR_ComTxRoutes[PDUR_NUM_COM_TX_ROUTES] =
{
    /* EngineStatus Tx route*/
    { ComConf_ComIPdu_EngineStatus, CanIfConf_Pdu_EngineStatus, PDUR_DEST_CANIF}
};

/* COM RX Routes: CanIf => PduR => COM*/
const PduR_CallbackRouteType PduR_CanIfRxRoutes[PDUR_NUM_CANIF_RX_ROUTES] =
{
    {CanIfConf_Pdu_VehicleCmd, ComConf_ComIPdu_VehicleCmd},
    {CanIfConf_Pdu_BrakeCmd, ComConf_ComIPdu_BrakeCmd},
    {CanIfConf_Pdu_BodyCmd, ComConf_ComIPdu_BodyCmd}
};

/* TxConfirmation Routes: CanIf => PduR => COM*/
const PduR_CallbackRouteType PduR_CanIfTxConfRoutes[PDUR_NUM_CANIF_TXCONF_ROUTES] =
{
    {CanIfConf_Pdu_EngineStatus, ComConf_ComIPdu_EngineStatus}
};

/* ===== CanIf → COM callback routes (TriggerTransmit) ===== */
const PduR_CallbackRouteType PduR_CanIfTrigTxRoutes[PDUR_NUM_CANIF_TRIGTX_ROUTES] = {
    { CanIfConf_Pdu_BrakeCmd,  ComConf_ComIPdu_BrakeCmd  },
    { CanIfConf_Pdu_BodyCmd,   ComConf_ComIPdu_BodyCmd   },
    { CanIfConf_Pdu_EngineStatus, ComConf_ComIPdu_EngineStatus }
};

/* ===== Post-Build Config ===== */

const PduR_PBConfigType PduR_ConfigPB =
{
    .ComTxRoutingTable        = PduR_ComTxRoutes,

#if PDUR_NUM_CANIF_RX_ROUTES > 0
    .CanIfRxRoutingTable      = PduR_CanIfRxRoutes,
#else
    .CanIfRxRoutingTable      = NULL,
#endif

#if PDUR_NUM_CANIF_TXCONF_ROUTES > 0
    .CanIfTxConfRoutingTable  = PduR_CanIfTxConfRoutes,
#else
    .CanIfTxConfRoutingTable  = NULL,
#endif

#if PDUR_NUM_CANIF_TRIGTX_ROUTES > 0
    .CanIfTrigTxRoutingTable  = PduR_CanIfTrigTxRoutes,
#else
    .CanIfTrigTxRoutingTable  = NULL,
#endif

    .ConfigId                 = 0u
};
