#ifndef _CONFIG_H
//#include "midgard.h"
#ifdef _MIDGARD_H
//size of heap memory for midgard (my own heap manager)
#define HEAP_SIZE 512
//allow debug for midgard
#define _MIDGARD_DEBUG 	0
#else
//default heap manager
#include <malloc.h>
#define m_alloc malloc
#define m_free free
#define m_alloc_nd malloc		//non debug version
#define m_free_nd free			//non debug version
#endif

//use CRC16 for file integrity
//disable CRC16 check for low performance machine
#define USE_CRC_CHECK		0
#if USE_CRC_CHECK
#define _USE_CRC
#endif

//operating system debug configuration
//disable dynamic memory analysis for debugging
//
#define _DMA_DEBUG 		0
#define _IOMAN_DEBUG 		0
//heap analysis for debugging
#if _DMA_DEBUG
#define	HEAP_CALC	 	0	 /* Should be made 0 to turn OFF debugging */
#endif
//use define _X86_ARCH for X86 PC
#define _X86_ARCH
#define _CONFIG_H
#endif