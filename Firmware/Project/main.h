#ifndef __MAIN_H__
#define __MAIN_H__

#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_ssp.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_adciu.h"

#include "delay.h"
#include <math.h> 

#define SSD1306_USE_SPI
#define SSD1306_SPI_PORT		
// MOSI - PA15
// MISO - PA14
// SCK - PA13
#define LED_Port				MDR_PORTB
#define LED_Pin					PORT_Pin_6

#define SSD1306_CS_Port         MDR_PORTA
#define SSD1306_CS_Pin          PORT_Pin_0
#define SSD1306_DC_Port         MDR_PORTA
#define SSD1306_DC_Pin          PORT_Pin_11
#define SSD1306_Reset_Port      MDR_PORTA
#define SSD1306_Reset_Pin       PORT_Pin_12
//#define SSD1306_INVERSE_COLOR

#define ST7789_CS_Port			MDR_PORTA
#define ST7789_CS_Pin			PORT_Pin_0
#define ST7789_DC_Port			MDR_PORTA
#define ST7789_DC_Pin			PORT_Pin_11
#define ST7789_Reset_Port		MDR_PORTA
#define ST7789_Reset_Pin		PORT_Pin_12

// Для удобства (и привычки :D)
#define HAL_Delay(ms) _delay_ms(ms)


// ADCIU
#define ADCIU_MAX				0xFFFFFF
#define ADCIU_MIN				0x0
#define ADCIU_VREF				1.20F
#define ADCIU_AGAIN				1.0F
#define ADCIU_ADCPGA			00.0F//42.0F
#define ADCIU_DGAIN				(powf(10,(ADCIU_ADCPGA/20.0F)))
//#define ADCIU_GETV(bits)		(float)((2.0F * ADCIU_VREF * ADCIU_AGAIN * ADCIU_DGAIN * powf(2,22)) / (bits - powf(2,23)))
#define ADCIU_GETV_F(bits)		(float)((2.0F * ADCIU_VREF * ADCIU_AGAIN * ADCIU_DGAIN * 4194304.0F) / (bits - 8388608.0F))
#define ADCIU_GETV_I32(bits)	(int32_t)(100.0F * ((2.0F * ADCIU_VREF * ADCIU_AGAIN * ADCIU_DGAIN * 4194304.0F) / (bits - 8388608.0F)))
#endif // __MAIN_H__
