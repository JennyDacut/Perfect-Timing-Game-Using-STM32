#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "stm32f4xx_hal.h"

// I2C Address (Shifted to the left by 1)
// Common addresses: 0x27 -> 0x4E, or 0x3F -> 0x7E
#define LCD_ADDR (0x27 << 1)

void lcd_init (void);   // Initialize LCD
void lcd_send_cmd (char cmd);  // Send command to LCD
void lcd_send_data (char data);  // Send data to LCD
void lcd_send_string (char *str);  // Send string to LCD
void lcd_put_cur(int row, int col);  // Put cursor at the entered position row (0 or 1), col (0-15);
void lcd_clear (void);  // Clear LCD screen

#endif /* INC_I2C_LCD_H_ */
