#include "Dcm.h"

#include <stdio.h>
#include <string.h>

/* Khai báo hàm in log từ hệ thống hiện hành*/
extern void Log_Print(const char* str);

/* Bộ đệm nhận dữ liệu chuẩn đoán nội bộ của DCM*/

/* RX Buffer (CanTp => PduR => DCM)*/
static uint8 Dcm_RxBuffer[DCM_RX_BUFFER_SIZE];

static PduLengthType Dcm_RxLength;
static PduLengthType Dcm_RxOffset;

static boolean Dcm_IsReceiving;

/* TX Buffer (DCM => PduR => CanTp)*/
static uint8 Dcm_TxBuffer[DCM_TX_BUFFER_SIZE];

static PduLengthType Dcm_TxLength;
static PduLengthType Dcm_TxOffset;

static boolean Dcm_IsTransmitting;

static boolean Dcm_Initialized;

/**
 * Internal Helper Functions
 */
static void Dcm_ResetRxState(void)
{
    Dcm_RxLength = 0u;
    Dcm_RxOffset = 0u;
    Dcm_IsReceiving = FALSE;
}

static void Dcm_ResetTxState(void)
{
    Dcm_TxLength = 0u;
    Dcm_TxOffset = 0u;
    Dcm_IsTransmitting = FALSE;
}

/**
 * @brief Khởi tạo module DCM
 */
void Dcm_Init(void)
{
    memset(Dcm_RxBuffer, 0, DCM_RX_BUFFER_SIZE);
    memset(Dcm_TxBuffer, 0, DCM_TX_BUFFER_SIZE);

    Dcm_ResetRxState();
    Dcm_ResetTxState();

    Dcm_Initialized = TRUE;

    Log_Print("[DCM] Initialized successfully.\r\n");
}

/**
 * @brief Bắt đầu quá trình nhận Diagnostic PDU (gọi khi có FF, hoặc SF)
 */
BufReq_ReturnType Dcm_StartOfReception(
    PduIdType id,
    const PduInfoType* info,
    PduLengthType TpSduLength,
    PduLengthType* bufferSizePtr
)
{
    (void)id;
    (void)info;

    /* Kiểm tra xem dung lượng yêu cầu có vượt quá bộ đệm của DCM không*/
    if (!Dcm_Initialized)
    {
        Log_Print("[DCM] StartOfReception: DCM not initialized.\r\n");
        return BUFREQ_E_NOT_OK;
    }
    if(bufferSizePtr == NULL)
    {
        Log_Print("[DCM] StartOfReception: bufferSizePtr is NULL.\r\n");
        return BUFREQ_E_NOT_OK;
    }
    if(TpSduLength > DCM_RX_BUFFER_SIZE)
    {
        Log_Print("[DCM] StartOfReception: Requested PDU length exceeds buffer size.\r\n");
        return BUFREQ_E_OVFL;
    }

    /* Reset trạng thái nhận dữ liệu */
    Dcm_IsReceiving = TRUE;
    Dcm_RxLength = 0;
    memset(Dcm_RxBuffer, 0, DCM_RX_BUFFER_SIZE);

    /* Cấp phát bộ đệm cho tầng dưới biết*/
    if(bufferSizePtr != NULL)
    {
        *bufferSizePtr = DCM_RX_BUFFER_SIZE;
    }

    return BUFREQ_OK;
}

/**
 * @brief Sao chép dữ liệu từ tầng dưới (PduR/CanTp) vào bộ đệm của DCM
 */
BufReq_ReturnType Dcm_CopyRxData(
    PduIdType id,
    const PduInfoType* info,
    PduLengthType* bufferSizePtr
)
{
    (void)id;

    if (!Dcm_IsReceiving || info == NULL || info->SduDataPtr == NULL) {
        return BUFREQ_E_NOT_OK;
    }

    if (Dcm_RxLength + info->SduLength > DCM_RX_BUFFER_SIZE) {
        return BUFREQ_E_OVFL;
    }

    /* Sao chép từng mảng dữ liệu (Payload) vào buffer tĩnh của DCM*/
    memcpy(&Dcm_RxBuffer[Dcm_RxLength], info->SduDataPtr, info->SduLength);
    Dcm_RxLength += info->SduLength;

    /* Cập nhật kích thước buffer còn lại */
    if (bufferSizePtr != NULL) {
        *bufferSizePtr = DCM_RX_BUFFER_SIZE - Dcm_RxLength;
    }

    return BUFREQ_OK;
}

/**
 * @brief Quá trình nhận dữ liệu kết thúc (gói hoàn chỉnh hoặc bị lỗi ngang)
 */
void Dcm_TpRxIndication(PduIdType id, NotifResultType result)
{
    (void)id;

    if (!Dcm_Initialized || !Dcm_IsReceiving) {return;}

    if(result == NTFRSLT_OK)
    {
        /**
         * Yêu cầu đã nhận thành công, xử lý dữ liệu trong Dcm_RxBuffer
         * UDS sẽ được bổ sung sau
         */
        Dcm_ProcessRequest();
    }

    /* Reception is finished*/
    Dcm_ResetRxState();
}

/**
 * @brief Sao chép dữ liệu từ bộ đệm của DCM sang tầng dưới (PduR/CanTp) để gửi đi
 */
BufReq_ReturnType Dcm_CopyTxData(
    PduIdType id,
    const PduInfoType* info,
    PduLengthType* availableDataPtr
)
{
    (void)id;

    if (!Dcm_Initialized || !Dcm_IsTransmitting || info == NULL || info->SduDataPtr == NULL) {
        return BUFREQ_E_NOT_OK;
    }

    if(info->SduLength > (Dcm_TxLength - Dcm_TxOffset)) {
        return BUFREQ_E_OVFL;
    }

    /**
     * Sao chép dữ liệu từ bộ đệm DCM sang tầng dưới
     */
    memcpy(info->SduDataPtr, &Dcm_TxBuffer[Dcm_TxOffset], info->SduLength);
    Dcm_TxOffset += info->SduLength;

    /* Cập nhật kích thước buffer còn lại */
    if (availableDataPtr != NULL) {
        *availableDataPtr = Dcm_TxLength - Dcm_TxOffset;
    }

    return BUFREQ_OK;
}

/**
 * DCM Diagnostic Request Processing
 */
void Dcm_ProcessRequest(void)
{
    uint8 sid;
    sid = Dcm_RxBuffer[0];

    if (Dcm_RxOffset == 0u) {return;}

    switch (sid)
    {
        case 0x10u:
            Dcm_HandleDiagnosticSessionControl();
        break;

        case 0x22u:
        Dcm_HandleReadDataByIdentifier();
        break;

        default:
        {
            uint8 negativeResponse[3];

            negativeResponse[0] = 0x7Fu;
            negativeResponse[1] = sid;
            negativeResponse[2] = 0x11u;

            if (Dcm_SetResponse(negativeResponse, 3u) == E_OK)
            {
                (void)Dcm_StartResponse();
            }

            break;
        }
    }
}

Std_ReturnType Dcm_SetResponse(const uint8* data, PduLengthType length)
{
    if(!Dcm_Initialized || data == NULL || length > DCM_TX_BUFFER_SIZE)
    {
        return E_NOT_OK;
    }
    if(Dcm_IsTransmitting)
    {
        return E_NOT_OK;
    }

    /* Copy dữ liệu phản hồi vào bộ đệm TX của DCM */
    memcpy(Dcm_TxBuffer, data, length);
    Dcm_TxLength = length;
    Dcm_TxOffset = 0;

    return E_OK;
}

Std_ReturnType Dcm_StartResponse(void)
{
    PduInfoType pduInfo;

    if(!Dcm_Initialized || Dcm_TxLength == 0)
    {
        return E_NOT_OK;
    }
    if(Dcm_IsTransmitting)
    {
        return E_NOT_OK;
    }

    /* Lớp thấp hơn sẽ yêu cầu dữ liệu thực tế thông qua Dcm_CopyTxData */
    pduInfo.SduDataPtr = NULL;  /* CanTp sẽ gọi Dcm_CopyTxData để lấy dữ liệu */
    pduInfo.SduLength = Dcm_TxLength;

    Dcm_TxOffset = 0;
    Dcm_IsTransmitting = TRUE;

    if(PduR_DcmTransmit(DCM_TX_PDU_ID, &pduInfo) != E_OK)
    {
        Dcm_ResetTxState();
        return E_NOT_OK;
    }

    return E_OK;
}

static void Dcm_HandleDiagnosticSessionControl(void)
{
    uint8 response[6];

    if (Dcm_RxLength != 2u)
    {
        return;
    }

    if ((Dcm_RxBuffer[1] != 0x01u) && (Dcm_RxBuffer[1] != 0x03u))
    {
        uint8 negativeResponse[3];

        negativeResponse[0] = 0x7Fu;
        negativeResponse[1] = 0x10u;
        negativeResponse[2] = 0x12u;

        if (Dcm_SetResponse(negativeResponse, 3u) == E_OK)
        {
            (void)Dcm_StartResponse();
        }

        return;
    }

    response[0] = 0x50u;
    response[1] = Dcm_RxBuffer[1];

    response[2] = 0x00u;
    response[3] = 0x32u;

    response[4] = 0x01u;
    response[5] = 0xF4u;

    if (Dcm_SetResponse(response, 6u) == E_OK)
    {
        (void)Dcm_StartResponse();
    }
}

void Dcm_TxConfirmation(PduIdType TxPduId, NotifResultType Result)
{
    (void)TxPduId;

    if (!Dcm_Initialized)
    {
        return;
    }

    if (Result == NTFRSLT_OK)
    {
        Dcm_IsTransmitting = FALSE;
        Dcm_TxLength = 0u;
        Dcm_TxOffset = 0u;
    }
    else
    {
        Dcm_IsTransmitting = FALSE;
    }
}