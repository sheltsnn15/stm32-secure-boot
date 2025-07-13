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
/* Includes
 * ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f407xx.h"

/* Private includes
 * ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sha-256.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uECC.h"
#include <stdint.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef
 * -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define
 * ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_ADDRESS 0x08010000U
#define APP_SIZE 0x70000U
#define HASH_SIZE 32U
#define SIGNATURE_SIZE 64U
#define RESERVED_SIZE (HASH_SIZE + SIGNATURE_SIZE)
#define APP_HASH_END (APP_ADDRESS + APP_SIZE - RESERVED_SIZE)

/* USER CODE END PD */

/* Private macro
 * -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// extern const uint8_t ecdsa_pub_x[32];
// extern const uint8_t ecdsa_pub_y[32];

/* USER CODE END PM */

/* Private variables
 * ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes
 * -----------------------------------------------*/
void SystemClock_Config (void);
static void MX_GPIO_Init (void);
static void MX_USART2_UART_Init (void);
/* USER CODE BEGIN PFP */

void jump_to_application (uint32_t application_address);

/* USER CODE END PFP */

/* Private user code
 * ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int
main (void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init ();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config ();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init ();
    MX_USART2_UART_Init ();
    /* USER CODE BEGIN 2 */
    HAL_UART_Transmit (&huart2, (uint8_t *)"Bootloader mode\r\n", 17,
                       HAL_MAX_DELAY);

    // Allocate buffers
    uint8_t computed_hash[HASH_SIZE];
    uint8_t stored_hash[HASH_SIZE];
    uint8_t stored_signature[SIGNATURE_SIZE];
    uint8_t pubkey[64];

    const uint8_t ecdsa_pub_x[32]
        = { 0x47, 0x97, 0x03, 0x07, 0xc6, 0x05, 0xfe, 0xcf, 0x44, 0x20, 0x2a,
            0x54, 0xa1, 0xe1, 0xd5, 0x9f, 0xa7, 0x8d, 0xe7, 0x4f, 0x5a, 0xd7,
            0x8d, 0xee, 0xd8, 0xcf, 0x0d, 0xde, 0x06, 0x51, 0x8d, 0x78 };

    const uint8_t ecdsa_pub_y[32]
        = { 0xaf, 0x83, 0x37, 0x95, 0xb2, 0xdd, 0xb7, 0xf6, 0x3a, 0x60, 0xf8,
            0xe0, 0xb5, 0xfc, 0x96, 0x8b, 0xd6, 0x80, 0x2e, 0x55, 0x41, 0x60,
            0x2f, 0x49, 0x14, 0xbf, 0xcf, 0xb9, 0xa1, 0x70, 0x94, 0xe7 };

    // Prepare public key (x || y)
    memcpy (pubkey, ecdsa_pub_x, 32);
    memcpy (pubkey + 32, ecdsa_pub_y, 32);

    // Compute SHA-256 hash of app code region
    struct Sha_256 ctx;
    sha_256_init (&ctx, computed_hash);
    sha_256_write (&ctx, (const uint8_t *)APP_ADDRESS,
                   APP_HASH_END - APP_ADDRESS);
    sha_256_close (&ctx);

    // Load stored hash and signature
    memcpy (stored_hash, (const uint8_t *)APP_HASH_END, HASH_SIZE);
    memcpy (stored_signature, (const uint8_t *)(APP_HASH_END + HASH_SIZE),
            SIGNATURE_SIZE);

    for (int i = 0; i < SIGNATURE_SIZE; i++)
    {
        char msg[10];
        sprintf (msg, "%02X ", stored_signature[i]);
        HAL_UART_Transmit (&huart2, (uint8_t *)msg, strlen (msg),
                           HAL_MAX_DELAY);
    }
    HAL_UART_Transmit (&huart2, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

    // Compare hashes (integrity)
    if (memcmp (computed_hash, stored_hash, HASH_SIZE) != 0)
    {
        HAL_UART_Transmit (&huart2, (uint8_t *)"Hash check failed\r\n", 19,
                           HAL_MAX_DELAY);
        goto fail;
    }

    // Verify signature (authenticity)
    int result = uECC_verify (pubkey, computed_hash, HASH_SIZE,
                              stored_signature, uECC_secp256r1 ());

    if (result == 1)
    {
        HAL_UART_Transmit (&huart2, (uint8_t *)"Signature OK. Jumping...\r\n",
                           26, HAL_MAX_DELAY);
        jump_to_application (APP_ADDRESS);
    }
    else
    {
        HAL_UART_Transmit (&huart2, (uint8_t *)"Signature check failed\r\n",
                           24, HAL_MAX_DELAY);
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

fail:
    while (1)
    {
        /* USER CODE END WHILE */
        HAL_GPIO_TogglePin (GPIOA, GPIO_PIN_SET);
        HAL_Delay (500);

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void
SystemClock_Config (void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE ();
    __HAL_PWR_VOLTAGESCALING_CONFIG (PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig (&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler ();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig (&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler ();
    }
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_USART2_UART_Init (void)
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
    if (HAL_UART_Init (&huart2) != HAL_OK)
    {
        Error_Handler ();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void
MX_GPIO_Init (void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE ();
    __HAL_RCC_GPIOH_CLK_ENABLE ();
    __HAL_RCC_GPIOA_CLK_ENABLE ();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);

    /*Configure GPIO pin : PE4 */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init (GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pins : PA6 PA7 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init (GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void
jump_to_application (uint32_t application_address)
{
    typedef void (*pFunction) (void);
    uint32_t app_stack = *(__IO uint32_t *)(application_address);
    uint32_t app_reset_handler = *(__IO uint32_t *)(application_address + 4);
    pFunction jump_to_app = (pFunction)app_reset_handler;

    // 1. Disable interrupts
    __disable_irq ();
    // 2. Deinitialize HAL
    HAL_DeInit ();
    // 3. Disable Systick
    SysTick->CTRL = 0;
    // 4. Set vector table offset
    SCB->VTOR = application_address;
    // 5. Set application stack pointer
    __set_MSP (*(__IO uint32_t *)app_stack);
    // 6. Jump to app
    jump_to_app ();
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void
Error_Handler (void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq ();
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
void
assert_failed (uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
