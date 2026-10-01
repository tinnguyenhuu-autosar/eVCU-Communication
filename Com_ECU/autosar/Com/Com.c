/**********************************************************
 * @file    Com.c
 * @brief   AUTOSAR COM – Implementation
 * @details
 *      TX:
 *      Com_SendSignal()
 *          -> cập nhật shadow buffer
 *
 *      Com_TriggerIPDUSend()
 *          -> PduR_ComTransmit()
 *
 *  RX:
 *      CanIf
 *          -> PduR
 *          -> Com_RxIndication()
 *          -> shadow buffer
 *
 *      Com_ReceiveSignal()
 *          -> đọc signal từ shadow buffer
 * */

#include "Com.h"
#include "PduR_Com.h"

#include <string.h>
#include <stdint.h>

/*Kiểm tra cấu hình bắt buộc*/
#ifndef COM_MAX_IPDU_LEN
#error "COM_MAX_IPDU_LEN được định nghĩa trong Com_Cfg.h"
#endif

/*Các bảng cấu hình*/
static const Com_IPduCfgType* Com_IPduCfgPtr = NULL;
static const Com_SignalCfgType* Com_SignalCfgPtr = NULL;

static uint16 Com_NumIPdus = 0u;
static uint16 Com_NumSignals = 0u;

/* Bù an toàn nếu Header chưa định nghĩa mã SWS (giữ tính tương thích)*/
#ifndef COM_SERVICE_NOT_AVAILABLE
#define COM_SERVICE_NOT_AVAILABLE ((uint8)0x80u)
#endif
#ifndef COM_BUSY
#define COM_BUSY                  ((uint8)0x81u)
#endif

/* Shadow buffer cho từng I-PDU*/
static uint8 Com_TxIPduBuf[COM_NUM_TX_IPDUS * COM_MAX_IPDU_LEN];
static uint8 Com_RxIPduBuf[COM_NUM_RX_IPDUS * COM_MAX_IPDU_LEN];

/* Helpers nội bộ: tra cứu I-PDU & lấy Buffer*/

/**
 * @brief Tìm index I-PDU theo PduId trong bảng cấu hình
 * @param pduId ID I-PDU (ComConf_ComIPdu_*)
 * @return -1 nếu không thấy, ngược lại là index
 */
static int16_t prv_find_ipdu_index(PduIdType pduId)
{
    for (uint16 i = 0u; i < Com_NumIPdus; ++i)
    {
        if(Com_IPduCfgPtr[i].PduId == pduId) {return (int16)i;}
    }
    return (int16)-1;
}

/**
 * @brief Lấy con trỏ buffer I-PDU kèm dlc/dir
 * @param pduId ID I-PDU
 * @param outLen [opt] độ dài I-PDU (byte)
 * @param outDir [opt] hướng I-PDU (TX/RX)
 * @return Con trỏ shadow buffer hoặc NULL (nếu không hợp lệ)
 */
static uint8* prv_get_ipdu_buf(PduIdType pduId, PduLengthType* outLen, Com_PduDirection_e* outDir)
{
    int16_t idx = prv_find_ipdu_index(pduId);
    if(idx < 0) {return NULL;}

    const Com_IPduCfgType* cfg = &Com_IPduCfgPtr[(uint16)idx];

    if(outLen) *outLen = cfg->Length;
    if(outDir) *outDir = cfg->direction;
    if(cfg->direction == COM_PDU_DIR_TX)
    {
        /* TX I-Pdu: đếm riêng Tx index*/
        uint16 txIdx = 0u;
        for (uint16 i = 0u; i < (uint16)idx; ++i)
        {
            if (Com_IPduCfgPtr[i].direction == COM_PDU_DIR_TX)
            {
                txIdx++;
            }
        }
        return &Com_TxIPduBuf[txIdx * COM_MAX_IPDU_LEN];
    }
    else
    {
        /* Rx I-Pdu: đếm riêng Rx index*/
        uint16 rxIdx = 0u;
        for (uint16 i = 0u; i < (uint16)idx; ++i)
        {
            if (Com_IPduCfgPtr[i].direction == COM_PDU_DIR_RX)
            {
                rxIdx++;
            }
        }
        return &Com_RxIPduBuf[rxIdx * COM_MAX_IPDU_LEN];
    }
}

/* Pack helper (đặt giá trị vào byte/bit cụ thể)*/

/* Ghi 8bit vào vị trí byteIndex của đích*/
static inline void put_u8(uint8* dst, uint16 byteIndex, uint8 v)
{
    dst[byteIndex] = v;
}

/* Ghi 4bit (nibble) vào [byteIndex : bitOffset..bitOffset+3*/
static void put_nibble(uint8* dst, uint16 byteIndex, uint8 bitOffset, uint8 v4)
{
    const uint8 mask = (uint8)(0x0Fu << bitOffset);
    const uint8 val = (uint8)((v4 & 0x0Fu) << bitOffset);
    dst[byteIndex] = (uint8)((dst[byteIndex] & (uint8)(~mask)) | val);
}

/* Ghi 16bit vào vị trí byteIndex (Big Endian giả định cho RPM)*/
static inline void put_u16(uint8* dst, uint16 byteIndex, uint16 v)
{
    dst[byteIndex]      = (uint8)(v >> 8);
    dst[byteIndex + 1]  = (uint8)(v & 0xFFu);
}

/* Ghi 1 bit vào [byteIndex : bitOffset]*/
static void put_bit(uint8* dst, uint16 byteIndex, uint8 bitOffset, boolean b)
{
    const uint8 mask = (uint8)(1u << bitOffset);
    if(b == TRUE) {dst[byteIndex] |= mask;}
    else {dst[byteIndex] &= (uint8)(~mask);}
}

/* Unpack helpers (lấy giá trị từ byte/bit cụ thể)*/

/* Đọc 8bit từ vị trí byteIndex*/
static inline uint8 get_u8(const uint8* src, uint16 byteIndex)
{
    return src[byteIndex];
}

/* Đọc 16bit từ vị trí byteIndex (big endian giả định cho RPM)*/
static inline uint16 get_u16(const uint8* src, uint16 byteIndex)
{
    return (uint16)((uint16)src[byteIndex] << 8 | (uint16)src[byteIndex + 1u]);
}

/* Đọc 1bit từ [byteIndex : bitOffset]*/
static inline boolean get_bit(const uint8* src, uint16 byteIndex, uint8 bitOffset)
{
    return (src[byteIndex] & (uint8)(1u << bitOffset)) ? TRUE : FALSE;
}

static inline uint8 get_nibble(const uint8* src, uint16 byteIndex, uint8 bitOffset)
{
    return (uint8)((src[byteIndex] >> bitOffset) & 0x0Fu);
}

/* Application callback*/
__attribute ((weak)) void App_ComRxIndication(PduIdType ComRxPduId)
{
    (void)ComRxPduId;
}

/* Lifecycle*/

/**
 * @brief Khởi tạo COM (shadowbuffer/biến nội bộ)
 * @details Sau Com_Init, liên lạc inter_ECU vẫn chưa bật: cần start I-PDU
 *          theo cơ chế hệ thống nếu muốn truyền.
 */
void Com_Init(const Com_ConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL)
    {
        return;
    }

    Com_IPduCfgPtr = ConfigPtr->IPduCfg;
    Com_NumIPdus = ConfigPtr->NumIPdus;

    Com_SignalCfgPtr = ConfigPtr->SignalCfg;
    Com_NumSignals = ConfigPtr->Numsignals;

    /* Khởi tạo Tx I-PDU buffer*/
    (void)memset(Com_TxIPduBuf, 0, sizeof(Com_TxIPduBuf));

    /* Khởi tạo Rx I-PDU buffer*/
    (void)memset(Com_RxIPduBuf, 0, sizeof(Com_RxIPduBuf));
}


/**
 * @brief Cập nhật giá trị signal vào shadowbuffer của I-PDU
 * @param SignalId      ID của signal (idx vào Com_SignalCfg)
 * @param SignalDataPtr con trỏ dữ liệu nguồn
 * @return uint8 E_OK / COM_SERVICE_NOT_AVAILABLE / COM_BUSY
 * @note   luồng: Application => Com_SendSignal() => Com_IpduBuf[]
 */
uint8 Com_SendSignal(Com_SignalIdType SignalId, const void* SignalDataPtr)
{
    if (SignalDataPtr == NULL) {return COM_SERVICE_NOT_AVAILABLE;}
    if (SignalId >= (Com_SignalIdType)Com_NumSignals) {return COM_SERVICE_NOT_AVAILABLE;}

    const Com_SignalCfgType* cfg = &Com_SignalCfgPtr[SignalId];
    PduLengthType ipduLen;
    Com_PduDirection_e dir;
    uint8* ipdu = prv_get_ipdu_buf(cfg->PduId, &ipduLen, &dir);

    if ((ipdu == NULL) || (dir != COM_PDU_DIR_TX) || ((cfg->byteIndex) >= ipduLen) || ((cfg->direction) != COM_PDU_DIR_TX) ||
        ((cfg->bitLength == 16u) && ((cfg->byteIndex +1u) >= ipduLen)))
    {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    /* Pack theo cấu hình độ dài/kiểu*/
    switch (cfg->bitLength)
    {
        case 16u:
            put_u16(ipdu, cfg->byteIndex, *(const uint16*)SignalDataPtr);
            break;

        case 8u:
            if (cfg->type == COM_SIGTYPE_BOOLEAN)
            {
                const uint8 v = (*(const boolean*)SignalDataPtr) ? 1u : 0u;
                put_u8(ipdu, cfg->byteIndex, v);
            }
            else
            {
                put_u8 (ipdu, cfg->byteIndex, *(const uint8*)SignalDataPtr);
            }
            break;

        case 4u:
        {
            const uint8 v4 = (uint8)(*(const uint8*)SignalDataPtr & 0x0Fu);
            put_nibble(ipdu, cfg->byteIndex, cfg->bitOffset, v4);
        }
        break;

        case 1u:
        {
            const boolean b = (*(const boolean*)SignalDataPtr) ? TRUE : FALSE;
            put_bit(ipdu, cfg->byteIndex, cfg->bitOffset, b);
        }
        break;

        default:
        return COM_SERVICE_NOT_AVAILABLE;

    }
        return E_OK;

}

/**
 * @brief Kích phát gửi I-PDU qua PduR
 * @param PduId     ID I-PDU
 * @return Std_ReturnType E_OK / E_NOT_OK
 * @details Thực thị đơn giản: kiểm tra buffer/hướng/độ dài rồi gọi PduR_ComTransmit().
 *
 */
Std_ReturnType Com_TriggerIPDUSend(PduIdType PduId)
{
    PduLengthType len;
    Com_PduDirection_e dir;
    /* Lấy shadow buffer của I-Pdu*/
    uint8* buf = prv_get_ipdu_buf(PduId, &len, &dir);

    /* Kiểm tra i-PDU hợp lệ*/
    if ((buf == NULL) || (dir != COM_PDU_DIR_TX) || (len == 0u))
    {
        return E_NOT_OK;
    }

    /* Chuẩn bị I-PDU Infomation*/
    PduInfoType info;
    info.SduDataPtr = buf;
    info.SduLength = len;

    /* Gửi COM PDU xuống PduR*/
    return PduR_ComTransmit(PduId, &info);
}

/**
 * @brief Đọc giá trị signal từ buffer của I-PDU
 * @param SignalId      Id của Signal
 * @param SignalDataPtr Con trỏ đích để lưu dữ liệu
 * @return uint8 E_OK/  COM_SERVICE_NOT_AVAILABLE
 */
uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr)
{
    if (SignalDataPtr == NULL)
    {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    if (SignalId >= (Com_SignalIdType)Com_NumSignals)
    {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    /* Lấy cấu hình Signal*/
    const Com_SignalCfgType* cfg = &Com_SignalCfgPtr[SignalId];

    PduLengthType ipduLen;
    Com_PduDirection_e dir;

    /* Lấy buffer của I-PDU chứa Signal*/
    uint8* ipdu = prv_get_ipdu_buf(cfg->PduId, &ipduLen, &dir);

    if ((ipdu == NULL) || (dir != COM_PDU_DIR_RX) || ((cfg->byteIndex) >= ipduLen) || ((cfg->direction) != COM_PDU_DIR_RX))
    {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    /* Unpack theo cấu hình độ dài*/
    switch (cfg->bitLength)
    {
    case 16u:
        *(uint16*)SignalDataPtr = get_u16(ipdu, cfg->byteIndex);
        break;

    case 8u:
        if (cfg->type == COM_SIGTYPE_BOOLEAN)
        {
            *(boolean*)SignalDataPtr = (get_u8(ipdu, cfg->byteIndex) != 0) ? TRUE : FALSE;
        }
        else
        {
            *(uint8*)SignalDataPtr = get_u8(ipdu, cfg->byteIndex);
        }
        break;

    case 1u:
        *(boolean*)SignalDataPtr = get_bit(ipdu, cfg->byteIndex, cfg->bitOffset);
        break;

    case 4u:
        *(uint8*)SignalDataPtr = get_nibble(ipdu, cfg->byteIndex, cfg->bitOffset);
        break;

    default:
        return COM_SERVICE_NOT_AVAILABLE;

    }

    return E_OK;
}

/**
 * Callback/Callout cho PduR/CanIf
 */

/**
 * @brief Nhận một I-PDU từ PduR và cập nhật shadowbuffer của COM
 * @param ComRxPduId    ID của I-PDU RX.
 * @param PduInfoPtr    Con trỏ đến thông tin PDU
 */

void Com_RxIndication(PduIdType ComRxPduId, const PduInfoType* PduInfoPtr)
{
    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return;
    }

    PduLengthType len;
    Com_PduDirection_e dir;

    /* Lấy shadow buffer của I-PDU*/
    uint8* buf = prv_get_ipdu_buf(ComRxPduId, &len, &dir);

    /* Kiểm tra I-PDU hợp lệ và phải là RX*/
    if ((buf != NULL) && (dir == COM_PDU_DIR_RX))
    {
        (void)memcpy(buf, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
    }
}

/**
 * @brief Callback thông báo I-PDU TX đã được truyền thành công.
 * @param ComTxPduId    ID của I-PDU đã truyền
 */
void Com_TxConfirmation(PduIdType ComTxPduId)
{
    PduLengthType len;
    Com_PduDirection_e dir;

    /* Lấy shadow buffer của I-PDU*/
    uint8* buf = prv_get_ipdu_buf(ComTxPduId, &len, &dir);

    /* Kiểm tra I-PDU hợp lệ và phải là TX*/
    if ((buf == NULL) || (dir != COM_PDU_DIR_TX))
    {
        return;
    }
}

Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, PduInfoType* PduInfoPtr)
{
    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return E_NOT_OK;
    }

    PduLengthType len;
    Com_PduDirection_e dir;
    uint8* buf = prv_get_ipdu_buf(ComTxPduId, &len, &dir);
    if ((buf == NULL) || (dir != COM_PDU_DIR_TX) || (len == 0u))
    {
        return E_NOT_OK;
    }

    (void)memcpy(PduInfoPtr->SduDataPtr, buf, len);
    PduInfoPtr->SduLength = len;
    return E_OK;
}







