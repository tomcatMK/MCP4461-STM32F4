/****** STM32F429 HAL CUBEMX ************/


/*
 http://ww1.microchip.com/downloads/en/DeviceDoc/22265a.pdf
ADDR is 0101 110x ( for the MCP 4461, 0101 aaax for ones with more than 1 adress pin)
 
 x is the R/!W bit set by the wire library
 
 COMMAND char :  AAAA CC DD
 AAAA        : Adress in memory to do things
 CC     : What to do at AAAA ( R,W,Inc,Decr )
 00      : Write
 01      : increment (NV only)
 10      : Decrement (NV only)
 11      : Read
 DD  : D9 and D8 ( MSB of the value we are sending)
 
 
 Datachar     : DDDDDDDD ( 0-255 : 00-FF ), this gets used with D8 to give a full range of 00-257 : 0x00-0x100
 8 bit data ( 8 LSB of the 10 bit data, 7 or 8 bit devices will only use the lower 7 or 8 bits )
 memory map
 Data memory is only 8 bits wide
 
 ADDR    |FUNCTION    |CMDs
 00      |VOL WP 0    |R,W,Inc,Decr 
 01      |VOL WP 1    |R,W,Inc,Decr 
 02      |NV WP 0     |R,W,HVInc,HVDecr 
 03      |NV WP 1     |R,W,HVInc,HVDecr 
 04      |VOL TCON    |R,W
 05      |STATUS      |R
 06 - 0E |EEPROM      |R,W
 0F      |EEPROM      |R,W,HV_WPD,HV_RPD
 
 
 to send/recieve something:
 IIC addr + RW : 0101 110x
	
 COMMAND       : AAAA CCDD
 DATA          : DDDD DDDD * only read and write commands have data 
 */

#include "MCP4461.h"
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "i2c.h"  
#include "gpio.h"



/* Buffer used for transmission */
uint8_t aTxBuffer[] = {0, 0, 0, 0};

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];
/***************************************/
uint8_t _mcp4461_address;
uint8_t _wiper;
uint8_t _value;
int wiper_0_status;
int wiper_1_status;
int wiper_2_status;
int wiper_3_status;


//initialise the I2C interface as master ie local address is 0
void Init_MCP4461(void) 
{
	/*##(-1)- Configure the I2C peripheral #######################################*/
  I2cHandle.Instance             = I2Cx;
  
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.ClockSpeed      = 100000;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  I2cHandle.Init.OwnAddress1     = 0;
  I2cHandle.Init.OwnAddress2     = 0;
  
  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(3);    
  }
	
   
}

/**************************************************************************************/
void setVolatileWiper(uint8_t wiper, uint16_t wiper_value)
{ 

static uint16_t value;
static uint8_t d_byte;
static uint8_t c_byte;
static uint8_t num;	
	value = wiper_value;
	num=wiper;
  if (value > 0xFF) value = 0x100;
  d_byte = (uint8_t)value;
  
  if (value > 0xFF)
		c_byte = 0x1; //the 8th data bit is 1
  else
		c_byte =0;
  switch (num) 
	{
      case 0:
        c_byte |= MCP4461_VW0;
        break;
      case 1:
        c_byte |= MCP4461_VW1;
        break;
      case 2:
        c_byte |= MCP4461_VW2;
        break;
      case 3:
        c_byte |= MCP4461_VW3;
        break;
      default: 
        break; //not a valid wiper
  } 
  c_byte |= MCP4461_WRITE;
	
	aTxBuffer[0] = c_byte;
	aTxBuffer[1] = d_byte;
	
WriteBuffer(DEFAULT_ADDRESS, aTxBuffer, 2);
	  HAL_Delay(20); 	
  }

/**************************************************************************************/
void setNonVolatileWiper(uint8_t wiper, uint16_t wiper_value)
{
  static uint16_t value;
	static uint8_t d_byte;
	static uint8_t c_byte;
	static uint8_t num;	
  value = wiper_value;
	num=wiper;
	
  if (value > 0xFF) value = 0x100;
  d_byte = (uint8_t)value;
  
  if (value > 0xFF)c_byte = 0x1; //the 8th data bit is 1
  else c_byte =0;
	
  switch (num) 
	{
      case 0:
        c_byte |= MCP4461_NVW0;
        break;
      case 1:
        c_byte |= MCP4461_NVW1;
        break;
      case 2:
        c_byte |= MCP4461_NVW2;
        break;
      case 3:
        c_byte |= MCP4461_NVW3;
        break;
      default: 
        break; //not a valid wiper
  } 
	c_byte |= MCP4461_WRITE;

	aTxBuffer[0] = c_byte;
	aTxBuffer[1] = d_byte;
	WriteBuffer(DEFAULT_ADDRESS, aTxBuffer, 2);
	
  HAL_Delay(20); 
  }
/**************************************************************************************/
//return the status register
void getStatus()
{

static  uint16_t c_byte =0;
//  c_byte |= MCP4461_STATUS;
 //aTxBuffer[0]= MCP4461_READ; 
 c_byte =MCP4461_STATUS;
 c_byte|=(MCP4461_READ<<2);

 ReadBuffer(DEFAULT_ADDRESS, c_byte, aRxBuffer,2);

}

void disableWP(void)
{
	static uint8_t buff[3]={0};
	
	buff[0]=0xF4;
	
	WriteBuffer(DEFAULT_ADDRESS,  aTxBuffer, 2);
	HAL_I2C_Master_Transmit(&I2cHandle,DEFAULT_ADDRESS<<1,buff,1,1000);
}

/**************************************************************************************/
int unlockWiper(uint8_t wiper)
{
uint8_t memory_address;
uint8_t buff=0;
if(wiper == 0) 
	memory_address = MCP4461_NVW0;// 
else if(wiper == 1) 
	memory_address = MCP4461_NVW1;// 
else if(wiper == 2)
	memory_address = MCP4461_NVW2;//
else if(wiper == 3) 
	memory_address = MCP4461_NVW3;// 
else return -1;

buff =memory_address;
buff+=(MCP4461_DECREMENT << 2);


HAL_I2C_Master_Transmit(&I2cHandle,DEFAULT_ADDRESS<<1,&buff,1,1000);
//WriteBuffer(DEFAULT_ADDRESS, aTxBuffer, 1); 
	

return 0;
}
/**************************************************************************************/
int lockWiper(uint8_t wiper)
{
uint8_t memory_address;
if(wiper == 0) memory_address = MCP4461_NVW0; //<< 4;
else if(wiper == 1) memory_address = MCP4461_NVW1;// << 4;
else if(wiper == 2) memory_address = MCP4461_NVW2;// << 4;
else if(wiper == 3) memory_address =MCP4461_NVW3;// << 4;
else return -1;

aTxBuffer[0] = memory_address;
aTxBuffer[1] = MCP4461_INCREMENT << 2;
WriteBuffer(DEFAULT_ADDRESS, aTxBuffer, 2);	
	
if(wiper == 0) wiper_0_status = LOCKED;
else if(wiper == 1) wiper_1_status = LOCKED;
else if(wiper == 2) wiper_2_status = LOCKED;
else if(wiper == 3) wiper_3_status = LOCKED;
return 0;
}
/**************************************************************************************/
int getWiper(uint8_t wiper, char type)
{

	static  uint16_t c_byte =0;
 
uint8_t memory_address;
if(wiper == 0 && type == VOLATILE) memory_address = MCP4461_VW0;// << 4;
else if(wiper == 1 && type == VOLATILE) memory_address = MCP4461_VW1;// << 4;
else if(wiper == 2 && type == VOLATILE) memory_address = MCP4461_VW2;// << 4;
else if(wiper == 3 && type == VOLATILE) memory_address = MCP4461_VW3;// << 4;
else if(wiper == 0 && type == NON_VOLATILE)
{
//if(wiper_0_status == LOCKED) return -1;
memory_address = MCP4461_NVW0;// << 4;
}
else if(wiper == 1 && type == NON_VOLATILE)
{
//if(wiper_1_status == LOCKED) return -1;
memory_address = MCP4461_NVW1;// << 4;
}
else if(wiper == 2 && type == NON_VOLATILE)
{
//if(wiper_2_status == LOCKED) return -1;
memory_address = MCP4461_NVW2;// << 4;
}
else if(wiper == 3 && type == NON_VOLATILE)
{
//if(wiper_3_status == LOCKED) return -1;
memory_address = MCP4461_NVW3;// << 4;
}
else return -1;


c_byte |= memory_address;
c_byte |= MCP4461_READ << 2; 
ReadBuffer(DEFAULT_ADDRESS, c_byte, aRxBuffer, 2);
return aRxBuffer[1];

//return -1;
}
/**************************************************************************************/
/*	 I2C func 	*/
/**************************************************************************************/
void WriteBuffer(uint8_t I2C_ADDRESS, uint8_t *aTxBuffer, uint8_t txBuffSize) 
{
    /* -> Start the transmission process */
    /* While the I2C in reception process, user can transmit data through "aTxBuffer" buffer */
    while(HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)I2C_ADDRESS<<1, (uint8_t*)aTxBuffer, (uint16_t)txBuffSize, (uint32_t)1000)!= HAL_OK)
    {
        /*
         * Error_Handler() function is called when Timeout error occurs.
         * When Acknowledge failure occurs (Slave don't acknowledge it's address)
         * Master restarts communication
         */
 
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
            //DEBUG(3, "In I2C::WriteBuffer -> error");
            Error_Handler(1);
        }
 
    }
 
    /* -> Wait for the end of the transfer */
    /* Before starting a new communication transfer, you need to check the current
     * state of the peripheral; if it’s busy you need to wait for the end of current
     * transfer before starting a new one.
     * For simplicity reasons, this example is just waiting till the end of the
     * transfer, but application may perform other tasks while transfer operation
     * is ongoing.
     */
      while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
      {
      }
}

void ReadBuffer(uint8_t I2C_ADDRESS, uint8_t RegAddr, uint8_t *aRxBuffer, uint8_t txBuffSize)
{
    /* -> Lets ask for register's address */
    WriteBuffer(I2C_ADDRESS, &RegAddr, 1);
 
    /* -> Put I2C peripheral in reception process */
    while(HAL_I2C_Master_Receive(&I2cHandle, ((uint16_t)I2C_ADDRESS<<1)|1, aRxBuffer, (uint16_t)txBuffSize, (uint32_t)1000) != HAL_OK)
    {
        /* Error_Handler() function is called when Timeout error occurs.
         * When Acknowledge failure occurs (Slave don't acknowledge it's address)
         * Master restarts communication
         */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
            //DEBUG(3, "In I2C::WriteBuffer -> error");
            Error_Handler(2);
        }
    }
 
    /* -> Wait for the end of the transfer */
    /* Before starting a new communication transfer, you need to check the current
     * state of the peripheral; if it’s busy you need to wait for the end of current
     * transfer before starting a new one.
     * For simplicity reasons, this example is just waiting till the end of the
     * transfer, but application may perform other tasks while transfer operation
     * is ongoing.
     **/
    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
    {
    }
}


/**************************************************/
/********** Error *********************************/
/*************************** End of file ****************************/
void Error_Handler(uint8_t err_num)
{
  /* Turn LED4 on */
	switch(err_num)
	{
		case 1:
					LED_On(1);
					break;
		case 2:
					LED_On(2);
					break;
		default:
			LED_On(1);
			LED_On(2);
	}
  
  while(1)
  {
		LED_On(1);
		LED_On(2);
		HAL_Delay(300);
		LED_Off(1);
		LED_Off(2);
		HAL_Delay(200);
  }
}
/*****************************************************/
