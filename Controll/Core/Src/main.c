#include "main.h"
#include "Usart.h"
#include "ModBus.h"
#include "Rfid.h"
#include "Delay.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stm32f1xx_hal.h"

UART_HandleTypeDef huart4;//�ŵ���
UART_HandleTypeDef huart1;//������
UART_HandleTypeDef huart3;//RFID
//����Ԥ������
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
void Light_Task(void);//��·�������ʾ
void Mode_Config(void);//ģʽ����
void Infrared_Avoid(void);//������ϸı��־λ
void button_detect(void);//����ȥ�����
void Hall_run(void);//�ŵ���ѭ��
void speed_Config(uint16_t left, uint16_t right);
void delay_ms(unsigned int ms);

//RFID&�ŵ�����ͬ������
int CARD;
extern uint16_t left,right;
uint8_t speed[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x64, 0x00,0x00,0x00,0x64, 0x0B,0x15};//�������ݸ�ʽ
uint16_t cal_crc;
int COUNT=0;
//����
int BUTTON_RED=1;
int BUTTON_GREEN=1;
int BUTTON_BLUE=1;
int BUTTON=0;
int STOP=0x00;//�������FLAG
int ROTATE=0;

//��������֡����
//��uint8_t����Ϊ�޷����ַ���
uint8_t rotate[16]={0x01,0x16,0x00,0x38,0x00,0x04,0x00,0x00,0x00,0x96,0x00,0x01,0x00,0x96,0xE2,0x85};
uint8_t go[16]={0x01,0x16,0x00,0x38,0x00,0x04,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x64,0x0B,0x15};//Left=100��Right=100
uint8_t stop[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x00, 0x00,0x00 ,0x00,0x00, 0x7B,0x36};
uint8_t rotate_right[16]={0x01,0x16,0x00,0x38,0x00,0x04,0x00,0x00,0x00,0x64,0x00,0x01,0x00,0x64,0x5A,0xD5};//��ת
//�Ƕȿ���
uint8_t l_90[16] = {0x01,0x16,0x00,0x36,0x00,0x02 ,0x00,0x00,0x00,0x5A, 0x68,0x5A};//��ת90��
uint8_t l_45[16] = {0x01,0x16,0x00,0x36,0x00,0x02 ,0x00,0x00,0x00,0x2D, 0x28,0x7C};//��ת45��
uint8_t r_90[16] = {0x01,0x16,0x00,0x36,0x00,0x02 ,0x00,0x00,0x00,0x5A, 0x39,0x9A};//��ת90��
uint8_t r_45[16] = {0x01,0x16,0x00,0x36,0x00,0x02 ,0x00,0x00,0x00,0x2D, 0x79,0xBC};//��ת45��

//ģʽ����
	uint8_t s_mod[8] = {0x01,0x06,0x00,0x01,0x00,0x00,0xD8,0x0A};//����С��Ϊ�ٶ�ģʽ
	uint8_t w_mod[8] ={0x01,0x06,0x00,0x01,0x00,0x01,0x19,0xCA};//����С��Ϊλ��ģʽ
	uint8_t j_mod[8] = {0x01,0x06,0x00,0x01,0x00,0x02,0x59,0xCB};//����Ϊ�Ƕ�ģʽ

char test_buffer[256];

int main(void)
{
  HAL_Init();//HAL���ʼ��
  SystemClock_Config();//��ʼ��ϵͳʱ��
  MX_GPIO_Init();//GPIO���ų�ʼ��
  MX_UART4_Init();//�ŵ������������ڳ�ʼ��
  MX_USART1_UART_Init();//����ͨѶ����
  MX_USART3_UART_Init();//RFID���������ڳ�ʼ��
	
	Light_Task();//����Ƶ����
	
  printf("---- CAU Sensors Demo ----\r\n");
  printf("Build Time: %s, %s\r\n", __DATE__, __TIME__);
	Mode_Config();//����С��������ģʽ:1==speed��2==distance��3==angle
	HAL_Delay(100);
	//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//�����
  while (1)
	{
		UART_Task();  
		MODBUS_READ_HALL_SERSOR_Task(); 
		READ_RFID_BLOCK_Task();
		//�Ƶƣ�GPIOE_PIN_3 				���ƣ�GPIOE_PIN_4 				��ƣ�GPIOE_PIN_5  				�̵ƣ�GPIOE_PIN_6
		if(BUTTON==0x01)
		{
			Infrared_Avoid();
			if(STOP==0x00)
			{
				//FillUartTxBufN((uint8_t*)go, 16, 1
				//delay_ms(50);
				
				if(ROTATE==1)
				{
					FillUartTxBufN((uint8_t*)rotate_right, 16, 1);
					//delay_ms(100);
					//FillUartTxBufN((uint8_t*)stop, 16, 1);
				}
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);//�����
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);//�̵���
			}
			else
			{	
				FillUartTxBufN((uint8_t*)stop, 16, 1);
				//delay_ms(50);
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//�����
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//�̵���
			}
		}
		else
		{		
			FillUartTxBufN((uint8_t*)stop, 16, 1);
			//delay_ms(50);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//�����
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//�̵���
		}
		
    if(debug_show)
    {
      debug_show=0;
      if(USART_BYTE == 'H')         
      {
        sprintf(test_buffer, "HALL [%d] : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", MODBUS_HallSensor.read_success_num,
             HallValue[0],HallValue[1],HallValue[2],HallValue[3],
             HallValue[4],HallValue[5],HallValue[6],HallValue[7],
             HallValue[8],HallValue[9],HallValue[10],HallValue[11],
             HallValue[12],HallValue[13],HallValue[14],HallValue[15]
             );
         FillUartTxBufN((uint8_t*)test_buffer, strlen(test_buffer), DEBUG_USART_ENUM);        
      }
    }		

  }//while
}//main

//�жϷ���ص� �ó�����û�� һ��ʼ���Ե�ʱ����������������С��
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//PE9���жϷ����Ӻ���
{
	/*���*/
		COUNT+=1;
	if(GPIO_Pin==GPIO_PIN_11)
	{
		//STOP=1;
		if(COUNT%2==0)
			{
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);//�Ƶ���
				}
			else
				{
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);//�Ƶ���
					}
				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);//����жϱ�ǣ�����������ɵ��жϱ����λ
	}
	else if(GPIO_Pin==GPIO_PIN_9)
	{
		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9)==0)
		{
			BUTTON=0x01;
		}
		else
		{
			BUTTON=0x00;
		}
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);//����жϱ�ǣ�����������ɵ��жϱ����λ
	}
					
 }


 
 void delay_ms(unsigned int ms)
{
     SysTick_Config(72000);
     for(int i = 0 ; i < ms ;i++)  
    {
      	while(!((SysTick->CTRL)&(1<<16))); //��ʱ������16λ�Ƿ���1  ���Ϊ1˵��ʱ�䵽���˳�ѭ��
    }
     SysTick->CTRL =0x00;
}


void Hall_run(void)
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
		uint8_t speed1[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x64, 0x00,0x00,0x00,0x64, 0x0B,0x15};
	 if(HallValue[15]+HallValue[14]+HallValue[13]+HallValue[12]+HallValue[11]+HallValue[10]+HallValue[9]+HallValue[8]+HallValue[7]+HallValue[6]+HallValue[5]+HallValue[4]+HallValue[3]+HallValue[2]+HallValue[1]+HallValue[0] != 0)//���ź�
		 {
			 right =   HallValue[15]*q1+HallValue[14]*q2+HallValue[13]*q3+HallValue[12]*q4+HallValue[11]*q5+HallValue[10]*q6+HallValue[9]*q7+HallValue[8]*q8+_const;
			 left  =   HallValue[0]*q1+HallValue[1]*q2+HallValue[2]*q3+HallValue[3]*q4+HallValue[4]*q5+HallValue[5]*q6+HallValue[6]*q7+HallValue[7]*q8+_const;
			 speed1[7] = 0x00; //���ַ�����ת
			 speed1[11] = 0x00;//���ַ�������
			 speed1[9] = left&0xFF;
			 speed1[8] = left>>8;
			 speed1[13] = right&0xFF ;
			 speed1[12] = right>>8;
			 cal_crc=ModBus_CRC16_Calculate(speed1 , sizeof(speed1)-2);
			 speed1[14]=cal_crc&0xFF;
			 speed1[15]=cal_crc>>8; 
			 FillUartTxBufN((uint8_t*)speed1, 16, 1);
			 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);//������
			 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);//�����
		 }
		 else
			 //�������
			{
				FillUartTxBufN((uint8_t*)stop, 16, 1);
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);//������
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//�����
			}

}

void speed_Config(uint16_t left, uint16_t right)
{
	uint8_t speed1[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x64, 0x00,0x00,0x00,0x64, 0x0B,0x15};//�������ݸ�ʽ
	speed1[7] = 0x00; //���ַ�����ת
	speed1[11] = 0x00;//���ַ�������
	speed1[9] = left&0xFF;
	speed1[8] = left>>8;
	speed1[13] = right&0xFF ;
	speed1[12] = right>>8;
	cal_crc=ModBus_CRC16_Calculate(speed1 , sizeof(speed1)-2);
	speed1[14]=cal_crc&0xFF;
	speed1[15]=cal_crc>>8; 
	FillUartTxBufN((uint8_t*)speed1, 16, 1);
}

//�����·������״̬
void button_detect(void)
{
	//������������������==0
	//��ɫ��ť��GPIOE_PIN_9 			��ɫ��ť��GPIOE_PIN_10				��ɫ��ť��GPIOE_PIN_11
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9)==0)
		{
			HAL_Delay(30);
			if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9)==0)
			{
				BUTTON_BLUE =	0;
			}
			else
			{
				BUTTON_BLUE =	1;
			}
		}
		else
		{
			BUTTON_BLUE =	1;
		}
		
		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_10)==0)
		{
			HAL_Delay(30);
			if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_10)==0)
			{
				BUTTON_GREEN =	0;
			}
			else
			{
				BUTTON_GREEN =	1;
			}
		}
		else
		{
			BUTTON_GREEN =	1;
		}

		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_11)==0)
		{
			HAL_Delay(30);
			if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_11)==0)
			{
				BUTTON_RED =	0;
			}
			else
			{
				BUTTON_RED =	1;
			}
		}
		else
		{
			BUTTON_RED =	1;
		}		
}


void Infrared_Avoid(void)
{	
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15)+HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_8)>0)
	{
		STOP=0x01;
	}//�������
	else 
	{
		STOP=0x00;
	}
}


//��յƵ���ʾ
void Light_Task(void)
{
	//GPIO_PIN_RESET��Ϩ�� & GPIO_PIN_SET��������
	//�Ƶƣ�GPIOE_PIN_3 				���ƣ�GPIOE_PIN_4 				��ƣ�GPIOE_PIN_5  				�̵ƣ�GPIOE_PIN_6
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

}

void Mode_Config(void)
{
	FillUartTxBufN((uint8_t*)s_mod, 8, 1);
	//FillUartTxBufN((uint8_t*)j_mod, 8, 1);
	/*
	if(mode==1)
	{
		FillUartTxBufN((uint8_t*)s_mod, 8, 1);
	}
	else if(mode ==2)
	{
		FillUartTxBufN((uint8_t*)w_mod, 8, 1);
	}
	else
	{
		FillUartTxBufN((uint8_t*)j_mod, 8, 1);
	}
	*/
	//HAL_Delay(100);
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  /* ��ʱ�� */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

  /*��ʼ����λ0 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);//���������������˼�������Ǹ�ĳ������д0��1
  /*GPIO�ṹ�嶨�� */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//�ĸ���3/4/5/6�ĳ�ʼ��
	 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);//���õƳ�ʼ״̬
  GPIO_InitStruct.Pin =GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);//��������Ϊ���״̬
	
	//������ϴ�����
	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	//��������
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	//PE9�ж�
	

	GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);//PE9
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  //HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);//PE11
  //HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/