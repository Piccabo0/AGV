#include "stm32f1xx.h"
#include "string.h"
#include "Usart.h"
#include "ModBus.h"
#include "Rfid.h"

uint32_t NumOfSysTickInt=0;
uint16_t Timer_debug=0;
uint8_t debug_show=0;
//volatile unsigned long time_delay; // 延时时间，注意定义为全局变量


void SysTick_IrqHandler(void)
{
    NumOfSysTickInt++;

    if(Uart1RxTime!=0) Uart1RxTime--;
    if(Uart3RxTime!=0) Uart3RxTime--;
    if(Uart4RxTime!=0) Uart4RxTime--;

    if(HallSensor_Timeout!=0) HallSensor_Timeout--;
    if(RFID_ReadBlockTimeout!=0) RFID_ReadBlockTimeout--;
    if(RFID_ONLINE_Timeout) RFID_ONLINE_Timeout--;
    
    Timer_debug++;
    if(Timer_debug>=2000)//2000
    {
        Timer_debug=0;
        debug_show=1;
    }
}




















