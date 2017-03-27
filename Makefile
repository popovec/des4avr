
.PHONY:	help PC_TEST AVR_TEST clean all arduino_des_asm_program arduino_des_c_program which 

HAVE +=         -DHAVE_DES_IP
HAVE +=         -DHAVE_DES_IIP
CPU = atmega328

ARDUINO=`ls /dev/serial/by-id/usb-Arduino_Srl_Arduino_Uno_* 2>/dev/null|head -n1`

all:	help which arduino_pc_test arduino_des_asm_test.elf des_test

help:
	@echo "================================================================="
	@echo "to test PC implementation run:"
	@echo " make PC_TEST"
	@echo ""
	@echo "to test AVR implementation please upload test code to arduino uno"
	@echo "(testet only on original arduino, do not use china arduino uno clone"
	@echo "or rewrite ARDUINO variable to set ttyUSB for your arduino clone)"
	@echo ""
	@echo " make arduino_des_asm_program"
	@echo "or "
	@echo " make arduino_des_c_program"
	@echo "and then run:"
	@echo " make AVR_TEST"
	@echo ""
	@echo "to clear working files:"
	@echo " make clean"
	@echo "================================================================="

clean:
	@rm -f *.o
	@rm -f *.elf
	@rm -f *~
	@rm -f arduino_pc_test
	@rm -f des_test
	@rm -rf tmp/

which:
	@-which hexdump dd avrdude avr-gcc cc openssl >/dev/null || (echo "!!! Some binaries missing on your system !!!";echo "=================================================================")
	
# =======================================================================================================================================
# test suite for DES on ARDUINO-UNO
# PC software
arduino_pc_test:	arduino_pc_test.c
			gcc -Wall -o arduino_pc_test arduino_pc_test.c

# DES in ASM
arduino_des_asm_test.elf:	arduino_des_test.c des.h des_avr_asm.o
				avr-gcc -mmcu=$(CPU) $(HAVE) -g -Wall -Os arduino_des_test.c des_avr_asm.o -o arduino_des_asm_test.elf
				avr-size --format=SysV arduino_des_asm_test.elf
				avr-size arduino_des_asm_test.elf
				                        
# DES in C
arduino_des_c_test.elf:		arduino_des_test.c des.h des_avr_c.o
				avr-gcc -mmcu=$(CPU) $(HAVE) -g -Wall -Os arduino_des_test.c des_avr_c.o -o arduino_des_c_test.elf
				avr-size --format=SysV arduino_des_c_test.elf
				avr-size arduino_des_c_test.elf

# =======================================================================================================================================
# ARDUINO programming targets
# DES in ASM
arduino_des_asm_program:	arduino_des_asm_test.elf
	avrdude -P $(ARDUINO) -c arduino -p atmega328p -v -U flash:w:arduino_des_asm_test.elf

# DES in C
arduino_des_c_program:	arduino_des_c_test.elf
	avrdude -P $(ARDUINO) -c arduino -p atmega328p -v -U flash:w:arduino_des_c_test.elf
# =======================================================================================================================================
# AVR DES targets
# DES in ASM
des_avr_asm.o:	des.S
	avr-gcc -mmcu=$(CPU) $(HAVE) -Wall -Os -g -c -o des_avr_asm.o des.S
	avr-size --format=SysV des_avr_asm.o
	avr-size des_avr_asm.o


# DES in C
des_avr_c.o:	des.c des.h
	avr-gcc -mmcu=$(CPU) $(HAVE) -Wall -Os -g -c -o des_avr_c.o des.c
	avr-size --format=SysV des_avr_c.o
	avr-size des_avr_c.o
	
# =======================================================================================================================================
# DES test on PC

des_test:	des_test.c des.h des.c
	cc -g -Wall des.c des_test.c -o des_test 

# =======================================================================================================================================
# tests

PC_TEST:	des_test pc_longtest.sh
		./pc_longtest.sh

AVR_TEST:	arduino_pc_test arduino_testvector.sh
	 	./arduino_testvector.sh
	 	./arduino_pc_test tmp/test_vector_long.data $(ARDUINO)

