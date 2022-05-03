#include "Usart.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "stm32f1xx_hal_uart.h"
#include "ModBus.h"
#include "Rfid.h"


extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/*上一个接受或者发送的字符*/
uint8_t USART_Byte[5] = {0};
uint8_t USART_UpInforEnable = 0;
uint16_t Uart1RxTime;
uint16_t Uart3RxTime;
uint16_t Uart4RxTime;

UART_OPTION UART1_Optx={false,0,0};
UART_OPTION UART1_Oprx={true,0,0,};
UART_OPTION UART3_Optx={false,0,0};
UART_OPTION UART3_Oprx={true,0,0,};
UART_OPTION UART4_Optx={false,0,0};
UART_OPTION UART4_Oprx={true,0,0,};



/*用于比特串的接收超时(ms)*/
#define B_115200_SILENCE_TIME 2 
#define B_57600_SILENCE_TIME  2
#define B_38400_SILENCE_TIME  2   //5BYTES
#define B_19200_SILENCE_TIME  3   //5BYTES
#define B_9600_SILENCE_TIME   5   //5BYTES
#define B_4800_SILENCE_TIME   10  //5bytes
#define B_2400_SILENCE_TIME   20  //5bytes
#define B_1200_SILENCE_TIME   40  //5bytes

/*用于RS485,发送2接收的转换延时*/
#define RS485_SLAVE_TX_2_RX_DELAY_115200_TIME 2
#define RS485_SLAVE_TX_2_RX_DELAY_57600_TIME  2
#define RS485_SLAVE_TX_2_RX_DELAY_38400_TIME  2
#define RS485_SLAVE_TX_2_RX_DELAY_19200_TIME  (1+2)
#define RS485_SLAVE_TX_2_RX_DELAY_9600_TIME   (1+2)
#define RS485_SLAVE_TX_2_RX_DELAY_4800_TIME   (3+2)
#define RS485_SLAVE_TX_2_RX_DELAY_2400_TIME   (7+2)
#define RS485_SLAVE_TX_2_RX_DELAY_1200_TIME   (15+2)

const uint16_t RS485_SLAVE_TX_2_RX_DELAY_List[9]=
{
  RS485_SLAVE_TX_2_RX_DELAY_115200_TIME,//default
  RS485_SLAVE_TX_2_RX_DELAY_1200_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_2400_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_4800_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_9600_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_19200_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_38400_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_57600_TIME,
  RS485_SLAVE_TX_2_RX_DELAY_115200_TIME
};

uint16_t Uart1RxTimeReload=B_9600_SILENCE_TIME;
uint16_t Uart2RxTimeReload=B_9600_SILENCE_TIME;
uint16_t Uart3RxTimeReload=B_9600_SILENCE_TIME;
uint16_t Uart4RxTimeReload=B_9600_SILENCE_TIME;
uint16_t Uart5RxTimeReload=B_19200_SILENCE_TIME;

uint16_t RS485_SLAVE_TX_2_RX_Delay=RS485_SLAVE_TX_2_RX_DELAY_115200_TIME;


void UART1_ISR(void)
{ 
  if(USART1->SR&UART_FLAG_RXNE)//如果状态寄存器SR&接受寄存器=1；
  {
    USART_Byte[0] = USART1->DR;
    UART1_Oprx.Buf[UART1_Oprx.InIndex++] = USART_Byte[0];    
    Uart1RxTime=Uart1RxTimeReload;//20ms
  }
  if((USART1->SR&UART_FLAG_TXE)&&(UART1_Optx.Intrrupt==true))//如果状态寄存器SR&发送标志位=1，发送中断打开；
  {
    USART1->DR=UART1_Optx.Buf[UART1_Optx.OutIndex++];////fill can clear flag
    if(UART1_Optx.OutIndex==UART1_Optx.InIndex)
    {
      __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
      UART1_Optx.Intrrupt=false;
    }
  }
}

uint32_t rx3 = 0;
void UART3_ISR(void)
{
  if(USART3->SR&UART_FLAG_RXNE)
  {
    UART3_Oprx.Buf[UART3_Oprx.InIndex++]=USART3->DR;//fill can clear flag
    Uart3RxTime=Uart3RxTimeReload;
    rx3 += 1;

  }

  if((USART3->SR&UART_FLAG_TXE)&&(UART3_Optx.Intrrupt==true))
  {
    USART3->DR=UART3_Optx.Buf[UART3_Optx.OutIndex++];////fill can clear flag
    if(UART3_Optx.OutIndex==UART3_Optx.InIndex)
    {
      __HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE);
      UART3_Optx.Intrrupt=false;
    }
  }

}

void UART4_ISR(void)
{
  if(UART4->SR&UART_FLAG_RXNE)
  {
    UART4_Oprx.Buf[UART4_Oprx.InIndex++]=UART4->DR;//fill can clear flag
    Uart4RxTime=Uart4RxTimeReload;
  }
  
  if((UART4->SR&UART_FLAG_TXE)&&(UART4_Optx.Intrrupt==true))
  {
    UART4->DR=UART4_Optx.Buf[UART4_Optx.OutIndex++];////fill can clear flag
    if(UART4_Optx.OutIndex==UART4_Optx.InIndex)
    {
      __HAL_UART_DISABLE_IT(&huart4, UART_IT_TXE);
      UART4_Optx.Intrrupt=false;
    }
  }
}



void FillUartTxBufN(uint8_t* pData,uint8_t num,uint8_t U1_2_3)
{
  UART_OPTION* pUART;
  if(U1_2_3==1)
    pUART= &UART1_Optx;
  else if(U1_2_3==2)
    pUART= &UART1_Optx;
  else if(U1_2_3==3)
     pUART= &UART3_Optx;
  else if(U1_2_3==4)
     pUART= &UART4_Optx;  
  else 
     pUART= &UART1_Optx;
  for(uint8_t i=0;i<num;i++)
    pUART->Buf[pUART->InIndex++]=pData[i];
}

/*
	根据标志位使能对应的中断，并且根据串口的信息进行数据帧的解析
*/
void UART_Task(void)
{  
  if(UART1_Optx.Intrrupt==false)
  {
    if(UART1_Optx.OutIndex!=UART1_Optx.InIndex)
    {
      UART1_Optx.Intrrupt=true;
      __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
    }
  } 
  
  if(UART3_Optx.Intrrupt==false)
  {
    if(UART3_Optx.OutIndex!=UART3_Optx.InIndex)
    {
      UART3_Optx.Intrrupt=true;
      __HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);
			
			__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    }
  }    
  
  if(UART4_Optx.Intrrupt==false)
  {
    if(UART4_Optx.OutIndex!=UART4_Optx.InIndex)
    {
      UART4_Optx.Intrrupt=true;
      __HAL_UART_ENABLE_IT(&huart4, UART_IT_TXE);
    }
  }
  
  
  if(UART1_Oprx.InIndex!=UART1_Oprx.OutIndex)
  {
    
  }
  
  if(UART3_Oprx.InIndex!=UART3_Oprx.OutIndex)
  {
    Analysis_Receive_From_RFID(UART3_Oprx.Buf[UART3_Oprx.OutIndex++]);//RFID
  }
  
  if(UART4_Oprx.InIndex!=UART4_Oprx.OutIndex)
  {
    Analysis_Receive_From_HallSensor(UART4_Oprx.Buf[UART4_Oprx.OutIndex++], &MODBUS_HallSensor);//磁导航传感器
  }      
  
  if(Uart1RxTime==0)
  {

  }
  
  if(Uart3RxTime==0)
  {
    if(RfidMachineState)
    {
      RfidMachineState=0;//重新数据同步
    }
  }
  
  if(Uart4RxTime==0)
  {
    if(MODBUS_HallSensor.MachineState) 
    {
      MODBUS_HallSensor.MachineState = 0;
    }   
  }
}


int fputc(int ch, FILE *f)  
{ 
  DEBUG_USART->DR = (uint8_t) ch;  
  while((DEBUG_USART->SR & UART_FLAG_TXE) == RESET); 
  return ch; 
}
