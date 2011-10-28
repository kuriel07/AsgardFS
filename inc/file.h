#ifndef _SYS_FILE_H
#include "defs.h"
#include "crc.h"

#define ASGARD_MF 0x20
#define ASGARD_DF 0x80
#define ASGARD_EF 0x40

#define ASGARD_OPEN 0x01
#define ASGARD_CLOSE 0x00

//Asgard 2.0
struct asgard_fs{
	uint16 sys_ptr;
	uint16 sys_dir;
};
typedef struct asgard_fs asgard_fs;

struct ef_body{
	char data[DATA_SIZE];		//28byte
	uint16 next;				//2byte
	uint16 crc;					//2byte
};
typedef struct ef_body ef_body;

struct ef_header{
	uchar type;					//1byte
	uint16 size;				//2byte
	uint16 FID;					//2byte
	uchar structure;			//1byte
	uchar status;				//1byte 	invalidate	/	validate
	uchar access_rw;			//1byte
	uchar access_inc;			//1byte
	uchar access_ri;			//1byte
	uchar record_size;			//1byte
	char padding[7];			//7byte
	uint16 ef_key;				//2byte
	uint16 parent;				//2byte
	uint16 sibling;				//2byte
	uint16 child;				//2byte
	uint16 next;				//2byte
	uint16 crc;
};

struct df_header{
	uchar type;					//1byte
	uint16 size;				//2byte
	uint16 FID;					//2byte
	char name[14];				//14byte
	uchar name_length;			//1byte
	uint16 parent;				//2byte
	uint16 sibling;				//2byte
	uint16 child;				//2byte
	uchar number_of_df;			//1byte
	uchar number_of_ef;			//1byte
	uint16 crc;
};

typedef struct ef_header ef_header;
typedef struct df_header df_header;

#define file_write_cluster(x, no) { \
			char * stream = (char *)x;	\
			uint16 i; \
			ef_body *crcfile = (ef_body *)stream;\
			crcfile->crc = crcFast(stream, CRC_SIZE); \
			ioman_seek((no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET); \
			for(i=0;i<CLUSTER_SIZE;i++) { \
				ioman_write(*(stream+i)); \
			} \
		}

#define file_read_cluster(x, cluster_no) { \
			char * str = x;	\
			uint16 i; \
			ef_body *crcfile = (ef_body *)str;\
			if(crcfile->crc == crcFast(crcfile->data, CRC_SIZE)) { \
				ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET); \
				for(i=0;i<CLUSTER_SIZE;i++) { \
					*(str++) = ioman_read(); \
				} \
			} \
		}

#define check_crc(x) { \
			ef_header *h = (ef_header *)x; \
			if(h->crc!=crcFast((uchar *)h, CRC_SIZE)) { \
				free(h); \
				return ASGARD_CRC_ERROR; \
			}}

uint16 file_select(asgard_fs * fs, uint16 fid);
uint16 file_createfile(
	asgard_fs * fs, 
	uint16 fid,  
	char *buffer, 
	uint16 size);
uint16 file_read(asgard_fs * fs, uint16 offset, uint16 size, char *buffer);
uint16 file_write(asgard_fs * fs, uint16 offset, uint16 size, char *buffer);
uint16 file_createdirectory(asgard_fs * fs, uint16 fid, uchar *dirname, uchar len_name);
uint16 file_remove(asgard_fs * fs, uint16 file_id);
void   file_dirlist();
#define _SYS_FILE_H
#endif
