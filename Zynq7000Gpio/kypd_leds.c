/*
 * kypd_leds.c
 *
 *  Created on: 	23 March 2021
 *      Author: 	Alberto Sanchez
 *     Version:		1.0
 */

/************************************************************** 
*
*    SECTION: VERSION HISTORY
*
***************************************************************
*
*	v1.0 - 23 March 2021
*		First version created, modified from Zynq Book tutorial
*
***************************************************************/

/************************************************************** 
*
*    SECTION: DESCRIPTION
*
***************************************************************
*
* This file contains an example of using the GPIO driver to 
* provide control a keypad in JE Pmod in the Zybo Z7-20 Board.
* The system connects to the keypad by using AXI GPIO.
* The AXI GPIO is connected to the LEDs (CH1) and Keypad in 
* (CH2) on the Zybo.
*
* The provided code scans the keypad and shows the binary value 
* in the leds
*
**************************************************************/

/************************************************************** 
*
*    SECTION: LIBRARIES
*
***************************************************************/
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************************************************** 
*
*    SECTION: DEFINITIONS
*
***************************************************************/
#define GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID	/* GPIO device that LEDs are connected to */
#define LED_CHANNEL 1					/* GPIO port 1 for LEDs */
#define KEYPAD_CHANNEL 2				/* GPIO port 2 for KEYPAD */
#define printf xil_printf				/* smaller, optimised printf */

/************************************************************** 
*
*    SECTION: DEVICE INSTANCES
*
***************************************************************/
XGpio Gpio;		/* GPIO Device driver instance */

/************************************************************** 
*
*     SECTION: FUNCTION PROTOTYPES
*
***************************************************************/

int KEYPDLEDOutputExample(void);
void Delay(void);


/* Main function. */
int main(void){
	int Status;

	/* Execute the LED output. */
	Status = KEYPDLEDOutputExample();
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO output to the LEDs failed!\r\n");
	}

	return 0;
}

/**************************************************************
*
* SECTION: PROTOTYPE FUNCTION IMPLEMENTATIONS
*
**************************************************************/

int KEYPDLEDOutputExample(void){
	int Status;
	int led; 	/* Create variable to pass on to AXI to lightup LEDs */
	int cols = 0xe; 	/* Create variable to sweep columns */
	int rows = 0x0;   /* Create a variable to scan rows */
	int cols_msb = 0x0;

		/* GPIO driver initialization */
		Status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
		if (Status != XST_SUCCESS){
			return XST_FAILURE;
		}

		/*Set the direction for the LEDs to output. */
		XGpio_SetDataDirection(&Gpio, LED_CHANNEL, 0x0);

		/*Set the direction for the KEYPAD to input. */
		XGpio_SetDataDirection(&Gpio, KEYPAD_CHANNEL, 0xf0);

		/* Loop forever */
			while (1) {
				/* Write output to the Columns */
				XGpio_DiscreteWrite(&Gpio, KEYPAD_CHANNEL, cols);

				// Read the rows
				rows = XGpio_DiscreteRead(&Gpio, KEYPAD_CHANNEL);

				// flush the 4 least significant bits
				rows = rows >> 4;

				switch((cols & 0x0000000f)){
					case 0xe:
						switch(rows){
						case 0xf:
							led = 0x0;
							break;
						case 0xe:
							led = 0x1;
							break;
						case 0xd:
							led = 0x4;
							break;
						case 0xb:
							led = 0x7;
							break;
						case 0x7:
							led = 0xe;
							break;
						}
						break;
						case 0xd:
							switch(rows){
							case 0xf:
								led = 0x0;
								break;
							case 0xe:
								led = 0x2;
								break;
							case 0xd:
								led = 0x5;
								break;
							case 0xb:
								led = 0x8;
								break;
							case 0x7:
								led = 0x0;
								break;
							}
							break;
						case 0xb:
							switch(rows){
							case 0xf:
								led = 0x0;
								break;
							case 0xe:
								led = 0x3;
								break;
							case 0xd:
								led = 0x6;
								break;
							case 0xb:
								led = 0x9;
								break;
							case 0x7:
								led = 0xf;
								break;
							}
							break;
						case 0x7:
							switch(rows){
							case 0xf:
								led = 0x0;
								break;
							case 0xe:
								led = 0xa;
								break;
							case 0xd:
								led = 0xb;
								break;
							case 0xb:
								led = 0xc;
								break;
							case 0x7:
								led = 0xd;
								break;
							}
							break;
						}

				/* Write output to the LEDs. */
				XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, led);

				if(cols == 0x7){
					cols = 0xe;
				}
				else{
					// Shift the '0' in the cols to the left
					cols_msb = (cols >> 3) & 1;  // Saving the msb of cols
					cols = (cols << 1) | cols_msb; // rotate the 4 bit so cols to the left
				}
				}
			}


void Delay(void){
	int counter = 50000;
	while(counter>0)
		counter -= counter;
}



