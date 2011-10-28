/* A bottom layer of Asgard File System, which encapsulating internal
 * access to eeprom of the file system itself
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include "..\inc\defs.h"
#include "..\inc\fs.h"
#include "..\inc\ioman.h"
#include "..\inc\file.h"
#include "..\inc\crc.h"
#include "..\inc\mem.h"
#include <stdio.h>

ef_table table;

int fs_init()
{
	char *str = (char *)&table;
	uint16 i;
	#ifdef _MIDGARD_H
	m_init_alloc();
	#endif
	crcInit();	//inisialisasi untuk crcFast
	ioman_init();
	ioman_seek(0);
	if(ioman_read_pos(0)!=FS_TYPE){ return FS_UNFORMATTED; }	//file system not formatted
	for(i=0;i<sizeof(table);i++)
	{
		*(str++) = ioman_read();
	}
	//printf("%i %i\n", table.cluster_size, table.partition_size);
	return SUCCESS;
}

uint16 fs_get_next_available_space()
{
	uint16 i;
	uchar data;
	uchar mask = 1;
	uchar offset = 1;
	uint16 p_size = ((table.partition_size - ALLOCATION_DATA_OFFSET) / (table.cluster_size * 8));
	ioman_seek(ALLOCATION_TABLE_OFFSET);
	for(i=0;i<p_size;i++)
	{
		data=ioman_read();
		if(data!=0xff)
		{
			for(mask=1,offset=0;mask!=0;mask<<=1)
			{
				if((data & mask)==0) {					
					return ((i*8)+offset);
				}
				offset++;
			}
		}
	}
	return FS_NO_AVAILABLE_SPACE;	//no available space
}

void fs_write_available_space(uint16 pos, uchar set)
{
	uint16 segment = pos / 8;
	uchar offset = pos % 8;
	uchar mask = 1<<offset;
	uchar data = ioman_read_pos(segment + ALLOCATION_TABLE_OFFSET);
	if(set) {
		data |= mask;
	} else {
		data &= ~(mask);
	}
	ioman_write_pos(segment + ALLOCATION_TABLE_OFFSET, data);
}


//operasi format juga akan menulisi chv dan meletakkan master file pada cluster 0
void fs_format(uchar cluster_size, uint16 partition_size)
{
	char *str = (char *)&table;
	df_header *dfhead;
	uint16 i;
	char author_information[] = FS_DESCRIPTION;
	table.fs_type=FS_TYPE;
	table.cluster_size=cluster_size;
	table.partition_size=partition_size;
	ioman_seek(0);
	
	for(i=0;i<sizeof(table);i++)
	{
		ioman_write(*(str++));
	}
	for(i=0;i<(ALLOCATION_TABLE_SIZE - strlen(author_information));i++)
	{
		ioman_write(0x00);
	}
	for(i=0;i<FS_DESCRIPTION_SIZE;i++)
	{
		ioman_write(author_information[i]);
	}
	for(i=0;i<(partition_size - ALLOCATION_TABLE_SIZE);i++)
	{
		ioman_write(0x00);
	}
	fs_write_available_space(fs_get_next_available_space(), TRUE);
	//create MF here
	dfhead = (df_header *) m_alloc (sizeof(df_header));
	#if _DMA_DEBUG
	barren_insert(dfhead, sizeof(df_header));
	#endif
	dfhead->type = T_MF;
	dfhead->size = 0x0000;
	dfhead->FID = FID_MF;
	memclear(dfhead->name, 14);
	memcopy(dfhead->name, "master", 0, 6);
	dfhead->name_length = 6;
	dfhead->number_of_df = 0;
	dfhead->number_of_ef = 0;
	dfhead->parent = 0;
	dfhead->sibling = 0;
	dfhead->child = 0;
	str = (char *)dfhead;			//update data last sibling
	dfhead->crc = crcFast(str, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (0 * CLUSTER_SIZE));
	for(i=0;i<sizeof(df_header);i++)
	{
		ioman_write(*(str++));
	}
	m_free(dfhead);
	#if _DMA_DEBUG
	barren_eject(dfhead);
	#endif
	//createCHV(ACC_CHV1, "1234", "12345678", 3, 10);
	//createCHV(ACC_CHV2, "5678", "12345678", 3, 10);
	//disableCHV(ACC_CHV1, "1234\xff\xff\xff\xff");
	//disableCHV(ACC_CHV2, "5678\xff\xff\xff\xff");
}

uint16 fs_freespace()
{
	//ambil space bebas memanfaatkan allocation table
	//nilai yang direturn adalah jumlah cluster yang bebas
	uint16 i;
	uchar data;
	uint16 frspace = 0;
	uchar mask = 1;
	uchar offset = 1;
	ioman_seek(ALLOCATION_TABLE_OFFSET);
	for(i=0;i<((table.partition_size - ALLOCATION_DATA_OFFSET) / (table.cluster_size * 8) );i++)
	{
		data=ioman_read();
		if(data == 0x00)
		{
			frspace += 8;
		}
		else if(data != 0xff)
		{
			for(mask=1;mask!=0;mask<<=1)
			{
				if((data & mask)==0)
				{
					frspace++;
				}
				//mask<<=1;
			}
		}
		//1 byte bisa mengalamati 8 cluster, 1 cluster = 32 byte, jadi 1 byte bisa mengalamati (8 x 32) byte
		///if(i >= ((table.partition_size - ALLOCATION_DATA_OFFSET) / (table.cluster_size * 8) ))
		//{
		//	return frspace;
		//}
	}
	return frspace;
}

//reserve for future use, lebih efektif untuk media mekanik
void fs_defrag()
{
	//gunakan algoritma dfs pada file_dirlist untuk mencari file yang terfragmentasi
	//proses ini bisa lama
	uint16 cluster_no=0;			//pilih root, MF
	uint16 node_stack[QUEUE_SIZE];	//nodes queue, maksimum hanya menyimpan 5 child
	uint16 space_stack[QUEUE_SIZE];
	uchar stack_index=1;
	uchar space_index=1;
	char i,j=0,k;
	ef_header *curfile;				//inisialisasi
	char buffer[5] = { 0,0,0,0,0 };
	get_next_child:
	curfile = file_get_header(cluster_no);
	if(curfile->sibling!=0) {
		//enqueue child
		space_stack[space_index++] = j;
		node_stack[stack_index++] = curfile->sibling;
	}
	//printf
	/*for(k=0;k<j;k++) {
		if(k==(j-1)){
			putchar(0xc3);
		} else if((k%5)==4) {
			putchar(0xb3);
		} else {
			putchar(' ');
		}
	}
	//memcopy(buffer, curfile->id, 0, 4);
	printf("%x", curfile->FID);*/
	if(curfile->child!=0) {
		putchar(0xbf);
		putchar(0x0a);
		cluster_no = curfile->child;
		j=j+5;
		goto get_next_child;
	}
	putchar(0x0a);
	if(stack_index==1) { return; }
	//dequeue
	cluster_no = node_stack[--stack_index];
	j = space_stack[--space_index];	
	goto get_next_child;
}

void fs_dismount()
{
	ioman_close();
}
