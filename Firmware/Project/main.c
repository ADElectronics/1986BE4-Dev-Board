#include "main.h"
#include "image.h"
#include <stdio.h>
#include "lcd_st7789.h"
#include "lcd_fonts.h"

#define ADCIUBUFSIZE 240
static int32_t ADCIU_Ch1[ADCIUBUFSIZE];
static uint16_t ADCIU_Counter;
static int8_t ADCIU_BufferReady = 0;

void ADCIU_PlotBuf();

// Для вывода символов printf
int __io_putchar(int ch)
{
	unsigned char c = (uint8_t)ch;
	while (UART_GetFlagStatus(MDR_UART1, UART_FLAG_TXFE) != SET);
	UART_SendData(MDR_UART1, c);
	return 0;
}

int main(void)
{
	PORT_InitTypeDef PORT_InitStructure;
	ADCIU_InitTypeDef ADCIU_InitStruct;
	ADCIU_ChannelInitTypeDef ADCIU_ChannelInitStruct;
	SSP_InitTypeDef sSSP;
	UART_InitTypeDef UART_InitStructure;
	// для вывода на экран
	char buffer[30];

	// Включаем тактирование HSE
	RST_CLK_DeInit();
	RST_CLK_CPU_PLLcmd(DISABLE);
	RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	while (RST_CLK_HSEstatus() != SUCCESS) ;

	// Переключаем тактирование MCU от PLL ((8 / 2) * 8 = 32 МГц)
	//RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv2, RST_CLK_CPU_PLLmul8); 
	// Можно разогнать до 80 МГц !!!, но возможны проблемы с АЦП и т.д.
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10); 
	RST_CLK_CPU_PLLcmd(ENABLE);
	while (RST_CLK_CPU_PLLstatus() != SUCCESS) ;

	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);
	RST_CLK_CPU_PLLuse(ENABLE);
	// Всё тактируем от PLL
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3 /*0x104*/);
	//RST_CLK_PCLKPer1_C1_CLKSelection(RST_CLK_PER1_C1_CLK_SRC_LSI);
	RST_CLK_PCLKPer1_C2_CLKSelection(RST_CLK_PER1_C2_CLK_SRC_PLLCPU);
	SystemCoreClockUpdate();

	// Включаем тактирование на всех портах
	RST_CLK_PCLKcmd((RST_CLK_PCLK_PORTA | RST_CLK_PCLK_PORTB | RST_CLK_PCLK_PORTC), ENABLE);

	// Тактирование периферии
	RST_CLK_PCLKcmd(RST_CLK_PCLK_SSP1, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_UART1, ENABLE);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_ADCUI, ENABLE);

	// Инициализация NVIC
	SCB->AIRCR = 0x05FA0000 | ((uint32_t)0x500);
	// Выключение всех прерываний
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICER[0] = 0xFFFFFFFF;

	_delay_Init();

	PORT_DeInit(MDR_PORTB);
	PORT_DeInit(MDR_PORTA);

	PORT_InitStructure.PORT_Pin = LED_Pin;
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_FAST;
	PORT_Init(LED_Port, &PORT_InitStructure);

	// Led - ON
	PORT_SetBits(LED_Port, LED_Pin);

	// CS, DC, Reset
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_InitStructure.PORT_Pin = (SSD1306_DC_Pin | SSD1306_Reset_Pin | SSD1306_CS_Pin);
	PORT_Init(MDR_PORTA, &PORT_InitStructure);

	// PB0 - UART1_TX
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_MAIN;
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_Pin = PORT_Pin_0;
	PORT_Init(MDR_PORTB, &PORT_InitStructure);

	// PB1 - UART1_RX
	PORT_InitStructure.PORT_OE = PORT_OE_IN;
	PORT_InitStructure.PORT_Pin = PORT_Pin_1;
	PORT_Init(MDR_PORTB, &PORT_InitStructure);

	// SPI - SCK, MOSI
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_MAIN;
	PORT_InitStructure.PORT_Pin = (PORT_Pin_13 | PORT_Pin_15);
	PORT_Init(MDR_PORTA, &PORT_InitStructure);

	// SPI - MISO
	PORT_InitStructure.PORT_Pin = (PORT_Pin_14);
	PORT_InitStructure.PORT_OE = PORT_OE_IN;
	PORT_Init(MDR_PORTA, &PORT_InitStructure);
	
	UART_DeInit(MDR_UART1);
	UART_BRGInit(MDR_UART1, UART_HCLKdiv1);

	UART_InitStructure.UART_BaudRate = 256000;
	UART_InitStructure.UART_WordLength = UART_WordLength8b;
	UART_InitStructure.UART_StopBits = UART_StopBits1;
	UART_InitStructure.UART_Parity = UART_Parity_No;
	UART_InitStructure.UART_FIFOMode = UART_FIFO_OFF;
	UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE; //UART_HardwareFlowControl_None;

	UART_Init(MDR_UART1, &UART_InitStructure);
	UART_Cmd(MDR_UART1, ENABLE);

	SSP_DeInit(MDR_SSP1);
	SSP_BRGInit(MDR_SSP1, SSP_HCLKdiv1);

	SSP_StructInit(&sSSP);
	sSSP.SSP_SCR = 0;
	sSSP.SSP_CPSDVSR = 2;
	sSSP.SSP_Mode = SSP_ModeMaster;
	sSSP.SSP_WordLength = SSP_WordLength8b;
	sSSP.SSP_SPH = SSP_SPH_1Edge;
	sSSP.SSP_SPO = SSP_SPO_Low;
	sSSP.SSP_FRF = SSP_FRF_SPI_Motorola;
	sSSP.SSP_HardwareFlowControl = SSP_HardwareFlowControl_None;
	SSP_Init(MDR_SSP1, &sSSP);
	SSP_Cmd(MDR_SSP1, ENABLE);

	
	printf("MCU Started...\n");
	printf("Core clock: %luHz\n", SystemCoreClock);
	
	ST7789_Init();
	ST7789_FillScreen(ST7789_BLACK);

	printf("LCD Inited...\n");
	
	
	RST_CLK_ADCIUclkDeInit();
	RST_CLK_ADCIUclkSelection(RST_CLK_ADCIUclkPLLCPU);
	RST_CLK_ADCIUclkPrescaler(RST_CLK_ADCIUclkDIV128);
	RST_CLK_ADCIUclkCMD(ENABLE);
	
	ADCIU_DeInit();
	ADCIU_StructInit(&ADCIU_InitStruct);
	ADCIU_InitStruct.ADCIU_VoltageReference = ADCIU_REFEREVCE_VOLTAGE_INTERNAL;// ADCIU_REFEREVCE_VOLTAGE_EXTERNAL;
	ADCIU_InitStruct.ADCIU_BypassBufferReference = ADCIU_BYPASS_BUFFER_REFERNCE_ENABLE;//ADCIU_BYPASS_BUFFER_REFERNCE_ENABLE;
	//ADCIU_InitStruct.ADCIU_DigitalGain = 0x00;//Gain=1
	ADCIU_Init(&ADCIU_InitStruct);

	ADCIU_ChannelStructInit(&ADCIU_ChannelInitStruct);
	ADCIU_ChannelInitStruct.ADCIU_ChopperState = DISABLE;// DISABLE;
	ADCIU_ChannelInitStruct.ADCIU_IT_FIFO_NotEmptyState = ENABLE;
	ADCIU_ChannelInitStruct.ADCIU_IT_FIFO_OverflowState = DISABLE;
	ADCIU_InitChannel(ADCIU_CHANNEL_NUMBER1, &ADCIU_ChannelInitStruct);

	NVIC_EnableIRQ(ADCIU_CH1_IRQn);
	ADCIU_ChannelCmd(ADCIU_CHANNEL_NUMBER1, ENABLE);
	printf("ADCIU Inited...\n");
	
	ST7789_DrawImage(0, 0, ST7789_WIDTH, ST7789_HEIGHT, (uint16_t *)&img);
	HAL_Delay(2000);

	//printf("ADCIU Test...\n");
	//ST7789_WriteString(0, 0, "ADCIU Test...", Font_11x18, ST7789_CYAN, ST7789_BLACK);
	//HAL_Delay(500);
	for(;;)
	{
		//PORT_SetBits(LED_Port, LED_Pin);
		//HAL_Delay(1);
		PORT_ResetBits(LED_Port, LED_Pin);

		//sprintf(buffer, "RAW: %li          ", ADCIU_Ch1[0]);
		//ST7789_WriteString(0, 0, buffer, Font_11x18, ST7789_RED, ST7789_BLACK);
		ADCIU_PlotBuf();
	}
}

void ADCIU_CH1_IRQHandler()
{
	if (ADCIU_GetStatusFlag(ADCIU_CHANNEL_NUMBER1, ADCIU_FLAG_FITO_NOT_EMPTY)) 
	{
		if (ADCIU_BufferReady == 0)
		{
			ADCIU_Ch1[ADCIU_Counter] = ADCIU_GetResult(ADCIU_CHANNEL_NUMBER1);
			if (ADCIU_Ch1[ADCIU_Counter] & (1 << 21))
				ADCIU_Ch1[ADCIU_Counter] |= (0xFF << 24);

			if (ADCIU_Counter < ADCIUBUFSIZE)
				ADCIU_Counter++;
			else
			{
				ADCIU_BufferReady = 1;
				ADCIU_Counter = 0;
			}
		}
	}
	
	if (ADCIU_GetStatusFlag(ADCIU_CHANNEL_NUMBER1, ADCIU_FLAG_FIFO_OVERFLOW))
	{
		PORT_SetBits(LED_Port, LED_Pin);
	}
}

#define ADCIU_VSCALE_MIN		(ADCIU_MIN)
#define ADCIU_VSCALE_MAX		(ADCIU_MAX)
#define ADCIU_VSCALE_PIX_MIN	0
#define ADCIU_VSCALE_PIX_MAX	239
#define ADCIU_VSCALE_GETY(x)	(int16_t)(ADCIU_VSCALE_PIX_MIN + ( x / ((ADCIU_VSCALE_MAX - ADCIU_VSCALE_MIN) / (ADCIU_VSCALE_PIX_MAX - ADCIU_VSCALE_PIX_MIN))))

void ADCIU_PlotBuf()
{
	uint8_t x;
	// для выравнивания нуля посередине
	int16_t y_offset = 240;

	if (ADCIU_BufferReady)
	{
		for (x = 0; x < ST7789_WIDTH; x++)
		{
			int32_t y_cur = ADCIU_GETV_I32(ADCIU_Ch1[x]);

			ST7789_FillRectangle(x, 0, 1, ST7789_HEIGHT, ST7789_BLACK);
			ST7789_DrawPixel(x, y_cur + y_offset, ST7789_MAGENTA);
		}
		ADCIU_BufferReady = 0x00;
	}
}

