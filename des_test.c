/*
    des_test.c

    DES cipher implementation test code

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

    DES cipher implementation - test file
    -------------------------------------
    
    Please read des.c and des.S files for details

*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifndef __AVR__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include "des.h"

#ifdef __AVR__
#define MSG {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x01 }
#define KEY {0xc8, 0xec, 0xaa, 0x53, 0x30, 0x8e, 0x25, 0xe0 }
#define ENC {0x49, 0xe8, 0xd0, 0x96, 0xec, 0x1b, 0xbb, 0x38 }
#endif


int
main ()
{
#ifndef __AVR__
  uint8_t i;
#endif
#ifdef MSG
  uint8_t des_key[] = KEY;
  uint8_t msg[] = MSG;
  uint8_t msg_e[] = ENC;
#else
  uint8_t des_key[8];
  uint8_t msg[8];
  uint8_t msg_e[8];
#endif
  uint8_t ret[8];


#ifndef MSG
  {
    int f;
    uint8_t buffer[8];
    f = open ("tmp/testfile.dat", 0);
    if (f >= 0)
      {
	read (f, buffer, 8);
	close (f);
      }
    for (f = 0; f < 8; f++)
      {
	msg[f] = buffer[f];
      }
    f = open ("tmp/testfile.ossl.enc", 0);
    if (f >= 0)
      {
	read (f, buffer, 8);
	close (f);
      }
    for (f = 0; f < 8; f++)
      {
	msg_e[f] = buffer[f];
      }
    f = open ("tmp/keyfile.key", 0);
    if (f >= 0)
      {
	read (f, buffer, 8);
	close (f);
      }
    for (f = 0; f < 8; f++)
      {
	des_key[f] = buffer[f];
      }


  }
#endif
  memcpy (ret, msg, 8);
  asm volatile("nop\n");
  des_run (ret, des_key, ENCRYPTION_MODE);
  asm volatile("nop\n");

#ifndef __AVR__
  for (i = 0; i < 8; i++)
    {
      printf ("%02x ", ret[i]);
    }
  printf ("\n");
#endif

  if (0 != memcmp (ret, msg_e, 8))
    {
#ifndef __AVR__
      fprintf (stderr, "ENCRYPT FAIL\n");
#endif
      exit (1);
    }
// NOP instruction only for better readability in disasssm/breakpoints in gdb .. 
  asm volatile("nop\n");
  des_run (ret, des_key, DECRYPTION_MODE);
  asm volatile("nop\n");

#ifndef __AVR__
  for (i = 0; i < 8; i++)
    {
      printf ("%02x ", ret[i]);
    }
  printf ("\n");
#endif
  if (0 != memcmp (ret, msg, 8))
    {
#ifndef __AVR__
      fprintf (stderr, "DECRYPT FAIL\n");
#endif
      exit (1);
    }


  return 0;
}
