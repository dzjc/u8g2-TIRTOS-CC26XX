/*
 * u2g2_shim.c
 *
 *  Created on: 14 Apr 2017
 *      Author: Dominic
 */
#include "u8g2_shim.h"
#include "Board.h"

#include <xdc/runtime/Types.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/PIN/PINCC26XX.h>
#include <ti/drivers/timer/GPTimerCC26XX.h>




#define SDA Board_DIO21
#define SCL Board_DIO22

PIN_State hi2cPins;
PIN_Config i2cPins[] = {
		SDA | PINCC26XX_INPUT_EN| PINCC26XX_PULLUP,
		SCL | PINCC26XX_INPUT_EN| PINCC26XX_PULLUP,
		PIN_TERMINATE
};



GPTimerCC26XX_Handle hTimer;

Semaphore_Handle delaySem;


Void timerCallback(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask){
	Semaphore_post(delaySem);
	GPTimerCC26XX_stop(handle);
}


void delay_init(){
	Semaphore_Params delaySemParams;
	Semaphore_Params_init(&delaySemParams);
	delaySem = Semaphore_create(0, &delaySemParams, NULL);

	GPTimerCC26XX_Params params;
	GPTimerCC26XX_Params_init(&params);
	params.width          = GPT_CONFIG_32BIT;
	params.mode	          = GPT_MODE_ONESHOT_UP;
	params.debugStallMode = GPTimerCC26XX_DEBUG_STALL_ON;
	hTimer = GPTimerCC26XX_open(Board_GPTIMER0A, &params);
	if(hTimer == NULL) {
		Task_exit();
	}
	GPTimerCC26XX_registerInterrupt(hTimer, timerCallback, GPT_INT_TIMEOUT);

	if(!PIN_open(&hi2cPins,i2cPins)){
		Task_exit();
	}
}

void delay_microseconds(uint32_t delay){
	Types_FreqHz freq;
	BIOS_getCpuFreq(&freq);
	GPTimerCC26XX_Value loadVal = (freq.lo / 1000000) * delay - 1;
	GPTimerCC26XX_setLoadValue(hTimer, loadVal);
	GPTimerCC26XX_start(hTimer);
	Semaphore_pend(delaySem,BIOS_WAIT_FOREVER);
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
      delay_init();
      break;							// can be used to setup pins
    case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
    	/* not required for SW I2C*/
      break;
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
    	/* not used */
      break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
    	delay_microseconds(arg_int * 10);
      break;
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
    	delay_microseconds(arg_int * 1000);
      break;
    case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
    	delay_microseconds(arg_int<=2?5:1);
      break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_CLOCK:
      break;
    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_DATA:
      break;
    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
      if(arg_int == 0){
    	  //PIN_setConfig(&hi2cPins,PIN_INPUT_EN|PIN_PULLUP|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW,
    		//	  SCL|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW);
    	  PINCC26XX_setOutputValue(SCL, PINCC26XX_GPIO_LOW);
    	  PINCC26XX_setOutputEnable(SCL,true);
      } else if(arg_int == 1){
    	  //PIN_setConfig(&hi2cPins,PIN_INPUT_EN|PIN_PULLUP|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW,
    		//	  SCL|PIN_INPUT_EN|PIN_PULLUP);
    	  PINCC26XX_setOutputEnable(SCL,false);
      }
      break;							// arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
        if(arg_int == 0){
      	  //PIN_setConfig(&hi2cPins,PIN_INPUT_EN|PIN_PULLUP|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW,
      		//	  SDA|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW);
      	  PINCC26XX_setOutputValue(SDA, PINCC26XX_GPIO_LOW);
      	  PINCC26XX_setOutputEnable(SDA,true);
        } else if(arg_int == 1){
      	  //PIN_setConfig(&hi2cPins,PIN_INPUT_EN|PIN_PULLUP|PIN_GPIO_OUTPUT_EN|PIN_GPIO_LOW,
      		//	  SDA|PIN_INPUT_EN|PIN_PULLUP);
          PINCC26XX_setOutputEnable(SDA,false);

        }
      break;							// arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
      u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_NEXT:
      u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_PREV:
      u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_HOME:
      u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;
  }
  return 1;
}
