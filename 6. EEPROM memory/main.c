#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>


uint16_t data = 0;
volatile unsigned long millis = 0;
char str[50];
int pwm = 0;

ISR(ADC_vect)
{
	data = ADCL >> 6 | ADCH << 2;
	pwm = data / 4;
	OCR2 = pwm;
	
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

ISR(TIMER0_OVF_vect) {
	// Increment millis variable every 1ms
	millis++;
}

void USART_Init()
{
	// Baudrate
	unsigned int ubrr = F_CPU / 16 / 9600 - 1;
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	// Enable receiver and transmitter
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	// Set frame format
	UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

unsigned char USART_Receive(void)
{
	while ( !(UCSR0A | (1<<RXC0)) );
	return UDRE0;
}

void write(char* text)
{
	USART_Transmit('\r');
	USART_Transmit('\n');
	for(int i = 0; text[i] != '\0'; i++) USART_Transmit(text[i]);
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	// Wait for completion of previous write
	while(EECR & (1<<EEWE));
	
	// Set up address and data
	EEAR = uiAddress;
	EEDR = ucData;
	
	// Write one to EEMWE
	EECR |= (1<<EEMWE);
	// Start eeprom write
	EECR |= (1<<EEWE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	// Wait for completion of previous write
	while(EECR & (1<<EEWE));
	
	// Set up address
	EEAR = uiAddress;
	
	// Start eeprom read
	EECR |= (1<<EERE);
	
	return EEDR;
}

void EEPROM_write_time(unsigned long time)
{
	unsigned char bytes[4];

	// Extract individual bytes
	bytes[0] = (time >> 24) & 0xFF;
	bytes[1] = (time >> 16) & 0xFF;
	bytes[2] = (time >> 8) & 0xFF;
	bytes[3] = time & 0xFF;
	
	for(int i = 0; i < 4; i++)	EEPROM_write(i, bytes[i]);
	
	write("Time written to EEPROM");
}

unsigned long EEPROM_read_time()
{
	unsigned long time = 0;
	unsigned char bytes[4];
	
	for(int i = 0; i < 4; i++)	bytes[i] = EEPROM_read(i);
	
	// Combine bytes into unsigned long
	time = (unsigned long)bytes[0] << 24 |
		   (unsigned long)bytes[1] << 16 |
		   (unsigned long)bytes[2] << 8 |
		   (unsigned long)bytes[3];
	
	write("Time read from EEPROM");
	return time;
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
	PORTA |= (1 << PA0);
	
	// ADC
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADFR) | (1 << ADIE) | (1 << ADPS2);
	
	// Timer0
	TCCR0 = (1 << CS02);
	TIMSK = (1 << TOIE0);
	// Timer2
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20) | (0 << CS22) | (1 << CS21) | (0 << CS20);
	
	// USART
	USART_Init();
	
	// Check EEPROM for saved time value
	if (EEPROM_read(0) != 0xff)	millis = EEPROM_read_time();
	
	sei();
	
	while(1)
	{
		// Change motor direction on button press
		if (PINB == 0b00000010) PORTC = 0b00001001;
		if (PINB == 0b00000001) PORTC = 0b00000110;
		// Write time to EEPROM on button press
		if ((PINA & (1 << PA0)) == 0) EEPROM_write_time(millis);
		
		int btn1 = (PINB == 0b00000010);
		int btn2 = (PINB == 0b00000001);

		sprintf(str, "%.8lu: Button1 = %d Button2 = %d ADC = %d PWM = %d", millis, btn1, btn2, data, pwm);
		write(str);

	}
}
