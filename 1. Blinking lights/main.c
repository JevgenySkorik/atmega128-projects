#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	// Variables
	uint8_t data = 0b00000001;
	int8_t direction = 1;
	int8_t running = -1;
	
	// Output
	DDRD = 0xFF;
	PORTD = 0x00;
	
	// Input
	DDRB = 0x00;
	PORTB = 0b00000011;
	
    while(1)
    {
		// Change running state on button press
		if (PINB == 0b00000010) running = 1;
		if (PINB == 0b00000001) running = -1;
		
		if (running == 1) {
			PORTD = data;
			if (direction == 1) data = data << 1;
			if (direction == -1) data = data >> 1;
			if (data == 0b00000001 || data == 0b10000000) direction *= -1;
			_delay_ms(400);
		}
		if (running == -1) PORTD = 0x00;
    }
}