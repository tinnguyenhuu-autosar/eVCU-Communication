#ifndef DCM_CFG_H
#define DCM_CFG_H

#include "Std_Types.h"

/* =========================================================
 * DCM Buffer Configuration
 * =======================================================*/

/* Maximum UDS request length handled by DCM */
#define DCM_RX_BUFFER_SIZE        (512u)

/* Maximum UDS response length handled by DCM */
#define DCM_TX_BUFFER_SIZE        (512u)

/* =========================================================
 * DCM Protocol Configuration
 * =======================================================*/

/* Number of configured diagnostic protocols */
#define DCM_NUM_PROTOCOLS         (1u)

/*
 * PduR -> DCM PDU identifier.
 *
 * This corresponds to:
 *     PduR_CanTpRxSduId
 *
 * Current project has one diagnostic connection.
 */
#define DCM_RX_PDU_ID             (0u)

/*
 * DCM -> PduR/CanTp PDU identifier.
 *
 * Current project has one diagnostic connection.
 */
#define DCM_TX_PDU_ID             (0u)

#endif /* DCM_CFG_H */