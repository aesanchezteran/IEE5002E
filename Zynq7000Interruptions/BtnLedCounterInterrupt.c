/*
 * BtnLedCounterInterruptcode.c
 *
 *  Created on: 	23 June 2020
 *      Author: 	Alberto Sánchez
 *     Version:		1.0
 *     Recycled from Zynq Book tutorial 2
 */


#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

// GIC device ID
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID

// GPIO device ID one AXI GPIO Block with two channels
#define BTNS_LEDS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID

// GPIO Interrupt ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

// LEDs and BTN channels in GPIO
#define LED_CHANNEL 1								/* GPIO channel 1 for LEDs */
#define BTN_CHANNEL 2								/* GPIO channel 2 for BTNs */

//Definition of the button interrupt mask
#define BTN_INT 			XGPIO_IR_CH2_MASK

// Instance of the AXI GPIO
XGpio BtnLedInst;

// Instance of the GIC
XScuGic INTCInst;

static int led_data;
static int btn_value;

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------

// Interrupt handler
static void BTN_Intr_Handler(void *baseaddr_p);

// Interrupt handler configuration
static int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by button interrupt
//----------------------------------------------------


void BTN_Intr_Handler(void *InstancePtr)
{
	/******************************************************************
	 * The interrupt handler performs the following actions:
	 *
	 * 1. Disable the interrupt
	 * 2. Read the interrupt source
	 * 3. Perform an action depending on what it has read
	 * 4. Clear the interrupt
	 * 5. Enable the interrupt
	 * 6. End the interrupt handler
	 *
	 *******************************************************************/

	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BtnLedInst, BTN_INT);

	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BtnLedInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	btn_value = XGpio_DiscreteRead(&BtnLedInst, BTN_CHANNEL);
	// Increment counter based on button value

	led_data = led_data + btn_value;

    XGpio_DiscreteWrite(&BtnLedInst, LED_CHANNEL, led_data);

    // Clear the interrupt flag
    (void)XGpio_InterruptClear(&BtnLedInst, BTN_INT);

    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BtnLedInst, BTN_INT);
}

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{
  int status;
  //----------------------------------------------------
  // INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
  //----------------------------------------------------
  // Initialise BTNs and LEDs AXI GPIO
  status = XGpio_Initialize(&BtnLedInst, BTNS_LEDS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;


  // Set LEDs direction to outputs
  XGpio_SetDataDirection(&BtnLedInst, LED_CHANNEL, 0x00);

  // Set all buttons direction to inputs
  XGpio_SetDataDirection(&BtnLedInst, BTN_CHANNEL, 0xFF);

  //------------------------------------------------------
  // SETUP INTERRUPTION CONTROLLER AND HANDLER CONNECTION
  //-------------------------------------------------------


  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &BtnLedInst);
  if(status != XST_SUCCESS) return XST_FAILURE;

  //infinite loop waiting for interruption
  while(1);

  return 0;
}

//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr)
{
	// Pointer to Interruption Configuration
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation and success check
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;


	// Enable Exception handlers
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
								 &INTCInst);
	Xil_ExceptionEnable();


	// Connect GPIO interrupt to handler and check for success
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;


	// Enable GPIO interrupts
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);


	// Enable GIC
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	return XST_SUCCESS;
}



