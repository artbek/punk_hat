#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define uchar unsigned char
#define F_CPU 8000000

#define BLINK_DELAY 30

#define INPUT_LEFT PC2
#define INPUT_TOPLEFT PC3
#define INPUT_TOPRIGHT PC4
#define INPUT_RIGHT PC5

static uchar row_pins[4];
static volatile uint8_t *row_ports[4];
static uchar col_pins[4];
static volatile uint8_t *col_ports[4];

void init()
{
	row_ports[0] = &PORTB; row_pins[0] = PB1; DDRB |= _BV(PB1);
	row_ports[1] = &PORTB; row_pins[1] = PB2; DDRB |= _BV(PB2);
	row_ports[2] = &PORTB; row_pins[2] = PB3; DDRB |= _BV(PB3);
	row_ports[3] = &PORTB; row_pins[3] = PB4; DDRB |= _BV(PB4);

	col_ports[0] = &PORTB; col_pins[0] = PB0; DDRB |= _BV(PB0);
	col_ports[1] = &PORTD; col_pins[1] = PD7; DDRD |= _BV(PD7);
	col_ports[2] = &PORTD; col_pins[2] = PD6; DDRD |= _BV(PD6);

	DDRC &= ~_BV(INPUT_LEFT); PORTC |= _BV(INPUT_LEFT);
	DDRC &= ~_BV(INPUT_TOPLEFT); PORTC |= _BV(INPUT_TOPLEFT);
	DDRC &= ~_BV(INPUT_TOPRIGHT); PORTC |= _BV(INPUT_TOPRIGHT);
	DDRC &= ~_BV(INPUT_RIGHT); PORTC |= _BV(INPUT_RIGHT);
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

void eyes_front(uchar row_start, uchar col)
{
	on(row_start + 2, col);
	_delay_ms(BLINK_DELAY);
	on(row_start + 1, col);
	_delay_ms(BLINK_DELAY);
	on(row_start, col);
	_delay_ms(BLINK_DELAY);
}

void eyes_smiling()
{
	uchar c;
	for (c = 0; c < 2; c++) {
		cls();
		on(2, 0); on(2, 1); on(2, 2);
		_delay_ms(1000);
		cls();
		on(3, 0); on(3, 1); on(3, 2);
		_delay_ms(60);
		cls();
		on(2, 0); on(2, 1); on(2, 2);
		_delay_ms(60);
		cls();
		on(3, 0); on(3, 1); on(3, 2);
		_delay_ms(60);
	}
}

void eyes_scared()
{
	uchar c;
	for (c = 0; c < 2; c++) {
		cls();
		on(1, 0); on(1, 1); on(1, 2);
		on(2, 0);           on(2, 2);
		on(3, 0); on(3, 1); on(3, 2);
		_delay_ms(300);
		cls();
		on(2, 1);
		_delay_ms(1000);
	}
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

uchar next_face = 0;

void show_next_face()
{
	switch (next_face) {
		case (1): // eyes left
			eyes_front(1, 0);
			_delay_ms(2000);
			break;
		case (2): // eyes left-top
			eyes_front(0, 0);
			_delay_ms(2000);
			break;
		case (3): // eyes top
			eyes_front(0, 1);
			_delay_ms(2000);
			break;
		case (4): // eyes right-top
			eyes_front(0, 2);
			_delay_ms(2000);
			break;
		case (5): // eyes right
			eyes_front(1, 2);
			_delay_ms(2000);
			break;
		case (6): // eyes scared o.o
			eyes_scared();
			break;
		case (7): // eyes smiling -.-
			eyes_smiling();
			break;
	}

	next_face = 0;
}


void process_inputs2()
{
	if (bit_is_clear(PINC, INPUT_LEFT)) {
		if (bit_is_clear(PINC, INPUT_RIGHT)) {
			next_face = 6;
		} else if (bit_is_clear(PINC, INPUT_TOPLEFT)) {
			next_face = 2;
		} else {
			next_face = 1;
		}
	} else if (bit_is_clear(PINC, INPUT_RIGHT)) {
		if (bit_is_clear(PINC, INPUT_LEFT)) {
			next_face = 6;
		} else if (bit_is_clear(PINC, INPUT_TOPRIGHT)) {
			next_face = 4;
		} else {
			next_face = 5;
		}
	} else if (bit_is_clear(PINC, INPUT_TOPLEFT)) {
		if (bit_is_clear(PINC, INPUT_TOPRIGHT)) {
			next_face = 7;
		} else {
			next_face = 3;
		}
	} else if (bit_is_clear(PINC, INPUT_TOPRIGHT)) {
		if (bit_is_clear(PINC, INPUT_TOPLEFT)) {
			next_face = 7;
		} else {
			next_face = 3;
		}
	}
}

void process_inputs()
{
	if (bit_is_clear(PINC, INPUT_LEFT) && bit_is_clear(PINC, INPUT_RIGHT)) {
		next_face = 6;
	} else if (bit_is_clear(PINC, INPUT_TOPLEFT) && bit_is_clear(PINC, INPUT_TOPRIGHT)) {
		next_face = 7;
	} else if (bit_is_clear(PINC, INPUT_LEFT) && bit_is_clear(PINC, INPUT_TOPLEFT)) {
		next_face = 2;
	} else if (bit_is_clear(PINC, INPUT_RIGHT) && bit_is_clear(PINC, INPUT_TOPRIGHT)) {
		next_face = 4;
	} else if (bit_is_clear(PINC, INPUT_LEFT)) {
		next_face = 1;
	} else if (bit_is_clear(PINC, INPUT_RIGHT)) {
		next_face = 5;
	} else if (bit_is_clear(PINC, INPUT_TOPLEFT) || bit_is_clear(PINC, INPUT_TOPRIGHT)) {
		next_face = 3;
	}
}


/*** MAIN ***/

int __attribute__((noreturn)) main(void)
{
	init();

	cli();
	TCCR0B |= (1 << CS01) | (1 << CS00); // prescaler
	TIMSK0 |= (1 << TOIE0); // trigger interrupt on overflow
	TCNT0 = 0; // count from 0
	sei();

	// debug
	// DDRB |= _BV(PB6); PORTB |= _BV(PB6);

	uchar r = 0;

	while (1) {
		close_eyes();
		if (next_face) {
			show_next_face();
		} else {
			eyes_front(1, 1);
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

		process_inputs();
	}
}


ISR(TIMER0_OVF_vect)
{
	flush_display();
}

