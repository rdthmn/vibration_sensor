/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "board.h"



void delay(int counter) {

	counter = counter * 10000;

	for(int i = 0; i < counter; ++i);
}


int main(void) {

	BRD_init();

	int i = 0;

	for(;;) {

		delay(400);
		debug_printf("%d\n\r", i);

		i++;
		if (i > 25) {
			i = 0;
		}


		if (BRD_button_pushed()) {
			BRD_led_toggle();
		} else if (!BRD_button_pushed()) {
			BRD_led_off();
		}
	}
}
