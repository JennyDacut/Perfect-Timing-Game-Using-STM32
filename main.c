/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Reaction Game (Corrected Button PC1 Version)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "i2c_lcd.h" // Ensure you created this file!
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
typedef enum {
    STATE_IDLE,       // Screen: "Press DOWN"
    STATE_RANDOMIZING,// Screen: Rolling numbers...
    STATE_SHOW_TARGET,// Screen: "Target: 12"
    STATE_RUNNING,    // Timer: 0, 1, 2...
    STATE_RESULT      // Win/Lose
} GameState;

GameState current_state = STATE_IDLE;
uint32_t timer_ms = 0; // Changed to Milliseconds
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
    // Mario Kart - Coconut Mall Theme (Simplified)
    // Phrase 1: D# E F# B (High)
    Buzzer_Tone(311, 100); // D#4
    Buzzer_Tone(330, 100); // E4
    Buzzer_Tone(370, 100); // F#4
    Buzzer_Tone(494, 150); // B4
    HAL_Delay(50);

    // Phrase 2: D# E F# B (High)
    Buzzer_Tone(311, 100); // D#4
    Buzzer_Tone(330, 100); // E4
    Buzzer_Tone(370, 100); // F#4
    Buzzer_Tone(494, 150); // B4
    HAL_Delay(50);

    // Phrase 3: D# E F# B C# D# E (Run up)
    Buzzer_Tone(311, 80); // D#4
    Buzzer_Tone(330, 80); // E4
    Buzzer_Tone(370, 80); // F#4
    Buzzer_Tone(494, 80); // B4
    Buzzer_Tone(554, 80); // C#5
    Buzzer_Tone(622, 80); // D#5
    Buzzer_Tone(659, 200); // E5
}

void Play_Wrong() {
    // Sad "Game Over" Song (Descending tritone)
    Buzzer_Tone(523, 150); // C5
    Buzzer_Tone(392, 150); // G4
    Buzzer_Tone(330, 150); // E4

    Buzzer_Tone(440, 200); // A4
    Buzzer_Tone(494, 200); // B4
    Buzzer_Tone(440, 200); // A4
    Buzzer_Tone(415, 200); // G#4
    Buzzer_Tone(392, 400); // G4 (Long, sad finish)
}

void Play_Click() {
    Buzzer_Tone(2000, 20); // Short blip
}

// Check Button PC1 (Active Low)
// Returns 1 if pressed, 0 if not
int Is_Button_Pressed() {
    // CORRECTED PIN: PC1 (Down Button)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) {
        HAL_Delay(20); // Short debounce
        // Check again to confirm it wasn't noise
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET) {
            // Wait for release to prevent multiple triggers
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET);
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
  srand(HAL_GetTick()); // Seed random number generator

  // Initial Screen
  lcd_clear();
  HAL_Delay(5); // Wait for clear to finish
  lcd_put_cur(0, 1);
  lcd_send_string("Perfect Timing");
  lcd_put_cur(1, 1);
  lcd_send_string("Press DOWN Btn");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Add a tiny delay to loop to prevent CPU hogging
      // Reduced to 1ms for smoother millisecond counting
      HAL_Delay(1);

      switch (current_state) {

          case STATE_IDLE:
              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear(); // Clear screen before changing state
                  HAL_Delay(5); // Wait for clear
                  current_state = STATE_RANDOMIZING;
              }
              break;

          case STATE_RANDOMIZING:
              // Effect: Show numbers scrolling fast until button pressed
              target_time = (rand() % 15) + 5; // Generate new number (5-20)

              // Only update screen every 15ms so it's readable but faster
              sprintf(displayStr, "Target: %d   ", target_time);
              lcd_put_cur(0, 3); // Centered roughly
              lcd_send_string(displayStr);
              lcd_put_cur(1, 2); // Centered roughly
              lcd_send_string("...Rolling...");
              HAL_Delay(15); // Reduced delay for FASTER scrolling

              // Check button immediately
              if (Is_Button_Pressed()) {
                  Play_Click();
                  // Stop on the current number
                  lcd_clear();
                  HAL_Delay(5); // Wait for clear
                  sprintf(displayStr, "Target: %d.000s", target_time); // Show as seconds
                  lcd_put_cur(0, 1); // Centered roughly
                  lcd_send_string(displayStr);
                  lcd_put_cur(1, 1); // Centered roughly
                  lcd_send_string("Press to Start");
                  current_state = STATE_SHOW_TARGET;
              }
              break;

          case STATE_SHOW_TARGET:
              if (Is_Button_Pressed()) {
                  Play_Click();
                  // Start the actual game timer
                  lcd_clear();
                  HAL_Delay(5); // Wait for clear
                  lcd_put_cur(0, 6); // Centered
                  lcd_send_string("GO!");
                  timer_ms = 0; // Reset Timer (MS)
                  last_tick = HAL_GetTick();
                  current_state = STATE_RUNNING;
              }
              break;

          case STATE_RUNNING:
              // 1. Update Timer (Milliseconds)
              // HAL_GetTick() returns milliseconds
              timer_ms = HAL_GetTick() - last_tick;

              // Update screen every ~50ms to keep it readable but fast
              static uint32_t last_screen_update = 0;
              if (timer_ms - last_screen_update > 50) {
                  last_screen_update = timer_ms;
                  // Format: 12.345s
                  sprintf(displayStr, "Time: %lu.%03lus ", timer_ms / 1000, timer_ms % 1000);
                  lcd_put_cur(0, 1); // Centered roughly
                  lcd_send_string(displayStr);
              }

              if (timer_ms > 99000) { // Timeout at 99 seconds
                  lcd_clear();
                  HAL_Delay(5); // Wait for clear
                  lcd_put_cur(0, 3); // Centered
                  lcd_send_string("TOO SLOW!");
                  Play_Wrong();
                  current_state = STATE_RESULT;
              }

              // 2. Check for Stop
              if (Is_Button_Pressed()) {
                  lcd_clear(); // <--- ADDED CLEAR
                  HAL_Delay(5); // Wait for clear

                  // Convert target_time (seconds) to ms for comparison
                  uint32_t target_ms = target_time * 1000;

                  // WIN Condition: Between Target and Target + 100ms
                  if (timer_ms >= target_ms && timer_ms <= (target_ms + 100)) {
                      lcd_put_cur(0, 3); // Centered
                      lcd_send_string("VICTORY!!");
                      lcd_put_cur(1, 2); // Centered roughly
                      sprintf(displayStr, "Hit: %lu.%03lus", timer_ms/1000, timer_ms%1000);
                      lcd_send_string(displayStr);
                      Play_Victory();
                  } else {
                      lcd_put_cur(0, 4); // Centered
                      lcd_send_string("YOU LOST!"); // Changed from "WRONG!"
                      sprintf(displayStr, "Hit: %lu.%03lus", timer_ms/1000, timer_ms%1000);
                      lcd_put_cur(1, 2); // Centered roughly
                      lcd_send_string(displayStr);
                      Play_Wrong(); // Sad losing song
                  }

                  HAL_Delay(2000); // Longer pause to see result
                  lcd_clear(); // <--- ADDED CLEAR before resetting text
                  HAL_Delay(5); // Wait for clear
                  lcd_put_cur(0, 1); // Centered
                  lcd_send_string("Press to Reset");
                  current_state = STATE_RESULT;
              }
              break;

          case STATE_RESULT:
              if (Is_Button_Pressed()) {
                  Play_Click();
                  lcd_clear(); // <--- ADDED CLEAR
                  HAL_Delay(5); // Wait for clear
                  lcd_put_cur(0, 1);
                  lcd_send_string("Perfect Timing");
                  lcd_put_cur(1, 1);
                  lcd_send_string("Press DOWN Btn");
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 1. Buzzer (PB0)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 2. Onboard Button (PC1) - DOWN Button
  GPIO_InitStruct.Pin = GPIO_PIN_1; // Updated to correct Down button pin
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP; // Critical for button to work
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
