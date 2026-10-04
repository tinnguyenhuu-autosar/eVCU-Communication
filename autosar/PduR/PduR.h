/**
 * @file PduR.h
 * @brief AUTOSAR PDU Router - Header tổng quát
 * @details Cung cấp lifecycle & tiện ích chung (Init, GetVersionInfo,...)
 *          API cụ thể cho từng module đặt ở Header riêng:
 *          - PduR_Com.h
 *          - PduR_CanIf.h
 *          - PduR_CanTp.h
 */
#ifndef PDUR_H
#define PDUR_H

#ifdef __cplusplus
extern "C"{
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "PduR_Type.h"

#include "PduR_Cfg.h"

#define PDUR_VENDOR_ID      (0u)
#define PDUR_MODULE_ID      (51u)   /* theo AUTOSAR module list (ví dụ) */
#define PDUR_SW_MAJOR_VERSION (1u)
#define PDUR_SW_MINOR_VERSION (0u)
#define PDUR_SW_PATCH_VERSION (0u)

/* Trạng thái PduR (rút gọn theo SWS – State Management) */
typedef enum {
    PDUR_UNINIT = 0,
    PDUR_ONLINE = 1
} PduR_StateType;

/* ===== Development errors (rút gọn, theo bảng SWS_PduR_00100) ===== */
#define PDUR_E_INIT_FAILED                     ((uint8)0x00)
#define PDUR_E_UNINIT                          ((uint8)0x01)
#define PDUR_E_PDU_ID_INVALID                  ((uint8)0x02)
#define PDUR_E_ROUTING_PATH_GROUP_ID_INVALID   ((uint8)0x08)
#define PDUR_E_PARAM_POINTER                   ((uint8)0x09)

/* ===== Routing Group ID ===== */
typedef uint16 PduR_RoutingPathGroupIdType;

/* ===== API chung ===== */
void PduR_Init(const PduR_PBConfigType* ConfigPtr);

/** @brief Bật routing theo RoutingPathGroup. */
Std_ReturnType PduR_EnableRouting(PduR_RoutingPathGroupIdType id);

/** @brief Tắt routing theo RoutingPathGroup. */
Std_ReturnType PduR_DisableRouting(PduR_RoutingPathGroupIdType id);

/** @brief Trạng thái hiện tại của PduR. */
PduR_StateType PduR_GetState(void);

/** @brief Gửi PDU từ DCM đến PduR. */
Std_ReturnType PduR_DcmTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);

#ifdef __cplusplus
}
#endif

#endif /* PDUR_H */