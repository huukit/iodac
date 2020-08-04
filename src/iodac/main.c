#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdbool.h>

#include "i2c.h"

#define ADC_I2C_ADDR 0b1100000
#define I2C_READ_CMD_BIT 1
#define I2C_WRITE_CMD_BIT 0

#define DACMAX 4095

int16_t dacreg = 0;
int16_t increment = 0;
uint8_t write_done = true;

int16_t step1 EEMEM;
int16_t step2 EEMEM;
int16_t step3 EEMEM;
int16_t step4 EEMEM;
int16_t step5 EEMEM;

void delay_ms(int ms)
{
	while (ms > 0)
	{
		_delay_ms(1);
		--ms;
	}
}

/* IoDAC, (c) 2020 Tuomas Huuki, proximia.fi */

void check_limits_and_set(int16_t *value, int16_t increment){
	if((*value + increment) > DACMAX) *value = DACMAX;
	else if((*value + increment) < 0) *value = 0;
	else *value += increment;
}

int main() 
{
	// Set led and i2c-pins as output.
	DDRC |= ((1 << PINC0) | (1 << PINC4) | (1 << PINC5));
	
	// Level inputs as inputs with pullups.
	DDRD &= ~((1 << PIND3) | (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7));
	PORTD |= ((1 << PIND3) | (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7));
	
	// Configuration ports
	DDRD &= ~((1 << PIND0) | (1 << PIND1));
	PORTD |= ((1 << PIND0) | (1 << PIND1));
	
	PORTC &= ~(1 << PINC0);
	
	step1 = eeprom_read_word(&step1);
	step2 = eeprom_read_word(&step2);
	step3 = eeprom_read_word(&step3);
	step4 = eeprom_read_word(&step4);
	step5 = eeprom_read_word(&step5);
	
    I2C_Init();

    while(1)
    {
		// Increase or decrease output.
		increment = 0;
		
		if(!(PIND & (1 << PIND0))){
			increment += 20;
			write_done = false;
			_delay_ms(100);
		}
		if(!(PIND & (1 << PIND1))){
			increment -= 20;
			write_done = false;
			_delay_ms(100);
		}
		
		// Set new value and set dac.
		if(!(PIND & (1 << PIND7))){
			check_limits_and_set(&step5, increment);
			dacreg = step5;
		}
		else if(!(PIND & (1 << PIND6))){
			check_limits_and_set(&step4, increment);
			dacreg = step4;
		}
		else if(!(PIND & (1 << PIND5))){
			check_limits_and_set(&step3, increment);
			dacreg = step3;
		}
		else if(!(PIND & (1 << PIND4))){
			check_limits_and_set(&step2, increment);
			dacreg = step2;
		}
		else if(!(PIND & (1 << PIND3))){
			check_limits_and_set(&step1, increment);
			dacreg = step1;
		}
		else{
			dacreg = 0;
		}
		
		if(increment == 0 && write_done == false){
			write_done = true;
			eeprom_write_word(&step1, step1);
			eeprom_write_word(&step2, step2);
			eeprom_write_word(&step3, step3);
			eeprom_write_word(&step4, step4);
			eeprom_write_word(&step5, step5);
		}	
		
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

