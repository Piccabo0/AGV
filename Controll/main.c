#include "main.h"
#include "Usart.h"
#include "ModBus.h"
#include "Rfid.h"
#include "Delay.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stm32f1xx_hal.h"

UART_HandleTypeDef huart4;//磁导航
UART_HandleTypeDef huart1;//车底盘
UART_HandleTypeDef huart3;//RFID

//函数预先声明
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
void Light_Task(void);//四路灯控制
void Mode_Config(void);
void Infrared_Avoid(void);
void button_detect(void);

//按键
int BUTTON_ROUND;

//控制数据帧定义
//将uint8_t别名为无符号字符型
uint8_t rotate[16]={0x01,0x16,0x00,0x38,0x00,0x04,0x00,0x00,0x00,0x96,0x00,0x01,0x00,0x96,0xE2,0x85};
uint8_t go[16]={0x01,0x16,0x00,0x38,0x00,0x04,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x64,0x0B,0x15};//Left=100，Right=100
uint8_t stop[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x00, 0x00,0x00 ,0x00,0x00, 0x7B,0x36};

char test_buffer[256];

//L1-L4：外设J21 22 大功率，J21绿灯，J24黄的，J23蓝的，J22红的
//J32-J34：按键J32蓝色，J33绿的，J34红色

int main(void)
{
  HAL_Init();//HAL库初始化
  SystemClock_Config();//初始化系统时钟

  MX_GPIO_Init();//GPIO引脚初始化
  MX_UART4_Init();//磁导航传感器串口初始化
  MX_USART1_UART_Init();//底盘通讯串口
  MX_USART3_UART_Init();//RFID读卡器串口初始化

  printf("---- CAU Sensors Demo ----\r\n");
  printf("Build Time: %s, %s\r\n", __DATE__, __TIME__);
	Mode_Config();//配置小车的运行模式==速度模式
	
  while (1)
  {
    UART_Task();                    
    MODBUS_READ_HALL_SERSOR_Task(); 
    READ_RFID_BLOCK_Task();
		//Light_Task();
		//Infrared_Avoid();
		button_detect();
		
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
  }

}

void button_detect(void)
{
	//按键采样
	//蓝色按钮：GPIOE_PIN_9 	绿色按钮：GPIOE_PIN_10	红色按钮：GPIOE_PIN_11
	BUTTON_ROUND=	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_11);
	if(BUTTON_ROUND==0)
	{
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);//绿灯亮
	}
	else
	{
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);//绿灯亮
	}
}


void Infrared_Avoid(void)
{
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)+HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15)+HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_8)>0)
	{
		FillUartTxBufN((uint8_t*)stop, 16, 1);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//红灯亮
		
	}//红外避障
	else 
	{
		FillUartTxBufN((uint8_t*)go, 16, 1);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);//红灯灭
	}
}

void Light_Task(void)
{
	//GPIO_PIN_RESET（熄灭） & GPIO_PIN_SET（点亮）
	//黄灯：GPIOE_PIN_3 	蓝灯：GPIOE_PIN_4 	红灯：GPIOE_PIN_5  	绿灯：GPIOE_PIN_6
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);

}

void Mode_Config(void)
{
	uint8_t s_mod[8] = {0x01,0x06,0x00,0x01,0x00,0x00,0xD8,0x0A};//配置小车为速度模式
	//uint8_t w_mod[8] ={0x01,0x06,0x00,0x01,0x00,0x01,0x19,0xCA};//配置小车为位移模式 
	FillUartTxBufN((uint8_t*)s_mod, 8, 1);
	HAL_Delay(100);
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
	
  /* 打开时钟 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

  /*初始化复位0 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);//这个函数从字面意思来看就是给某个引脚写0或1
  /*GPIO结构体定义 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//四个灯3/4/5/6的初始化
	 /*初始化复位0 */
	 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);//设置灯初始状态
	 /*GPIO结构体定义 */
  GPIO_InitStruct.Pin =GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);//将灯配置为输出状态
	
	//红外避障
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
	
	//按键
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
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
