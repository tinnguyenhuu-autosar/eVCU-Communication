#ifndef PDUR_DCM_H
#define PDUR_DCM_H

#include "Std_Types.h"
#include "ComStack_Types.h"

Std_ReturnType PduR_DcmTransmit(
    PduIdType TxPduId,
    const PduInfoType* PduInfoPtr
);

#endif /* PDUR_DCM_H */