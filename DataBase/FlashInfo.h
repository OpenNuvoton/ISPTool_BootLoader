#ifndef INC__FLASH_INFO_H__
#define INC__FLASH_INFO_H__
#pragma once

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "ChipDefs.h"


/* Get Flash Info by DID */
typedef struct
{
	unsigned int uProgramMemorySize;
	unsigned int uLDSize;
	unsigned int uRAMSize;
	unsigned int uDID;

	unsigned int uFlashType;
} FLASH_INFO_BY_DID_T;


void GetFlashSize_NuMicro(unsigned int uConfig0,
						  unsigned int uConfig1,
						  unsigned int uProgramMemorySize,
						  unsigned int uFlashType,
						  unsigned int *puNVM_Addr,
						  unsigned int *puAPROM_Size,
						  unsigned int *puNVM_Size);

#endif

