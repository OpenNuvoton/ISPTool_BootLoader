// DialogScrollable.cpp : implementation file
//

#include "stdafx.h"
#include "DialogMain.h"
#include "resource.h"
#include <cstring>

#include "NuDataBase.h"

IMPLEMENT_DYNAMIC(CDialogMain, CDialog)

CDialogMain::CDialogMain(UINT nIDTemplate, CWnd *pParent /*=NULL*/)
    : CDialog(nIDTemplate, pParent)
{
    m_bIsInitialized = false;
    m_bShowScrollBar = false;
    m_ScrollBarWidth = 15;
}

CDialogMain::~CDialogMain()
{
}

BEGIN_MESSAGE_MAP(CDialogMain, CDialog)
    //{{AFX_MSG_MAP(CDialogMain)
    ON_WM_SIZE()
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
    ON_WM_GETMINMAXINFO()
    ON_WM_MOUSEWHEEL()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDialogMain::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
}

void CDialogMain::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    int nScrollMaxX;
    int nScrollMaxY;
    CRect rect;
    ScrollWindow(m_nScrollPosX, m_nScrollPosY);
    m_nScrollPosX = 0;
    m_nScrollPosY = 0;
    GetWindowRect(rect);

    if (!m_bIsInitialized) {
        nScrollMaxY = 0;
        nScrollMaxX = 0;
    } else {
        if (rect.Height() < m_rect.Height()) {
            nScrollMaxY = m_rect.Height() - rect.Height() + m_ScrollBarWidth;
        } else {
            nScrollMaxY = 0;
        }

        if (rect.Width() < m_rect.Width()) {
            nScrollMaxX = m_rect.Width() - rect.Width() + m_ScrollBarWidth;
        } else {
            nScrollMaxX = 0;
        }

        if (nScrollMaxY != 0 && nScrollMaxX == 0) {
            nScrollMaxX = 10;
        } else if (nScrollMaxY == 0 && nScrollMaxX != 0) {
            nScrollMaxY = 10;
        }
    }

    if (nScrollMaxX == 0 || nScrollMaxY == 0) {
        m_bShowScrollBar = false;
    } else {
        m_bShowScrollBar = true;
    }

    SCROLLINFO si_y;
    si_y.cbSize = sizeof(SCROLLINFO);
    si_y.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
    si_y.nMin = 0;
    si_y.nMax = nScrollMaxY;
    si_y.nPage = si_y.nMax / 10;
    si_y.nPos = 0;
    SetScrollInfo(SB_VERT, &si_y, TRUE);
    SCROLLINFO si_x;
    si_x.cbSize = sizeof(SCROLLINFO);
    si_x.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
    si_x.nMin = 0;
    si_x.nMax = nScrollMaxX;
    si_x.nPage = si_x.nMax / 10;
    si_x.nPos = 0;
    SetScrollInfo(SB_HORZ, &si_x, TRUE);
    // TODO: Add your message handler code here
}

void CDialogMain::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
    SCROLLINFO scrollinfo;
    GetScrollInfo(SB_VERT, &scrollinfo);
    int nMaxPos = scrollinfo.nMax;
    int nDelta;

    switch (nSBCode) {
        case SB_LINEDOWN:
            if (m_nScrollPosY >= nMaxPos) {
                return;
            }

            nDelta = min(nMaxPos / 100, nMaxPos - m_nScrollPosY);

            if (nMaxPos / 100 == 0) {
                nDelta = 1;
            }

            break;

        case SB_LINEUP:
            if (m_nScrollPosY <= 0) {
                return;
            }

            nDelta = -min(nMaxPos / 100, m_nScrollPosY);

            if (nMaxPos / 100 == 0) {
                nDelta = -1;
            }

            break;

        case SB_PAGEDOWN:
            if (m_nScrollPosY >= nMaxPos) {
                return;
            }

            nDelta = min(nMaxPos / 10, nMaxPos - m_nScrollPosY);
            break;

        case SB_THUMBPOSITION:
            nDelta = (int)nPos - m_nScrollPosY;
            break;

        case SB_PAGEUP:
            if (m_nScrollPosY <= 0) {
                return;
            }

            nDelta = -min(nMaxPos / 10, m_nScrollPosY);
            break;

        default:
            return;
    }

    m_nScrollPosY += nDelta;
    SetScrollPos(SB_VERT, m_nScrollPosY, TRUE);
    ScrollWindow(0, -nDelta);
    CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDialogMain::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
    SCROLLINFO scrollinfo;
    GetScrollInfo(SB_HORZ, &scrollinfo);
    int nMaxPos = scrollinfo.nMax;
    int nDelta;

    switch (nSBCode) {
        case SB_LINERIGHT:
            if (m_nScrollPosX >= nMaxPos) {
                return;
            }

            nDelta = min(nMaxPos / 100, nMaxPos - m_nScrollPosX);

            if (nMaxPos / 100 == 0) {
                nDelta = 1;
            }

            break;

        case SB_LINELEFT:
            if (m_nScrollPosX <= 0) {
                return;
            }

            nDelta = -min(nMaxPos / 100, m_nScrollPosX);

            if (nMaxPos / 100 == 0) {
                nDelta = -1;
            }

            break;

        case SB_PAGERIGHT:
            if (m_nScrollPosX >= nMaxPos) {
                return;
            }

            nDelta = min(nMaxPos / 10, nMaxPos - m_nScrollPosX);
            break;

        case SB_THUMBPOSITION:
            nDelta = (int)nPos - m_nScrollPosX;
            break;

        case SB_PAGELEFT:
            if (m_nScrollPosX <= 0) {
                return;
            }

            nDelta = -min(nMaxPos / 10, m_nScrollPosX);
            break;

        default:
            return;
    }

    m_nScrollPosX += nDelta;
    SetScrollPos(SB_HORZ, m_nScrollPosX, TRUE);
    ScrollWindow(-nDelta, 0);
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDialogMain::OnGetMinMaxInfo(MINMAXINFO *lpMMI)
{
    CDialog::OnGetMinMaxInfo(lpMMI);

    if (m_bIsInitialized) {
        lpMMI->ptMaxTrackSize.x = m_rect.Width();
        lpMMI->ptMaxTrackSize.y = m_rect.Height();
        lpMMI->ptMinTrackSize.x = 310;
        lpMMI->ptMinTrackSize.y = 310;
    }
}
BOOL CDialogMain::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CDialog::OnMouseWheel(nFlags, zDelta, pt);

    if (m_bShowScrollBar) {
        SCROLLINFO scrollinfo;
        GetScrollInfo(SB_VERT, &scrollinfo);
        int nMaxPos = scrollinfo.nMax;
        int nScrollPos = scrollinfo.nPos;
        int nDelta;

        if (zDelta < 0) {
            if (m_nScrollPosY >= nMaxPos) {
                return 0;
            }

            nDelta = min(nMaxPos / 10, nMaxPos - m_nScrollPosY);
        } else {
            if (m_nScrollPosY <= 0) {
                return 0;
            }

            nDelta = -min(nMaxPos / 10, m_nScrollPosY);
        }

        m_nScrollPosY += nDelta;
        SetScrollPos(SB_VERT, m_nScrollPosY, TRUE);
        ScrollWindow(0, -nDelta);
    }

    return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CDialogMain::ShowDlgItem(int nID, int nCmdShow)
{
    CWnd *pWnd = GetDlgItem(nID);

    if (pWnd != NULL) {
        pWnd->ShowWindow(nCmdShow);
    }
}

void CDialogMain::EnableDlgItem(int nID, BOOL bEnable)
{
    CWnd *pWnd = GetDlgItem(nID);

    if (pWnd != NULL) {
        pWnd->EnableWindow(bEnable);
    }
}

#ifndef _NO_CONFIGURATION_DLG

#include "DialogChipSetting_M460.h"

bool CDialogMain::ConfigDlgSel(unsigned int *pConfig, unsigned int size, unsigned int uSeriesCode /* = 0*/)
{
	bool ret = false;
	CDialog *pConfigDlg = NULL;
	unsigned int *Config;

	unsigned int uPDID = 0;
	unsigned int uProgramMemorySize = 0;
	unsigned int uPage_Size = 0;

	if (uSeriesCode == 0)
	{
		// uSeriesCode = 0; online mode (load chip info. from gsChipCfgInfo)
		uPDID				= gsChipCfgInfo.uPDID;
		uSeriesCode			= gsChipCfgInfo.uSeriesCode;
		uProgramMemorySize	= gsChipCfgInfo.uProgramMemorySize;

		// (FlashInfo.cpp) Page Size Type: 0x000 (512 Bytes, default), 0x200 (2K), 0x300 (4K)
		uPage_Size = 1 << (((gsChipCfgInfo.uFlashType & 0x0000FF00) >>  8) + 9);
	}

	switch (uSeriesCode)
	{
		case PROJ_M460HD:
		case PROJ_M460LD:
			pConfigDlg = new CDialogChipSetting_M460(uProgramMemorySize, uPage_Size, uSeriesCode);

			Config = (((CDialogChipSetting_M460 *)pConfigDlg)->m_uConfigValue);
			break;

		default:
			printf("or Unknow Configuration Dialog %X\n", uSeriesCode);
			return false;
	}

	if (pConfigDlg != NULL)
	{
		// Pass User Config to Configuration Dialog
		memcpy(Config, pConfig, size);

		if (pConfigDlg->DoModal() == IDOK)
		{
			// Update User Config from Configuration Dialog
			memcpy(pConfig, Config, size);
			ret = true;
		}

		delete pConfigDlg;
	}

	return ret;
}
#endif // #ifndef _NO_CONFIGURATION_DLG

BOOL CDialogMain::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_bIsInitialized = true;
    GetWindowRect(m_rect);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
};


UINT CDialogMain::ScanPCCom()
{
    m_SelComPort.ResetContent();
    m_SelComPort.AddString(_T("Scan Port"));
    UINT nComNum = 0;
    HKEY hKEY;
    LONG hResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hKEY);

    if (hResult != ERROR_SUCCESS) { //如果無法打開hKEY,則中止程式的執行
        //AfxMessageBox("錯誤：無法打開有關註冊表項");
        return  0 ;
    }

    TCHAR strInf[30];
    DWORD type_1 = REG_SZ;
    DWORD cbData_1 = 10;
    DWORD aa = 30, num = 0, a1, a2, a3, a4, a5, a6, a7;
    hResult = ::RegQueryInfoKey(hKEY, strInf, &a7, NULL, &a3, &a1, &a2, &num, &a4, &a5, &a6, NULL);

    if (hResult != ERROR_SUCCESS) { //如果無法打開hKEY,則中止程式的執行
        //AfxMessageBox("錯誤：無法打開有關註冊表項");
        RegCloseKey(hKEY);
        return   0;
    }

    BYTE portName[30];
    CString csr;

    for (DWORD i = 0 ; i < num ; i++) {
        aa = 30 ;
        cbData_1 = 30;
        hResult = ::RegEnumValue(hKEY, i, strInf, &aa, NULL, &type_1, portName, &cbData_1);

        if ((hResult != ERROR_SUCCESS) && (hResult != ERROR_MORE_DATA)) { //如果無法打開hKEY,則中止程式的執行
            //AfxMessageBox("錯誤：無法獲取有關註冊表項");
            continue;
        }

        csr.Format(_T("%s"), portName);
        m_SelComPort.AddString(csr);
        nComNum++;
    }

    RegCloseKey(hKEY);
    m_SelComPort.SetCurSel(0);
    return nComNum;
}

void CDialogMain::InitComboBox(int iType)
{
}

void CDialogMain::OnSelchangeInterface()
{
    m_SelComPort.EnableWindow(m_SelInterface.GetCurSel() == 1);

    if (m_SelInterface.GetCurSel() != 1) {
        EnableDlgItem(IDC_BUTTON_CONNECT, true);
    } else {
        OnComboChange();
    }
}

void CDialogMain::OnComboChange()
{
    if (m_SelComPort.GetCurSel() == 0) {
        int portcnt = ScanPCCom();
        printf("Num Port = %d\n", portcnt);
    }

    EnableDlgItem(IDC_BUTTON_CONNECT, m_SelComPort.GetCurSel());
}

void CDialogMain::EnableInterface(bool bEnable)
{
    if (bEnable) {
        m_SelInterface.EnableWindow(1);
        OnSelchangeInterface();
    } else {
        m_SelInterface.EnableWindow(0);
        m_SelComPort.EnableWindow(0);
    }
}

LRESULT CDialogMain::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CDialog::WindowProc(message, wParam, lParam);
}
