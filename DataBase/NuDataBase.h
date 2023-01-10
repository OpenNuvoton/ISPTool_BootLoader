#ifndef INC__NU_DATA_BASE_H__
#define INC__NU_DATA_BASE_H__
#pragma once

#include <stdio.h>
#include <string> // std::string
#include <vector> // std::vector

/* NuMicro Series */
#include "ChipDefs.h"
#include "PartNumID.h"


struct CChipConfigInfo
{
	// Static Info.
	unsigned int uPDID;
	unsigned int uSeriesCode;
	unsigned int uProductLine;	// 0: Unknown, 1: 8051-1T, 2: NuMicro, 3: Audio
	char szPartNumber[100];
	unsigned int uProgramMemorySize;
	unsigned int uFlashType;	// used by GetFlashSize_NuMicro(...)

	// Dynamic info. according to user configuration (CONFIG0, CONFIG1)
	unsigned int uAPROM_Addr;
	unsigned int uAPROM_Size;
	unsigned int uNVM_Addr;
	unsigned int uNVM_Size;
	unsigned int uLDROM_Addr;
	unsigned int uLDROM_Size;

	unsigned int uCONFIG_Addr;
	unsigned int uCONFIG[4];
};

extern CChipConfigInfo gsChipCfgInfo;

bool GetChipDynamicInfo(unsigned int uPDID, unsigned int uAPROM_TotalSize, unsigned int uLDROM_Size, unsigned int uCONFIG[]);

extern std::vector<CPartNumID> g_NuMicroChipSeries;

#endif
