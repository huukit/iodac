#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"

void I2C_Init()
{
    TWSR=0x00; 
    TWBR=0x46; // SCL frequency is 50K with 16Mhz
    TWCR=0x04; // Enable I2c.
}

void I2C_Start()
{
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));
    while (!(TWCR & (1<<TWINT)));
}

void I2C_Stop(void)
{
    TWCR = ((1<< TWINT) | (1<<TWEN) | (1<<TWSTO));
	_delay_us(100) ; 
}

void I2C_Write(uint8_t v_i2cData_u8)
{
    TWDR = v_i2cData_u8 ;
    TWCR = ((1<< TWINT) | (1<<TWEN));
    while (!(TWCR & (1 <<TWINT)));
}

uint8_t I2C_Read(uint8_t v_ackOption_u8)
{
    TWCR = ((1<< TWINT) | (1<<TWEN) | (v_ackOption_u8<<TWEA));
    while ( !(TWCR & (1 <<TWINT)));
    return TWDR;
}
