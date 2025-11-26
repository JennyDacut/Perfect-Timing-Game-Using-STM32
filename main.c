/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Reaction Game (Dual Button: PC1 Onboard + PG3 External)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "i2c_lcd.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
typedef enum {
    STATE_IDLE,
    STATE_RANDOMIZING,
    STATE_SHOW_TARGET,
    STATE_RUNNING,
    STATE_RESULT
} GameState;

GameState current_state = STATE_IDLE;
uint32_t timer_ms = 0;
int target_time = 0;
char displayStr[17];
uint32_t last_tick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void Buzzer_Tone(uint32_t frequency_hz, uint32_t duration_ms);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint32_t us) {
    uint32_t count = us * (SystemCoreClock / 1000000) / 5;
    while(count--);
}

void Buzzer_Tone(uint32_t frequency_hz, uint32_t duration_ms) {
    uint32_t period_us = 1000000 / frequency_hz;
    uint32_t half_period = period_us / 2;
    uint32_t cycles = (frequency_hz * duration_ms) / 1000;
    for (uint32_t i = 0; i < cycles; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        delay_us(half_period);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        delay_us(half_period);
    }
}

void Play_Victory() {
    // Turn ON Green LED (PG4) - Active High
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET); // Red Off

    // Mario Kart - Coconut Mall Theme (Simplified)
    Buzzer_Tone(311, 100);
    Buzzer_Tone(330, 100);
    Buzzer_Tone(370, 100);
    Buzzer_Tone(494, 150);
    HAL_Delay(50);

    Buzzer_Tone(311, 100);
    Buzzer_Tone(330, 100);
    Buzzer_Tone(370, 100);
    Buzzer_Tone(494, 150);
    HAL_Delay(50);

    Buzzer_Tone(311, 80);
    Buzzer_Tone(330, 80);
    Buzzer_Tone(370, 80);
    Buzzer_Tone(494, 80);
    Buzzer_Tone(554, 80);
    Buzzer_Tone(622, 80);
    Buzzer_Tone(659, 200);
}

void Play_Wrong() {
    // Turn ON Red LED (PG2) - Active High
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET); // Green Off

    // Sad "Game Over" Song
    Buzzer_Tone(523, 150);
    Buzzer_Tone(392, 150);
    Buzzer_Tone(330, 150);

    Buzzer_Tone(440, 200);
    Buzzer_Tone(494, 200);
    Buzzer_Tone(440, 200);
    Buzzer_Tone(415, 200);
    Buzzer_Tone(392, 400);
}

void Reset_LEDs() {
    // Turn BOTH OFF (Active High -> Set to Low)
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
}

void Play_Click() {
    Buzzer_Tone(2000, 20);
}

// Check if EITHER button is pressed
int Is_Button_Pressed() {
    if ((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) ||
        (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET))
    {
        HAL_Delay(20);
        if ((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) ||
            (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET))
        {
            while ((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) ||
                   (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET));
            return 1;
        }
    }
    return 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  lcd_init();
  srand(HAL_GetTick());

  Reset_LEDs();

  lcd_clear();
  HAL_Delay(5);
  lcd_put_cur(0, 1);
  lcd_send_string("PERFECT TIMING");
  lcd_put_cur(1, 1);
  lcd_send_string("PressTheButton");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      HAL_Delay(1);

      switch (current_state) {

          case STATE_IDLE:
              Reset_LEDs();
              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear();
                  HAL_Delay(5);
                  current_state = STATE_RANDOMIZING;
              }
              break;

          case STATE_RANDOMIZING:
              target_time = (rand() % 15) + 5;

              sprintf(displayStr, "Target: %d   ", target_time);
              lcd_put_cur(0, 3);
              lcd_send_string(displayStr);
              lcd_put_cur(1, 2);
              lcd_send_string("...Rolling...");
              HAL_Delay(15);

              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear();
                  HAL_Delay(5);
                  sprintf(displayStr, "Target: %d.000s", target_time);
                  lcd_put_cur(0, 1);
                  lcd_send_string(displayStr);
                  lcd_put_cur(1, 1);
                  lcd_send_string("Press to Start");
                  current_state = STATE_SHOW_TARGET;
              }
              break;

          case STATE_SHOW_TARGET:
              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear();
                  HAL_Delay(5);
                  lcd_put_cur(0, 6);
                  lcd_send_string("GO!");
                  timer_ms = 0;
                  last_tick = HAL_GetTick();
                  current_state = STATE_RUNNING;
              }
              break;

          case STATE_RUNNING:
              timer_ms = HAL_GetTick() - last_tick;

              static uint32_t last_screen_update = 0;
              if (timer_ms - last_screen_update > 50) {
                  last_screen_update = timer_ms;
                  sprintf(displayStr, "Time: %lu.%03lus ", timer_ms / 1000, timer_ms % 1000);
                  lcd_put_cur(0, 1);
                  lcd_send_string(displayStr);
              }

              if (timer_ms > 99000) {
                  lcd_clear();
                  HAL_Delay(5);
                  lcd_put_cur(0, 3);
                  lcd_send_string("TOO SLOW!");
                  Play_Wrong();
                  current_state = STATE_RESULT;
              }

              if (Is_Button_Pressed()) {
                  lcd_clear();
                  HAL_Delay(5);

                  uint32_t target_ms = target_time * 1000;

                  if (timer_ms >= target_ms && timer_ms <= (target_ms + 100)) {
                      lcd_put_cur(0, 3);
                      lcd_send_string("VICTORY!!");
                      lcd_put_cur(1, 2);
                      sprintf(displayStr, "Hit: %lu.%03lus", timer_ms/1000, timer_ms%1000);
                      lcd_send_string(displayStr);
                      Play_Victory();
                  } else {
                      lcd_put_cur(0, 4);
                      lcd_send_string("YOU LOST!");
                      sprintf(displayStr, "Hit: %lu.%03lus", timer_ms/1000, timer_ms%1000);
                      lcd_put_cur(1, 2);
                      lcd_send_string(displayStr);
                      Play_Wrong();
                  }

                  HAL_Delay(2000);
                  lcd_clear();
                  HAL_Delay(5);
                  lcd_put_cur(0, 1);
                  lcd_send_string("Press to Reset");
                  current_state = STATE_RESULT;
              }
              break;

          case STATE_RESULT:
              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear();
                  HAL_Delay(5);
                  lcd_put_cur(0, 1);
                  lcd_send_string("PERFECT TIMING");
                  lcd_put_cur(1, 1);
                  lcd_send_string("PressTheButton");
                  current_state = STATE_IDLE;
              }
              break;
      }
  }
  /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) { Error_Handler(); }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) { Error_Handler(); }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable Clocks
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  // 1. Buzzer (PB0)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 2. Buttons (PC1, PG3)
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  // 3. External LEDs (PG2 Red, PG4 Green)
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2|GPIO_PIN_4, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
