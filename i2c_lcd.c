#include "i2c_lcd.h"

// Import the I2C handler from main.c
extern I2C_HandleTypeDef hi2c1;

// Function to send command
void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (cmd & 0xf0);
  data_l = ((cmd << 4) & 0xf0);
  data_t[0] = data_u | 0x0C;  // en=1, rs=0
  data_t[1] = data_u | 0x08;  // en=0, rs=0
  data_t[2] = data_l | 0x0C;  // en=1, rs=0
  data_t[3] = data_l | 0x08;  // en=0, rs=0

  // Increased timeout to 1000ms to prevent cutoff
  HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, (uint8_t *)data_t, 4, 1000);
}

// Function to send data
void lcd_send_data (char data)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data & 0xf0);
  data_l = ((data << 4) & 0xf0);
  data_t[0] = data_u | 0x0D;  // en=1, rs=1
  data_t[1] = data_u | 0x09;  // en=0, rs=1
  data_t[2] = data_l | 0x0D;  // en=1, rs=1
  data_t[3] = data_l | 0x09;  // en=0, rs=1

  // Increased timeout to 1000ms
  HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, (uint8_t *)data_t, 4, 1000);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
    // Reset cursor to home
    lcd_send_cmd(0x80);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }
    lcd_send_cmd (col);
}

void lcd_init (void)
{
	// 4 bit initialization
	HAL_Delay(50);
	lcd_send_cmd (0x30);
	HAL_Delay(5);
	lcd_send_cmd (0x30);
	HAL_Delay(1);
	lcd_send_cmd (0x30);
	HAL_Delay(10);
	lcd_send_cmd (0x20);
	HAL_Delay(10);

  // Display initialization
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd (0x01);  // clear display
	HAL_Delay(2);         // Increased delay for Clear command (needs >1.52ms)
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C = 0, B = 0  ---> display on
}

void lcd_send_string (char *str)
{
	while (*str) {
        lcd_send_data (*str++);
        // Add a tiny delay between characters to prevent buffer overflow
        // This fixes the "Perfe" cutoff issue
        // HAL_Delay(1); // Uncomment if still glitchy
    }
}
