/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define BNO055_I2C_ADDR          (0x28 << 1)

#define BNO055_PAGE_ID           0x07
#define BNO055_OPR_MODE          0x3D

#define BNO055_MODE_CONFIG       0x00
#define BNO055_MODE_AMG          0x07

#define BNO055_ACCEL_DATA_X_LSB  0x08
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
HAL_StatusTypeDef bno055_init_for_accel(void);
HAL_StatusTypeDef bno055_read_accel(int16_t *ax, int16_t *ay, int16_t *az);
HAL_StatusTypeDef bno055_read_u8(uint8_t reg, uint8_t *value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t i2c_found_addr[128];
uint8_t i2c_found_count = 0;
uint8_t bno055_found = 0;
uint8_t bno055_addr = 0;

void I2C_Scan(void)
{
	i2c_found_count = 0;
	bno055_found = 0;
	bno055_addr = 0;

	for (uint8_t addr = 1; addr < 128; addr++)
	{
		if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 10) == HAL_OK)
		{
			i2c_found_addr[i2c_found_count] = addr;
			i2c_found_count++;

			if (addr == 0x28 || addr == 0x29)
			{
				bno055_found = 1;
				bno055_addr = addr;
			}
		}
	}
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  I2C_Scan();

  if (bno055_init_for_accel() == HAL_OK)
  {
      printf("BNO055 init OK\r\n");
  }
  else
  {
      printf("BNO055 init ERR\r\n");
  }

  uint8_t chip_id = 0;
  uint8_t page_id = 0;
  uint8_t opr_mode = 0;

  bno055_read_u8(0x00, &chip_id);
  bno055_read_u8(0x07, &page_id);
  bno055_read_u8(0x3D, &opr_mode);

  printf("CHIP_ID=0x%02X PAGE_ID=0x%02X OPR_MODE=0x%02X\r\n",
         chip_id, page_id, opr_mode);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  printf("timestamp_ms,ax,ay,az\r\n");

  uint32_t start_ms = HAL_GetTick();
  uint32_t last_ms = 0;

  while (1)
  {
	uint32_t now_ms = HAL_GetTick();
	uint32_t elapsed_ms = now_ms - start_ms;

	if ((now_ms - last_ms) >= 100)
	{
		last_ms = now_ms;

		int16_t ax = 0;
		int16_t ay = 0;
		int16_t az = 0;

		if (bno055_read_accel(&ax, &ay, &az) == HAL_OK)
		{
			printf("%lu,%d,%d,%d\r\n",
				   elapsed_ms,
				   ax,
				   ay,
				   az);
		}
		else
		{
			printf("%lu,ERR,ERR,ERR\r\n", elapsed_ms);
		}
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	if (bno055_found) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(500); // found. blink slowly
	}
	else
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(100); // not found: blink fast
	}
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
HAL_StatusTypeDef bno055_init_for_accel(void)
{
    HAL_StatusTypeDef status;

    // まず設定モードへ
    status = HAL_I2C_Mem_Write(
        &hi2c1,
        BNO055_I2C_ADDR,
        BNO055_OPR_MODE,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t[]){BNO055_MODE_CONFIG},
        1,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(25);

    // Register Page 0 に戻す
    status = HAL_I2C_Mem_Write(
        &hi2c1,
        BNO055_I2C_ADDR,
        BNO055_PAGE_ID,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t[]){0x00},
        1,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(10);

    // AMGモードへ
    status = HAL_I2C_Mem_Write(
        &hi2c1,
        BNO055_I2C_ADDR,
        BNO055_OPR_MODE,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t[]){BNO055_MODE_AMG},
        1,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(25);

    return HAL_OK;
}

HAL_StatusTypeDef bno055_read_accel(int16_t *ax, int16_t *ay, int16_t *az)
{
	uint8_t buf[6];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
		&hi2c1,
		BNO055_I2C_ADDR,
		BNO055_ACCEL_DATA_X_LSB,
		I2C_MEMADD_SIZE_8BIT,
		buf,
		6,
		100
	);

	if (status != HAL_OK)
	{
		return status;
	}

	*ax = (int16_t)((buf[1] << 8) | buf[0]);
	*ay = (int16_t)((buf[3] << 8) | buf[2]);
	*az = (int16_t)((buf[5] << 8) | buf[4]);

	printf("raw=%02X %02X %02X %02X %02X %02X\r\n",
	       buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

	return HAL_OK;
}

HAL_StatusTypeDef bno055_read_u8(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(
        &hi2c1,
        BNO055_I2C_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        value,
        1,
        100
    );
}

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
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
