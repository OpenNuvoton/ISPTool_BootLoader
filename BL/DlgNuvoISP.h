// DlgNuvoISP.h : header file
//

#if !defined(_DLG_NUVOISP_H_)
#define _DLG_NUVOISP_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Resource.h"
#include "DialogHex.h"
#include "DialogMain.h"
#include "ISPProc.h"


/////////////////////////////////////////////////////////////////////////////
// CBootLoaderISPDlg dialog

class CBootLoaderISPDlg: public CDialogMain, public CISPProc
{
// Construction
public:
	enum { IDD = IDD_DIALOG_NUVOISP_BOOTLOADER };

	CBootLoaderISPDlg(UINT Template = CBootLoaderISPDlg::IDD, CWnd *pParent = NULL);	// standard constructor

	virtual ~CBootLoaderISPDlg();

	// CDialogMain
	virtual void InitComboBox(int iDummy = 1);
	virtual void OnSelchangeInterface();

	BOOL			m_bConnect;
	CButton			m_ButtonConnect;
	CButton			m_ButtonResetConnect;

	CComboBox		m_SelClock;		// For Connect
	CComboBox		m_SelClock2;	// For Program
	CComboBox		m_SelReBoot;

	CString			m_sCaption;
	CString			m_sConnect;
	CString			m_sStatus;

	CTabCtrl		m_TabData;

	CProgressCtrl	m_Progress;

	CDialogHex		*pViewer[NUM_VIEW];

	WINCTRLID		m_CtrlID[NUM_VIEW];

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange *pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();

	void OnButtonBinFile(int idx, TCHAR *szPath = NULL);
	void ChangeBtnText(int nBtn, LPTSTR pszText);
	void EnableProgramOption(BOOL bEnable);

	void UpdateAddrOffset();

	virtual void ShowChipInfo_OffLine(void);
	virtual void ShowChipInfo_OnLine(void);

	// Generated message map functions
	//{{AFX_MSG(CBootLoaderISPDlg)

	afx_msg void OnButtonLoadFile();

	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg LRESULT OnDeviceChange(WPARAM  nEventType, LPARAM  dwData);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaint();
	afx_msg void OnSelchangeTabData(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	virtual afx_msg void OnButtonConnect();
	virtual afx_msg void OnButtonStart();
	virtual afx_msg void OnKillfocusEditAPROMOffset();
	virtual afx_msg void OnButtonConfig();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	void RegisterNotification();
	void UnregisterNotification();

	HDEVNOTIFY m_hNotifyDevNode;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_DLG_NUVOISP_H_)
