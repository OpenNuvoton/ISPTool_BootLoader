#include "stdafx.h"
#include "FlashInfo.h"
#include "Nu_Link.h"


/* Calculate APROM and NVM size of NuMicro */
void GetFlashSize_NuMicro(unsigned int uConfig0,
						  unsigned int uConfig1,
						  unsigned int uProgramMemorySize,
						  unsigned int uFlashType,
						  unsigned int *puNVM_Addr,
						  unsigned int *puAPROM_Size,
						  unsigned int *puNVM_Size)
{
	bool bShare;
	unsigned int uType = uFlashType & 0xFF;
	unsigned int uAPROM_Size, uNVM_Size;

	bShare = (uType == 0)? true : false;

	if ((uType == 2) && ((uConfig0 & NUC1XX_FLASH_CONFIG_DFVSEN) == 0))
	{
		uProgramMemorySize += 0x1000;
		bShare = true;
	}

	if (bShare)
	{
		if ((uConfig0 & NUC1XX_FLASH_CONFIG_DFEN) == 0)
		{
			unsigned int uNVM_Addr, uPage_Size;

			uPage_Size = ((uFlashType & 0x0000FF00) >>  8) + 9;

			uNVM_Addr = uConfig1 & 0x00FFFFFF;	// Needed?
			uNVM_Addr &= ~((1 << uPage_Size) - 1);
			uAPROM_Size = (uProgramMemorySize > uNVM_Addr)? uNVM_Addr : uProgramMemorySize;
			uNVM_Size = uProgramMemorySize - uAPROM_Size;
		}
		else
		{
			uAPROM_Size = uProgramMemorySize;
			uNVM_Size = 0;
		}

		*puNVM_Addr = uAPROM_Size;
	}
	else
	{
		uAPROM_Size = uProgramMemorySize;
		uNVM_Size = 0x1000;
		*puNVM_Addr = 0x1F000;
	}

	*puAPROM_Size = uAPROM_Size;
	*puNVM_Size = uNVM_Size;
}

