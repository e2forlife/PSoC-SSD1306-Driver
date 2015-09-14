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
	
	/* Print a message on the display */
	SSD1306_PrintString("Hello World!");
	SSD1306_Refresh();

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
