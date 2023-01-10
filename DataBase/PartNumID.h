// PartNumID.h: interface for the CPartNum class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTNUMID_H__224714FF_DE1A_41FC_81B6_4B998BDC9FE6__INCLUDED_)
#define AFX_PARTNUMID_H__224714FF_DE1A_41FC_81B6_4B998BDC9FE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <sstream>
#include "ChipDefs.h"


#define PROJ_M460HD      0x3D
#define PROJ_M460LD      0x40

struct CPartNumID
{
	char szPartNumber[32];
	unsigned int uID;
	unsigned int uProjectCode;
};

unsigned int GetNumOfPart(void);

#endif // !defined(AFX_PARTNUMID_H__224714FF_DE1A_41FC_81B6_4B998BDC9FE6__INCLUDED_)
