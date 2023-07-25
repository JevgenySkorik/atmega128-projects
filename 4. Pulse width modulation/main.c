#define F_CPU 7800000UL

#include <avr/io.h>
#include <avr/interrupt.h>


uint16_t data = 0;

ISR(ADC_vect)
{
	data = ADCL >> 6 | ADCH << 2;
	OCR2 = data / 4;
	
	if (data >= 128)	PORTD = PORTD | 0b10000000;
	if (data >= 256)	PORTD = PORTD | 0b01000000;
	if (data >= 384)	PORTD = PORTD | 0b00100000;
	if (data >= 512)	PORTD = PORTD | 0b00010000;
	if (data >= 640)	PORTD = PORTD | 0b00001000;
	if (data >= 768)	PORTD = PORTD | 0b00000100;
	if (data >= 896)	PORTD = PORTD | 0b00000010;
	if (data >= 1020)	PORTD = PORTD | 0b00000001;
	
	if (data <= 128)	PORTD = PORTD & ~0b10000000;
	if (data <= 256)	PORTD = PORTD & ~0b01000000;
	if (data <= 384)	PORTD = PORTD & ~0b00100000;
	if (data <= 512)	PORTD = PORTD & ~0b00010000;
	if (data <= 640)	PORTD = PORTD & ~0b00001000;
	if (data <= 768)	PORTD = PORTD & ~0b00000100;
	if (data <= 896)	PORTD = PORTD & ~0b00000010;
	if (data <= 1020)	PORTD = PORTD & ~0b00000001;
}


int main(void)
{
	// Output
	DDRD = 0xFF;
	PORTD = 0x00;
	DDRB = (1 << PB7);
	DDRC = 0b00001111;
	
	// Input
	PORTB = 0b00000011;
	
	// ADC
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADFR) | (1 << ADIE) | (1 << ADPS2);
	
	// Timer
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20) | (0 << CS22) | (1 << CS21) | (0 << CS20);
	
	sei();
	
	while(1)
	{
		// Change motor direction on button press
		if (PINB == 0b00000010) PORTC = 0b00001001;
		if (PINB == 0b00000001) PORTC = 0b00000110;
	}
}