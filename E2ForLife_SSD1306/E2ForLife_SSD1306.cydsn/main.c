/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

#include "ssd1306.h"

int main()
{
	uint8 c;
	char str[21];
	
	I2C_Start();
	OLED_RST_Write(0);
	CyDelay(10);
	OLED_RST_Write(1);
	CyDelay( 200 );
	
	CyGlobalIntEnable; /* Enable global interrupts. */
	
	/*
	 * Initialize the I2C port and the diplay driver
	 */
	SSD1306_Start();
	SSD1306_Refresh();
	SSD1306_PrintString("{tall}Hello!\n{wide}Hello\n{normal}Hola!");
	c = 0;
    for(;;)
    {
		/* Print a message on the display */
//		SSD1306_PutChar((char)c++);
		SSD1306_Refresh();
    }
}

/* [] END OF FILE */
