/*
 * Copyright (c) 2015, E2ForLife
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the E2ForLife nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL E2FORLIFE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ------------------------------------------------------------------------ */
#include <cytypes.h>
#include <cylib.h>
#include <stdio.h>

#include "ssd1306.h"

#include "i2c.h"

#if defined(CY_SCB_I2C_H)
	#include "i2c_i2c.h"
#endif

/* ------------------------------------------------------------------------ */
/* Global Component Data */
uint8 SSD1306_initVar = 0u;
//extern uint8 I2C_initVar;

/* The display raster buffer used to refresh the display during "vblank" */
uint8 SSD1306_Raster[ SSD1306_RASTER_SIZE + 1];
/*
 * When using Text output, these variables define the present row and column
 * of the display output. These can be manipulated by the PrintString function
 * escape sequences, and are updated after writing a character to the display
 */
uint8 SSD1306_CursorX;
uint8 SSD1306_CursorY;

/* the font bitmap, from SSD1306_font.c */
extern const uint8 SSD1306_Font[];

const uint8 SSD1306_DisplayInit[] =
{
	0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14,
	0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x02, 0x81, 0x8F, 0xD9, 0xF1,
	0xDB, 0x40, 0xA4, 0xA6
};
/* 
 * the number command elements to be transmitted to the display controller
 * in order to initialize the display hardware.
 */
#define SSD1306_INIT_SIZE     ( 24 )
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
void SSD1306_Init( void )
{
	int idx;
	
	/*
	 * Clear the Raster buffer when starting up (essentially, erase
	 * the display contents, and reset the cursor to the top left position.
	 */
	memset( (void*)&SSD1306_Raster[1],0,SSD1306_RASTER_SIZE );
	/*
	 * initialize the starting position of the character output cursor
	 * so that characters are drawn in nice rows adn columns
	 */
	SSD1306_CursorX = 0;
	SSD1306_CursorY = 0;
	
	/*
	 * Send Initialization table to the display
	 */
	for (idx = 0; idx < SSD1306_INIT_SIZE;++idx) {
		SSD1306_SendCommand( SSD1306_DisplayInit[idx] );
	}
	
	SSD1306_initVar = 1;
}
/* ------------------------------------------------------------------------ */
void SSD1306_Enable( void )
{
	/*
	 * Just turn on the display
	 */
	SSD1306_SendCommand( 0xAF );
}
/* ------------------------------------------------------------------------ */
void SSD1306_Start( void )
{
	if (SSD1306_initVar != 1) {
		SSD1306_Init();
	}
	SSD1306_Enable();
}
/* ------------------------------------------------------------------------ */
void SSD1306_SendCommand( uint8 command )
{
	uint8 cmd[2];
	
	/*
	 * Create a buffer with a control byte, and the command byte and send
	 * both to the I2C port using the SCB call to write a buffer using the
	 * complete transfer mode.  Then, wait for the command to complete, so
	 * before returning.
	 */
	cmd[0] = 0x00;
	cmd[1] = command;
#if defined(CY_SCB_I2C_H)
	I2C_I2CMasterWriteBuf(SSD1306_I2C_ADDRESS,&cmd[0],2,I2C_I2C_MODE_COMPLETE_XFER);


	while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))
    {
		/* TODO: Merge Region for Idle processing when sending commands */
    }
	I2C_I2CMasterClearStatus();
#else	
	I2C_MasterWriteBuf(SSD1306_I2C_ADDRESS,&cmd[0],2,I2C_MODE_COMPLETE_XFER);
	while(I2C_MasterStatus() & I2C_MSTAT_XFER_INP);
	
	I2C_MasterClearStatus();
#endif


}
/* ------------------------------------------------------------------------ */
void SSD1306_Refresh( void )
{
	/*
	 * redraw the screen using the I2C interface to send the buffer to the
	 * display VRAM.
	 */
	SSD1306_Raster[0] = 0x40;
#if defined(CY_SCB_I2C_H)
	I2C_I2CMasterWriteBuf(SSD1306_I2C_ADDRESS,&SSD1306_Raster[0],SSD1306_RASTER_SIZE+1,I2C_I2C_MODE_COMPLETE_XFER);

	while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))
    {
		/* TODO: Merge Region for Idle processing when sending commands */
    }
	I2C_I2CMasterClearStatus();	
#else
	int idx;
	
	I2C_MasterSendStart(SSD1306_I2C_ADDRESS, 0);
	for(idx = 0;idx<SSD1306_RASTER_SIZE+1;++idx) {
		I2C_MasterWriteByte( SSD1306_Raster[idx] );
	}
	I2C_MasterSendStop();
#endif


}
/* ------------------------------------------------------------------------ */
void SSD1306_ScrollUp(int lines)
{
	int cnt;
	
	for(cnt=0;cnt<lines;++cnt) {	
		memcpy((void*)&SSD1306_Raster[1], 
			(void*)&SSD1306_Raster[SSD1306_DISPLAY_WIDTH+1],
			SSD1306_RASTER_SIZE-SSD1306_DISPLAY_WIDTH);
		
		memset((void*)&SSD1306_Raster[(((SSD1306_DISPLAY_HEIGHT/8)-1)*SSD1306_DISPLAY_WIDTH)+1],0,SSD1306_DISPLAY_WIDTH);
	}
}
/* ------------------------------------------------------------------------ */
void SSD1306_PutChar( char c )
{
	int xpos;
	int ypos;
	
	/*
	 * Render a character on the display, then update the cursor location
	 * to the next available space.
	 */
	if (SSD1306_CursorY > (SSD1306_DISPLAY_HEIGHT/8)) {
		/*
		 * The new character is giong to display below the bottom of the
		 * display, so, scroll the display up one line, and clear the bottom
		 * row. Lastly, set the cursor y location to the bottom row.
		 */
		SSD1306_ScrollUp(1);
		SSD1306_CursorY = (SSD1306_DISPLAY_HEIGHT/8)-1;
	}
	/* 
	 * Check for CR and NL characters. These are non-printable (even though
	 * there is a font icon for them), and control the cursor location.
	 */
	if (c == '\n') {
		SSD1306_CursorY++;
		SSD1306_CursorX = 0;
	}
	else if (c == '\r') {
		SSD1306_CursorX = 0;
	}
	else {
		/*
		 * Otherwise, this is a real character that needs to be displayed.
		 * Fortunately, the font is buit in FLASH with an identical column
		 * structure to copy the data in to the raster without modifying
		 * the data.
		 */
		#if (SSD1306_FONT_INCLUDE_LOWERCASE == 0)
			c = toupper((int)c);
		#endif
		if ( (c < 127) || ((c>126)&&(SSD1306_FONT_INCLUDE_EXT256!=0)) ) {
			#if (SSD1306_FONT_INCLUDE_CTRL_CHARS == 0)
				/* Fix braces, bar, and tilde */
				#if (SSD1306_FONT_INCLUDE_LOWERCASE == 0)
				if (c > 122) {
					/* 
					 * this puts the open brace at the ASCII position of
					 * lowercase 'a', and maps it to the propoer font
					 * location when using the reduced memory versions
					 */
					c -= 26;
				}
				#endif
				/*
				 * align the space character at ASCII 0, since the control
				 * chars are omitted from the font
				 */
				c = c - ' ';
			#endif
			/* 
			 * There are 5 columns to each font, so, the start index is a
			 * multiple of 5.
			 */
			xpos = c * 5;
			/*
			 * Now, calculate the x,y position of the pixel start, or, more
			 * accurately, the index in to the raster array to which the font
			 * bitmap will be copied. We add one to this value to account
			 * for the command byte stored at index 0.
			 */
			ypos = ((SSD1306_CursorX*6) + (SSD1306_CursorY*128))+1;
			memcpy((void*)&SSD1306_Raster[ypos], (void*)&SSD1306_Font[xpos], 5 );
			SSD1306_Raster[ypos+5] = 0;			
		}
		/* move the cursor to the next location */
		SSD1306_CursorX++;
		if (SSD1306_CursorX >= (SSD1306_DISPLAY_WIDTH/6)) {
			SSD1306_CursorX = 0;
			SSD1306_CursorY++;
		}
	}
}
/* ------------------------------------------------------------------------ */
void SSD1306_SizedPutChar(char c, uint8 xsize, uint8 ysize)
{
	int xpos;
	int ypos;
	int idx;
	uint8 mask;
	uint8 pixel;
	uint8 bmp[24];
	
	/*
	 * Render a character on the display, then update the cursor location
	 * to the next available space.
	 */
	if (SSD1306_CursorY >= (SSD1306_DISPLAY_HEIGHT/8)) {
		/*
		 * The new character is giong to display below the bottom of the
		 * display, so, scroll the display up one line, and clear the bottom
		 * row. Lastly, set the cursor y location to the bottom row.
		 */
		SSD1306_ScrollUp((ysize!=0)?2:1);
		SSD1306_CursorY = (SSD1306_DISPLAY_HEIGHT/8)- ((ysize!=0)?2:1);
	}
	/* 
	 * Check for CR and NL characters. These are non-printable (even though
	 * there is a font icon for them), and control the cursor location.
	 */
	if (c == '\n') {
		SSD1306_CursorY += (ysize!=0)?2:1;
		SSD1306_CursorX = 0;
	}
	else if (c == '\r') {
		SSD1306_CursorX = 0;
	}
	else {
		/*
		 * Otherwise, this is a real character that needs to be displayed.
		 * Fortunately, the font is buit in FLASH with an identical column
		 * structure to copy the data in to the raster without modifying
		 * the data.
		 */
		#if (SSD1306_FONT_INCLUDE_LOWERCASE == 0)
			c = toupper((int)c);
		#endif
		if ( (c < 127) || ((c>126)&&(SSD1306_FONT_INCLUDE_EXT256!=0)) ) {
			#if (SSD1306_FONT_INCLUDE_CTRL_CHARS == 0)
				/* Fix braces, bar, and tilde */
				if (c > 122) {
					/* 
					 * this puts the open brace at the ASCII position of
					 * lowercase 'a', and maps it to the propoer font
					 * location when using the reduced memory versions
					 */
					c -= 26;
				}
				/*
				 * align the space character at ASCII 0, since the control
				 * chars are omitted from the font
				 */
				c = c - ' ';
			#endif
			/* 
			 * There are 5 columns to each font, so, the start index is a
			 * multiple of 5.
			 */
			xpos = c * 5;
			/*
			 * Expand the font in to the bitmap that is to be copied to the
			 * raster buffer.
			 * The first thing to do is to copy the column data to make the
			 * font wider.
			 */
			memset((void*)&bmp[0],0,24);
			if (xsize != 0) {
				for (idx=0;idx<5;idx++) {
					bmp[(idx<<1)] = SSD1306_Font[xpos+idx];
					bmp[(idx<<1)+1] = SSD1306_Font[xpos+idx];
				}
			}
			else {
				for (idx=0;idx<5;idx++) {
					bmp[idx] = SSD1306_Font[xpos+idx];
				}
			}				
			/*
			 * now that the font was stretched in the X direction, stretch
			 * it in the Y direction as well.  this is a little harder since
			 * we now have to expand the bits in to the upper portion of the
			 * bitmap.
			 */
			if (ysize != 0) {
				for(idx=0;idx<12;++idx) {
					pixel = 0xC0;
					for (mask=0x80;mask!=0x08;mask>>=1) {
						if (bmp[idx]&mask) {
							bmp[idx+12] |= pixel;
						}
						pixel >>= 2;
					}
				}
				/*
				 * and now that the lower portion of the letter was expanded,
				 * spread the upper bits in the same manner
				 */
				for(idx=0;idx<12;++idx) {
					pixel = 0xC0;
					ypos = bmp[idx];
					for(mask=0x08;mask!=0;mask>>=1) {
						ypos &= ~pixel;
						if (bmp[idx]&mask) {
							ypos |= pixel;
						}
						pixel >>= 2;
					}
					bmp[idx] = ypos;
				}
			}
			/*
			 * Now, calculate the x,y position of the pixel start, or, more
			 * accurately, the index in to the raster array to which the font
			 * bitmap will be copied. We add one to this value to account
			 * for the command byte stored at index 0.
			 */
			ypos = ((SSD1306_CursorX*6) + (SSD1306_CursorY*128))+1;
			memcpy((void*)&SSD1306_Raster[ypos], (void*)&bmp[0], (xsize!=0)?12:6 );
			if (ysize != 0) {
				memcpy((void*)&SSD1306_Raster[ypos+128], (void*)&bmp[12],(xsize!=0)?12:6);
			}
		}
		
		/* move the cursor to the next location */
		SSD1306_CursorX += (xsize!=0)?2:1;
		if (SSD1306_CursorX >= (SSD1306_DISPLAY_WIDTH/6)) {
			SSD1306_CursorX = 0;
			SSD1306_CursorY += (ysize!=0)?2:1;
		}
	}
}
/* ------------------------------------------------------------------------ */
void SSD1306_PrintString( char *str )
{
	int idx = 0;
	int cmdix = 0;
	uint8 mode;
	uint8 expandX,expandY;
	char cmd[21];
	int value;
	
	/* default the mode to "Normal output" */
	mode = 0;
	expandX = 0; /* default to normal sized fonts */
	expandY = 0;
	
	/*
	 * Loop through the string and process commands or, output the character
	 * until the end of string is detected.
	 */
	while (str[idx] != 0) {
		/*
		 * When an escape sequence is detected, process the command, or, 
		 * buffer the data.
		 */
		if (mode != 0) {
			if ((str[idx] == ';')||(str[idx]=='}')) {
				/*
				 * the end of the command was detected, so process the
				 * command sequence.
				 */
				if (strncmpi(cmd,"up",2) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
					}
					else {
						value = 1;
					}
					for (cmdix=0;cmdix<value;++cmdix) {
						if (SSD1306_CursorY > 0) {
							SSD1306_CursorY--;
						}
					}
				}
				else if (strncmpi(cmd,"down",4) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
					}
					else {
						value = 1;
					}
					for (cmdix=0;cmdix<value;++cmdix) {
						if (SSD1306_CursorY < (SSD1306_DISPLAY_HEIGHT/8)) {
							SSD1306_CursorY++;
						}
					}
				}
				else if (strncmpi(cmd,"right",5) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
					}
					else {
						value = 1;
					}
					for (cmdix=0;cmdix<value;++cmdix) {
						if (SSD1306_CursorX < (SSD1306_DISPLAY_WIDTH/6)) {
							SSD1306_CursorX++;
						}
					}
				}
				else if (strncmpi(cmd,"left",4) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
					}
					else {
						value = 1;
					}
					for (cmdix=0;cmdix<value;++cmdix) {
						if (SSD1306_CursorX > 0) {
							SSD1306_CursorX--;
						}
					}
				}
				else if (strncmpi(cmd,"clr",3) == 0) {
					if (cmd[3] == '1') {
						/* up to the current row */
						value = (SSD1306_CursorY*128);
						memset((void*)&SSD1306_Raster[1],0,value);
					}
					else if (cmd[3] == '2') {
						/* from current row down */
						value = (SSD1306_CursorY*128);
						memset((void*)&SSD1306_Raster[value+1],0,SSD1306_RASTER_SIZE-value);
					}
					else if (cmd[3] == '3') {
						/* clear current row */
						value = SSD1306_CursorY;
					}
					else if (cmd[3] == '4') {
						/* clear from current pos to end */
						value = ((SSD1306_CursorX*6) + (SSD1306_CursorY*128)) + 1;
						memset((void*)&SSD1306_Raster[value+1],0,128-(SSD1306_CursorX*6));
					}
					else if (cmd[3] == '5') {
						/* clear from current position to beginning */
						value = (SSD1306_CursorY*128);
						memset((void*)&SSD1306_Raster[value+1],0,SSD1306_CursorX*6);
					}
					else {
						/* clear whole display */
						memset((void*)&SSD1306_Raster[1],0,SSD1306_RASTER_SIZE);
					}

				}
				else if (strncmpi(cmd,"row",3) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
						if (value > (SSD1306_DISPLAY_HEIGHT/8)) {
							value = SSD1306_DISPLAY_HEIGHT/8 - 1;
						}
						else if (value < 0) {
							value = 0;
						}
					}
					else {
						value = 1;
					}
					SSD1306_CursorY = value;
				}
				else if (strncmpi(cmd,"col",3) == 0) {
					if (isdigit((int)cmd[3]) ) {
						sscanf(&cmd[3],"%d",&value);
						if (value > (SSD1306_DISPLAY_WIDTH/6)) {
							value = SSD1306_DISPLAY_WIDTH/6 - 1;
						}
						else if (value < 0) {
							value = 0;
						}
					}
					else {
						value = 1;
					}
					SSD1306_CursorX = value;				
				}
				else if (strncmpi(cmd,"big",5) == 0) {
					expandX = 1;
					expandY = 1;
				}
				else if (strncmpi(cmd,"wide",4) == 0) {
					expandX = 1;
					expandY = 0;
				}
				else if (strncmpi(cmd,"tall",4) == 0) {
					expandX = 0;
					expandY = 1;
				}
				else if (strncmpi(cmd,"normal",6) == 0) {
					expandX = 0;
					expandY = 0;
				}
				cmdix = 0;
				memset((void*)&cmd[0],0,21);
				if (str[idx] == '}') {
					mode = 0;
				}
			}
			else {
				if (cmdix < 20) {
					cmd[cmdix++] = str[idx];
					cmd[cmdix] = 0;
				}
			}
		}
		else if (str[idx] == '{') {
			cmdix = 0;
			mode = 1;
		}
		else {
			if (( expandX != 0)||(expandY != 0) ) {
				SSD1306_SizedPutChar( str[idx],expandX,expandY );
			}
			else {
				SSD1306_PutChar( str[idx] );
			}
		}
		++idx;
	}
}

/* ------------------------------------------------------------------------ */
/* [] END OF FILE */
