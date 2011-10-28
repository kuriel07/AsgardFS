#include "defs.h"
#include "config.h"
#include "crc.h"
#include "file.h"
#include <stdio.h>

//global access buffer
#ifdef _YGGDRASIL_MICRO_KERNEL
extern apdu_command *iobuf;
#endif

void ioman_init();
char ioman_read_pos(uint16 pos);
char ioman_read();
void ioman_write_pos(uint16 pos, char data);
void ioman_write(char data);
void ioman_seek(uint16 pos);
void ioman_close();

#ifdef _YGGDRASIL_MICRO_KERNEL		//prosedur/fungsi berikut didalam sistem operasi
void ioman_transmit(uchar size, char * buffer, uint16 status);
uchar ioman_receive(uchar * buffer);
#endif


