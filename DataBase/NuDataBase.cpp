#include "stdafx.h"
#include "NuDataBase.h"
#include "..\resource.h"

#include "FlashInfo.h"


extern struct CPartNumID g_PartNumIDs[];

CChipConfigInfo gsChipCfgInfo;

/**
  * @brief Check if any given uID is available in g_PartNumIDs (ref: PartNumID.cpp).
  * @param[in] uID The PDID read from the target device.
  * @retval false The given uID can not be found in the database, it may belong to Audio series or not suport yet.
  * @retval true  The given uID is found.
  * @details Search the PDID through the part number list of NuMicro family (M0, M23, M4, 80511T and Motor series).
  */
bool GetChipStaticInfo(unsigned int uPDID)
{
    if (gsChipCfgInfo.uPDID == uPDID)
	{
        return true;
    }

	char pName[] = "Unknown Chip";

	memset(&gsChipCfgInfo, 0, sizeof(gsChipCfgInfo));

	memcpy(gsChipCfgInfo.szPartNumber, pName, sizeof(pName));

    unsigned int i = 0;

	// Step1: get part no. from PartNumID
	for (i = 0; i < GetNumOfPart(); ++i)
	{
        if (g_PartNumIDs[i].uID == uPDID)
		{
            gsChipCfgInfo.uPDID = uPDID;
            gsChipCfgInfo.uSeriesCode = g_PartNumIDs[i].uProjectCode;
            memcpy(gsChipCfgInfo.szPartNumber, g_PartNumIDs[i].szPartNumber, 32);

			break;
        }
    }

	// Step2: get flash info. from FlashInfo
	if (i == GetNumOfPart())
		return false;

	if ((gsChipCfgInfo.uSeriesCode == PROJ_M460HD) ||  
		(gsChipCfgInfo.uSeriesCode == PROJ_M460LD))

	{
		gsChipCfgInfo.uProductLine = 2; // NuMicro

		gsChipCfgInfo.uFlashType = 0x300;

		gsChipCfgInfo.uAPROM_Addr = NUMICRO_FLASH_APROM_ADDR;
		gsChipCfgInfo.uLDROM_Addr = NUMICRO_FLASH_LDROM_ADDR + NUMICRO_SPECIAL_FLASH_OFFSET;
		gsChipCfgInfo.uCONFIG_Addr = NUMICRO_FLASH_CONFIG_ADDR + NUMICRO_SPECIAL_FLASH_OFFSET;
	}

	return true;
}

bool GetChipDynamicInfo(unsigned int uPDID, unsigned int uAPROM_TotalSize, unsigned int uLDROM_Size, unsigned int uCONFIG[])
{
    if (gsChipCfgInfo.uPDID == uPDID)
	{
        if ((uCONFIG != NULL) && (gsChipCfgInfo.uCONFIG[0] == uCONFIG[0]) && (gsChipCfgInfo.uCONFIG[1] == uCONFIG[1]))
		{
            return true;
        }
    }

    if (GetChipStaticInfo(uPDID))
	{
        unsigned int uProductLine = gsChipCfgInfo.uProductLine;

		if (uProductLine == 2)
		{
			gsChipCfgInfo.uProgramMemorySize = uAPROM_TotalSize;

			if (uCONFIG != NULL)
			{
				gsChipCfgInfo.uCONFIG[0]	= uCONFIG[0];
				gsChipCfgInfo.uCONFIG[1]	= uCONFIG[1];
				gsChipCfgInfo.uCONFIG[2]	= uCONFIG[2];
				gsChipCfgInfo.uCONFIG[3]	= uCONFIG[3];
			}
			else
			{
				gsChipCfgInfo.uCONFIG[0]	= 0xFFFFFFFF;
				gsChipCfgInfo.uCONFIG[1]	= 0xFFFFFFFF;
				gsChipCfgInfo.uCONFIG[2]	= 0xFFFFFFFF;
				gsChipCfgInfo.uCONFIG[3]	= 0xFFFFFFFF;
			}

			unsigned int uAPROM_Size;
			unsigned int uNVM_Addr, uNVM_Size;

			//GetFlashSize_NuMicro(uCONFIG[0], uCONFIG[1],
			//					 gsChipCfgInfo.uProgramMemorySize, gsChipCfgInfo.uFlashType,
			//					 &uNVM_Addr, &uAPROM_Size, &uNVM_Size);
			GetFlashSize_NuMicro(gsChipCfgInfo.uCONFIG[0], gsChipCfgInfo.uCONFIG[1],
								 gsChipCfgInfo.uProgramMemorySize, gsChipCfgInfo.uFlashType,
								 &uNVM_Addr, &uAPROM_Size, &uNVM_Size);

			gsChipCfgInfo.uAPROM_Size	= uAPROM_Size;
			gsChipCfgInfo.uNVM_Addr		= uNVM_Addr;
			gsChipCfgInfo.uNVM_Size		= uNVM_Size;
			gsChipCfgInfo.uLDROM_Size	= uLDROM_Size;

			//gsChipCfgInfo.uCONFIG[0]	= uCONFIG[0];
		    //gsChipCfgInfo.uCONFIG[1]	= uCONFIG[1];
			//gsChipCfgInfo.uCONFIG[2]	= uCONFIG[2];
		    //gsChipCfgInfo.uCONFIG[3]	= uCONFIG[3];

			return true;
		}
    }	

	return false;
}

