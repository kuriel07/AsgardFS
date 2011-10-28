#include "..\inc\mem.h"
#include "..\inc\defs.h"

uint16 memcopy(char *pdest, char *psrc, uint16 offset, uint16 size)
{
	uint16 wrote = 0;
	while(size>offset){
		*((eint8*)pdest+size-1)=*((eint8*)psrc+size-1);
		size--;
		wrote++;
	}
	return wrote;
}

uint16 memadd(char *pdest, char *psrc, uint16 offset, uint16 size)
{
	uint16 wrote = 0;
	while(size>offset){
		*((eint8*)pdest+size-1) = *((eint8*)pdest+size-1) + *((eint8*)psrc+size-1);
		size--;
		wrote++;
	}
	return wrote;
}

uchar memcompare(char *pdest, char *psrc, uint16 offset, uint16 size)
{
	while(size>offset){
		if(*((eint8*)pdest+size-1)!=*((eint8*)psrc+size-1)) return FALSE;
		size--;
	}
	return TRUE;
}

uchar mempadding(char *buffer, char pad_char, uchar size)
{
	uchar len_buf = strlen(buffer);
	while(size>len_buf) {
		buffer[--size] = pad_char;
	}
	return size;
}

uint16 memcat(char *pdest, char *psrc, uint16 offset, uint16 size)
{
	uint16 start_offset = offset;
	while(size!=0) {
		pdest[offset] = psrc[(offset++)-start_offset];
		size--;
	}
	return offset;	//total size
}


