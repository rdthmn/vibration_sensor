/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"

#include <stdarg.h>
#include <ctype.h>

#include <stdio.h>
#include <string.h>

UART_HandleTypeDef UartHandle;

#define DEBOUNCE_THRESHOLD	400

static uint8_t button = NOT_PUSHED;
static uint16_t buttonPressTime = 0;

static void SystemClock_Config(void);
static void err_handler(void);
void EXTI15_10_IRQHandler(void);

/**
  * @brief  Initialise the NP2 board by turning on the power headers
  * @retval None
  */
void BRD_init() {

	HAL_Init();
	BRD_debuguart_init();
	BRD_led_init();
	BRD_push_button_init();

	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;

	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Start DWT cycle counter used for HAL_Delayus();
}

void BRD_led_init(void) {

	//Enable GPIOA clock
	LED_ENABLE();

	// Define GPIO
	GPIO_InitTypeDef LED_InitStruct;

	LED_InitStruct.Pin = LED_PIN;
	LED_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStruct.Speed = GPIO_SPEED_FAST;
	LED_InitStruct.Pull = GPIO_PULLUP;

	HAL_GPIO_Init(LED_PORT, &LED_InitStruct);
}

void BRD_led_on(void) {

	// GPIO A5 ON
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, 1);
}

void BRD_led_off(void) {

	// GPIO A5 OFF
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, 0);
}

void BRD_led_toggle() {

	// GPIO A5 toggle
	HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}


void BRD_push_button_init(void) {

	GPIO_InitTypeDef Button_InitStruct;

	BUTTON_ENABLE();

	Button_InitStruct.Mode = GPIO_MODE_IT_RISING;
	Button_InitStruct.Pull = GPIO_NOPULL;
	Button_InitStruct.Pin  = BUTTON_PIN;
	HAL_GPIO_Init(BUTTON_PORT, &Button_InitStruct);

	HAL_NVIC_SetPriority(BUTTON_EXTI_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(BUTTON_EXTI_IRQn);

}

uint8_t BRD_button_pushed(void) {

	if (button == PUSHED) {
		return PUSHED;
	}
	return NOT_PUSHED;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BUTTON_PIN) {

		uint16_t time = HAL_GetTick();

		if(time > buttonPressTime + DEBOUNCE_THRESHOLD){
			button = !button;

			buttonPressTime = time;
		}
	}
}


void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(BUTTON_PIN);
}


/* Initialise Debug UART */
void BRD_debuguart_init()
{
	/* Configure the system clock to 100 MHz */
	SystemClock_Config();

	/*##-1- Configure the UART peripheral ######################################*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART1 configured as follow:
	  - Word Length = 8 Bits
	  - Stop Bit = One Stop bit
	  - Parity = ODD parity
	  - BaudRate = 9600 baud
	  - Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance          = USARTx;

	UartHandle.Init.BaudRate     = 115200;
	UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits     = UART_STOPBITS_1;
	UartHandle.Init.Parity       = UART_PARITY_ODD;
	UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode         = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;


	GPIO_InitTypeDef GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();
    __USART2_CLK_ENABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
	/* Initialization Error */
	err_handler();
	}
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 100000000
  *            HCLK(Hz)                       = 100000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 16
  *            PLL_N                          = 400
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /* Enable HSI Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    err_handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    err_handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void err_handler(void)
{
  /* Turn LED2 on */
  BSP_LED_On(LED2);
  while(1)
  {
  }
}

//Transmit char through debug uart and USB, if enabled
void BRD_debuguart_putc(unsigned char c)
{
	//__HAL_UART_FLUSH_DRREGISTER(&UartHandle) = (uint8_t) c;
	HAL_UART_Transmit(&UartHandle, &c, 1, 0xFFFF);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
//Transmit string through debug uart and USB, if enabled
void BRD_debuguart_puts(unsigned char *c)
{
	int i;

	for (i = 0; i < (int) strlen(c); i++) {
	__HAL_UART_FLUSH_DRREGISTER(&UartHandle) = (uint8_t) (*(c + i));

	}

}
#pragma GCC diagnostic pop

//Transmit message through debug uart and USB, if enabled
void BRD_debuguart_putm(unsigned char *c, int len)
{
	int i;
	for (i = 0; i < len; i++) {
	__HAL_UART_FLUSH_DRREGISTER(&UartHandle) = (uint8_t) (*(c + i));
	}

}


/* Debug UART getc */
unsigned char BRD_debuguart_getc() {

	uint8_t rx_char = '\0';

	//Non Block receive - 0 delay (set to HAL_MAX_DELAY for blocking)
	if (HAL_UART_Receive(&UartHandle, &rx_char, 1, 0) == HAL_OK) {
		return rx_char;
	} else {
		return '\0';
	}

	//return (uint8_t)(__HAL_UART_FLUSH_DRREGISTER(&UartHandle) & (uint8_t)0x00FF);
}

void HAL_Delayus(uint32_t us) {
	volatile uint32_t cycles = (SystemCoreClock/1000000L)*us;
	volatile uint32_t start = DWT->CYCCNT;
	do {

	} while(DWT->CYCCNT - start < cycles);

}


//#define SIDEBOARD_IN_USE

void debug_putc(char c) {

	BRD_debuguart_putc(c);

}

void debug_flush() {

}

unsigned char debug_getc(void) {
	uint8_t c = '\0';


	c = BRD_debuguart_getc();

	return c;
}

void debug_rxflush() {

}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  // Place your implementation of fputc here
  // e.g. write a character to the EVAL_COM1 and Loop until the end of transmission
  //HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
	//__HAL_UART_FLUSH_DRREGISTER(&UartHandle) = (uint8_t) ch;
	//HAL_Delay(100);

	BRD_debuguart_putc( (uint8_t)ch);
  return ch;
}//*/


GETCHAR_PROTOTYPE
{
	uint8_t c = '\0';

	c = BRD_debuguart_getc();

	//if (c != '\0') {
	//	return -1;
	//} else {
		return c;
	//}

}//*/

void debug_printf (const char *fmt, ...) {

//	printf("debug here\n");
	va_list args;

	  va_start (args, fmt);


	  vprintf (fmt, args);

	  va_end (args);
}


