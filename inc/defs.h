#ifndef _DEFS_DEFINED
#include "config.h"
//compiler warning disable
#pragma warning(disable : 4996)
#pragma warning(disable : 4761)
#pragma warning(disable : 4047)
#pragma warning(disable : 4715)

#define FS_TYPE						'A'
#define FS_UNFORMATTED 				3
#define FS_DESCRIPTION				"Asgard File System - ©2010 Agus Purwanto"	//size=40
#define FS_DESCRIPTION_SIZE			40
#define FILE_NOT_FOUND				0xff03
#define INSUFFICIENT_MEMORY			0xff02
#define FILE_CANNOT_BE_CREATED		0xff04
#define FILE_CRC_ERROR				0xff01
#define FS_NO_AVAILABLE_SPACE		0xffff
#define SUCCESS 					1
#define FAIL						0x80
#define CLUSTER_SIZE 				32
#define CRC16_SIZE					2
#define NEXT_POINTER_SIZE			2
#define DATA_SIZE					(CLUSTER_SIZE - (CRC16_SIZE + NEXT_POINTER_SIZE))
#define CRC_SIZE					(DATA_SIZE + CRC16_SIZE)
#define QUEUE_SIZE 					8	//for BFS dan DFS
#define TRUE						1
#define FALSE						0


#define ALLOCATION_TABLE_OFFSET 	4
#define ALLOCATION_TABLE_SIZE 		508
#define ALLOCATION_DATA_OFFSET 		(ALLOCATION_TABLE_OFFSET + ALLOCATION_TABLE_SIZE)

#define EF_NULL						0xFF
#define EF_TRANSPARENT 				0x00
#define EF_LINIER					0x01
#define EF_CYCLIC					0x03
#define EF_EXECUTE					0x10
#define EF_WORKING					0x20
#define EF_INTERNAL					0x40

#define T_RFU						0x00
#define T_MF						0x01
#define T_DF						0x02
#define T_EF						0x04
#define T_CHV						0x08

#define DF_RESPONSE_SIZE			0x17
#define EF_RESPONSE_SIZE			0x0F

#define ASGARD_SUCCESS				0x9000
#define ASGARD_MEMORY_PROBLEM			0x9240
#define ASGARD_NO_EF_SELECTED			0x9400
#define ASGARD_FILE_NOT_FOUND			0x9404
#define ASGARD_OUT_OF_RANGE			0x9402
#define ASGARD_FILE_INCONSISTENT		0x9408
#define ASGARD_NO_CHV_INIT			0x9802
#define ASGARD_ACCESS_DENIED			0x9804
#define ASGARD_INCONTRADICTION_W_CHV	0x9808
#define ASGARD_INVALID_STATUS			0x9810
#define ASGARD_CHV_LAST_ATTEMPT		0x9840
#define ASGARD_MAX_VALUE_REACHED		0x9850
#define ASGARD_SUCCESS_RESPONSE		0x9F00
#define ASGARD_COMMAND_INVALID		0x6981
#define ASGARD_SECURITY_STATE_ERROR	0x6982
#define ASGARD_AUTH_BLOCKED			0x6983
#define ASGARD_INCONSISTENT_PARAMETER 0x6A87
#define ASGARD_DATA_NOT_FOUND			0x6a88
#define ASGARD_WRONG_PARAMETER		0x6B00
#define ASGARD_INSTRUCTION_INVALID	0x6D00
#define ASGARD_CLASS_INVALID			0x6E00
#define ASGARD_FATAL_ERROR			0x6F00			//no further description
#define ASGARD_CRC_ERROR				0x6581			//memory failure, eeprom write error
#define ASGARD_WRONG_LENGTH			0x6700	
#define ASGARD_FUNCTION_INVALID		0x6A81
#define ASGARD_NO_AVAILABLE_SPACE		ASGARD_MAX_VALUE_REACHED

#define STAT_INVALID					0x00
#define STAT_VALID					0x01
#define FID_MF						0x0000	

//#define NULL						0

//type definition
typedef long int32;
typedef long eint32;
typedef unsigned long uint32;
typedef unsigned long euint32;
typedef unsigned long ulong;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned short euint16;
typedef unsigned short uint;
typedef unsigned char uchar;
typedef char eint8;
typedef unsigned char euint8;
typedef char int8;
typedef unsigned char uint8;
typedef const unsigned char uint8_t;	//taruh di code program
typedef const unsigned int uint16_t;	//taruh di code program
typedef const unsigned long uint32_t;	//taruh di code program
typedef unsigned long u_ptr;			//typedef cast untuk pointer, diganti menyesuaikan target hardware
//@dir unsigned long * @dir zpage_ptr_to_zero_page;
#define _DEFS_DEFINED
#endif
