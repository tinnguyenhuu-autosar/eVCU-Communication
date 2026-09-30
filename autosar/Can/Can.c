/**
 * @file Can.c
 * @brief AUTOSAR Can Driver - bxCAN nội bộ STM32F103
 *
 * @details Module MCAL điều khiển CAN tích hợp (bxCAN) trên
 *         vi điều khiển STM32F103 thông qua thư viện SPL(Standard
 *        Peripheral Library).
 */

#include "Can.h"
#include "CanIf.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "misc.h"
#include <stddef.h>

/**
 * Định nghĩa phần cứng
 */
// #define CAN_PERIPH CAN1
// #define CAN_GPIO_PORT GPIOA
// #define CAN_RX_PIN GPIO_Pin_11
// #define CAN_TX_PIN GPIO_Pin_12

/**
 * Biến trạng thái nội bộ
 */
#define CAN_TX_SLOTS 6u

static boolean s_inited = FALSE;
static const Can_ConfigType* Can_ConfigPtr = NULL;

static volatile PduIdType s_swPduHandle[CAN_TX_SLOTS];
static volatile boolean s_txPending[CAN_TX_SLOTS];
static volatile uint8 s_txMailbox[CAN_TX_SLOTS];

/**
 * Private Functions
 */

/**
 * @brief Cấu hình GPIO cho CAN
 * @details Cấu hình chân PA11/PA12 cho chức năng CAN_RX/CAN_TX
 */
static void prv_CAN_GPIO_Init(void)
{
    /**
     * Bật Clock cho GPIOA, AFIO và CAN1
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /**
     * Cấu hình chân PA12 (CAN_TX) là Alternate Function Push-Pull
     */
    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);

    gpio.GPIO_Pin = Can_ConfigPtr->Controller.Tx.Pin;
    gpio.GPIO_Speed = Can_ConfigPtr->Controller.Tx.Speed;
    gpio.GPIO_Mode = Can_ConfigPtr->Controller.Tx.Mode;
    GPIO_Init(Can_ConfigPtr->Controller.Tx.Port, &gpio);

    /**
     * Cấu hình chân PA11 (CAN_RX) là Input Floating
     */
    gpio.GPIO_Pin = Can_ConfigPtr->Controller.Rx.Pin;
    gpio.GPIO_Mode = Can_ConfigPtr->Controller.Rx.Mode;
    GPIO_Init(Can_ConfigPtr->Controller.Rx.Port, &gpio);
}

/**
 * @brief Cấu hình phần cứng CAN
 * @details Cấu hình các thông số cơ bản của CAN, bao gồm baudrate, mode, filter, v.v.
 * Gồm 2 phần: A.Cấu hình CAN(CAN_Init): bit timing, mode, tính năng
 *             B. Cấu hình Filter(CAN_FilterInit): bộ lọc nhận
 */
static void prv_CAN_Periph_Init(void)
{
    /*Reset Can peripheral về trạng thái mặc định*/
    CAN_DeInit(Can_ConfigPtr->Controller.Instance);

    /*A. Cấu hình CAN*/
    CAN_InitTypeDef canInit;
    CAN_StructInit(&canInit);

    /*Bit timing*/
    canInit.CAN_Prescaler = Can_ConfigPtr->Controller.BitTiming.Prescaler;
    canInit.CAN_SJW = Can_ConfigPtr->Controller.BitTiming.SJW;
    canInit.CAN_BS1 = Can_ConfigPtr->Controller.BitTiming.BS1;
    canInit.CAN_BS2 = Can_ConfigPtr->Controller.BitTiming.BS2;

    /*Mode*/
    canInit.CAN_Mode = Can_ConfigPtr->Controller.CanMode;

    /*Tính năng*/
    canInit.CAN_TTCM = Can_ConfigPtr->Controller.Feature.TTCM;
    canInit.CAN_ABOM = Can_ConfigPtr->Controller.Feature.ABOM;
    canInit.CAN_AWUM = Can_ConfigPtr->Controller.Feature.AWUM;
    canInit.CAN_NART = Can_ConfigPtr->Controller.Feature.NART;
    canInit.CAN_RFLM = Can_ConfigPtr->Controller.Feature.RFLM;
    canInit.CAN_TXFP = Can_ConfigPtr->Controller.Feature.TXFP;

    /*Khởi tạo bxCAN với cấu hình trên */
    CAN_Init(Can_ConfigPtr->Controller.Instance, &canInit);

    /**
     * B. Cấu hình Filter
     */
    CAN_FilterInitTypeDef filterInit;
    filterInit.CAN_FilterNumber     = Can_ConfigPtr->Filter.Number;
    filterInit.CAN_FilterMode       = Can_ConfigPtr->Filter.Mode;
    filterInit.CAN_FilterScale      = Can_ConfigPtr->Filter.Scale;
    filterInit.CAN_FilterIdHigh     = Can_ConfigPtr->Filter.IdHigh;
    filterInit.CAN_FilterIdLow      = Can_ConfigPtr->Filter.IdLow;
    filterInit.CAN_FilterMaskIdHigh = Can_ConfigPtr->Filter.MaskIdHigh;
    filterInit.CAN_FilterMaskIdLow  = Can_ConfigPtr->Filter.MaskIdLow;
    filterInit.CAN_FilterActivation = Can_ConfigPtr->Filter.Activation;
    CAN_FilterInit(&filterInit);
}

/**
 * @brief Can_Init - API công khai: Khởi tạo CAN Driver
 * @details Hàm này thực hiện các bước sau:
 */
void Can_Init(const Can_ConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL)
    {
        return;
    }

    /* Lưu địa chỉ cấu hình vào driver*/
    Can_ConfigPtr = ConfigPtr;

    /*Bước 1 cấu hình GPIO*/
    prv_CAN_GPIO_Init();

    /*Bước 2 cấu hình bxCAN peripheral (bit timing, filter)*/
    prv_CAN_Periph_Init();

    /*Bước 3 Clear trạng thái TX cho tất cả các slot*/
    for (uint8 i = 0; i < CAN_TX_SLOTS; ++i)
    {
        s_txPending[i] = FALSE;
        s_swPduHandle[i] = 0u;
        s_txMailbox[i] = 0u;
    }

    /*Bước 4 bật CAN RX FIFO interrupt*/
    CAN_ITConfig(Can_ConfigPtr->Controller.Instance, CAN_IT_FMP0, ENABLE);

    /*Bước 5 Cấu hình NVIC cho CAN1 RX0 (IRQ 20 = USB_LP_CAN1_RX0)*/
    NVIC_InitTypeDef nvicInit;
    nvicInit.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 1u;
    nvicInit.NVIC_IRQChannelSubPriority = 0u;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

    /*Bước 6 đánh dấu driver đã sẵn sàng*/
    s_inited = TRUE;
}
/**
 * @brief Can_Write - API công khai: Gửi một PDU CAN
 * @details Hàm này thực hiện việc gửi một PDU qua giao thức CAN
 */
Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType *PduInfo)
{
    /*Bước 1: Kiểm tra điều kiện tiên quyết*/
    if (s_inited == FALSE || PduInfo == NULL || PduInfo->sdu == NULL)
    {
        return CAN_NOT_OK;
    }

    /*Bước 2: giới hạn chỉ số slot (bxCAN có 3 slot mailbox: 0 1 2 )*/
    uint8 slot = (Hth < CAN_TX_SLOTS) ? (uint8)Hth : 0u;

    /*Kiểm tra slot có đang chờ TX hoàn tất không*/
    if (s_txPending[slot])
    {
        return CAN_BUSY;
    }

    /*Bước 3: chuyển đổi Can_PduType => CanTxMsg (SPL)*/
    CanTxMsg txMsg;

    /*3a. DLC (Data Length Code): giới hạn tối da 8byte*/
    txMsg.DLC = (PduInfo->length <= 8u) ? PduInfo->length : 8u;

    /*3b. CAN ID: phân biệt Standard (11bit) và Extended (29bit)*/
    if (PduInfo->id > 0x7FFu)
    {
        txMsg.IDE = CAN_Id_Extended;
        txMsg.ExtId = PduInfo->id & 0x1FFFFFFFu;
        txMsg.StdId = 0u;
    }
    else
    {
        txMsg.IDE = CAN_Id_Standard;
        txMsg.StdId = PduInfo->id & 0x7FFu;
        txMsg.ExtId = 0u;
    }

    /*3c. RTR: Data Frame (Not Remote Frame)*/
    txMsg.RTR = CAN_RTR_Data;

    /*3d. copy dữ liệu payload*/
    for (uint8 i = 0; i < txMsg.DLC; i++)
    {
        txMsg.Data[i] = PduInfo->sdu[i];
    }

    /*Bước 4: gọi SPL API để nạp frame vào mailbox bxCAN*/
    uint8_t mailbox = CAN_Transmit(Can_ConfigPtr->Controller.Instance, &txMsg);

    if (mailbox == CAN_TxStatus_NoMailBox)
    {
        return CAN_BUSY;
    }

    /*Bước 5: lưu thông tin cho callback TxConfirmation*/
    s_swPduHandle[slot] = PduInfo->swPduHandle;
    s_txMailbox[slot] = mailbox;
    s_txPending[slot] = TRUE;

    return CAN_OK;
}

/**
 * @brief Can_MainFunction_Write - Polling kiểm tra TX hoàn tất
 * @details được gọi định kỳ trong vòng lặp chính
 */
void Can_MainFunction_Write(void)
{
    if (s_inited == FALSE)
        return;

    for (uint8 i = 0; i < CAN_TX_SLOTS; ++i)
    {
        if (s_txPending[i])
        {
            /*hỏi SPL: mailbox này đã truyền xong chưa?*/
            uint8_t status = CAN_TransmitStatus(Can_ConfigPtr->Controller.Instance, s_txMailbox[i]);

            if (status == CAN_TxStatus_Ok)
            {
                /*TX thành công -> báo lên CanIf qua callback*/
                PduIdType handle = s_swPduHandle[i];
                s_txPending[i] = FALSE;
                CanIf_TxConfirmation(handle);
            }
            else if (status == CAN_TxStatus_Failed)
            {
                /*TX thất bại (bug error, arbitration lost,...)*/
                s_txPending[i] = FALSE;
            }
            /*CAN_TxStatus_Pending -> bxCAN đang truyền, chờ tiếp*/
        }
    }
}

/**
 * @brief Can_MainFunction_Read - Polling kiểm tra RX
 * @details trích xuất Msg từ FIFO0, FIFO1 sau đó báo lên CanIf_RxIndication
 */
void Can_MainFunction_Read(void)
{
    if (s_inited == FALSE)
        return;

    CanRxMsg rxMsg;
    PduInfoType pduInfo;

    /*Kiểm tra FIFO (backup polling - chính đã xử lý qua ISR)*/
    while (CAN_MessagePending(Can_ConfigPtr->Controller.Instance, CAN_FIFO0) > 0)
    {
        CAN_Receive(Can_ConfigPtr->Controller.Instance, CAN_FIFO0, &rxMsg);

        pduInfo.SduDataPtr = rxMsg.Data;
        pduInfo.SduLength = rxMsg.DLC;

        CanIf_RxIndication(rxMsg.StdId, &pduInfo);
    }

    /*Kiểm tra FIFO1*/
    while (CAN_MessagePending(Can_ConfigPtr->Controller.Instance, CAN_FIFO1) > 0)
    {
        CAN_Receive(Can_ConfigPtr->Controller.Instance, CAN_FIFO1, &rxMsg);

        pduInfo.SduDataPtr = rxMsg.Data;
        pduInfo.SduLength = rxMsg.DLC;

        CanIf_RxIndication(rxMsg.StdId, &pduInfo);
    }
}

/**
 * @brief USB_LP_CAN1_RX0_IRQHandler - ISR nhận CAN frame từ FIFO
 * @details Renode STMCAN model kích hoạt interrupt khi frame đến FIFO.
 * ISR này đọc frame và chuyển lên CanIf_RxIndication.
 */
__attribute__((weak)) void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CanRxMsg rxMsg;
    PduInfoType pduInfo;
}
