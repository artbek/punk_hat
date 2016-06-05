#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 8000000

#define BLINK_DELAY 30

#define uchar unsigned char

// rows +
// cols -

static uchar row_pins[4];
static volatile uint8_t *row_ports[4];
static uchar col_pins[4];
static volatile uint8_t *col_ports[4];

void init()
{
	row_ports[0] = &PORTB; row_pins[0] = PB1; DDRB |= _BV(PB1);
	row_ports[1] = &PORTB; row_pins[1] = PB2; DDRB |= _BV(PB2);
	row_ports[2] = &PORTD; row_pins[2] = PD5; DDRD |= _BV(PD5);
	row_ports[3] = &PORTB; row_pins[3] = PB7; DDRB |= _BV(PB7);

	col_ports[0] = &PORTB; col_pins[0] = PB0; DDRB |= _BV(PB0);
	col_ports[1] = &PORTD; col_pins[1] = PD7; DDRD |= _BV(PD7);
	col_ports[2] = &PORTD; col_pins[2] = PD6; DDRD |= _BV(PD6);
}

static uchar eye_display[4][3] = {
	{ 0, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 1, 0 },
	{ 0, 1, 0 }
};

void row_on(uchar row)
{
	uchar r;
	for (r = 0; r < 4; r++) {
		*(row_ports[r]) &= ~_BV(row_pins[r]);
	}
	*(row_ports[row]) |= _BV(row_pins[row]);
}


void flush_col(uchar row)
{
	uchar c;
	for (c = 0; c < 3; c++) {
		*(col_ports[c]) |= _BV(col_pins[c]);
	}

	for (c = 0; c < 3; c++) {
		if (eye_display[row][c]) {
			*(col_ports[c]) &= ~_BV(col_pins[c]);
		}
		_delay_us(10);
		*(col_ports[c]) |= _BV(col_pins[c]);
	}
}


void flush_display()
{
	uchar r;

	for (r = 0; r < 4; r++) {
		row_on(r);
		flush_col(r);
	}

}


void on(uchar row, uchar col) { eye_display[row][col] = 1; }

void off(uchar row, uchar col) { eye_display[row][col] = 0; }

void cls()
{
	off(0, 0); off(0, 1); off(0, 2);
	off(1, 0); off(1, 1); off(1, 2);
	off(2, 0); off(2, 1); off(2, 2);
	off(3, 0); off(3, 1); off(3, 2);
}


void close_eyes()
{
	off(0, 0); off(0, 1); off(0, 2);
	_delay_ms(BLINK_DELAY);

	off(1, 0); off(1, 1); off(1, 2);
	_delay_ms(BLINK_DELAY);

	off(2, 0); off(2, 1); off(2, 2);
	_delay_ms(BLINK_DELAY);

	off(3, 0); off(3, 1); off(3, 2);
	_delay_ms(BLINK_DELAY);
}

void eyes_front()
{
	on(3, 1);
	_delay_ms(BLINK_DELAY);
	on(2, 1);
	_delay_ms(BLINK_DELAY);
	on(1, 1);
	_delay_ms(BLINK_DELAY);
}

static const int random_times[8] = {
	1000,
	200,
	2000,
	500,
	2000,
	200,
	1000,
	500
};

/*** MAIN ***/

int __attribute__((noreturn)) main(void)
{
	init();

	cli();
	TCCR0B |= (1 << CS01) | (1 << CS00); // prescaler
	TIMSK0 |= (1 << TOIE0); // trigger interrupt on overflow
	TCNT0 = 0; // count from 0
	sei();

	DDRB |= _BV(PB6);
	DDRC |= _BV(PC5);
	PORTB |= _BV(PB6);
	PORTC |= _BV(PC5);

	uchar r = 0;

	while (1) {
		close_eyes();
		eyes_front();
		if (random_times[r] == 1000) {
			_delay_ms(1000);
		} else if (random_times[r] == 200) {
			_delay_ms(200);
		} else if (random_times[r] == 500) {
			_delay_ms(500);
		} else if (random_times[r] == 2000) {
			_delay_ms(2000);
		}
		r++;
		if (r > 8) r = 0;
	}

}

ISR(TIMER0_OVF_vect)
{
	flush_display();
}

