/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint8_t rx_byte;
static char line_buf[128];
static volatile uint16_t line_len = 0;
static volatile uint8_t line_ready = 0;

typedef enum State{
	FOCUSED,
	DISTRACTED,
	IDLE,
	END,
	MENU
} State_t;

static State_t state = MENU;

static uint32_t last_ms = 0;
static uint32_t focused_ms = 0;
static uint32_t distracted_ms = 0;

static uint32_t distracted_episodes = 0;
static uint8_t in_distracted = 0;

static uint32_t pet_health = 100;

static volatile uint8_t end_session_flag = 0;
static volatile uint8_t idle_flag = 0;
uint8_t endTrack = 0;

static uint32_t idleFlag = 0;
static uint32_t focusedFlag = 0;
static uint32_t menuFlag = 0;
static uint32_t endFlag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void time_count(uint32_t now){
	//ms
	uint32_t diff = now - last_ms;

	if (state == FOCUSED){
		focused_ms += diff;
	}
	else if (state == DISTRACTED){
		distracted_ms += diff;
	}

	last_ms = now;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  HAL_UART_Transmit(&huart2, (uint8_t*)"STM32 READY\r\n", 12, HAL_MAX_DELAY);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
 //	  printf("hello world \n\r");
 //	  HAL_Delay(1000);

	  //interrupt state changes handled here
	  if (idleFlag){
		  state = IDLE;
		  idleFlag = 0;
		  printf("idle\n\r");
	  }
	  else if (focusedFlag){
		  state = FOCUSED;
		  focusedFlag = 0;
			line_ready = 0;
			line_len = 0;
			line_buf[0] = '\0';
		  printf("focused\n\r");
	  }
	  else if (menuFlag){
	  		  state = MENU;
	  		  menuFlag = 0;
	  		printf("menu\n\r");
	  	  }
	  else if (endFlag){
	  		  state = END;
	  		  endFlag = 0;
	  		printf("end\n\r");
	  	  }



 	  uint32_t now = HAL_GetTick();
 	  //init
 	  if(last_ms == 0){
 		  last_ms = now;
 	  }

 	 time_count(now);

 	  //display summary here, can be triggered by pin8 only
 	 static uint8_t endPrint = 0;

 	  if(state == END && !endPrint){

 		    char msg[128];
 		    snprintf(msg, sizeof(msg),
 		             "SESSION END\r\nFOCUSED=%lus\r\nDISTRACTED=%lus\r\nEPISODES=%lu\r\n",
 		             (unsigned long)(focused_ms/1000),
 		             (unsigned long)(distracted_ms/1000),
 		             (unsigned long)distracted_episodes);
 		    HAL_UART_Transmit(&huart2, (uint8_t*)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);


 		    //resetting

 		    focused_ms = 0;
 		    distracted_ms = 0;
 		    distracted_episodes = 0;
 		    in_distracted = 0;
 		    last_ms = HAL_GetTick();
// 		    state = MENU;

 		    HAL_UART_Transmit(&huart2, (uint8_t*)"SESSION RESET\r\n", 15, HAL_MAX_DELAY);
 		    endPrint = 1;
// 		    endTrack = 1;
 	  }

 	  if (state != END){
 		  endPrint = 0;
 	  }

 	  //idle state. triggered by pin5, exit using pin5 again
 	  static uint8_t idlePrint = 0;
 	  if (state == IDLE){
 		  if (!idlePrint){
 	 		 HAL_UART_Transmit(&huart2, (uint8_t*)"STUDYING PAUSED\r\n", 17, HAL_MAX_DELAY);

 			    char msg[128];
 			    snprintf(msg, sizeof(msg),
 			             "CURRENT STATS\r\nFOCUSED=%lus\r\nDISTRACTED=%lus\r\nEPISODES=%lu\r\n",
 			             (unsigned long)(focused_ms/1000),
 			             (unsigned long)(distracted_ms/1000),
 			             (unsigned long)distracted_episodes);
 			    HAL_UART_Transmit(&huart2, (uint8_t*)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
 			    idlePrint = 1;
 		  }
 	  }

 	  if(state != IDLE){
 		  idlePrint = 0;
 	  }

 	  //MENU state
 	  static uint8_t menuPrint = 0;
 	  if (state == MENU && !menuPrint){
		    focused_ms = 0;
		    distracted_ms = 0;
		    distracted_episodes = 0;
		    in_distracted = 0;
		    last_ms = HAL_GetTick();
 		     HAL_UART_Transmit(&huart2,(uint8_t*)"MENU\r\n",15,HAL_MAX_DELAY);
// 		     state = FOCUSED;
 		     menuPrint = 1;
 	  }

 	  if (state != MENU){
 		  menuPrint = 0;
 	  }
 	  //change the states
 	 if (line_ready){
 		 HAL_UART_Transmit(&huart2, (uint8_t*)"ACK\r\n", 5, HAL_MAX_DELAY);
 		  line_ready = 0;
 		  //line + newline
 		  HAL_UART_Transmit(&huart2, (uint8_t*)"RX: ", 4, HAL_MAX_DELAY);
 		  HAL_UART_Transmit(&huart2, (uint8_t*)line_buf, (uint16_t)strlen(line_buf), HAL_MAX_DELAY);
 		  HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
 	        if (!strcmp(line_buf, "FOCUSED")) {
 	        	if(in_distracted){
 		        	state = FOCUSED;
 		            HAL_UART_Transmit(&huart2, (uint8_t*)"STATE=FOCUSED\r\n", 15, HAL_MAX_DELAY);
 		            in_distracted = 0;
 	        	}
 	        } else if (!strcmp(line_buf, "DISTRACTED")) {
 	        	if(!in_distracted){
 		        	state = DISTRACTED;
 		            HAL_UART_Transmit(&huart2, (uint8_t*)"STATE=DISTRACTED\r\n", 18, HAL_MAX_DELAY);

 		        	if(!in_distracted){
 		        		distracted_episodes++;
 		        	}

 		            in_distracted = 1;

 	        	}
 	        } else {
 	            HAL_UART_Transmit(&huart2, (uint8_t*)"STATE=UNKNOWN\r\n", 15, HAL_MAX_DELAY);
 	        }
 	        line_len = 0;
 	  }
    /* USER CODE BEGIN 3 */
 	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  }
  /* USER CODE END 3 */
 }


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (line_ready) {
	    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	    return;
	}

	if (huart->Instance == USART2){
		char c = (char)rx_byte;

		//handle carriage return and newline
		if (c == '\r'){
		}
		else if (c == '\n') {
			if (line_len >= sizeof(line_buf)){
				line_len = sizeof(line_buf) - 1;
			}
			line_buf[line_len] = '\0';
			line_ready = 1;
			line_len = 0;
		}
//		else{
//				line_buf[sizeof(line_buf) - 1] = '\0';
//			}

		else{
			if (line_len < (sizeof(line_buf) - 1)){
				line_buf[line_len++] = c;
			}
			else{
				//overflow handling
				line_len = 0;
			}
		}
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	}
}

//debounce
static uint32_t last_pb5 = 0;
static uint32_t last_pa8 = 0;



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint32_t button_now = HAL_GetTick();
	//idle/break
    if (GPIO_Pin == GPIO_PIN_5) {
    	if(button_now - last_pb5 > 50){
        	if(state != IDLE){
        		idleFlag = 1;
        	}
        	else if(state == IDLE){
        		focusedFlag = 1;
        	}
        	last_pb5 = button_now;
        	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    	}
    }
    //start/stop session
    if (GPIO_Pin == GPIO_PIN_8) {
    	if(button_now - last_pa8 > 50){

			if(state == END){
				menuFlag = 1;
			}
			else if(state == MENU){
				focusedFlag = 1;
			}
			else if (state == FOCUSED){
				endFlag = 1;
			}
			last_pa8 = button_now;
			HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    	}
    }
}


#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

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
#ifdef USE_FULL_ASSERT
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
