#include "timer.h"
#include "Usart.h"
#include "ModBus.h"
#include "Rfid.h"
#include "Delay.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stm32f1xx_hal_tim.h"
#include  "main.h"
#include  "rs485.h"


TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 
HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);
/*
int BUTTON_ALL=0;
int BUTTON_ROUND=0;
int abc;
int CARD=0;
int COUNT=0;
uint8_t a=0;
uint8_t weight[16]={55,50,45,40,35,30,20,10,10,20,30,35,40,45,50,55};
uint16_t left,right;
uint8_t s_buffer[256];
uint8_t speed[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x64, 0x00,0x00,0x00,0x64, 0x0B,0x15};
uint16_t cal_crc;
uint8_t d_mod[8] ={0x01,0x06,0x00,0x01,0x00,0x02,0x59,0xCB};
*/
//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //ͨ�ö�ʱ��3
    TIM3_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM3_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM3_Handler);   
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE   
}

//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM3_IRQn,2,3);    //�����ж����ȼ�����ռ���ȼ�2�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //����ITM3�ж�   
	}
}

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}









/*
////////////////////////////���¾����дRFID��������ָ��/////////////////////////////////////////////
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		
		int q1=65;
		int q2=40;
		int q3=30;
		int q4=25;
		int q5=25;
		int q6=25;
		int q7=25;
		int q8=20;	
		int _const =130;
	
	
	 if(htim==(&TIM3_Handler))
	 {
		 
		 if( BUTTON_ALL ==0)
		 {
			 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
			 
			 
		 if(HallValue[15]+HallValue[14]+HallValue[13]+HallValue[12]+HallValue[11]+HallValue[10]+HallValue[9]+HallValue[8]+HallValue[7]+HallValue[6]+HallValue[5]+HallValue[4]+HallValue[3]+HallValue[2]+HallValue[1]+HallValue[0]+CARD > 0)//���ź�
		 {
			 
			 if(CARD == 0)
			 {
				 right =   HallValue[15]*q1+HallValue[14]*q2+HallValue[13]*q3+HallValue[12]*q4+HallValue[11]*q5+HallValue[10]*q6+HallValue[9]*q7+HallValue[8]*q8+_const;
				 left  =   HallValue[0]*q1+HallValue[1]*q2+HallValue[2]*q3+HallValue[3]*q4+HallValue[4]*q5+HallValue[5]*q6+HallValue[6]*q7+HallValue[7]*q8+_const;					
										
										speed[7] = 0x00; 
										speed[11] = 0x00;
										speed[9] = left&0xFF;
										speed[8] = left>>8;
										speed[13] = right&0xFF ;
										speed[12] = right>>8;
										cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
										speed[14]=cal_crc&0xFF;
										speed[15]=cal_crc>>8; 
										FillUartTxBufN((uint8_t*)speed, 16, 1);
			 }
			 else if(CARD == 1)
			 {
				// HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
				 //HAL_Delay(500);
				// HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
					if(COUNT<20)
					{
												right= 200;
												left= 200;
												HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);								
					}
					else
					{
						CARD=0;
						COUNT=0;
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
					}
					
			 }
			 else if(CARD == 2)
			 {
				// HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
				 //HAL_Delay(1000);
				// HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
									if(COUNT <7)
										{
												right= 100;
												left= 10;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);													
										}
										else 
										{
											CARD = 0;
											COUNT = 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
										}
			 }	
			 
			 else if(CARD == 3)
			 {
				
				 if(COUNT <7)
										{
												right= 100;
												left= 10;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);													
										}
										else 
										{
											CARD = 0;
											COUNT = 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
										}
			 }	
			 else if(CARD == 4)
			 {
		
				 if(COUNT <7)
										{
												right= 10;
												left= 100;
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);													
										}
										else 
										{
											CARD = 0;
											COUNT = 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
										}
			 }	
			 else if(CARD == 5)
			 {
		
				 if(COUNT <2)
										{
												right= 16;
												left= 160;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);													
										}
										else 
										{
											CARD = 0;
											COUNT = 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
										}
			 }	
			 else if(CARD == 6)
			 {
				
				 if(COUNT <20)
										{
												right= 0;
												left= 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);	
												HAL_Delay(10);
												
										}
										else 
										{
											CARD = 0;
											COUNT = 0;
											HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
										}
			 }	
			 else if(CARD == 7)
			 {
			
				 	if(COUNT<5)
					{
												right= 200;
												left= 200;
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);											
					}
					else
					{
						CARD=0;
						COUNT=0;
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
					}
					
			 }				 
			 else if(CARD == 8)
			 {

				 	
												right= 0;
												left= 0;
												COUNT++;										
												speed[7] = 0x00; 
												speed[11] = 0x00;
												speed[9] = left&0xFF;
												speed[8] = left>>8;
												speed[13] = right&0xFF ;
												speed[12] = right>>8;
												cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
												speed[14]=cal_crc&0xFF;
												speed[15]=cal_crc>>8; 
												FillUartTxBufN((uint8_t*)speed, 16, 1);		
											
												
					}
			
			 }
			 else
			 {
								speed[7] = 0x00; 
								speed[11] = 0x00;
								speed[9] = left&0xFF;
								speed[8] = left>>8;
								speed[13] = right&0xFF ;
								speed[12] = right>>8;
								cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
								speed[14]=cal_crc&0xFF;
								speed[15]=cal_crc>>8; 
								FillUartTxBufN((uint8_t*)speed, 16, 1);
						}
			 
		 } 
			 else

			 
		 {
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
						speed[7] = 0x00;
						speed[9] = 0x00;
						speed[11] = 0x00;
						speed[8] = 0x00;
						speed[13] = 0x00;
						speed[12] = 0x00;
						cal_crc=ModBus_CRC16_Calculate(speed , sizeof(speed)-2);
						speed[14]=cal_crc&0xFF;
						speed[15]=cal_crc>>8;  
						FillUartTxBufN((uint8_t*)speed, 16, 1);
		 }
	 }
 }
*/