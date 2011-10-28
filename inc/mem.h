#include "defs.h"

uint16 memcopy(char *pdest, char *psrc, uint16 offset, uint16 size);
uchar memcompare(char *pdest, char *psrc, uint16 offset, uint16 size);
uchar mempadding(char *buffer, char pad_char, uchar size);
uint16 memcat(char *pdest, char *psrc, uint16 offset, uint16 size);
uint16 memadd(char *pdest, char *psrc, uint16 offset, uint16 size);
//macro memory clear
//7 july 2010 : macro memclear diperbaiki, sebelumnya tidak mampu menghapus byte ke 0 dari semua sz-- menjadi --sz
#define memclear(buf, s) {  \
			uint16 sz = s; \
			char * bx = (char *)buf; \
			while(sz) { \
 				bx[--sz] = 0x00; \
			} \
		}

