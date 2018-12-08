#include "main.h"
#include "lcd_st7789.h"

void ST7789_Select();
void ST7789_Unselect();

__inline void ST7789_Select()
{
	PORT_ResetBits(ST7789_CS_Port, ST7789_CS_Pin);
}

__inline void ST7789_Unselect()
{
	PORT_SetBits(ST7789_CS_Port, ST7789_CS_Pin);
}

static void ST7789_WriteCommand(uint8_t cmd) 
{
	PORT_ResetBits(ST7789_DC_Port, ST7789_DC_Pin);
	ST7789_Select();
	while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
	SSP_SendData(MDR_SSP1, cmd); HAL_Delay(1);
	ST7789_Unselect();
}

static void ST7789_WriteCommand_NCS(uint8_t cmd)
{
	PORT_ResetBits(ST7789_DC_Port, ST7789_DC_Pin);
	while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
	SSP_SendData(MDR_SSP1, cmd); HAL_Delay(1);
}

static void ST7789_Reset()
{
	PORT_SetBits(ST7789_CS_Port, ST7789_CS_Pin);
	PORT_ResetBits(ST7789_Reset_Port, ST7789_Reset_Pin);
	HAL_Delay(100);
	PORT_SetBits(ST7789_Reset_Port, ST7789_Reset_Pin);
	HAL_Delay(100);

	ST7789_WriteCommand(ST7789_SWRESET);
	HAL_Delay(500);
}

static void ST7789_WriteData(uint8_t* buffer, uint16_t buff_size)
{
	uint16_t i;
	uint8_t b;
	
	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);
	ST7789_Select();
	
	for (i = 0; i < buff_size; i++)
	{
		b = *buffer++;
		while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
		SSP_SendData(MDR_SSP1, b); HAL_Delay(1);
	}
	ST7789_Unselect();
}

static void ST7789_WriteData_NCS(uint8_t* buffer, uint16_t buff_size)
{
	uint16_t i;
	uint8_t b;

	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);
	for (i = 0; i < buff_size; i++)
	{
		b = *buffer++;
		while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
		SSP_SendData(MDR_SSP1, b);
	}
}

static void ST7789_WriteDataSingle(uint8_t buffer)
{
	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);
	ST7789_Select();
	while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
	SSP_SendData(MDR_SSP1, buffer); HAL_Delay(1);
	ST7789_Unselect();
}

static void ST7789_WriteDataSingle_NCS(uint8_t buffer)
{
	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);
	while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
	SSP_SendData(MDR_SSP1, buffer); HAL_Delay(1);
}

static void ST7789_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	// Адрес столбца
	ST7789_WriteCommand_NCS(ST7789_CASET);
	uint8_t dataX[] = { (x0 >> 8), (uint8_t)(x0), (uint8_t)(x1 >> 8), (uint8_t)(x1) };
	ST7789_WriteData_NCS(dataX, sizeof(dataX));

	// Адрес строки
	ST7789_WriteCommand_NCS(ST7789_RASET);
	uint8_t dataY[] = { (y0 >> 8), (uint8_t)(y0), (uint8_t)(y1 >> 8), (uint8_t)(y1) };
	ST7789_WriteData_NCS(dataY, sizeof(dataY));

	// Запись в RAM
	ST7789_WriteCommand_NCS(ST7789_RAMWR);
}

void ST7789_SetRotation(uint8_t r) // НЕ ДОПИСАНО!
{
	//rotation = r % 4;
	ST7789_WriteCommand(ST7789_MADCTL);
	ST7789_WriteDataSingle(ST7789_MAD_MX | ST7789_MAD_MY | ST7789_MAD_RGB);
	HAL_Delay(10);
}

void ST7789_Init() 
{ 
	ST7789_Reset();
	
	ST7789_Select();
	// Выходим из сна
	ST7789_WriteCommand_NCS(ST7789_SLPOUT); 
	HAL_Delay(100);

	// Включаем дисплей
	ST7789_WriteCommand_NCS(ST7789_NORON); 

	// Формат цвета
	ST7789_WriteCommand_NCS(ST7789_MADCTL);
#ifdef CGRAM_OFFSET
	ST7789_WriteDataSingle_NCS(0x48); // BGR
#else
	ST7789_WriteDataSingle_NCS(0x40); // RGB
#endif

	ST7789_WriteCommand_NCS(ST7789_COLMOD);
	ST7789_WriteDataSingle_NCS(0x55);

	// Frame rate
	ST7789_WriteCommand_NCS(ST7789_PORCTRL);
	ST7789_WriteDataSingle_NCS(0x0c);
	ST7789_WriteDataSingle_NCS(0x0c);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x33);
	ST7789_WriteDataSingle_NCS(0x33);

	// Напряжения: VGH / VGL
	ST7789_WriteCommand_NCS(ST7789_GCTRL); 
	ST7789_WriteDataSingle_NCS(0x35);

	// Питание
	ST7789_WriteCommand_NCS(ST7789_VCOMS);
	ST7789_WriteDataSingle_NCS(0x2B);

	ST7789_WriteCommand(ST7789_LCMCTRL);
	ST7789_WriteDataSingle(0x7f);

	ST7789_WriteCommand_NCS(ST7789_VDVVRHEN);
	ST7789_WriteDataSingle_NCS(0x01);
	ST7789_WriteDataSingle_NCS(0xFF);
	
	// Напряжение VRHS
	ST7789_WriteCommand_NCS(ST7789_VRHS); 
	ST7789_WriteDataSingle_NCS(0x11);

	ST7789_WriteCommand_NCS(ST7789_VDVSET);
	ST7789_WriteDataSingle_NCS(0x20);

	ST7789_WriteCommand_NCS(ST7789_FRCTR2);
	ST7789_WriteDataSingle_NCS(0x0f);

	ST7789_WriteCommand_NCS(ST7789_PWCTRL1);
	ST7789_WriteDataSingle_NCS(0xa4);
	ST7789_WriteDataSingle_NCS(0xa1);

	// Гамма
	ST7789_WriteCommand_NCS(ST7789_PVGAMCTRL);
	ST7789_WriteDataSingle_NCS(0xd0);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x05);
	ST7789_WriteDataSingle_NCS(0x0e);
	ST7789_WriteDataSingle_NCS(0x15);
	ST7789_WriteDataSingle_NCS(0x0d);
	ST7789_WriteDataSingle_NCS(0x37);
	ST7789_WriteDataSingle_NCS(0x43);
	ST7789_WriteDataSingle_NCS(0x47);
	ST7789_WriteDataSingle_NCS(0x09);
	ST7789_WriteDataSingle_NCS(0x15);
	ST7789_WriteDataSingle_NCS(0x12);
	ST7789_WriteDataSingle_NCS(0x16);
	ST7789_WriteDataSingle_NCS(0x19);

	ST7789_WriteCommand_NCS(ST7789_NVGAMCTRL);
	ST7789_WriteDataSingle_NCS(0xd0);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x05);
	ST7789_WriteDataSingle_NCS(0x0d);
	ST7789_WriteDataSingle_NCS(0x0c);
	ST7789_WriteDataSingle_NCS(0x06);
	ST7789_WriteDataSingle_NCS(0x2d);
	ST7789_WriteDataSingle_NCS(0x44);
	ST7789_WriteDataSingle_NCS(0x40);
	ST7789_WriteDataSingle_NCS(0x0e);
	ST7789_WriteDataSingle_NCS(0x1c);
	ST7789_WriteDataSingle_NCS(0x18);
	ST7789_WriteDataSingle_NCS(0x16);
	ST7789_WriteDataSingle_NCS(0x19);

	// Выключена инверсия
	ST7789_WriteCommand_NCS(ST7789_INVOFF);

	// Сколько столбцов
	ST7789_WriteCommand_NCS(ST7789_CASET); 
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0xef); // 239

	// Сколько строк
	ST7789_WriteCommand_NCS(ST7789_RASET); 
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0x00);
	ST7789_WriteDataSingle_NCS(0xef); // 239

	ST7789_Unselect();
	// Включаем
	ST7789_WriteCommand(ST7789_DISPON);
	HAL_Delay(120);

	//ST7789_SetRotation(0);
}

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
		return;

	ST7789_Select();
	ST7789_SetAddressWindow(x, y, x + 1, y + 1);
	uint8_t data[] = { color >> 8, color & 0xFF };
	ST7789_WriteData_NCS(data, sizeof(data));
	ST7789_Unselect();
}

static void ST7789_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) 
{
	uint32_t i, b, j;

	ST7789_Select();
	ST7789_SetAddressWindow(x, y, x + font.FontWidth - 1, y + font.FontHeight - 1);

	for (i = 0; i < font.FontHeight; i++) 
	{
		b = font.data[(ch - 32) * font.FontHeight + i];
		for (j = 0; j < font.FontWidth; j++) 
		{
			if ((b << j) & 0x8000) 
			{
				uint8_t data[] = { color >> 8, color & 0xFF };
				ST7789_WriteData_NCS((uint8_t *)&data, sizeof(data));
			}
			else 
			{
				uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
				ST7789_WriteData_NCS((uint8_t *)&data, sizeof(data));
			}
		}
	}
	ST7789_Unselect();
}

void ST7789_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)
{
	ST7789_Select();

	while (*str) 
	{
		if (x + font.FontWidth >= ST7789_WIDTH) 
		{
			x = 0;
			y += font.FontHeight;
			if (y + font.FontHeight >= ST7789_HEIGHT) 
				break;
			// Пропускаем пробелы на новой линии
			if (*str == ' ') 
			{
				str++;
				continue;
			}
		}

		ST7789_WriteChar(x, y, *str, font, color, bgcolor);
		x += font.FontWidth;
		str++;
	}

	ST7789_Unselect();
}

void ST7789_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) 
{
	uint8_t data[] = { color >> 8, color & 0xFF };

	if ((x > ST7789_WIDTH) || (y > ST7789_HEIGHT)) return;
	if ((x + w - 1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
	if ((y + h - 1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

	ST7789_Select();
	ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);

	for (y = h; y > 0; y--) 
	{
		for (x = w; x > 0; x--) 
		{
			while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
			SSP_SendData(MDR_SSP1, data[0]);
			while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
			SSP_SendData(MDR_SSP1, data[1]);
		}
	}
	ST7789_Unselect();
}

void ST7789_FillScreen(uint16_t color) 
{
    ST7789_FillRectangle(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
	uint32_t i;
	if ((x > ST7789_WIDTH) || (y > ST7789_HEIGHT)) return;
	if ((x + w - 1) >= ST7789_WIDTH) return;
	if ((y + h - 1) >= ST7789_HEIGHT) return;

	ST7789_Select();
	ST7789_SetAddressWindow(x, y, (x + w - 1), (y + h - 1));
	PORT_SetBits(ST7789_DC_Port, ST7789_DC_Pin);
	for (i = 0; i < (w * h - 1); i++)
	{
		uint16_t datac = *data++;
		while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
		SSP_SendData(MDR_SSP1, (uint8_t)(datac >> 8));
		while (SSP_GetFlagStatus(MDR_SSP1, SSP_FLAG_BSY) == SET) {};
		SSP_SendData(MDR_SSP1, (datac & 0xFF));
	}
	ST7789_Unselect();
}

void ST7789_InvertColors(uint8_t invert) 
{
    ST7789_Select();
    ST7789_WriteCommand(invert ? ST7789_INVON : ST7789_INVOFF);
    ST7789_Unselect();
}
