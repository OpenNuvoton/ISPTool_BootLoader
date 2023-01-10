#include "stdafx.h"
#include "Resource.h"
#include "DialogConfiguration.h" // Resource ID
#include "DlgNuvoISP.h"
#include "About.h"

#include <dbt.h>

#include "NuDataBase.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

inline std::string size_str(unsigned int size)
{
    char buf[128];

    if (size == 0) {
        _snprintf_s(buf, sizeof(buf), _TRUNCATE, "0K", size);
    } else if (size <= 1) {
        _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d byte", size);
    } else if (size < 1024) {
        _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d bytes", size);
    } else if (((size / 1024.) + 0.005) < 1024) {
        double f = (size / 1024.) + 0.005;
        unsigned int i = (unsigned int)f;
        unsigned int j = (unsigned int)((f - i) * 100.);

        if (j == 0) {
            _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%dK", i);
        } else {
            _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d.%02dK", i, j);
        }
    } else {
        double f = (size / 1024. / 1024.) + 0.005;
        unsigned int i = (unsigned int)f;
        unsigned int j = (unsigned int)((f - i) * 100.);

        if (j == 0) {
            _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%dM", i);
        } else {
            _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d.%02dM", i, j);
        }
    }

    return buf;
}


/////////////////////////////////////////////////////////////////////////////
// CBootLoaderISPDlg dialog

CBootLoaderISPDlg::CBootLoaderISPDlg(UINT Template,
									 CWnd *pParent /*=NULL*/)
	: CDialogMain(Template, pParent)
	, CISPProc(&m_hWnd)
{
	m_sCaption = _T("Nuvoton NuMicro Boot Loader ISP Tool 1.00");

	m_bConnect = false;

	for (int i = 0; i < NUM_VIEW; i++)
	{
		pViewer[i] = NULL;
	}

	WINCTRLID buddy[] = 
	{
		{IDC_BUTTON_APROM, IDC_EDIT_FILEPATH_APROM, IDC_STATIC_FILEINFO_APROM},
		{IDC_BUTTON_LDROM, IDC_EDIT_FILEPATH_LDROM, IDC_STATIC_FILEINFO_LDROM},
	};

	memcpy(&m_CtrlID, buddy, sizeof(m_CtrlID));
}

CBootLoaderISPDlg::~CBootLoaderISPDlg()
{
	for (int i = 0; i < NUM_VIEW; i++)
	{
		if (pViewer[i] != NULL)
		{
			pViewer[i]->DestroyWindow();
		}

		delete pViewer[i];
		pViewer[i] = NULL;
	}

	UnregisterNotification();
}

void CBootLoaderISPDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogMain::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CBootLoaderISPDlg)
	DDX_Control(pDX, IDC_COMBO_COM_PORT,	m_SelComPort);
	DDX_Control(pDX, IDC_COMBO_INTERFACE,	m_SelInterface);
	DDX_Control(pDX, IDC_COMBO_CLOCK,		m_SelClock);
	DDX_Control(pDX, IDC_COMBO_CLOCK2,		m_SelClock2);
	DDX_Control(pDX, IDC_COMBO_REBOOT,		m_SelReBoot);

	DDX_Control(pDX, IDC_BUTTON_CONNECT,	m_ButtonConnect);
	DDX_Text(pDX, IDC_STATIC_CONNECT,		m_sConnect);

	DDX_Control(pDX, IDC_CHECK_RESET,		m_ButtonResetConnect);

	DDX_Control(pDX, IDC_TAB_DATA,			m_TabData);

	DDX_Control(pDX, IDC_PROGRESS,			m_Progress);
	DDX_Text(pDX, IDC_STATIC_STATUS,		m_sStatus);

	DDX_Check(pDX, IDC_CHECK_RESET,			m_bResetConnect);

	DDX_Check(pDX, IDC_CHECK_APROM,			m_bProgram_APROM);
	DDX_Check(pDX, IDC_CHECK_LDROM,			m_bProgram_LDROM);
	DDX_Check(pDX, IDC_CHECK_CONFIG,		m_bProgram_Config);
	DDX_Check(pDX, IDC_CHECK_ERASE,			m_bErase);
	DDX_Check(pDX, IDC_CHECK_RUN_APROM,		m_bRunAPROM);
	DDX_Check(pDX, IDC_CHECK_CLOCK,			m_bSet_CLOCK);
	DDX_Check(pDX, IDC_CHECK_USBDISP,		m_bSet_USBDISP);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBootLoaderISPDlg, CDialog)
	//{{AFX_MSG_MAP(CBootLoaderISPDlg)
	ON_BN_CLICKED(IDC_BUTTON_APROM,					OnButtonLoadFile)
	ON_BN_CLICKED(IDC_BUTTON_LDROM,					OnButtonLoadFile)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG,				OnButtonConfig)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT,				OnButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_START,					OnButtonStart)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_DATA,			OnSelchangeTabData)

	ON_CBN_SELCHANGE(IDC_COMBO_INTERFACE,			OnSelchangeInterface)
	ON_CBN_SELCHANGE(IDC_COMBO_COM_PORT,			OnComboChange)

	ON_EN_KILLFOCUS(IDC_EDIT_APROM_BASE_ADDRESS,	OnKillfocusEditAPROMOffset)

	ON_MESSAGE(WM_DEVICECHANGE,						OnDeviceChange)
	//}}AFX_MSG_MAP

	ON_WM_CTLCOLOR()
	//ON_WM_DEVICECHANGE()
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


BOOL CBootLoaderISPDlg::OnInitDialog()
{
	CDialogMain::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu *pSysMenu = GetSystemMenu(FALSE);

	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu = _T("About Nuvoton NuMicro Boot Loader ISP Tool(&A)");

		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_sConnect = _T("Disconnected");
	UpdateData(FALSE);

	// Title
	SetWindowText(m_sCaption);

	// Set data view area
	// Btn Text --> Tab Text
	for (int i = 0; i < NUM_VIEW; i++)
	{
		CString strTab;
		GetDlgItemText(m_CtrlID[i].btn, strTab);
		m_TabData.InsertItem(i, strTab);
	}

	CRect rcClient;
	m_TabData.GetClientRect(rcClient);
	m_TabData.AdjustRect(FALSE, rcClient);

	for (int i = 0; i < NUM_VIEW; i++)
	{
		CDialogHex *pDlg = new CDialogHex;
		BOOL result = pDlg->Create(CDialogHex::IDD, &m_TabData);
		int error = ::GetLastError();
		pViewer[i] = pDlg;
		pViewer[i]->MoveWindow(rcClient);
		pViewer[i]->ShowWindow(SW_HIDE);
	}

	pViewer[0]->ShowWindow(SW_SHOW);
	m_Progress.SetRange(0, 100);
	EnableDlgItem(IDC_BUTTON_START, m_bConnect);
	InitComboBox();

	SetDlgItemText(IDC_EDIT_APROM_BASE_ADDRESS, _T("00000000"));
	SetDlgItemText(IDC_EDIT_VECMAP, _T("00000000"));

	Set_ThreadAction(&CISPProc::Thread_Idle);

	RegisterNotification();

	return TRUE;	// return TRUE  unless you set the focus to a control
}

void CBootLoaderISPDlg::OnButtonBinFile(int idx, TCHAR *szPath)
{
	CString FileName = _T("");
	// Backup current directory
	TCHAR szCurDir[MAX_PATH];

	if (GetCurrentDirectory(sizeof(szCurDir) / sizeof(szCurDir[0]), szCurDir) == 0) {
		szCurDir[0] = (TCHAR)'\0';
	}

	// Open file dialog
	CFileDialog dialog(TRUE, NULL, NULL,
					   OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					   _T("Binary Files (*.bin)|*.bin|All Files|*.*||"),
					   this);

	if (szPath != NULL) {
		FileName = szPath;
	} else if (dialog.DoModal() == IDOK) {
		FileName = dialog.GetPathName();
	}

	// Restore current directory
	if (szCurDir[0] != (TCHAR)'\0') {
		SetCurrentDirectory(szCurDir);
	}

	if (FileName != _T(""))
	{
		int sz = getFilesize(FileName.GetBuffer(0));

		if (UpdateFileInfo(FileName, &(m_sFileInfo[idx])))
		{
			// File Path
			SetDlgItemText(m_CtrlID[idx].path, FileName);

			// Size & Check Sum Info
			if (m_CtrlID[idx].sizecksum)
			{
				CString str, strInfo;

				if (sz >= 1024 * 1024 * 10)
					str.Format(_T("size: %.1fM Bytes"), (float)(sz / 1024.0 / 1024.0));
				else if (sz >= 1024 * 10)
					str.Format(_T("size: %.1fK Bytes"), (float)(sz / 1024.0));
				else
					str.Format(_T("size: %d Bytes"), sz);

				strInfo.Format(_T("%s, checksum: %04x"), str, m_sFileInfo[idx].usCheckSum);
				SetDlgItemText(m_CtrlID[idx].sizecksum, strInfo);
				pViewer[idx]->SetHexData(&(m_sFileInfo[idx].vbuf));
				m_TabData.SetCurSel(idx);
				OnSelchangeTabData(NULL, NULL);
			}
		}
	}
}

void CBootLoaderISPDlg::OnButtonLoadFile()
{
	const MSG *msg = GetCurrentMessage();
	int BtnID = LOWORD(msg->wParam);

	for (int idx = 0; idx < NUM_VIEW; idx++)
	{
		if (BtnID == m_CtrlID[idx].btn)
		{
			OnButtonBinFile(idx);
			return;
		}
	}
}

HBRUSH CBootLoaderISPDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  在此變更 DC 的任何屬性
    switch (pWnd->GetDlgCtrlID()) {
        case IDC_STATIC_CONNECT:
            if (m_bConnect) {
                pDC->SetTextColor(RGB(0, 128, 0));	//DarkGreen
            } else {
                pDC->SetTextColor(RGB(255, 0, 0));	//Red
            }

            break;

        case IDC_STATIC_CONFIG_VALUE_0:
            if (!m_bConnect) {
                break;
            }

            if (m_CONFIG_User[0] == m_CONFIG[0]) {
                pDC->SetTextColor(RGB(0, 128, 0));	//DarkGreen
            } else {
                pDC->SetTextColor(RGB(255, 0, 0));	//Red
            }

            break;

        case IDC_STATIC_CONFIG_VALUE_1:
            if (!m_bConnect) {
                break;
            }

            if (m_CONFIG_User[1] == m_CONFIG[1]) {
                pDC->SetTextColor(RGB(0, 128, 0));	//DarkGreen
            } else {
                pDC->SetTextColor(RGB(255, 0, 0));	//Red
            }

            break;

        case IDC_STATIC_CONFIG_VALUE_2:
            if (!m_bConnect) {
                break;
            }

            if (m_CONFIG_User[2] == m_CONFIG[2]) {
                pDC->SetTextColor(RGB(0, 128, 0));	//DarkGreen
            } else {
                pDC->SetTextColor(RGB(255, 0, 0));	//Red
            }

            break;

        case IDC_STATIC_CONFIG_VALUE_3:
            if (!m_bConnect) {
                break;
            }

            if (m_CONFIG_User[3] == m_CONFIG[3]) {
                pDC->SetTextColor(RGB(0, 128, 0));	//DarkGreen
            } else {
                pDC->SetTextColor(RGB(255, 0, 0));	//Red
            }

            break;

        default:
            break;
    }

    // TODO:  如果預設值並非想要的，則傳回不同的筆刷
    return hbr;
}

void CBootLoaderISPDlg::OnDropFiles(HDROP hDropInfo)
{
    // TODO: Add your message handler code here and/or call default
    TCHAR szPath[MAX_PATH];
    POINT point;
    CRect rect;

    //Returns a count of the files dropped
    while ((DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0) > 0)
            && (m_fnThreadProcStatus != &CISPProc::Thread_ProgramFlash)) {
        //Retrieves the position of the mouse pointer
        DragQueryPoint(hDropInfo, &point);
        //Retrieves the name of dropped file
        DragQueryFile(hDropInfo, 0, szPath, _countof(szPath));

        if (1) {
            for (int idx = 0; idx < NUM_VIEW; idx++) {
                GetDlgItem(m_CtrlID[idx].path)->GetWindowRect(&rect);
                ScreenToClient(&rect);

                if (PtInRect(&rect, point)) {
                    OnButtonBinFile(idx, szPath);
                    break;
                }
            }
        }

        break;
    }

    DragFinish(hDropInfo);
    CDialog::OnDropFiles(hDropInfo);
}

void CBootLoaderISPDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        //顯示Logo
        //CDialog::OnPaint();
        CPaintDC   dc(this);   //Device context
        CDC   dcMem;
        dcMem.CreateCompatibleDC(&dc);
        CBitmap   bmpBackground;
        bmpBackground.LoadBitmap(IDB_BITMAP_NUVOTON);
        BITMAP   bitmap;
        bmpBackground.GetBitmap(&bitmap);
        CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
        CRect MainRect;
        GetClientRect(&MainRect);
        //把dcMem拷貝到dc的相應位置
        //dc.StretchBlt(0,0,MainRect.Width(),bitmap.bmHeight,&dcMem,0,0,
        dc.StretchBlt(0, 0, bitmap.bmWidth, bitmap.bmHeight, &dcMem, 0, 0,
                      bitmap.bmWidth, bitmap.bmHeight, SRCCOPY); //bitmap.bmHeight = 44
        dcMem.SelectObject(pbmpOld);
    }
}

void CBootLoaderISPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CString sTitle;
		GetWindowText(sTitle);
		CAboutDlg dlgAbout(sTitle);
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

LRESULT CBootLoaderISPDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_USER_EVENT)
    {
        if (wParam == MSG_UPDATE_CONNECT_STATUS)
        {
            CString sMessage;
            UpdateData(true);
            m_sStatus = _T("");

            switch (lParam) {
                case CONNECT_STATUS_NONE:
                    EnableInterface(true);
                    m_sConnect		= _T("Disconnected");

                    switch (m_eProcSts) {
                        case EPS_OK:
                            m_sConnect = _T("Disconnected");
                            break;

                        case EPS_ERR_OPENPORT:
                            m_sConnect = _T("Open Port Error");
                            break;

                        case EPS_ERR_CONNECT:
                            m_sConnect = _T("CMD_CONNECT Error");
                            break;

                        default:
                            break;
                    }

                    ShowChipInfo_OffLine();
                    break;

                case CONNECT_STATUS_USB:
                    m_sConnect		= _T("Waiting for device connection");
                    m_ButtonConnect.SetWindowText(_T("Stop check"));
                    break;

                case CONNECT_STATUS_CONNECTING:
                    m_sConnect		= _T("Getting Chip Information ...");
                    m_ButtonConnect.SetWindowText(_T("Stop check"));
                    break;

                case CONNECT_STATUS_OK:
                    m_sConnect		= _T("Connected");
                    EnableProgramOption(TRUE);
                    ShowChipInfo_OnLine();

                    switch (m_eProcSts) {
                        case EPS_ERR_ERASE:
                            AfxMessageBox(_T("Erase failed"));
                            break;

                        case EPS_ERR_CONFIG:
                            AfxMessageBox(_T("Update CONFIG failed"));
                            break;

                        case EPS_ERR_APROM:
                            AfxMessageBox(_T("Update APROM failed"));
                            break;

                        case EPS_ERR_LDROM:
                            AfxMessageBox(_T("Update LDROM failed"));
                            break;

                        case EPS_ERR_SIZE:
                            AfxMessageBox(_T("File Size > Flash Size"));
                            break;

                        case EPS_PROG_DONE:
                            sMessage.Format(_T("Programming flash, OK! (%d secs)"), m_uProgramTime);
                            MessageBox(sMessage);
                            break;

                        default:
                            m_sStatus = _T("");
                            break;
                    }

                    m_ButtonConnect.SetWindowText(_T("Disconnected"));
                    break;

                default:
                    break;
            }

            m_bConnect = (lParam == CONNECT_STATUS_OK);
            EnableDlgItem(IDC_BUTTON_START, m_bConnect);
            UpdateData(false);
        }
        else if (wParam == MSG_UPDATE_ERASE_STATUS)
        {
            CString sText;
            sText.Format(_T("Erase %d%%"), (int)lParam);
            m_sStatus = sText;
            m_Progress.SetPos((int)lParam);
            UpdateData(FALSE);
        }
        else if (wParam == MSG_UPDATE_WRITE_STATUS)
        {
            CString sText;
            sText.Format(_T("Program %d%%"), (int)lParam);
            m_sStatus = sText;
            m_Progress.SetPos((int)lParam);
            UpdateData(FALSE);
        }
    }

    return CDialogMain::WindowProc(message, wParam, lParam);
}

void CBootLoaderISPDlg::EnableProgramOption(BOOL bEnable)
{
	/* WYLIWYP : What You Lock Is What You Program*/
	EnableDlgItem(IDC_CHECK_APROM,		bEnable);
	EnableDlgItem(IDC_BUTTON_APROM,		bEnable);
	EnableDlgItem(IDC_CHECK_LDROM,		bEnable);
	EnableDlgItem(IDC_BUTTON_LDROM,		bEnable);
	EnableDlgItem(IDC_CHECK_CONFIG,		bEnable);
	EnableDlgItem(IDC_BUTTON_CONFIG,	bEnable);
	EnableDlgItem(IDC_CHECK_ERASE,		bEnable);
	EnableDlgItem(IDC_CHECK_RUN_APROM,	bEnable);
	EnableDlgItem(IDC_CHECK_CLOCK,		bEnable);
	EnableDlgItem(IDC_CHECK_USBDISP,	bEnable);
	EnableDlgItem(IDC_BUTTON_START,		bEnable);
}

void CBootLoaderISPDlg::ShowChipInfo_OnLine()
{
	bool bSizeValid = UpdateSizeInfo(m_uDevice_PDID, m_uAPROM_TotalSize, m_uLDROM_Size, m_CONFIG);

	ShowDlgItem(IDC_STATIC_APOFFSET, 1);
	ShowDlgItem(IDC_EDIT_APROM_BASE_ADDRESS, 1);
	EnableDlgItem(IDC_EDIT_APROM_BASE_ADDRESS, 1);

	// Update Part Number & CONFIG0 ~ CONFIG3 for all series
	CString strTmp = _T("");
	strTmp = gsChipCfgInfo.szPartNumber;
	SetDlgItemText(IDC_EDIT_PARTNO, strTmp);
	strTmp.Format(_T("0x%08X"), m_CONFIG_User[0]);
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_0, strTmp);
	strTmp.Format(_T("0x%08X"), m_CONFIG_User[1]);
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_1, strTmp);
	strTmp.Format(_T("0x%08X"), m_CONFIG_User[2]);
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_2, strTmp);
	strTmp.Format(_T("0x%08X"), m_CONFIG_User[3]);
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_3, strTmp);

	if (bSizeValid)
	{
		std::ostringstream os;
		os << "APROM: " << size_str(m_uAPROM_Size) << ","
			  " Data: " << size_str(m_uNVM_Size) << ","
			  " LDROM: " << size_str(m_uLDROM_Size);

		std::string cstr = os.str();
		std::wstring wcstr(cstr.begin(), cstr.end());
		CString str = wcstr.c_str();

		CString tips;
		tips.Format(_T("Information of target chip,\n\n%s"), str);

		CString info;
		info.Format(_T("%s\nFW Ver: 0x%X"), wcstr.c_str(), m_uFW_Version);
		SetDlgItemText(IDC_STATIC_PARTNO, info);
	}
	else
	{
		CString tips;
		tips.Format(_T("PDID: 0x%X, FW Ver: 0x%X"), m_uDevice_PDID, m_uFW_Version);
		SetDlgItemText(IDC_STATIC_PARTNO, tips);
	}

	Invalidate();
}

void CBootLoaderISPDlg::ShowChipInfo_OffLine(void)
{
	m_uAPROM_Offset = 0;
	m_ButtonConnect.SetWindowText(_T("Connect"));

	SetDlgItemText(IDC_EDIT_PARTNO, _T(""));
	SetDlgItemText(IDC_STATIC_PARTNO, _T(""));
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_0, _T(""));
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_1, _T(""));
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_2, _T(""));
	SetDlgItemText(IDC_STATIC_CONFIG_VALUE_3, _T(""));
	ChangeBtnText(0, _T("APROM"));
	ChangeBtnText(1, _T("LDROM"));
	ShowDlgItem(IDC_CHECK_CONFIG, 1);
	ShowDlgItem(IDC_CHECK_ERASE, 1);
	EnableDlgItem(IDC_BUTTON_CONFIG, 0);
	ShowDlgItem(IDC_STATIC_APOFFSET, 1);
	ShowDlgItem(IDC_EDIT_APROM_BASE_ADDRESS, 1);
	ShowDlgItem(IDC_STATIC_FLASH_BASE_ADDRESS, 0);
	ShowDlgItem(IDC_EDIT_FLASH_BASE_ADDRESS, 0);

	EnableProgramOption(TRUE);

	Invalidate(1);
}

void CBootLoaderISPDlg::OnKillfocusEditAPROMOffset()
{
	CString strAddr;

	GetDlgItemText(IDC_EDIT_APROM_BASE_ADDRESS, strAddr);
	m_uAPROM_Offset = ::_tcstoul(strAddr, 0, 16);
	strAddr.Format(_T("%08X"), m_uAPROM_Offset);
	SetDlgItemText(IDC_EDIT_APROM_BASE_ADDRESS, strAddr);
}

void CBootLoaderISPDlg::InitComboBox(int iDummy)
{
	m_SelInterface.ResetContent();
	m_SelInterface.AddString(_T("USB"));
	m_SelInterface.AddString(_T("UART"));
	m_SelInterface.AddString(_T("SPI"));
	m_SelInterface.AddString(_T("I2C"));
	m_SelInterface.AddString(_T("CAN"));
	m_SelInterface.SetCurSel(0);

	m_SelClock.ResetContent();
	m_SelClock.AddString(_T("115200"));
	m_SelClock.AddString(_T("230400"));
	m_SelClock.AddString(_T("460800"));
	m_SelClock.AddString(_T("921600"));
	m_SelClock.SetCurSel(0);

	m_SelClock2.ResetContent();
	m_SelClock2.AddString(_T("115200"));
	m_SelClock2.AddString(_T("230400"));
	m_SelClock2.AddString(_T("460800"));
	m_SelClock2.AddString(_T("921600"));
	m_SelClock2.SetCurSel(0);

	OnSelchangeInterface();

	m_SelReBoot.ResetContent();
	m_SelReBoot.AddString(_T("CMD_RST_SRC_CHIP"));
	m_SelReBoot.AddString(_T("CMD_RST_SRC_CPU"));
	m_SelReBoot.AddString(_T("CMD_RST_SRC_SYS"));
	m_SelReBoot.AddString(_T("CMD_EXEC_ADDR"));
	m_SelReBoot.SetCurSel(0);

	if (ScanPCCom())
	{
		m_SelComPort.SetCurSel(0);
	}
}

void CBootLoaderISPDlg::OnSelchangeInterface()
{
	static int _clock = 1;
	int _interface = m_SelInterface.GetCurSel();

	m_ButtonResetConnect.EnableWindow(_interface != 0);
	m_SelComPort.EnableWindow(_interface == 1);

	if ((_interface == 1) || (_interface == 4))
	{
		EnableDlgItem(IDC_COMBO_CLOCK, true);
		EnableDlgItem(IDC_COMBO_CLOCK2, true);

		if (_clock != _interface)
		{
			m_SelClock.ResetContent();
			int _sel = m_SelClock2.GetCurSel();
			m_SelClock2.ResetContent();
			_clock = _interface;

			if (_clock == 1)
			{
				m_SelClock.AddString(_T("115200"));
				m_SelClock.AddString(_T("230400"));
				m_SelClock.AddString(_T("460800"));
				m_SelClock.AddString(_T("921600"));

				m_SelClock2.AddString(_T("115200"));
				m_SelClock2.AddString(_T("230400"));
				m_SelClock2.AddString(_T("460800"));
				m_SelClock2.AddString(_T("921600"));
			}
			else
			{
				m_SelClock.AddString(_T("250K"));
				m_SelClock.AddString(_T("500K"));
				m_SelClock.AddString(_T("750K"));
				m_SelClock.AddString(_T("1000K"));

				m_SelClock2.AddString(_T("250000"));
				m_SelClock2.AddString(_T("500000"));
				m_SelClock2.AddString(_T("750000"));
				m_SelClock2.AddString(_T("1000000"));
			}

			m_SelClock.SetCurSel(0);
			m_SelClock2.SetCurSel(_sel);
		}
	}
	else
	{
		EnableDlgItem(IDC_COMBO_CLOCK, false);
		EnableDlgItem(IDC_COMBO_CLOCK2, false);
	}

	if (_interface == 1)
	{
		OnComboChange();
	}
	else
	{
		EnableDlgItem(IDC_BUTTON_CONNECT, true);
	}
}

void CBootLoaderISPDlg::OnSelchangeTabData(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    int nSelect = m_TabData.GetCurSel();

    for (int i = 0; i < sizeof(pViewer) / sizeof(pViewer[0]); ++i) {
        if (i != nSelect) {
            pViewer[i]->ShowWindow(SW_HIDE);
        }
    }

    pViewer[nSelect]->ShowWindow(SW_SHOW);

    if (pResult != NULL) {
        *pResult = 0;
    }
}

void CBootLoaderISPDlg::OnButtonConnect()
{
	if ((m_fnThreadProcStatus == &CISPProc::Thread_Idle) || (m_fnThreadProcStatus == &CISPProc::Thread_Pause))
	{
		/* Connect */
		m_SelComPort.GetWindowText(m_ComNum);

		//
		DWORD Clock = 115200;
		int CurSelIntf = m_SelInterface.GetCurSel();
		int CurSelClock = m_SelClock.GetCurSel();

		if (CurSelIntf == 1)
		{
			DWORD arrClock[4] = { 115200, 230400, 460800, 921600, };
			Clock = arrClock[CurSelClock];
		}
		else if (CurSelIntf == 4)
		{
			Clock = 250000 * (CurSelClock + 1); // 250K, 500K, 750K, 1000K
		}

		SetInterface(m_SelInterface.GetCurSel() + 1, m_ComNum, Clock);
		EnableInterface(false);
		Set_ThreadAction(&CISPProc::Thread_CheckUSBConnect);
	}
	else if (m_fnThreadProcStatus != NULL)
	{
		/* Disconnect */
		Set_ThreadAction(&CISPProc::Thread_Idle);
	}
}

void CBootLoaderISPDlg::OnButtonStart()
{
	UpdateData(TRUE);

	if (m_bProgram_APROM || m_bProgram_LDROM || m_bProgram_Config || m_bErase || m_bRunAPROM || m_bSet_CLOCK || m_bSet_USBDISP)
	{
		// Check Standart ISP Options
	}
	else
	{
		MessageBox(_T("No Operation is Selected."), NULL, MB_ICONSTOP);
		return;
	}

	/* WYLIWYP : What You Lock Is What You Program*/
	/* Lock ALL */
	EnableProgramOption(FALSE);

	/* Check thread status */
	CString strErr = _T("");
	LockGUI();

	if (m_fnThreadProcStatus == &CISPProc::Thread_CheckDisconnect)
		// if(m_fnThreadProcStatus == &CNuvoISPDlg::Thread_Idle)
	{
		/* Check write size */
		if (strErr.IsEmpty() && m_bProgram_APROM)
		{
			if (m_sFileInfo[0].st_size == 0)
			{
				strErr = _T("Can not load APROM file for programming!");
			}
		}

		if (strErr.IsEmpty() && m_bProgram_LDROM)
		{
			if (m_sFileInfo[1].st_size == 0)
			{
				strErr = _T("Can not load LDROM file for programming!");
			}
		}

		// In case user press "Enter" after typing offset, need to call OnKillfocusEditAPRomOffset manually
		OnKillfocusEditAPROMOffset();
		UpdateAddrOffset();

		if (m_bSet_CLOCK) {
			m_uClock = GetDlgItemInt(IDC_COMBO_CLOCK2);
		}

		if (m_bRunAPROM)
		{
			m_uReBootSrc = (1 << m_SelReBoot.GetCurSel());

			CString strAddr;
			GetDlgItemText(IDC_EDIT_VECMAP, strAddr);
			m_uVecMap = ::_tcstoul(strAddr, 0, 16);

			if (m_uReBootSrc == CMD_RST_SRC_CHIP) {
			} else if (m_uReBootSrc == CMD_RST_SRC_CPU) {
			} else if (m_uReBootSrc == CMD_RST_SRC_SYS) {
			} else if (m_uReBootSrc == CMD_EXEC_ADDR) {
			} else {
			}

			strAddr.Format(_T("%08X"), m_uVecMap);
			SetDlgItemText(IDC_EDIT_VECMAP, strAddr);
		}

		if (strErr.IsEmpty())
		{
			Set_ThreadAction(&CISPProc::Thread_ProgramFlash);
		}
	}
	else
	{
		strErr = _T("Please wait for ISP operation.");
	}

	if (!strErr.IsEmpty())
	{
		MessageBox(strErr, NULL, MB_ICONSTOP);
		EnableProgramOption(TRUE);
	}

	UnlockGUI();
}

void CBootLoaderISPDlg::OnButtonConfig()
{
	// online mode - must connect to real chip. gsChipCfgInfo must be valid
	if (ConfigDlgSel(m_CONFIG_User, sizeof(m_CONFIG_User)))
	{
		CString strTmp = _T("");

		strTmp.Format(_T("0x%08X"), m_CONFIG_User[0]);
		SetDlgItemText(IDC_STATIC_CONFIG_VALUE_0, strTmp);
		strTmp.Format(_T("0x%08X"), m_CONFIG_User[1]);
		SetDlgItemText(IDC_STATIC_CONFIG_VALUE_1, strTmp);
		strTmp.Format(_T("0x%08X"), m_CONFIG_User[2]);
		SetDlgItemText(IDC_STATIC_CONFIG_VALUE_2, strTmp);
		strTmp.Format(_T("0x%08X"), m_CONFIG_User[3]);
		SetDlgItemText(IDC_STATIC_CONFIG_VALUE_3, strTmp);
	}
}

void CBootLoaderISPDlg::UpdateAddrOffset()
{
	m_uAPROM_Addr = 0;
	CString strAddr;
	GetDlgItemText(IDC_EDIT_FLASH_BASE_ADDRESS, strAddr);

	unsigned int uAddr = ::_tcstoul(strAddr, 0, 16);

	return;
}

void CBootLoaderISPDlg::RegisterNotification()
{
    DEV_BROADCAST_DEVICEINTERFACE devIF = {0};
    // Globally Unique Identifier (GUID) for HID class devices.  Windows uses GUIDs to identify things.
    GUID hidGuid = {0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30};
    devIF.dbcc_size = sizeof(devIF);
    devIF.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    devIF.dbcc_classguid = hidGuid;
    m_hNotifyDevNode = RegisterDeviceNotification(GetSafeHwnd(), &devIF, DEVICE_NOTIFY_WINDOW_HANDLE);
}

void CBootLoaderISPDlg::UnregisterNotification()
{
    if (m_hNotifyDevNode) {
        UnregisterDeviceNotification(m_hNotifyDevNode);
        m_hNotifyDevNode = NULL;
    }
}

LRESULT CBootLoaderISPDlg::OnDeviceChange(WPARAM  nEventType, LPARAM  dwData)
{
    PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE)dwData;
    CString DevPathName = pdbi->dbcc_name;

    switch (nEventType) {
        case DBT_DEVICEARRIVAL:
            // A device has been inserted and is now available.
            //…
            break;

        case DBT_DEVICEREMOVECOMPLETE:

            // Device has been removed.
            //…
            if (pdbi->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                if (DevPathName.CompareNoCase(m_ISPDev.m_strDevPathName) == 0) {
                    Set_ThreadAction(&CISPProc::Thread_Idle);
                }
            }

            break;

        default:
            break;
    }

    return TRUE;
}

void CBootLoaderISPDlg::ChangeBtnText(int nBtn, LPTSTR pszText)
{
	if (0 == nBtn)
	{
		SetDlgItemText(IDC_BUTTON_APROM, pszText);
		SetDlgItemText(IDC_CHECK_APROM, pszText);
	}
	else if (1 == nBtn)
	{
		SetDlgItemText(IDC_BUTTON_LDROM, pszText);
		SetDlgItemText(IDC_CHECK_LDROM, pszText);
	}
	else
	{
		return;
	}

	TC_ITEM ti;
	ti.mask = TCIF_TEXT;
	ti.pszText = pszText;
	VERIFY(m_TabData.SetItem(nBtn, &ti));
}

