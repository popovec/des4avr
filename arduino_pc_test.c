/*
    des_test.c

    DES cipher implementation test code (for ARDUINO UNO, PC part)

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

*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <sys/ioctl.h>

struct termios ts;

//#define DO_RESET
static int
serial_init (char *name)
{
  int fd;
#ifdef DO_RESET
  int DTR_flag = TIOCM_DTR;
#endif

  fd = open (name, O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);
  if (fd < 0)
    return -1;
  if (flock (fd, LOCK_EX | LOCK_NB) != 0)
    return -2;
  if (tcflush (fd, TCIOFLUSH) != 0)
    return -3;
  if (tcgetattr (fd, &ts) != 0)
    return -4;
  ts.c_line = N_TTY;

  ts.c_cflag = 0;
  ts.c_cflag |= CS8;		// set 8 bits per character

  ts.c_cflag |= CREAD;		// enable receiver
  ts.c_cflag |= CLOCAL;		// Ignore modem control lines.
  //ts.c_cflag |= HUPCL;                // hang up after close (lower modem control lines)

  ts.c_iflag = IGNBRK;		// disable all input processing, ignore break
  ts.c_oflag = 0;		// disable all output processing
  ts.c_lflag = 0;		// disable all local modes

  ts.c_cc[VTIME] = 0;
  ts.c_cc[VMIN] = 0;		//noblock read if no character is received


  if (cfsetspeed (&ts, B38400) != 0)
    return -5;

// set serminal parameters
  if (tcsetattr (fd, TCSANOW, &ts) != 0)
    return -6;

#ifdef DO_RESET
  if (ioctl (fd, TIOCMBIC, &DTR_flag) != 0)
    return -7;
  sleep (1);
  if (ioctl (fd, TIOCMBIS, &DTR_flag) != 0)
    return -7;
#endif

  return fd;
}


static int
serial_blocking_mode (int fd)
{
  int flags;

  if (-1 == fcntl (fd, F_GETFL, &flags))
    return -6;
  if (-1 == fcntl (fd, F_SETFL, flags & (~FNDELAY)))
    return -6;

  ts.c_cc[VMIN] = 1;
  if (tcsetattr (fd, TCSANOW, &ts) != 0)
    return -6;
  return 0;
}


static int
arduino_sync (int serial_fd)
{
  uint8_t c1, c2 = 0;

  int i;
// sync arduino

  for (i = 0;; i++)
    {
      if (i > 5000)
	return -1;
      if (1 == read (serial_fd, &c1, 1))
	break;

      write (serial_fd, &c2, 1);
      // usb serial - here tcdrain does not (tcdrain is unable to get state of usb transmit buffer)
      // tcdrain(serial_fd);
      usleep (20000);
    }

  // if read does not return character, arduino is synced
  for (;;)
    if (1 != read (serial_fd, &c1, 1))
      break;
  printf ("Arduino synced %d\n", i);
  return 0;
}

int
main (int argc, char *argv[])
{
  uint8_t data[8];
  uint8_t key[8];
  uint8_t result[8];
  uint8_t mode;
  uint8_t time[4];
  uint8_t i;
  int count = 0;

  int serial_fd;
  int file_fd;

  if (argc != 3)
    {
      fprintf (stderr,
	       "Please provide filename of testvector and device name (or use arduino-test-all.sh)\n");
      exit (1);
    }

  serial_fd = serial_init (argv[2]);
  if (serial_fd < 0)
    {
      fprintf (stderr, "Unable to open serial device %s", argv[2]);
      exit (1);

    }
  file_fd = open (argv[1], O_RDONLY);
  if (file_fd < 0)
    {
      fprintf (stderr, "Unable to open test vector file %s", argv[1]);
      exit (1);
    }

  if (0 != arduino_sync (serial_fd))
    {
      fprintf (stderr, "Unable to sync with arduino\n");
      exit (2);
    }
  if (0 != serial_blocking_mode (serial_fd))
    {
      fprintf (stderr, "Unable to sitch toblocking moe\n");
      exit (3);
    }


  for (;;)
    {

      if (8 != read (file_fd, data, 8))
	break;
      if (8 != read (file_fd, key, 8))
	break;
      if (1 != read (file_fd, &mode, 1))
	break;
      if (8 != read (file_fd, result, 8))
	break;

      if (8 != write (serial_fd, data, 8))
	{
	  printf ("Unable to write data to arduino\n");
	  exit (10);
	}
      // tcdrain(serial_fd);
      if (8 != write (serial_fd, key, 8))
	{
	  printf ("Unable to write key to arduino\n");
	  exit (11);
	}
      // tcdrain(serial_fd);

      if (1 != write (serial_fd, &mode, 1))
	{
	  printf ("Unable to write mode to arduino\n");
	  exit (12);
	}
      // tcdrain(serial_fd);
      // 17 chars write, 8 read  at 38400 => 250 bits at 38400 = 6512us
      // DES 50000 clock cycles at 16000000 = 3125us
      // But there ist USB transport of serial data .. need abount 5ms
      // but no usleep is used, blocking read is better

//      usleep (15000); 
      for (i = 0; i < 8; i++)

	if (1 != read (serial_fd, &data[i], 1))
	  {
	    printf ("Unable to read result from arduino\n");
	    exit (13);
	  }

      for (i = 0; i < 4; i++)
	if (1 != read (serial_fd, &time[i], 1))
	  {
	    printf ("Unable to read timer result from arduino\n");
	    exit (13);
	  }

      if (0 != memcmp (data, result, 8))
	{
	  printf ("DES fail\n");
	  exit (100);
	}
      else
	printf ("%6d DES OK, clock cycles %d\n", ++count,
		time[0] + (time[1] << 8) + (time[2] << 16));
    }

  close (file_fd);
  close (serial_fd);

  return 0;
}
