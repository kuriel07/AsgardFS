#include "..\inc\file.h"
#include "..\inc\defs.h"
#include "..\inc\ioman.h"
#include "..\inc\fs.h"
#include "..\inc\mem.h"
#include "..\inc\crc.h"

//jadikan static biar tidak bisa diakses modul lainnya
//static uint16 fs->sys_ptr1 = 0x0000;	//diisi dengan cluster_no dari ef/df yang diselect
//static uint16 efk_ptr1 = 0x0000;	//diisi dengan cluster_no dari ef_key active file
//static uint16 fs->sys_dir1 = 0x0000;	//pointer yang menunjukkan current directory

ef_header * file_get_header(uint16 cluster_no)
{
	ef_header *newfile;
	char *str;
	uchar i;
	newfile = (ef_header *) m_alloc (sizeof(ef_header));
	#if _DMA_DEBUG
	barren_insert(newfile,sizeof(ef_header));
	#endif
	str = (char *)newfile;
	ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
	for(i=0;i<sizeof(ef_header);i++)
	{
		*(str++) = ioman_read();
	}
	return newfile;	//jangan dihapus karena akan dipakai
}

uint16 file_select(asgard_fs * fs, uint16 fid)	//system select, menselect file tanpa memperhatikan attribut
{
	ef_header *newfile;		//
	uint16 cluster_no;	//
	uint16 cluster_parent;	//
	if(fid == FID_MF)	//
	{
		fs->sys_ptr = 0x0000;
		fs->sys_dir = 0x0000;
		return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
	} else {
		cluster_no = fs->sys_ptr;
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID == fid) {
			if(newfile->type == T_EF) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif 
				return ASGARD_SUCCESS_RESPONSE | EF_RESPONSE_SIZE; 
			}
			else if(newfile->type == T_DF) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif 
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE; 
			}
		}	//memilih file yang sama
		if(newfile->type == T_EF) { 
			cluster_no = newfile->parent;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			if(newfile->FID == fid) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				fs->sys_ptr = cluster_no;
				fs->sys_dir = cluster_no;
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
			cluster_no = newfile->child;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling; 
		}		//kalau posisi sudah di EF cukup cek sibling saja
		//dilakukan jika current file bertipe DF
		//jika bukan EF
		cluster_no = newfile->child;
		cluster_parent = newfile->parent;
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		newfile = (ef_header *)file_get_header(cluster_parent);	//ambil anak pertama
		if(newfile->FID == fid) {
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			fs->sys_ptr = cluster_no;
			fs->sys_dir = cluster_no;
			return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
		}
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		//cek child
		cek_child:
		if(cluster_no == 0) {
			cluster_no = fs->sys_ptr;
			newfile = (ef_header *)file_get_header(cluster_no);
			cluster_no = newfile->parent;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			cluster_no = newfile->child;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling_df;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID == fid) {
			fs->sys_ptr = cluster_no;
			if(newfile->type == T_EF)
			{
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			}
			else
			{
				fs->sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_child;
		}

		cek_sibling_df:	//hanya mencari sibling yang bertipe DF
		if(cluster_no == 0) {
			return ASGARD_FILE_NOT_FOUND;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			if(fs->sys_ptr == 0) {	//jika DF maka arahkan pointer
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				fs->sys_ptr = cluster_no;
				fs->sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}

			if(newfile->type == T_DF) {	//jika DF maka arahkan pointer
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				fs->sys_ptr = cluster_no;
				fs->sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			} else {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_FILE_NOT_FOUND;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling_df;
		}

		//dilakukan jika current file bertipe EF
		//cek sibling, mencari semua sibling dengan tipe apa saja
		cek_sibling:
		if(cluster_no == 0) {
			return ASGARD_FILE_NOT_FOUND;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			fs->sys_ptr = cluster_no;
			if(newfile->type == T_EF) {
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			} else {
				fs->sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return ASGARD_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling;
		}
	}
}

uint16 file_createfile(
	asgard_fs * fs,
	uint16 fid,  
	char *buffer, 
	uint16 size)
{
	ef_header *header = (ef_header *)file_get_header(fs->sys_dir);		//buat file pada current directory
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uint16 parent_no = fs->sys_dir;
	uint16 sibling_no;
	uchar i;
	char *str;
	check_crc(header);
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//m_free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_ef += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	#ifdef _USE_CRC
	dfhead->crc = crcFast(newdata->data, CRC_SIZE);
	#endif
	ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	if(cluster_no==0) {
		//create file here(buat child baru)
		dfhead->child = fs_get_next_available_space();
		if(dfhead->child == FS_NO_AVAILABLE_SPACE) {
			return ASGARD_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		fs->sys_ptr = dfhead->child;		//select header baru
		#ifdef _USE_CRC
		dfhead->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		m_free(dfhead);
		#if _DMA_DEBUG
		barren_eject(dfhead);
		#endif
		//tulis header file yang baru dibuat
		header = (ef_header *) m_alloc (sizeof(ef_header));
		#if _DMA_DEBUG
		barren_insert(header,sizeof(ef_header));
		#endif
		newdata = (ef_body *)header;
		header->type = T_EF;
		header->size = size;
		header->FID = fid;
		header->structure = EF_INTERNAL;
		header->status = STAT_VALID;	//status harus selalu valid untuk system file
		header->access_rw = 0x00;		//always read and write
		header->access_inc = 0x00;
		header->access_ri = 0xFF;		//tidak bisa direhab dan invalidate
		header->record_size = 0x00;
		header->parent = parent_no;
		header->sibling = 0x0000;
		header->child = 0x0000;
		header->next = 0;
		#ifdef _USE_CRC
		header->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		str = (char *)header;
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		//tulis data dengan bantuan file_write
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return file_write(fs, 0, size, buffer);
	} else {
		//scan till last child
		get_last_child:
		if(cluster_no==0) {
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				if(header->sibling == FS_NO_AVAILABLE_SPACE) {
					return ASGARD_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				fs->sys_ptr = header->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				#ifdef _USE_CRC
				header->crc = crcFast(newdata->data, CRC_SIZE);
				#endif
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				}
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				if(dfhead->sibling == FS_NO_AVAILABLE_SPACE) {
					return ASGARD_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				fs->sys_ptr = dfhead->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				#ifdef _USE_CRC
				dfhead->crc = crcFast(newdata->data, CRC_SIZE);
				#endif
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}
				m_free(dfhead);
				#if _DMA_DEBUG
				barren_eject(dfhead);
				#endif
			}
			//tulis header file yang baru dibuat
			header = (ef_header *) m_alloc (sizeof(ef_header));
			#if _DMA_DEBUG
			barren_insert(header,sizeof(ef_header));
			#endif
			newdata = (ef_body *)header;
			header->type = T_EF;
			header->size = size;
			header->FID = fid;
			header->structure = EF_INTERNAL;
			header->status = STAT_VALID;	//status harus selalu valid untuk system file
			header->access_rw = 0x00;		//always read and write
			header->access_inc = 0x00;
			header->access_ri = 0xFF;		//tidak bisa direhab dan invalidate
			header->record_size = 0x00;
			header->parent = parent_no;
			header->sibling = 0x0000;
			header->child = 0x0000;
			header->next = 0;
			#ifdef _USE_CRC
			header->crc = crcFast(newdata->data, CRC_SIZE);
			#endif
			str = (char *)header;			//update data last sibling
			ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			}
			//tulis data dengan bantuan file_write
			m_free(header);
			#if _DMA_DEBUG
			barren_eject(header);
			#endif
			return file_write(fs, 0, size, buffer);
		}
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		sibling_no = cluster_no;
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		if(header->FID == fid) {
			m_free(header);
			#if _DMA_DEBUG
			barren_eject(header);
			#endif
			return ASGARD_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

uint16 file_read(asgard_fs * fs, uint16 offset, uint16 size, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(fs->sys_ptr);
	uint16 cluster_no = header->next;
	uint16 c_size = header->size;
	uint16 _ptr = 0;
	uchar i,j=0;
	uchar status;
	ef_body *newdata;
	char *str = (char *)header;
	check_crc(header);
	//cek current file dengan perintah sekarang
	if(header->type != T_EF){
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_NO_EF_SELECTED;
	}
	if(header->structure != EF_INTERNAL) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_FILE_INCONSISTENT;
	}
	//TIDAK MEMPEDULIKAN ACCESS CONTROL DARI FILE
	/*
	i = header->access_rw >> 4;
	#ifdef _YGGDRASIL_MICRO_KERNEL
	if(v_chv_status[i] != CHV_VERIFIED) {	//CHV belum diverifikasi, cek chv status dulu
	#endif
		status = getCHVstatus(i);	//cek status chv, access denied jika chv tidak terpenuhi
		switch(status) {
			case CHV_NEVER:
			case CHV_ENABLE:
			case CHV_BLOCK:
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
				return ASGARD_ACCESS_DENIED;
				break;
			case CHV_DISABLE:
			case CHV_UNBLOCK:
			case CHV_ALWAYS:
			default:
				break;
		}
	#ifdef _YGGDRASIL_MICRO_KERNEL
	}
	#endif
	*/

	//TIDAK MEMPERDULIKAN STATUS DARI FILE
	/*
	if(header->status == STAT_INVALID) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_INVALID_STATUS;
	}
	*/

	if(header->type != T_EF) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_NO_EF_SELECTED;
	}
	if(offset>header->size) { //wrong offset
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif 
		return ASGARD_WRONG_PARAMETER; 
	}
	if(offset+size>header->size) { //wrong offset
		m_free(header); 
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_WRONG_LENGTH; 
	} else {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
	}
	
	if(cluster_no==0) {
		return ASGARD_OUT_OF_RANGE;	//no data available
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = offset - _ptr;
		goto get_next_cluster;
	}

	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size read = %i, total size = %i\n",DATA_SIZE - c_size, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, DATA_SIZE - c_size);		//22, 8 deteksi ukuran tersisa
		#ifdef _USE_CRC
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
		#endif
			m_free(newdata);
			#if _DMA_DEBUG
			barren_eject(newdata);
			#endif
			return ASGARD_MEMORY_PROBLEM;
		#ifdef _USE_CRC
		}
		#endif
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size read = %i, total size = %i\n",size-j, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, size-j);		//22, 8 deteksi ukuran tersisa
		#ifdef _USE_CRC
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
		#endif
			m_free(newdata);
			#if _DMA_DEBUG
			barren_eject(newdata);
			#endif
			return ASGARD_MEMORY_PROBLEM;
		#ifdef _USE_CRC
		}
		#endif
		cluster_no = newdata->next;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
	}
	return ASGARD_SUCCESS;
}

uint16 file_write(asgard_fs * fs, uint16 offset, uint16 size, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(fs->sys_ptr);
	uint16 c_size = header->size;
	uint16 filesize = c_size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uchar i,j=0;
	uchar status;
	ef_body *newdata;
	char *str = (char *)header;	
	check_crc(header);
	//cek current file dengan perintah sekarang
	if(header->type != T_EF) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_NO_EF_SELECTED;
	}
	if(header->structure != EF_INTERNAL) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_FILE_INCONSISTENT;
	}

	//TIDAK MEMPEDULIKAN ACCESS CONTROL DARI FILE
	/*
	i = header->access_rw & 0x0f;
	#ifdef _YGGDRASIL_MICRO_KERNEL
	if(v_chv_status[i] != CHV_VERIFIED) {	//CHV belum diverifikasi, cek chv status dulu
	#endif
		status = getCHVstatus(i);	//cek status chv, access denied jika chv tidak terpenuhi
		switch(status) {
			case CHV_NEVER:
			case CHV_ENABLE:
			case CHV_BLOCK:
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
				return ASGARD_ACCESS_DENIED;
				break;
			case CHV_DISABLE:
			case CHV_UNBLOCK:
			case CHV_ALWAYS:
			default:
				break;
		}
	#ifdef _YGGDRASIL_MICRO_KERNEL
	}
	#endif
	*/
	
	//TIDAK MEMPEDULIKAN STATUS DARI FILE
	/*
	if(header->status == STAT_INVALID) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_INVALID_STATUS;
	}
	*/

	/*if(offset>header->size) { //wrong offset
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif 
		return ASGARD_OUT_OF_RANGE; 
	} else {*/
		//if((header->access_inc & 0xf0) != 0xf0) {	//dapat melakukan increase tidak NEVER
	if((size+offset)>header->size) {	//	SECARA OTOMATIS DAPAT MELAKUKAN INCREASE
		header->size =  size+offset;
	}
		//} else {
			//if((size+offset)>header->size) {
				//m_free(header);
				///#if _DMA_DEBUG
				///barren_eject(header);
				//#endif
				///return ASGARD_WRONG_LENGTH;
			//}
		//}		//write new header here
		//<TODO>
		//cek access control untuk penulisan
	if(header->next==0) {					
		header->next = fs_get_next_available_space();
		if(header->next == FS_NO_AVAILABLE_SPACE) {
			return ASGARD_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(header->next, TRUE);
		cluster_no = header->next;
	}
	newdata = (ef_body *)str;
	#ifdef _USE_CRC
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	#endif
	ioman_seek(ALLOCATION_DATA_OFFSET + (fs->sys_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	m_free(header);
	#if _DMA_DEBUG
	barren_eject(header);
	#endif
	//}
	
	if(cluster_no==0) {
		cluster_no = fs_get_next_available_space();	//create new cluster for data
		if(cluster_no == FS_NO_AVAILABLE_SPACE) {
				return ASGARD_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(cluster_no, TRUE);
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = offset - _ptr;
		goto get_next_cluster;
	}
	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size wrote = %i, total size = %i\n",DATA_SIZE-c_size, size); 
		str = (char *)newdata;
		j += memcopy(newdata->data+c_size, buffer+j, 0, DATA_SIZE-c_size);
		
		if(newdata->next==0){
			newdata->next = fs_get_next_available_space();	//create new cluster for data
			if(newdata->next == FS_NO_AVAILABLE_SPACE) {
				return ASGARD_NO_AVAILABLE_SPACE;
			}
			fs_write_available_space(newdata->next, TRUE);
		}
		newdata = (ef_body *)str;
		#ifdef _USE_CRC
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;		
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size wrote = %i, total size = %i\n",size-j, size);
		str = (char *)newdata;	
		j += memcopy(newdata->data+c_size, buffer+j, 0, size-j);
		if((offset + size) > filesize) {
			newdata->next = 0;
		}
		#ifdef _USE_CRC
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
	}
	return ASGARD_SUCCESS;
}

uint16 file_createdirectory(asgard_fs * fs, uint16 fid, uchar *dirname, uchar len_name)
{
	ef_header *header = (ef_header *)file_get_header(fs->sys_dir);		//buat direktori pada current directory
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uchar i;
	uint16 parent_no = fs->sys_dir;
	uint16 sibling_no;
	char *str;
	check_crc(header);
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//m_free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return ASGARD_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_df += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	#ifdef _USE_CRC
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	#endif
	ioman_seek(ALLOCATION_DATA_OFFSET + (fs->sys_dir * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	if(cluster_no==0)
	{
		dfhead->child = fs_get_next_available_space();
		if(dfhead->child == FS_NO_AVAILABLE_SPACE) {
			return ASGARD_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		//printf("%i\n", cluster_no);
		fs->sys_ptr = dfhead->child;		//select header baru
		fs->sys_dir = fs->sys_ptr;
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		newdata = (ef_body *)dfhead;
		#ifdef _USE_CRC
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		m_free(dfhead);
		#if _DMA_DEBUG
		barren_eject(dfhead);
		#endif
		dfhead = (df_header *) m_alloc (sizeof(df_header));
		#if _DMA_DEBUG
		barren_insert(dfhead, sizeof(df_header));
		#endif
		dfhead->type = T_DF;
		dfhead->size = 0x0000;
		dfhead->FID = fid;
		memclear(dfhead->name, 14);
		memcopy(dfhead->name, dirname, 0, len_name);
		dfhead->name_length = len_name;
		dfhead->number_of_df = 0;
		dfhead->number_of_ef = 0;
		dfhead->parent = parent_no;
		dfhead->sibling = 0;
		dfhead->child = 0;
		str = (char *)dfhead;			//update data last sibling
		newdata = (ef_body *)dfhead;
		#ifdef _USE_CRC
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		m_free(dfhead);
		#if _DMA_DEBUG
		barren_eject(dfhead);
		#endif
		return ASGARD_SUCCESS;
	} else {
		get_last_child:
		if(cluster_no==0)
		{
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				if(header->sibling == FS_NO_AVAILABLE_SPACE) {
					return ASGARD_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				fs->sys_ptr = header->sibling;		//select header baru
				fs->sys_dir = fs->sys_ptr;
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				#ifdef _USE_CRC
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				#endif
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				}
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				if(dfhead->sibling == FS_NO_AVAILABLE_SPACE) {
					return ASGARD_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				fs->sys_ptr = dfhead->sibling;		//select header baru
				fs->sys_dir = fs->sys_ptr;
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				#ifdef _USE_CRC
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				#endif
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}
				m_free(dfhead);
				#if _DMA_DEBUG
				barren_eject(dfhead);
				#endif
			}
			dfhead = (df_header *) m_alloc (sizeof(df_header));
			#if _DMA_DEBUG
			barren_insert(dfhead, sizeof(df_header));
			#endif
			dfhead->type = T_DF;
			dfhead->size = 0x0000;
			dfhead->FID = fid;
			memcopy(dfhead->name, dirname, 0, len_name);
			dfhead->name_length = len_name;
			dfhead->number_of_df = 0;
			dfhead->number_of_ef = 0;
			dfhead->parent = parent_no;
			dfhead->sibling = 0;
			dfhead->child = 0;
			str = (char *)dfhead;			//update data last sibling
			newdata = (ef_body *)dfhead;
			#ifdef _USE_CRC
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			#endif
			ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			}
			m_free(dfhead);
			#if _DMA_DEBUG
			barren_eject(dfhead);
			#endif
			return ASGARD_SUCCESS;
		}
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		sibling_no = cluster_no;
		//printf("sibling no : %i\n", sibling_no);
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		#ifdef _USE_CRC
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		#endif
		if(header->FID == fid) {
			m_free(header);
			#if _DMA_DEBUG
			barren_eject(header);
			#endif
			return ASGARD_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

uint16 file_remove(asgard_fs * fs, uint16 file_id)	//belum dites
{
	uint16 cluster_no, c_pos, parent_pos = fs->sys_ptr;
	uint16 temp, sibling;
	ef_header *newfile;
	df_header *df_head;
	ef_body *newdata;
	uchar i;
	uchar curfiletype;
	char *str;
	if(file_select(fs, file_id) == ASGARD_FILE_NOT_FOUND) {	//dicoba diselect dulu, kalau gagal brarti file tidak ditemukan
		return ASGARD_FILE_NOT_FOUND;
	}
	c_pos = fs->sys_ptr;
	fs->sys_ptr = parent_pos;
	newfile = file_get_header(c_pos);
	curfiletype = newfile->type;
	temp = newfile->parent;		//inisialisasi mulai dari parent
	sibling = newfile->sibling;
	//printf("sibling dari file yang dihapus = %i\n", sibling);
	m_free(newfile);
	#if _DMA_DEBUG
	barren_eject(newfile);
	#endif
	//if(c_pos==FILE_NOT_FOUND) { fs->sys_ptr = parent_pos; return ASGARD_FILE_NOT_FOUND; }
	//cluster_no = fs->sys_ptr;
	newfile =  file_get_header(fs->sys_ptr);	//dapat header parent
	df_head = (df_header *)newfile;
	str = (char *)newfile;
	if(curfiletype == T_EF) {
		df_head->number_of_ef -= 1;	//ubah nilai number of ef
	} else {
		df_head->number_of_df -= 1;	//ubah nilai number of df
	}
	//update parent header
	newdata = (ef_body *)str;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (fs->sys_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++)
	{
		ioman_write(*(str++));
	}
	//temp = fs->sys_ptr;
	//mulai melakukan searching child dari child yang paling awal
	temp = newfile->child;
	cluster_no = newfile->child;
	delete_cluster_chain:
	if(cluster_no==c_pos)	{	//adalah file yang akan dihapus
		str = (char *)newfile;
		//curfiletype = newfile->type; 
		if(cluster_no==temp) {
			//jika ternyata adalah parent
			newfile->child = sibling;
			//operasi write header
			newdata = (ef_body *)str;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			ioman_seek(ALLOCATION_DATA_OFFSET + (fs->sys_ptr * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++) {
				ioman_write(*(str++));
			}
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
		} else {	//jika bukan parent, brarti sibling
			newfile->sibling = sibling;
			//operasi write header
			newdata = (ef_body *)str;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			ioman_seek(ALLOCATION_DATA_OFFSET + (temp * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++) {
				ioman_write(*(str++));
			}
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
		}
	} else {	//bukan file yang akan dihapus
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		temp = cluster_no;
		newfile = file_get_header(cluster_no);	
		str = (char *)newfile;
		cluster_no = newfile->sibling;
		goto delete_cluster_chain;
	}
	
	cluster_no = c_pos;			///delete header yang sebenarnya
	delete_allocation_table:
	newfile = file_get_header(cluster_no);
	fs_write_available_space(cluster_no, FALSE);
	//next pointer harus diisi 0, jika tidak maka ketika header tersebut dipakai lagi maka dianggap masih ada pointer
	//ke data selanjutnya, padahal data tersebut seharusnya sudah terhapus.
	ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE) + DATA_SIZE);
	for(i=0;i<sizeof(uint16);i++) {
		ioman_write(0);	//hapus pointer ke data selanjutnya, tanpa menghapus datanya
	}

	if(newfile->next!=0 && curfiletype == T_EF) {
		//yang dihapus hanya allocation table saja, datanya masih
		cluster_no = newfile->next;
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		goto delete_allocation_table;
	}
	m_free(newfile);
	#if _DMA_DEBUG
	barren_eject(newfile);
	#endif 
	return ASGARD_SUCCESS;
}

void file_dirlist()
{
	uint16 cluster_no=0;			//pilih root, MF
	uint16 node_stack[QUEUE_SIZE];	//nodes queue, maksimum hanya menyimpan 5 child
	uint16 space_stack[QUEUE_SIZE];
	uchar stack_index=1;
	uchar space_index=1;
	uint16 j=0,k;
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
	for(k=0;k<j;k++) {
		if(k==(j-1)){
			putchar(0xc3);
		} else if((k%5)==4) {
			putchar(0xb3);
		} else {
			putchar(' ');
		}
	}
	//memcopy(buffer, curfile->id, 0, 4);
	printf("%04x", curfile->FID);
	if(curfile->child!=0) {
		putchar(0xbf);
		putchar(0x0a);
		cluster_no = curfile->child;
		j=j+5;
		m_free(curfile);
		#if _DMA_DEBUG
		barren_eject(curfile);
		#endif
		goto get_next_child;
	}
	putchar(0x0a);
	if(stack_index==1) { 
		m_free(curfile);
		#if _DMA_DEBUG
		barren_eject(curfile);
		#endif
		return; 
	}
	//dequeue
	cluster_no = node_stack[--stack_index];
	j = space_stack[--space_index];
	m_free(curfile);
	#if _DMA_DEBUG
	barren_eject(curfile);
	#endif	
	goto get_next_child;
}
