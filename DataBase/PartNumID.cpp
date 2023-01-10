// PartNumID.cpp: implementation of the CPartNum class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChipDefs.h"
#include "PartNumID.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct CPartNumID g_PartNumIDs[] =
{
	/* M460 */
	{"M467SJHAE", 0x01B46710, PROJ_M460HD},
	{"M467KJHAE", 0x01B46740, PROJ_M460HD},
	{"M467JJHAE", 0x01B46750, PROJ_M460HD},
	{"M467HJHAE", 0x01B46760, PROJ_M460HD},
	{"M467H3KJHAE", 0x01B46F40, PROJ_M460HD},
	{"M467SJHAN", 0x01B46F11, PROJ_M460HD},
	{"M467KJHAN", 0x01B46F41, PROJ_M460HD},
	{"M467JJHAN", 0x01B46F52, PROJ_M460HD},
	{"M467HJHAN", 0x01B46F61, PROJ_M460HD},
	{"M467S2JHAE", 0x01B46712, PROJ_M460HD},
	{"M467K2JHAE", 0x01B46742, PROJ_M460HD},
	{"M467J2JHAE", 0x01B46752, PROJ_M460HD},
	{"M467H2JHAE", 0x01B46762, PROJ_M460HD},

	{"M463KGCAE", 0x01C46340, PROJ_M460LD},
	{"M464KGCAE", 0x01C46440, PROJ_M460LD},
	{"M463VGCAE", 0x01C46330, PROJ_M460LD},
	{"M463SGCAE", 0x01C46310, PROJ_M460LD},
	{"M464SGCAE", 0x01C46410, PROJ_M460LD},
	{"M463AGCAE", 0x01C46380, PROJ_M460LD},
	{"M464AGCAE", 0x01C46480, PROJ_M460LD},
	{"M463LGCAE", 0x01C46300, PROJ_M460LD},
	{"M464LGCAE", 0x01C46400, PROJ_M460LD},
	{"M463YGCAE", 0x01C46390, PROJ_M460LD},
	{"M464YGCAE", 0x01C46490, PROJ_M460LD},
	{"M460KGCAE", 0x01C46040, PROJ_M460LD},
	{"M460SGCAE", 0x01C46010, PROJ_M460LD},
	{"M460AGCAE", 0x01C46080, PROJ_M460LD},
	{"M460LGCAE", 0x01C46000, PROJ_M460LD},
	{"M460YGCAE", 0x01C46090, PROJ_M460LD},
};


unsigned int GetNumOfPart(void)
{
	return (sizeof(g_PartNumIDs) / sizeof(CPartNumID));
}

