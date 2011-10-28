#include "defs.h"
#ifndef _FS_H
struct ef_table{
	char fs_type;
	uchar cluster_size;
	uint16 partition_size;
};
typedef struct ef_table ef_table;
extern ef_table table;

int fs_init();
uint16 fs_get_next_available_space();
void fs_write_available_space(uint16 pos, uchar set);
void fs_format(uchar cluster_size, uint16 partition_size);
uint16 fs_freespace();
void fs_defrag();
void fs_dismount();

#define _FS_H
#endif
