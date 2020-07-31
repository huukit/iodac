#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>

#include "i2c.h"

#define ADC_I2C_ADDR 0b1100000
#define I2C_READ_CMD_BIT 1
#define I2C_WRITE_CMD_BIT 0

#define DACMAX 4095
#define STEPS 5

uint16_t step = DACMAX / STEPS;
uint16_t dacreg = 0;

void delay_ms(int ms)
{
	while (ms > 0)
	{
		_delay_ms(1);
		--ms;
	}
}

/* IoDAC, (c) 2020 Tuomas Huuki, proximia.fi */

int main() 
{
	// Set led and i2c-pins as output.
	DDRC |= ((1 << PINC0) | (1 << PINC4) | (1 << PINC5));
	
	// Level inputs as inputs with pullups.
	DDRD &= ~((1 << PIND3) | (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7));
	PORTD |= ((1 << PIND3) | (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7));
	
	PORTC &= ~(1 << PINC0);
	
    I2C_Init();

    while(1)
    {
		if(!(PIND & (1 << PIND7)))
			dacreg = 5 * step;
		else if(!(PIND & (1 << PIND6)))
			dacreg = 4 * step;
		else if(!(PIND & (1 << PIND5)))
			dacreg = 3 * step;
		else if(!(PIND & (1 << PIND4)))
			dacreg = 2 * step;
		else if(!(PIND & (1 << PIND3)))
			dacreg = 1 * step;						
		else
			dacreg = 0;
			
		I2C_Start();
		I2C_Write((ADC_I2C_ADDR << 1) | I2C_WRITE_CMD_BIT);
		I2C_Write(0x0F & (uint8_t)(dacreg >> 8));
		I2C_Write((uint8_t)dacreg);
		I2C_Stop();
						
		PORTC &= ~(1 << PINC0);
		delay_ms(100 * (dacreg / 500 + 1));
		PORTC |= (1 << PINC0);
		delay_ms(100 * (dacreg / 500 + 1));
    }
    return (0);
}

