/*
 * cs145proj5.c
 *
 * Created: 3/13/2018 9:10:19 PM
 * Author : Sheng
 */ 

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

#define WDR() asm volatile("wdr"::)
#define NOP() asm volatile("nop"::)
#define RST() for(;;);

#define DDR     DDRB
#define PORT    PORTB
#define RS_PIN  0
#define RW_PIN  1
#define EN_PIN  2

#define STOP 0
#define A3 110
#define B3 123
#define C3 131
#define D3 147
#define E3 165
#define F3 175
#define G3 196
#define A4 220
#define B4 247
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A5 440
#define B5 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784
#define W 8
#define H 4
#define Q 2
#define E 1

void ini_avr(void)
{
	WDTCR = 15;
}

void wait_avr(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

void wait_avr2(unsigned short tenthmsec)
{
	TCCR0 = 2;
	while (tenthmsec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 8) * 0.0001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}


int is_pressed(int r, int c)
{
	DDRC = 0x00;
	PORTC = 0x00;
	CLR_BIT(DDRC, c+4);
	SET_BIT(PORTC, c+4);
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	wait_avr(1);
	if (!GET_BIT(PINC, c + 4)) return 1;
	return 0;
}

int get_key(void)
{
	int r;
	int result = -1;
	for (r = 0; r < 4; ++r) {
		int c;
		for (c = 0; c < 4; ++c) {
			if (is_pressed(r, c)) {
				result =  r * 4 + c + 1;
			}
		}
	}
/*	if (result >= 1 && result <= 3) {

	}
	else if (result >= 5 && result <= 7) {
		result -= 1;
	}
	else if (result >= 9 && result <= 11) {
		result -= 2;
	}
	else if (result == 14) {
		result = 0;
	}
	else {
		result = -1;
	}
*/
	return result;
}


static inline void set_data(unsigned char x) {
	PORTD = x;
	DDRD = 0xff;
}

static inline unsigned char get_data(void) {
	DDRD = 0x00;
	return PIND;
}

static inline void sleep_700ns(void) {
	NOP();
	NOP();
	NOP();
}

static unsigned char input(unsigned char rs) {
	unsigned char d;
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	SET_BIT(PORT, RW_PIN);
	get_data();
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	d = get_data();
	CLR_BIT(PORT, EN_PIN);
	return d;
}

static void output(unsigned char d, unsigned char rs) {
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	CLR_BIT(PORT, RW_PIN);
	set_data(d);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	CLR_BIT(PORT, EN_PIN);
}

static void write(unsigned char c, unsigned char rs) {
	while (input(0) & 0x80);
	output(c, rs);
}

void ini_lcd(void) {
	SET_BIT(DDR, RS_PIN);
	SET_BIT(DDR, RW_PIN);
	SET_BIT(DDR, EN_PIN);
	wait_avr(16);
	output(0x30, 0);
	wait_avr(5);
	output(0x30, 0);
	wait_avr(1);
	write(0x3c, 0);
	write(0x0c, 0);
	write(0x06, 0);
	write(0x01, 0);
}

void clr_lcd(void) {
	write(0x01, 0);
}

void pos_lcd(unsigned char r, unsigned char c) {
	unsigned char n = r * 40 + c;
	write(0x02, 0);
	while (n--) {
		write(0x14, 0);
	}
}

void put_lcd(char c) {
	write(c, 1);
}

void puts_lcd1(const char *s) {
	char c;
	while ((c = pgm_read_byte(s++)) != 0) {
		write(c, 1);
	}
}

void puts_lcd2(const char *s) {
	char c;
	while ((c = *(s++)) != 0) {
		write(c, 1);
	}
}

struct note{
	unsigned int freq;
	unsigned int duration;
};

struct note Twinkle[42] = {{C4,Q},{C4,Q},{G4,Q},{G4,Q},{A5,Q},{A5,Q},{G4,H},
{F4,Q},{F4,Q},{E4,Q},{E4,Q},{D4,Q},{D4,Q},{C4,H},
{G4,Q},{G4,Q},{F4,Q},{F4,Q},{E4,Q},{E4,Q},{D4,H},
{G4,Q},{G4,Q},{F4,Q},{F4,Q},{E4,Q},{E4,Q},{D4,H},
{C4,Q},{C4,Q},{G4,Q},{G4,Q},{A5,Q},{A5,Q},{G4,H},
{F4,Q},{F4,Q},{E4,Q},{E4,Q},{D4,Q},{D4,Q},{C4,H}};

void play_note(unsigned int freq, unsigned int duration){
	int K = freq*duration/4;
	int i;
	for (i = 0; i < K; ++i){
		SET_BIT(PORTB,3);
		wait_avr2(5000.0/freq);
		CLR_BIT(PORTB,3);
		wait_avr2(5000.0/freq);
	}
}

void play_music(struct note Song[]){
	int i;
	for (i = 0; i < 42; ++i){
		play_note(Song[i].freq,Song[i].duration);
	}
}

struct date_time {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
} dt;

int isleap(struct date_time dt) {
	if (dt.year%4 == 0) {
		if (dt.year%100 != 0) {
			return 1;
		}
		else {
			if (dt.year%400 == 0) {
				return 1;
			}
		}
	}
	return 0;
}

void showclock(struct date_time dt, char* ampm) {
	int hour = dt.hour;
	int minute = dt.minute;
	int second = dt.second;

	char str[16];

	sprintf(str,"%02d:%02d:%02d %s",hour,minute,second,ampm);
	pos_lcd(1,0);
	puts_lcd2(str);
}

void showclock2(struct date_time dt, char* ampm) {
	int hour = dt.hour;
	int minute = dt.minute;
	int second = dt.second;

	char str[16];
	if (strcmp(ampm, "AM") == 0) {
		if (hour == 12) {
			hour = 0;
		}
	}
	else {
		if (hour != 12) {
			hour += 12;
		}
	}
	sprintf(str,"%02d:%02d:%02d        ",hour,minute,second);
	pos_lcd(1,0);
	puts_lcd2(str);
}

void showcalendar(struct date_time dt, char* mode) {
	int year = dt.year;
	int month = dt.month;
	int day = dt.day;
	
	char str[16];

	sprintf(str,"%02d/%02d/%04d     %s",month,day,year,mode);
	pos_lcd(0,0);
	puts_lcd2(str);
}

void show(struct date_time dt, char* ampm, int print24, char* mode) {
	if (print24 == 0) {
		showclock(dt, ampm);
	}
	else {
		showclock2(dt, ampm);
	}
	showcalendar(dt, mode);
}

void put_i(int x) {
	pos_lcd(0,0);
	assert (x>=0 && x<100);
	put_lcd('0' + x/10);
	put_lcd('0' + x%10);
}


int main(void) {
	SET_BIT(DDRB,0);
	SET_BIT(DDRB,3);
    /* Replace with your application code */
	ini_lcd();
	struct date_time dt1;
	dt1.year = 1996;
	dt1.month = 12;
	dt1.day = 31;
	dt1.hour = 9;
	dt1.minute = 50;
	dt1.second = 30;
	char* ampm = "AM";
	int print24 = 0;
	char* mode = "R";

	char* alarm = "OFF";
	struct date_time dt2;
	char* ampm2 = "AM";
	while (1)
    {
		if (strcmp(alarm, "SET") == 0) {
			show(dt2, ampm2, print24, "C");
		}
		else {
			show(dt1, ampm, print24, mode);
		}

		if (strcmp(alarm, "ONN") == 0) {
			if (get_key() == 8) {
				alarm = "OFF";
			}
			if (dt2.hour == dt1.hour) {
				if (dt2.minute == dt1.minute) {
					if (dt2.second == dt1.second) {
						if (strcmp(ampm2, ampm) == 0) {
							play_music(Twinkle);
							alarm = "OFF";
						}
					}
				}
			}
		}

		if (get_key() == 16) {
			if (print24 == 0) {
				print24 = 1;
			}
			else {
				print24 = 0;
			}
		}
		
		if (get_key() == 4) {
			if (strcmp(mode, "R") == 0) {
				mode = "E";
			}
			else {
				mode = "R";
			}
		}

		if (get_key() == 12) {
			if (strcmp(alarm, "OFF") == 0) {
				alarm = "SET";
				dt2.year = dt1.year;
				dt2.month = dt1.month;
				dt2.day = dt1.day;
				dt2.hour = dt1.hour;
				dt2.minute = dt1.minute;
				dt2.second = dt1.second;
				ampm2 = ampm;
			}
			else if (strcmp(alarm, "SET") == 0) {
				alarm = "ONN";
			}
		}

		if (strcmp(mode, "E") == 0) {
			wait_avr(1000);

			if (get_key() == 1) {
				if (dt1.month == 1|| dt1.month == 3 || dt1.month == 5 || dt1.month == 7 || dt1.month == 8 || dt1.month == 10 || dt1.month == 12) {
					if (dt1.day > 28) {
						dt1.day = 28;
					}
				}
				dt1.month += 1;
				if (dt1.month > 12) {
					dt1.month = 1;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 5) {
				if (dt1.month == 1|| dt1.month == 3 || dt1.month == 5 || dt1.month == 7 || dt1.month == 8 || dt1.month == 10 || dt1.month == 12) {
					if (dt1.day > 28) {
						dt1.day = 28;
					}
				}
				dt1.month -= 1;
				if (dt1.month < 1) {
					dt1.month = 12;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 2) {
				dt1.day += 1;
				if (dt1.day > 30 && (dt1.month == 4 || dt1.month == 6 || dt1.month == 9 || dt1.month == 11)) {
					dt1.day = 1;
				}
				else if (dt1.day > 31 && (dt1.month == 1|| dt1.month == 3 || dt1.month == 5 || dt1.month == 7 || dt1.month == 8 || dt1.month == 10 || dt1.month == 12)) {
					dt1.day = 1;
				}
				else if (dt1.day > 29 && dt1.month == 2 && (isleap(dt1) == 1)) {
					dt1.day = 1;
				}
				else if (dt1.day > 28 && dt1.month == 2 && (isleap(dt1) == 0)) {
					dt1.day = 1;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 6) {
				dt1.day -= 1;
				if (dt1.day < 1) {
					if (dt1.month == 4 || dt1.month == 6 || dt1.month == 9 || dt1.month == 11) {
						dt1.day = 30;
					}
					else if (dt1.month == 1|| dt1.month == 3 || dt1.month == 5 || dt1.month == 7 || dt1.month == 8 || dt1.month == 10 || dt1.month == 12) {
						dt1.day = 31;
					}
					else if (dt1.month == 2 && (isleap(dt1) == 1)) {
						dt1.day = 29;
					}
					else if (dt1.month == 2 && (isleap(dt1) == 0)) {
						dt1.day = 28;
					}
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 3) {
				if (dt1.month == 2) {
					if (dt1.day == 29 && (isleap(dt1) == 1)) {
						dt1.day = 28;
					}
				}
				dt1.year += 1;
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 7) { 
				if (dt1.month == 2) {
					if (dt1.day == 29 && (isleap(dt1) == 1)) {
						dt1.day = 28;
					}
				}
				dt1.year -= 1;
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 9) {
				dt1.hour += 1;
				if (dt1.hour == 12) {
					if (strcmp(ampm, "AM") == 0) {
						ampm = "PM";
					}
					else {
						ampm = "AM";
					}
				}
				if (dt1.hour > 12) {
					dt1.hour = 1;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 13) {
				if (dt1.hour == 12) {
					if (strcmp(ampm, "AM") == 0) {
						ampm = "PM";
					}
					else {
						ampm = "AM";
					}
				}				
				dt1.hour -= 1;
				if (dt1.hour < 1) {
					dt1.hour = 12;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 10) {
				dt1.minute += 1;
				if (dt1.minute > 59) {
					dt1.minute = 0;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 14) {
				dt1.minute -= 1;
				if (dt1.minute < 0) {
					dt1.minute = 59;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 11) {
				dt1.second += 1;
				if (dt1.second > 59) {
					dt1.second = 59;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}

			if (get_key() == 15) {
				dt1.second -= 1;
				if (dt1.second < 0) {
					dt1.second = 59;
				}
				show(dt1, ampm, print24, mode);
				continue;
			}
		}
		if (strcmp(mode, "R") == 0) {
			if (strcmp(alarm, "SET") == 0) {
				wait_avr(500);
			}
			else {
				wait_avr(1000);
			}
			dt1.second += 1;
			if (dt1.second > 59) {
				dt1.minute += 1;
				dt1.second = 0;
				if (dt1.minute > 59) {
					if (dt1.hour == 11) {
						if (strcmp(ampm, "AM") == 0) {
							ampm = "PM";
						}
						else {
							ampm = "AM";
							dt1.day += 1;
							if (dt1.day > 30 && (dt1.month == 4 || dt1.month == 6 || dt1.month == 9 || dt1.month == 11)) {
								dt1.month += 1;
								dt1.day = 1;
							}
							else if (dt1.day > 31 && (dt1.month == 1|| dt1.month == 3 || dt1.month == 5 || dt1.month == 7 || dt1.month == 8 || dt1.month == 10 || dt1.month == 12)) {
								dt1.month += 1;
								dt1.day = 1;
								if (dt1.month > 12) {
									dt1.year += 1;
									dt1.month = 1;
								}
							}
							else if (dt1.day > 29 && dt1.month == 2 && (isleap(dt1) == 1)) {
								dt1.month += 1;
								dt1.day = 1;
							}
							else if (dt1.day > 28 && dt1.month == 2 && (isleap(dt1) == 0)) {
								dt1.month += 1;
								dt1.day = 1;
							}
						}
					}
					dt1.hour += 1;
					dt1.minute = 0;
					if (dt1.hour > 12) {
						dt1.hour = 1;
					}
				}
			} // end if 59
			if (strcmp(alarm, "SET") == 0) {
				wait_avr(500);
				if (get_key() == 9) {
					dt2.hour += 1;
					if (dt2.hour == 12) {
						if (strcmp(ampm2, "AM") == 0) {
							ampm2 = "PM";
						}
						else {
							ampm2 = "AM";
						}
					}
					if (dt2.hour > 12) {
						dt2.hour = 1;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}

				if (get_key() == 13) {
					if (dt2.hour == 12) {
						if (strcmp(ampm2, "AM") == 0) {
							ampm2 = "PM";
						}
						else {
							ampm2 = "AM";
						}
					}
					dt2.hour -= 1;
					if (dt2.hour < 1) {
						dt2.hour = 12;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}

				if (get_key() == 10) {
					dt2.minute += 1;
					if (dt2.minute > 59) {
						dt2.minute = 0;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}

				if (get_key() == 14) {
					dt2.minute -= 1;
					if (dt2.minute < 0) {
						dt2.minute = 59;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}

				if (get_key() == 11) {
					dt2.second += 1;
					if (dt2.second > 59) {
						dt2.second = 59;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}

				if (get_key() == 15) {
					dt2.second -= 1;
					if (dt2.second < 0) {
						dt2.second = 59;
					}
					show(dt2, ampm2, print24, "C");
					continue;
				}
			} // end alarm set mode */
		} // end run
	} // end while
}


