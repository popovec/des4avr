/*
    des.c

    DES cipher implementation (for ARDUINO UNO, microcontroller part)

    This is designed to be a part of OsEID (Open source Electronic ID)
    https:/oseid.sourceforge.io

    Copyright (C) 2017 Peter Popovec, popovec.peter@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    test suite (for arduino uno)
*/
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include "des.h"

#define F_CPU 16000000L
#define BAUD 38400
#define UBRR_DIV (F_CPU/16/BAUD-1)
void
serial_init ()
{
// set speed
  UBRR0H = UBRR_DIV >> 8;
  UBRR0L = UBRR_DIV & 0xff;
// enable receiver, transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
// set bits per char 8), stop bits (1) parity (none) 
  UCSR0C = (3 << UCSZ00);
}


int
main ()
{
  uint8_t data[8];
  uint8_t key[8];
  uint8_t mode;
  uint8_t i;
  uint8_t time[3];
  serial_init ();


  

  for (;;)
    {

      for (i = 0; i < 8; i++)
	{
	  while (!(UCSR0A & (1 << RXC0)));
	  data[i] = UDR0;
	}

      for (i = 0; i < 8; i++)
	{
	  while (!(UCSR0A & (1 << RXC0)));
	  key[i] = UDR0;
	}


      while (!(UCSR0A & (1 << RXC0)));
      mode = UDR0;
//////////////  TIMER INIT //////////////////
// configure 24 bit timer
      TCCR0A = 0;
      TCCR0B = 1;		// clock/1
      TCCR1A = 0;
      TCCR1B = 4;		// clock/256
      
      // synchronize counters
      GTCCR=0x80;
      GTCCR=0x81;
     
      TCNT0 = 0;
      TCNT1 = 0;
// start timer(s)
      GTCCR = 0;
//////////////  TIMER INIT END //////////////////
      
      des_run (data, key, mode);

//////////////  TIMER READ //////////////////
// stop timer(s) (by prescaller)
      GTCCR = 0x81;
      time[0]=TCNT0;
      time[1]=TCNT1L;
      time[2]=TCNT1H;
//////////////  TIMER READ END//////////////////

      for (i = 0; i < 8; i++)
	{
	  while (!(UCSR0A & (1 << UDRE0)));
	  UDR0 = data[i];
	}

      while (!(UCSR0A & (1 << UDRE0)));
      UDR0 = time[0];
      while (!(UCSR0A & (1 << UDRE0)));
      UDR0 = time[1];
      while (!(UCSR0A & (1 << UDRE0)));
      UDR0 = time[2];
      while (!(UCSR0A & (1 << UDRE0)));
      UDR0 = 0;
    }
  return 0;
}
