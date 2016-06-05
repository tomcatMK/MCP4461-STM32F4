#ifndef MCP4461_H
#define MCP4461_H

#include "stm32f4xx_hal.h"

#include "stdint.h"
/*
*	Default Address
*A1- 1
*A0- 0
*					A6 A5 A4 A3 A2 A1 A0 R/W	
*					0		1	 0  1  1  1  0	
*/
#define DEFAULT_ADDRESS  						0x2E 
#define DEFAULT_WIPER_VALUE						0x80  //Default to the wipers in midrange
#define I2Cx									I2C3

/************ old *********************/
// meory addresses (all shifted 4 bits left)
//For all the Wipers 0x100 = Full scale, 0x80 = mid scale, 0x0 = Zero scale
#define MCP4461_VW0 0x00
#define MCP4461_VW1 0x10
#define MCP4461_NVW0 0x20
#define MCP4461_NVW1 0x30
//TCON0: D8: Reserved D7:R1HW D6: R1A D5:R1W D4:R1B D3:R0HW D2:R0A D1:R0W D0: R0B
#define MCP4461_TCON0 0x40
//STATUS: D8:D7: Reserved D6: WL3 D5:WL2 D4:EEWA D3:WL1 D2:WL0 D1:Reserved D0: WP
#define MCP4461_STATUS 0x50
#define MCP4461_VW2 0x60
#define MCP4461_VW3 0x70
#define MCP4461_NVW2 0x80
#define MCP4461_NVW3 0x90
//TCON0: D8: Reserved D7:R3HW D6: R3A D5:R3W D4:R3B D3:R2HW D2:R2A D1:R2W D0: R2B
#define MCP4461_TCON1 0xA0




//control commands
#define MCP4461_WRITE 			0x0	
#define MCP4461_INCREMENT 		0x1 
#define MCP4461_DECREMENT 		0x2  
#define MCP4461_READ 			0x3 
// State
#define LOCKED				 0
#define UNLOCKED			 1
// Memory Type
#define NON_VOLATILE		 0
#define VOLATILE 			 1

extern I2C_HandleTypeDef I2cHandle;

void Init_MCP4461(void) ;
void setVolatileWiper(uint8_t, uint16_t);
void setNonVolatileWiper(uint8_t, uint16_t);
/*******  Protection *******************/
void getStatus(void);
void disableWP(void);
int unlockWiper(uint8_t wiper);
int lockWiper(uint8_t wiper);


/******** Read Wiper status **************/
int getWiper(uint8_t wiper, char type);

/***** I2C Read/Write ***********************/

void WriteBuffer(uint8_t I2C_ADDRESS, uint8_t *aTxBuffer, uint8_t txBuffSize);
void ReadBuffer(uint8_t I2C_ADDRESS, uint8_t RegAddr, uint8_t *aRxBuffer, uint8_t txBuffSize);

/***** STM32F429 led error handler ***********************/

void Error_Handler(uint8_t err_num);

/*********************************************************/
/* Size of Transmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      TXBUFFERSIZE
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#endif //MCP4461_H
