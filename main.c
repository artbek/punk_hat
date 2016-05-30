#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 8000000
#define PWM_OUT PB0
#define PWM_IN PB1


void uart_init()
{
	UBRR0L = F_CPU / (2400 * 16L) - 1; // baud rate
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	// Set frame format: 8-bit data
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
}


unsigned char uart_putc(char data)
{
	if (! data) { return 0; }

	while ( ! (UCSR0A & (1<<UDRE0)) );
	UDR0 = data;

	return 1;
}


void uart_print(char *data)
{
	while (uart_putc(*data++));
}



unsigned int counter = 0;
char signal_arrived = 0;

/*** MAIN ***/

int __attribute__((noreturn)) main(void)
{
	uart_init();
	uart_print("Start...\n");

	//DDRD |= _BV(PD7); PORTD |= _BV(PD7); // set high

	DDRB |= _BV(PWM_OUT); // set to output
	DDRB &= ~_BV(PWM_IN); // set to input

	PORTB |= _BV(PWM_OUT); // set high
	PORTB &= ~_BV(PWM_IN); // set high

	cli();
	TCCR0B |= (1 << CS00); // prescaler
	TIMSK0 |= (1 << TOIE0); // trigger interrupt on overflow
	TCNT0 = 0; // count from 0
	sei();

	char counter_string[10];

	while (1) {
		if (! signal_arrived && bit_is_set(PORTB, PWM_OUT) && bit_is_set(PINB, PWM_IN)) {
			signal_arrived = 1;
			itoa(counter, counter_string, 10);
			uart_print(counter_string);
			uart_print("\n");
		}
		
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}

}

ISR(TIMER0_OVF_vect)
{
	counter++;
	if ((counter % 100) == 0) {
		PORTB ^= _BV(PWM_OUT); // toggle
		asm("nop");
		if (bit_is_set(PORTB, PWM_OUT)) {
			counter = 0;
			signal_arrived = 0;
		}
	}
}

