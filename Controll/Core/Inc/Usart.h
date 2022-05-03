
#ifndef _Usart_h
#define _Usart_h

/******************************************************************************/
/*预处理部分*/
#include "stm32f1xx.h"
/*使用Printf*/
#define  PRINTF_EN

#ifdef   PRINTF_EN
  #include "stdio.h"
#endif 

#define DEBUG_USART_ENUM  1
#define DEBUG_USART       USART1  //UART5 USART1

#define false 0
#define true 1
/******************************************************************************/
/*--------------------宏定义--------------------*/


/******************************************************************************/
/*--------------------全局变量外包括--------------------*/
extern uint8_t USART_Byte[5];
                

          
#define USART_BYTE USART_Byte[DEBUG_USART_ENUM - 1]


typedef enum
{
  RS485_IDEL = 0,
  RS485_RX_ENABLE,
  RS485_TX_INIT,
  RS485_TX_ENABLE,
  RS485_STATUS_NUM  
}RS485_STATUS;

typedef struct
{
  uint8_t Intrrupt;
  uint8_t InIndex;
  uint8_t OutIndex;
  uint8_t Buf[256];
}UART_OPTION;
extern UART_OPTION UART_Optx;
extern UART_OPTION UART_Oprx;


extern uint16_t Uart1RxTime;
extern uint16_t Uart4RxTime;

extern uint16_t Uart3RxTime;
extern uint16_t Uart5RxTime;
extern UART_OPTION UART1_Optx;
extern UART_OPTION UART1_Oprx;
extern UART_OPTION UART2_Optx;
extern UART_OPTION UART2_Oprx;
extern UART_OPTION UART5_Oprx;
extern uint16_t RS485_SLAVE_TX_2_RX_Delay;
extern const uint16_t RS485_SLAVE_TX_2_RX_DELAY_List[9];

void Usart2_Init(void);
void UART1_ISR(void);
void UART2_ISR(void);
void UART_Task(void);
void FillUartTxBuf(uint8_t data);
uint8_t Deal_UART_Infor(uint8_t* pINFOR,uint8_t Length);
uint8_t CheckSum(uint8_t *ptr,uint8_t length);
void FillUartTxBufN(uint8_t* pData,uint8_t num,uint8_t U1_2);

uint8_t Deal_ModBus_Infor(uint8_t* pINFOR,uint8_t Length);

void DEBUG_COM_Protocol(void);
void UART3_ISR(void);
void Usart3_Init(void);
void Usart4_Init(void);
void Usart5_Init(void);
void UART4_ISR(void);
void UART5_ISR(void);




#define RS485_1_DIR_PIN_RE       GPIO_PIN_14
#define RS485_1_DIR_R_PORT       GPIOA
#define RS485_1_DIR_PIN_DE       GPIO_PIN_15
#define RS485_1_DIR_D_PORT       GPIOA
#define RS485_1_RX_Active()      RS485_1_DIR_R_PORT->BRR=RS485_1_DIR_PIN_RE;RS485_1_DIR_D_PORT->BRR=RS485_1_DIR_PIN_DE
#define RS485_1_TX_Active()      RS485_1_DIR_R_PORT->BSRR=RS485_1_DIR_PIN_RE;RS485_1_DIR_D_PORT->BSRR=RS485_1_DIR_PIN_DE


extern uint32_t rx3;



#endif

