#include <stdio.h>

#include "stm32f10x_it.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "clock_calendar.h"
#include "hw_config.h"

#include "timer.h"

volatile uint16_t IC2Value = 0;
volatile uint16_t IC1Value = 0;

volatile uint32_t EXTI15_10_num = 0;
volatile uint32_t TIM1_UP_num = 0;

const char HEXCONV[] = "0123456789abcdef";
const char HARDFAULT[] = "HardFault: Stack @ 0x";
const char BUSFAULT[] = "BusFault: Stack @ 0x";

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
	char stack_pos;
	register char *start, *end;

	start = (char *) HARDFAULT;
	end = start + sizeof(HARDFAULT) - 1;

	for(; start < end; start++) {
		USART1->DR = *start;
		while ((USART1->SR & USART_FLAG_TXE) == 0);
	} /* for(; start <= end; start++) */

	start = (char *) ((uint32_t) &stack_pos);

	USART1->DR = HEXCONV[((uint32_t) start & 0xf0000000)>>28];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0f000000)>>24];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x00f00000)>>20];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x000f0000)>>16];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0000f000)>>12];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x00000f00)>>8];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x000000f0)>>4];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0000000f)];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = '\r';
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = '\n';
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	start = (char *) ((uint32_t) &stack_pos & 0xffffff00);
	end = start + 0xff;

	for(; start <= end; start++) {
		USART1->DR = '0';
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = 'x';
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = HEXCONV[(*start & 0xf0)>>4];
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = HEXCONV[*start & 0x0f];
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = ' ';
		while ((USART1->SR & USART_FLAG_TXE) == 0);
	} /* for(start &= 0xffffff00; start <= end; start++) */

	/* Go to infinite loop when Hard Fault exception occurs */
	while (1);
}

void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1);
}

void BusFault_Handler(void)
{
	char stack_pos;
	register char *start, *end;

	start = (char *) BUSFAULT;
	end = start + sizeof(BUSFAULT);

	for(; start < end; start++) {
		USART1->DR = *start;
		while ((USART1->SR & USART_FLAG_TXE) == 0);
	} /* for(; start <= end; start++) */

	start = (char *) ((uint32_t) &stack_pos);

	USART1->DR = HEXCONV[((uint32_t) start & 0xf0000000)>>28];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0f000000)>>24];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x00f00000)>>20];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x000f0000)>>16];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0000f000)>>12];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x00000f00)>>8];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x000000f0)>>4];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = HEXCONV[((uint32_t) start & 0x0000000f)];
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = '\r';
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	USART1->DR = '\n';
	while ((USART1->SR & USART_FLAG_TXE) == 0);

	start = (char *) ((uint32_t) &stack_pos & 0xffffff00);
	end = start + 0xff;

	for(; start <= end; start++) {
		USART1->DR = '0';
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = 'x';
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = HEXCONV[(*start & 0xf0)>>4];
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = HEXCONV[*start & 0x0f];
		while ((USART1->SR & USART_FLAG_TXE) == 0);

		USART1->DR = ' ';
		while ((USART1->SR & USART_FLAG_TXE) == 0);
	} /* for(start &= 0xffffff00; start <= end; start++) */

	/* Go to infinite loop when Bus Fault exception occurs */
	while (1);
}

void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1);
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
	tick_increment();
}

/*--------------------------------------------------
* void USB_LP_CAN1_RX0_IRQHandler(void)
* { 
* 	USB_Istr();
* }
*--------------------------------------------------*/

void DMA1_Channel1_IRQHandler(void)
{  
	DMA_ClearFlag(DMA1_FLAG_TC1);
}

/*--------------------------------------------------
* void SPI2_IRQHandler(void)
* {
* 	if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE)) {
* 		/ * Store SPI2 received data * /
* 		cc1020_get_spi_char(SPI_I2S_ReceiveData(SPI2));
* 	} / * if (SPI_GetITStatus(SPI2, SPI_IT_RXNE)) * /
* 
* 	if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE)) {
* 		/ * Send SPI2 data * /
* 		SPI_I2S_SendData(SPI2, cc1020_get_next_char_to_send(NULL));
* 	} / * if (SPI_GetITStatus(SPI2, SPI_IT_TXE)) * /
* }
*--------------------------------------------------*/

/*--------------------------------------------------
* void SPI1_IRQHandler(void)
* {
* 
* 	if (SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE)) {
* 		/ * Store SPI1 received data * /
* 		serial_flash_get_spi_char(SPI_I2S_ReceiveData(SPI1));
* 	} / * if (SPI_GetITStatus(SPI1, SPI_IT_RXNE)) * /
* 
* 	if (SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)) {
* 		/ * Send SPI1 data * /
* 		SPI_I2S_SendData(SPI1, serial_flash_get_next_char_to_send());
* 	} / * if (SPI_GetITStatus(SPI1, SPI_IT_TXE)) * /
* 
* }
*--------------------------------------------------*/

void USART1_IRQHandler(void)
{ 
	USART1_Istr();
}

/*--------------------------------------------------
* void TIM1_UP_IRQHandler(void)
* {
* 	uint8_t b;
* 	uint32_t u;
* 	static uint8_t v;
* 
* 	TIM1_UP_num++;
* 
* 	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
* 
* 	b = GPIO_ReadInputDataBit(DIO_PORT, DIO_PIN);
* 
* 	if (b != v) {
* 
* 		v = b;
* 
* 		u =  (sys_clock_freq_stepping / 1000) - SysTick->VAL;
* 
* 		if (b != 0) {
* 			u |= 0x80000000;
* 		} / * if (b != 0) * /
* 
* 		cc1020_recv_data_ook(u);
* 
* 	} / * if (b != v) * /
* 
* }
*--------------------------------------------------*/
/**
 * @brief  This function handles RTC_IRQHandler .
 * @param  None
 * @retval : None
 */
void RTC_IRQHandler(void)
{
	uint8_t Month,Day;
	uint16_t Year;
	uint32_t sec_counter;

	Month = BKP_ReadBackupRegister(BKP_DR2);
	Day = BKP_ReadBackupRegister(BKP_DR3);
	Year = BKP_ReadBackupRegister(BKP_DR4);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	RTC_ClearITPendingBit(RTC_IT_ALR);

	sec_counter = get_sec_counter() + RTC_PERIOD_VALUE;
	set_sec_counter(sec_counter);

	RTC_SetCounter(0);
	RTC_SetAlarm(RTC_ALARM_SET_VALUE);

	/*--------------------------------------------------
	* / * If counter is equal to 86399: one day was elapsed * /
	* / * This takes care of date change and resetting of counter in case of
	* 	power on - Run mode/ Main supply switched on condition* /
	* if(sec_counter == 86399)
	* {
	* 	/ * Reset counter value * /
	* 	sec_counter = 0;
	* 	set_sec_counter(0);
	* 	/ * Increment the date * /
	* 	DateUpdate();
	* }
	* if( (sec_counter / 3600 == 1) 
	* 		&& (((sec_counter % 3600) / 60) == 59) 
	* 		&& (((sec_counter % 3600) % 60) == 59) )
	* {
	* 	/ * March Correction * /
	* 	if((Month == 3) && (Day >24))
	* 	{
	* 		if(WeekDay(Year, Month, Day)==0)
	* 		{
	* 			if((SummerTimeCorrect & 0x8000) == 0x8000)
	* 			{
	* 				sec_counter += 3601;
	* 				set_sec_counter(sec_counter);
	* 				/ * Reset March correction flag * /
	* 				SummerTimeCorrect &= 0x7FFF;
	* 				/ * Set October correction flag  * /
	* 				SummerTimeCorrect |= 0x4000;
	* 				SummerTimeCorrect |= Year;
	* 				BKP_WriteBackupRegister(BKP_DR7, SummerTimeCorrect);
	* 			}
	* 		}
	* 	}
	* 	/ * October Correction * /
	* 	if((Month == 10) && (Day >24))
	* 	{
	* 		if(WeekDay(Year, Month, Day) == 0)
	* 		{
	* 			if((SummerTimeCorrect & 0x4000) == 0x4000)
	* 			{
	* 				sec_counter -= 3599;
	* 				set_sec_counter(sec_counter);
	* 				/ * Reset October correction flag * /
	* 				SummerTimeCorrect &= 0xBFFF;
	* 				/ * Set March correction flag  * /
	* 				SummerTimeCorrect |= 0x8000;
	* 				SummerTimeCorrect |= Year;
	* 				BKP_WriteBackupRegister(BKP_DR7, SummerTimeCorrect);
	* 			}
	* 		}
	* 	}
	* }
	*--------------------------------------------------*/
}


