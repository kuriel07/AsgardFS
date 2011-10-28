/* IO Man (IO Manager), provides access to the hardware such as EEPROM
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include <stdio.h>
#include "..\inc\ioman.h"
#include "..\inc\defs.h"
//#define _CRT_SECURE_NO_DEPRECATE 1
//<TO DO>
//ganti dengan macro biar pemakaian RAM lebih irit dan kecepatan akses lebih baik
//jika kompiler mendukung ketika diimplementasikan ke EEPROM, ganti dengan assembly

#ifdef _YGGDRASIL_MICRO_KERNEL
apdu_command *iobuf;		//buffer input/output
#endif

//<TO DO>
//ini untuk desktop saja, ketika implementasi ke eeprom tidak diperlukan
#ifdef _X86_ARCH
static FILE *eeprom;
#endif
static uint16 data_pointer;

/****************************************************
 *  ioman_init()                                    *
 *  initialize memory such as transfer rate,        *
 *  hardware interface, HW/SW initialization        *
 *                                                  *
 ****************************************************/
void ioman_init()
{
	//init pertama mode baca dulu
	#ifdef _X86_ARCH
	eeprom = fopen(".\\32K.eeprom", "r+b");
	if(eeprom==NULL) { fopen(".\\32K.eeprom", "w+b"); 
	eeprom = fopen(".\\32K.eeprom", "r+b");}
	#endif
}

/****************************************************
 *  ioman_read_pos(uint16 pos)                      *
 *  read data at specified address                  *
 *  pos = address, return = byte read               *
 ****************************************************/
char ioman_read_pos(uint16 pos)
{
	char data = 0;
	#ifdef _X86_ARCH	
	fseek(eeprom, pos, SEEK_SET );
	data=fgetc(eeprom);
	#endif
	return data;
}

/****************************************************
 *  ioman_read()                                    *
 *  read data (burst mode), auto-increment address  *
 *  return = byte read                              *
 ****************************************************/
char ioman_read()
{
	char data = 0;
	#ifdef _X86_ARCH
	data = fgetc(eeprom);
	#if _IOMAN_DEBUG
	printf(" * read %x at %x\n", data, eeprom->_ptr);
	#endif
	#endif
	return data;
}

/****************************************************
 *  ioman_write_pos(uint16 pos, char data)          *
 *  Write Memory at specified address               *
 *  pos = address, data = byte of data              *
 ****************************************************/
void ioman_write_pos(uint16 pos, char data)
{
	#ifdef _X86_ARCH
	fseek(eeprom, pos, SEEK_SET );	
	fputc(data, eeprom);
	#endif
}

/****************************************************
 *  ioman_seek(uint16 pos)                          *
 *  set memory address to specified value           *
 *  pos = address                                   *
 ****************************************************/
void ioman_seek(uint16 pos)
{
	#ifdef _X86_ARCH
	fseek(eeprom, pos, SEEK_SET );
	#if _IOMAN_DEBUG
	printf(" * address of EEPROM : %x\n", pos);
	#endif
	#endif
}

/******************************************************
 *  ioman_write(char data)                            *
 *  write memory (burst mode), auto-increment address *
 *  data = byte of data                               *
 ******************************************************/
void ioman_write(char data)
{
	#ifdef _X86_ARCH
	fputc(data, eeprom);
	#if _IOMAN_DEBUG
	printf(" * write %x at %x\n", data, eeprom->_ptr);
	#endif
	#endif
}

/****************************************************
 *  ioman_close()                                   *
 *  power down memory, no operation                 *
 *                                                  *
 ****************************************************/
void ioman_close()
{
	#ifdef _X86_ARCH
	fclose(eeprom);
	#endif
}

void ioman_send_atr()
{
	//char buffer[] = { IOMAN_ATR };
}

