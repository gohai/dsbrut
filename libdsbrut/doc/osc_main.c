/*
 *	NDS UART Test Application
 *
 *	Copyright Gottfried Haider & Gordan Savicic 2008.
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "nds.h"
#include <nds/arm9/console.h>
#include <stdio.h>
#include <math.h>
#include "uart.h"
#include "reboot.h"


static const unsigned int uart_speeds[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };
touchPosition oldtouch;
volatile int frame = 0;
int shape_width,shape_height;
static uint8 dat_array1[564];
static uint8 dat_array2[564];
static int ready = 0;
static int trigger_level = 35;
static int new_trigger_level;

//---------------------------------------------------------------------------------


void draw_shape(int x, int y, uint16* buffer, uint16 color)
{
int i,j;
  buffer += y * SCREEN_WIDTH + x;
  for(i = 0; i < shape_height; ++i) {
    uint16* line = buffer + (SCREEN_WIDTH * i);
    for(j = 0; j < shape_width; ++j) {
      *line++ = color;
    }
  }
}

void display_data(uint8* datbuf, uint16* buffer, uint16 color)
{
int i,j,ii,len,inc,start,bstrt;
uint16* bufstrt;
i=8;
while (datbuf[i]>=trigger_level)i++;
while (datbuf[i]<trigger_level)i++;
if (i>128) bstrt = 128;
else bstrt=i;
//bstrt=0;
for (i=1;i<256;i++){
	ii = i+bstrt;
	if (datbuf[ii]>datbuf[ii-1]){ len = datbuf[ii] - datbuf[ii-1]; inc = 1;}
	else {len = datbuf[ii-1] - datbuf[ii]; inc = -1;}
	if (len==0){len = 1; inc = 1;}
	start = datbuf[ii-1];

//	buffer += (start * SCREEN_WIDTH) + i;
	for (j = 0;j<len;j++){
		bufstrt = buffer + (start * SCREEN_WIDTH) + i;
		*bufstrt ^= color; //Exclusive OR to invert the color
		start+=inc;
    }
  }
}

//FROM: http://www.local-guru.net/blog/tag/ds
void setPixel( int x, int y, int color ) {
    VRAM_A[x + y * 256] = color;
}

void line( int x1, int y1, int x2, int y2, uint16 c ) {
    bool swap = abs( y2 - y1 ) > abs ( x2 - x1 );
    int x1t =  swap ? y1 : x1;
    int y1t =  swap ? x1 : y1;
    int x2t =  swap ? y2 : x2;
    int y2t =  swap ? x2 : y2;

    int xs =  x1t < x2t ? x1t : x2t;
    int ys =  x1t < x2t ? y1t : y2t;
    int xt =  x1t < x2t ? x2t : x1t;
    int yt =  x1t < x2t ? y2t : y1t;

    int dx = xt - xs;
    int dy = abs(yt - ys);

    int dT = 2 * ( dy - dx );
    int dS = 2 * dy;
    int d = 2 * dy - dx;
    int x = xs;
    int y = ys;

    if ( swap )  {
        setPixel( y, x, c );
    } else {
        setPixel( x, y, c );
    }

    while ( x < xt ) {
        x++;
        if ( d < 0 ) {
            d = d + dS;
        } else {
            if ( ys < yt ) {
                y++;
            } else {
                y--;
            }
            d = d + dT;
        }
        if ( swap )  {
            setPixel( y, x, c );
        } else {
            setPixel( x, y, c );
        }
    }
}

void circle( int mx, int my, int r, uint16 color ) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y) {
        setPixel( mx  + x, my + y, color );
        setPixel( mx  - x, my + y, color );
        setPixel( mx  - x, my - y, color );
        setPixel( mx  + x, my - y, color );
        setPixel( mx  + y, my + x, color );
        setPixel( mx  - y, my + x, color );
        setPixel( mx  - y, my - x, color );
        setPixel( mx  + y, my - x, color );

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * ( x - y ) + 10;
            y --;
        }
        x++;
    }
}

void DrawGrid (uint16 color){
int i;
// Outline First
line(0,0,255,0,color);
line(0,0,0,191,color);
line(0,191,255,191,color);
line(255,0,255,191,color);
// Draw grid lines
for (i=19;i<191;i+=19) line (1,i,254,i,color);
for (i=25;i<255;i+=25) line (i,1,i,190,color);
}

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
if (frame==0) frame = 1;
else frame = 0;
}


int main(void)
{
	int cur_speed = 0x09;
	int i,pressed;
	touchPosition touch = { 0 };
	
irqSet(IRQ_VBLANK, Vblank);

//  powerON(POWER_ALL);
  videoSetMode(MODE_FB0);
  vramSetBankA(VRAM_A_LCD);
//Form a dummy waveform to show during boot
for (i=0;i<384;i++){ dat_array1[i] = 96+(96*sin((2*3.1416*i)/32)*sin(2*3.1416*i/256));}
for (i=0;i<383;i++){dat_array2[i]=dat_array1[i+1];}
dat_array2[383] = dat_array1[0];
shape_width = 256;
shape_height = 192;  

draw_shape(0, 0, VRAM_A, RGB15(8, 8, 8));//screen background gray
DrawGrid(RGB15(0, 0, 0));//Draw a gray background
//  if (first==1){
display_data(dat_array1,VRAM_A,RGB15(31,31,31));  
//first = 0;
//  }	

	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);

	REG_BG0CNT_SUB = BG_MAP_BASE(31);

	BG_PALETTE_SUB[255] = RGB15(31,31,31);

	consoleDemoInit();

	iprintf("NDS Oscilloscope\n");
	iprintf("build %s %s\n\n", __DATE__, __TIME__);

	iprintf("initializing UART.. ");
	if (uartOpen())
		iprintf("successful.\n\n");
	else
		iprintf("failed!\n\n");
	cur_speed = 10;
	uartSetBaud(uart_speeds[cur_speed]);	
	iprintf("\n\tsetting baud rate to %u\n", uart_speeds[cur_speed]);
//	iprintf("\nUsage:");
	iprintf("\n\n\t[A] .. Enter/Exit 20kHz Scope Mode");
//	iprintf("\n\t[B] .. Enter/Exit 66kHz Scope Mode");
//	iprintf("\n\t[Y] .. read software version");
//	iprintf("\n\t[X] .. exit");
//	iprintf("\n\n\t[\x11] .. digital read on PC5");
//	iprintf("\n\t[\x10] .. analog read on PC5");
//	iprintf("\n\t[\x1e] .. set pins high/low");
//	iprintf("\n\nUse the stylo to do PWM on PD5\n");
//	iprintf("\n[left] .. atmega reset test");
//	iprintf("\n[rght] .. accelerometer test\n");
//uint32 pressed;
	oldtouch.px = oldtouch.py = -1;
	while (true){
	scanKeys();
	pressed = keysDown();
	
	
		if (keysHeld() & KEY_TOUCH) {
				// TODO: we should clear the output buffer here
				touchRead(&touch);
				if ((touch.px!=oldtouch.px)||(touch.py!=oldtouch.py)){					
					iprintf("\x1b[23;0HPD5 %3u PD6 %3u", touch.px, (touch.py+16));
					}				
				if (touch.px!=oldtouch.px){
					uartAnalogWrite(PD5, touch.px);
					oldtouch.px = touch.px;}
				if (touch.py!=oldtouch.py){	
					uartAnalogWrite(PD6, (touch.py+16));
					oldtouch.py = touch.py;}
		} // Drive the PWMs
	
	if (pressed & KEY_A) {
		new_trigger_level = trigger_level;
		ready = 1;
		while(ready) {
			swiWaitForVBlank();
			if (frame==0){
				uartAnalogRead20kHz(1,dat_array2);
				display_data(dat_array1,VRAM_A,RGB15(31,31,31));
				trigger_level = new_trigger_level;
				display_data(dat_array2,VRAM_A,RGB15(31,31,31));  
				for (i=0;i<384;i++) dat_array1[i]=dat_array2[i];
				}
			else { //frame = 1
				scanKeys();
				pressed = keysDown();
				if (pressed & KEY_A) { // Leave scope mode
				ready=0;
				}
			else if (pressed & KEY_UP) { //Increase Trigger Level
				if (trigger_level < 185) new_trigger_level=trigger_level+10;
				}
			else if (pressed & KEY_DOWN) { //Decrease Trigger Level
				if (trigger_level >=15 ) new_trigger_level=trigger_level-10;
				}
			}
		}// while(ready)	
	} 

} //while (true)...
		

	uartClose();
	if (can_reboot())
		reboot();
	return 0;
}
