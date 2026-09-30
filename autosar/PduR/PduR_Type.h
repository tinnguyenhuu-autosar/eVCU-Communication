// PduR_Types.h
#ifndef PDUR_TYPES_H
#define PDUR_TYPES_H

#include "Std_Types.h"

typedef struct
{
    const void* ComTxRoutingTable;
    const void* CanIfRxRoutingTable;
    const void* CanIfTxConfRoutingTable;
    const void* CanIfTrigTxRoutingTable;
    uint32      ConfigId;
} PduR_PBConfigType;

#endif