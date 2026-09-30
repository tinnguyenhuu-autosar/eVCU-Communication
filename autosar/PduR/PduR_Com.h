/**
 * @file PduR_Com.h
 * @details Đây là Interface dành cho COM -> PduR
 */
#ifndef PDUR_COM_H
#define PDUR_COM_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"

/**
 * @brief COM yêu cầu PduR truyền một PDU
 *
 * Luồng:
 * COM => PduR => CanIf
 */
Std_ReturnType PduR_ComTransmit(PduIdType ComTxPduId, const PduInfoType* PduInfoPtr);

#ifdef __cplusplus
}
#endif
#endif /*PDUR_COM_H*/