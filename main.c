/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
#define SENSOR1_GPIO_Port GPIOA
#define SENSOR1_Pin GPIO_PIN_8

#define SENSOR2_GPIO_Port GPIOA
#define SENSOR2_Pin GPIO_PIN_9

#define SENSOR3_GPIO_Port GPIOA
#define SENSOR3_Pin GPIO_PIN_10

#define SENSOR4_GPIO_Port GPIOA
#define SENSOR4_Pin GPIO_PIN_11

#define SENSOR5_GPIO_Port GPIOA
#define SENSOR5_Pin GPIO_PIN_12

//#define SENSOR6_GPIO_Port GPIOA
//#define SENSOR6_Pin GPIO_PIN_13
//#define LEDON_GPIO_Port GPIOB
//#define LEDON_Pin GPIO_PIN_2
	  int sensor_read = 0x00000;
	  	int position;

	  	float Kp = 0.002; //set up the constants value
	  	float Ki = 0.001;
	  	float Kd = 15;
	  	float Kr = 0;
	  	int P, I, D, R;
	  	int lastError = 0;
	  	int errors[10] = {0,0,0,0,0,0,0,0,0,0};// chú ý cái này để lưu lại giá trị các sai lệch đo được trong quá khư
	  	//int error_sum = 0;//
	  	int last_end = 0;	// 0 -> Left, 1 -> Right
	  	int last_idle = 0;
	  	int actives;

	  	const uint8_t maxspeedr = 100;
	  	const uint8_t maxspeedl = 100;
	  	const uint8_t basespeedr = 92;
	  	const uint8_t basespeedl = 92;
	  	const int ARR = 10;
	  	void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
	  	{
	  		GPIO_InitTypeDef GPIO_InitStruct = {0};
	  		GPIO_InitStruct.Pin = GPIO_Pin;
	  		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  		HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	  	}

	  	void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
	  	{
	  		GPIO_InitTypeDef GPIO_InitStruct = {0};
	  		GPIO_InitStruct.Pin = GPIO_Pin;
	  		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  		GPIO_InitStruct.Pull = GPIO_PULLUP;
	  		HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	  	}

	  	void motor_control (double pos_right, double pos_left)
	  	{
	  		if (pos_left < 0 )
	  		{
	  			__HAL_TIM_SET_COMPARE (&htim3, TIM_CHANNEL_1, ARR*0);
	  			__HAL_TIM_SET_COMPARE (&htim3, TIM_CHANNEL_2, ARR*pos_left);
	  		}
	  		else
	  		{
	  			__HAL_TIM_SET_COMPARE (&htim3, TIM_CHANNEL_1, ARR*pos_left);
	  			__HAL_TIM_SET_COMPARE (&htim3, TIM_CHANNEL_2, ARR*0);
	  		}
	  		if (pos_right < 0 )
	  		{
	  			__HAL_TIM_SET_COMPARE (&htim4, TIM_CHANNEL_1, ARR*0);
	  			__HAL_TIM_SET_COMPARE (&htim4, TIM_CHANNEL_2, ARR*pos_right);
	  		}
	  		else
	  		{
	  			__HAL_TIM_SET_COMPARE (&htim4, TIM_CHANNEL_1, ARR*pos_right);
	  			__HAL_TIM_SET_COMPARE (&htim4, TIM_CHANNEL_2, ARR*0);
	  		}
	  	}

	  	void sharp_turn () {

	  		if (last_idle < 15)
	  		{
	  			if (last_end == 1)
	  				motor_control(-20, 100);
	  			if (last_end == 2)
	  				motor_control(100, 100);

	  			else
	  				motor_control(100, -20);
	  		}
	  		else
	  		{
	  			if (last_end == 1)
	  				motor_control(-53, 70);
	  			if (last_end == 2)
	  				motor_control(100, 100);
	  			else
	  				motor_control(70, -53);
	  		}
	  	}

	  	int QTR8_read ()
	  	{
	  	//	HAL_GPIO_WritePin(LEDON_GPIO_Port, LEDON_Pin, 1);

	  		Set_Pin_Output(SENSOR1_GPIO_Port, SENSOR1_Pin);
	  		Set_Pin_Output(SENSOR2_GPIO_Port, SENSOR2_Pin);
	  		Set_Pin_Output(SENSOR3_GPIO_Port, SENSOR3_Pin);
	  		Set_Pin_Output(SENSOR4_GPIO_Port, SENSOR4_Pin);
	  		Set_Pin_Output(SENSOR5_GPIO_Port, SENSOR5_Pin);
	  		//Set_Pin_Output(SENSOR6_GPIO_Port, SENSOR6_Pin);
	  		//Set_Pin_Output(SENSOR7_GPIO_Port, SENSOR7_Pin);
	  		//Set_Pin_Output(SENSOR8_GPIO_Port, SENSOR8_Pin);

	  		HAL_GPIO_WritePin (SENSOR1_GPIO_Port, SENSOR1_Pin, 1);
	  		HAL_GPIO_WritePin (SENSOR2_GPIO_Port, SENSOR2_Pin, 1);
	  		HAL_GPIO_WritePin (SENSOR3_GPIO_Port, SENSOR3_Pin, 1);
	  		HAL_GPIO_WritePin (SENSOR4_GPIO_Port, SENSOR4_Pin, 1);
	  		HAL_GPIO_WritePin (SENSOR5_GPIO_Port, SENSOR5_Pin, 1);
	  		//HAL_GPIO_WritePin (SENSOR6_GPIO_Port, SENSOR6_Pin, 1);
	  		//HAL_GPIO_WritePin (SENSOR7_GPIO_Port, SENSOR7_Pin, 1);
	  		//HAL_GPIO_WritePin (SENSOR8_GPIO_Port, SENSOR8_Pin, 1);

	  		//delay_us(12);

	  		Set_Pin_Input(SENSOR1_GPIO_Port, SENSOR1_Pin);
	  		Set_Pin_Input(SENSOR2_GPIO_Port, SENSOR2_Pin);
	  		Set_Pin_Input(SENSOR3_GPIO_Port, SENSOR3_Pin);
	  		Set_Pin_Input(SENSOR4_GPIO_Port, SENSOR4_Pin);
	  		Set_Pin_Input(SENSOR5_GPIO_Port, SENSOR5_Pin);
	  		//Set_Pin_Input(SENSOR6_GPIO_Port, SENSOR6_Pin);
	  		//Set_Pin_Input(SENSOR7_GPIO_Port, SENSOR7_Pin);
	  		//Set_Pin_Input(SENSOR8_GPIO_Port, SENSOR8_Pin);

	  		// Threshold
	  		//delay_us(6000);
	  		HAL_Delay(10);
	  		sensor_read = 0x00000;
	  		int pos = 0;
	  	  int active = 0;

	  		if (HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin)) {
	  			sensor_read |= 0x00001;
	  			pos += 1000;
	  	    active++;
	  			last_end = 1;
	  		}
	  		if (HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin)) {
	  			sensor_read |= 0x00010;
	  			pos += 2000;
	  	    active++;
	  	     last_end = 1;
	  	  }
	  		if (HAL_GPIO_ReadPin(SENSOR3_GPIO_Port, SENSOR3_Pin)) {
	  			sensor_read |= 0x00100;
	  			pos += 3000;
	  	    active++;
	  	    last_end = 2;
	  	  }
	  		if (HAL_GPIO_ReadPin(SENSOR4_GPIO_Port, SENSOR4_Pin)) {
	  			sensor_read |= 0x01000;
	  			pos += 4000;
	  	    active++;
	  	    last_end = 0;
	  	  }
	  		if (HAL_GPIO_ReadPin(SENSOR5_GPIO_Port, SENSOR5_Pin)) {
	  			sensor_read |= 0x10000;
	  			pos += 5000;
	  	    active++;
	  	    last_end = 0 ;
	  	  }
	  		//if (HAL_GPIO_ReadPin(SENSOR6_GPIO_Port, SENSOR6_Pin)) {
	  			//sensor_read |= 0x00100000;
	  			//pos += 6000;
	  	    //active++;
	  	  //}
	  		//if (HAL_GPIO_ReadPin(SENSOR7_GPIO_Port, SENSOR7_Pin)) {
	  			//sensor_read |= 0x01000000;
	  			//pos += 7000;
	  	  //  active++;
	  	  //}
	  		//if (HAL_GPIO_ReadPin(SENSOR8_GPIO_Port, SENSOR8_Pin)) {
	  			//sensor_read |= 0x10000000;
	  			//pos += 8000;
	  	    //active++;
	  			//last_end = 0;
	  	  //}

	  	 // HAL_GPIO_WritePin(LEDON_GPIO_Port, LEDON_Pin, 0);

	  	  actives = active;
	  		position = pos/active;

	  		if (actives == 0)
	  			last_idle++;
	  		else
	  			last_idle = 0;

	  		return position;//pos/active;
	  	}

	  	void forward_brake(int pos_right, int pos_left)
	  	{
	  		if (actives == 0)
	  			sharp_turn();
	  		else
	  		  motor_control(pos_right, pos_left);
	  	}

	  	void past_errors (int error)
	  	{
	  	  for (int i = 9; i > 0; i--)
	  	      errors[i] = errors[i-1];
	  	  errors[0] = error;
	  	}

	  	int errors_sum (int index, int abs)
	  	{
	  	  int sum = 0;
	  	  for (int i = 0; i < index; i++)
	  	  {
	  	    if (abs == 1 && errors[i] < 0)
	  	      sum += -errors[i];
	  	    else
	  	      sum += errors[i];
	  	  }
	  	  return sum;
	  	}

	  	void PID_control() {
	  		uint16_t position = QTR8_read();
	  	  int error = 3000 - position;
	  		past_errors(error);

	  	  P = error;
	  	  I = errors_sum(5, 0);
	  	  D = error - lastError;
	  	  R = errors_sum(5, 1);
	  	  lastError = error;

	  	  int motorspeed = P*Kp + I*Ki + D*Kd;

	  	  int motorspeedl = basespeedl + motorspeed - R*Kr;
	  	  int motorspeedr = basespeedr - motorspeed - R*Kr;

	  	  if (motorspeedl > maxspeedl)
	  	    motorspeedl = maxspeedl;
	  	  if (motorspeedr > maxspeedr)
	  	    motorspeedr = maxspeedr;

	  		forward_brake(motorspeedr, motorspeedl);
	  	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  PID_control();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 79;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 199;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 79;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 199;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA8 PA9 PA10 PA11
                           PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
