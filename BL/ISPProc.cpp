#include "stdafx.h"
#include "ISPProc.h"
#include "thread_proxy.h"	// thread_proxy
#include "NuDataBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

template <class T>
inline T *vector_ptr(std::vector<T> &v)
{
    if (v.size() > 0) {
        return &v[0];
    } else {
        return NULL;
    }
}

template <class T>
inline const T *vector_ptr(const std::vector<T> &v)
{
    if (v.size() > 0) {
        return &v[0];
    } else {
        return NULL;
    }
}


CISPProc::CISPProc(HWND *pWnd)
	: m_bResetConnect(TRUE)
	, m_bProgram_APROM(FALSE)
	, m_bProgram_LDROM(FALSE)
	, m_bProgram_Config(FALSE)
	, m_bErase(FALSE)
	, m_bRunAPROM(FALSE)
	, m_bSet_CLOCK(FALSE)
	, m_bSet_USBDISP(FALSE)
	, m_uClock(0)
	, m_uReBootSrc(CMD_RST_SRC_CHIP)
	, m_uVecMap(0)
{
    MainHWND = pWnd;
    m_hThreadMutex = ::CreateMutex(NULL, FALSE, NULL);
    m_fnThreadProcStatus = &CISPProc::Thread_Pause;
    m_fnThreadProcStatus_backup = &CISPProc::Thread_Pause;
    DWORD dwThreadID;
    m_pAssistThread = thread_proxy(
                          this,
                          NULL,
                          //LPSECURITY_ATTRIBUTES lpThreadAttributes,  // pointer to security attributes
                          0,
                          //DWORD dwStackSize,                         // initial thread stack size
                          &CISPProc::AssistThread,
                          //DWORD (T::*lpStartAddress)(LPVOID),        // pointer to thread function
                          NULL,
                          //LPVOID lpParameter,                        // argument for new thread
                          0,
                          //DWORD dwCreationFlags,                     // creation flags
                          &dwThreadID
                          //LPDWORD lpThreadId                         // pointer to receive thread ID
                      );

	m_CONFIG[0] = 0xFFFFFFFF;
	m_CONFIG[1] = 0xFFFFFFFF;
	m_CONFIG[2] = 0xFFFFFFFF;
	m_CONFIG[3] = 0xFFFFFFFF;

	m_CONFIG_User[0] = 0xFFFFFFFF;
	m_CONFIG_User[1] = 0xFFFFFFFF;
	m_CONFIG_User[2] = 0xFFFFFFFF;
	m_CONFIG_User[3] = 0xFFFFFFFF;

	m_pAssistThread->m_bAutoDelete = TRUE;
	m_eProcSts = EPS_OK;
}

CISPProc::~CISPProc()
{
    Set_ThreadAction(NULL);
    WaitForSingleObject(m_pAssistThread->m_hThread, 5000);
}

DWORD CISPProc::AssistThread(LPVOID pArg)
{
    while (1) {
        void (CISPProc::*fnThreadProcStatus)() = m_fnThreadProcStatus;
        {
            if (fnThreadProcStatus != NULL) {
                (this->*fnThreadProcStatus)();
            } else {
                break;
            }
        }
    }

    return 0;
}

void CISPProc::Set_ThreadAction(void (CISPProc::*fnThreadProcStatus)())
{
    LockGUI();
    m_fnThreadProcStatus_backup = m_fnThreadProcStatus;

    if (m_fnThreadProcStatus != NULL) {
        m_fnThreadProcStatus = fnThreadProcStatus;
    }

    UnlockGUI();
}

void CISPProc::Thread_Pause()
{
    while (m_fnThreadProcStatus == &CISPProc::Thread_Pause) {
        Sleep(100);
    }
}

void CISPProc::Thread_Idle()
{
    if (m_fnThreadProcStatus != &CISPProc::Thread_Idle) {
        return;
    }

    if (MainHWND != NULL) {
        PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_CONNECT_STATUS, CONNECT_STATUS_NONE);
    }

    m_ISPDev.Close_Port();
    m_eProcSts = EPS_OK;

    while (m_fnThreadProcStatus == &CISPProc::Thread_Idle) {
        Sleep(100);
    }
}

void CISPProc::Thread_CheckUSBConnect()
{
    if (m_fnThreadProcStatus != &CISPProc::Thread_CheckUSBConnect) {
        return;
    }

    if (MainHWND != NULL) {
        PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_CONNECT_STATUS, CONNECT_STATUS_USB);
    }

    m_ISPDev.Close_Port();
    DWORD dwWait = 0;
    DWORD dwStart = 0;

	while (m_fnThreadProcStatus == &CISPProc::Thread_CheckUSBConnect)
	{
		if (m_ISPDev.Open_Port())
		{
			m_eProcSts = EPS_ERR_CONNECT;

			try
			{
				if (m_ISPDev.CMD_UART_Connect(m_bResetConnect, 10))
				{
					Sleep(10);
					// Re-Open COM Port to clear previous status
					m_ISPDev.ReOpen_Port();
					break;
				}
			} catch (...) {
				Set_ThreadAction(&CISPProc::Thread_Idle);
			}
		}
		else
		{
			m_eProcSts = EPS_ERR_OPENPORT;
			dwStart = GetTickCount();

			while (m_fnThreadProcStatus == &CISPProc::Thread_CheckUSBConnect) {
				if ((GetTickCount() - dwStart) > 1000) {
					break;
				}
			}
		}
	}

	while (m_fnThreadProcStatus == &CISPProc::Thread_CheckUSBConnect)
	{
		if (m_ISPDev.Open_Port())
		{
			m_eProcSts = EPS_ERR_CONNECT;
			dwStart = GetTickCount();

			try
			{
				if (m_ISPDev.CMD_Connect(m_bResetConnect, 40))
				{
					Set_ThreadAction(&CISPProc::Thread_CheckDeviceConnect);
				}
			}
			catch (...)
			{
				Set_ThreadAction(&CISPProc::Thread_Idle);
			}
		}
		else
		{
			m_eProcSts = EPS_ERR_OPENPORT;
			dwStart = GetTickCount();

			while (m_fnThreadProcStatus == &CISPProc::Thread_CheckUSBConnect) {
				if ((GetTickCount() - dwStart) > 1000) {
					break;
				}
			}
		}
	}
}

void CISPProc::Thread_CheckDeviceConnect()
{
	if (m_fnThreadProcStatus != &CISPProc::Thread_CheckDeviceConnect) {
		return;
	}

	if (MainHWND != NULL) {
		PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_CONNECT_STATUS, CONNECT_STATUS_CONNECTING);
	}

	try
	{
		while (m_fnThreadProcStatus == &CISPProc::Thread_CheckDeviceConnect)
		{
			if (m_ISPDev.Check_USB_Link())
			{
				// Re-Open COM Port to clear previous status
				m_ISPDev.ReOpen_Port();

				m_uDevice_PDID		= m_ISPDev.GetChipInfo(0);
				m_uFW_Version		= m_ISPDev.GetChipInfo(1);
				m_uAPROM_TotalSize	= m_ISPDev.GetChipInfo(2);
				m_uLDROM_Size		= m_ISPDev.GetChipInfo(3);

				m_CONFIG[0]			= m_ISPDev.GetChipInfo(5);
				m_CONFIG[1]			= m_ISPDev.GetChipInfo(6);
				m_CONFIG[2]			= m_ISPDev.GetChipInfo(7);
				m_CONFIG[3]			= m_ISPDev.GetChipInfo(8);

				memcpy(m_CONFIG_User, m_CONFIG, sizeof(m_CONFIG));

				m_eProcSts = EPS_OK;

				if (MainHWND != NULL) { // UI Mode
					Set_ThreadAction(&CISPProc::Thread_CheckDisconnect);
				} else { // Command Line Mode
					Set_ThreadAction(&CISPProc::Thread_ProgramFlash);
				}
			}
			else
			{
				Set_ThreadAction(&CISPProc::Thread_CheckUSBConnect);
			}
		}
	}
	catch (...)
	{
		Set_ThreadAction(&CISPProc::Thread_Idle);
	}
}


void CISPProc::Thread_CheckDisconnect()
{
    if (m_fnThreadProcStatus != &CISPProc::Thread_CheckDisconnect) {
        return;
    }

    if (MainHWND != NULL) {
        PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_CONNECT_STATUS, CONNECT_STATUS_OK);
    }

    while (m_fnThreadProcStatus == &CISPProc::Thread_CheckDisconnect) {
        Sleep(100);
    }
}

#include <time.h>
void CISPProc::Thread_ProgramFlash()
{
    if (m_fnThreadProcStatus != &CISPProc::Thread_ProgramFlash) {
        return;
    }

    unsigned int uAddr, uSize;
    unsigned char *pBuffer;
    m_eProcSts = EPS_OK;

	try
	{
		time_t start = time(NULL);

		if (m_bErase)
		{
			if (m_ISPDev.EraseAll())
			{
				m_ISPDev.ReadConfig(m_CONFIG);
			}
			else
			{
				m_eProcSts = EPS_ERR_ERASE;
				Set_ThreadAction(&CISPProc::Thread_CheckDisconnect);
				return;
			}
		}

		if (m_bProgram_Config)
		{
			m_ISPDev.UpdateConfig(m_uCONFIG_Addr, m_CONFIG_User, m_CONFIG);

			if ((m_CONFIG_User[0] != m_CONFIG[0]) || 
				(m_CONFIG_User[1] != m_CONFIG[1]) || 
				(m_CONFIG_User[2] != m_CONFIG[2]) || 
				(m_CONFIG_User[3] != m_CONFIG[3]))
			{
				m_eProcSts = EPS_ERR_CONFIG;
				Set_ThreadAction(&CISPProc::Thread_CheckDisconnect);
				return;
			}
		}

		if ((m_bErase || m_bProgram_Config) && (m_bProgram_APROM || m_bProgram_LDROM))
		{
			UpdateSizeInfo(m_uDevice_PDID, m_uAPROM_TotalSize, m_uLDROM_Size, m_CONFIG);
		}

        if (m_bProgram_APROM)
        {
            uAddr = m_uAPROM_Addr + m_uAPROM_Offset;
            uSize = m_sFileInfo[0].st_size;
            pBuffer = vector_ptr(m_sFileInfo[0].vbuf);

            if (MainHWND != NULL)
            {
                PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_WRITE_STATUS, 0);
            }

            for (unsigned long i = 0; i < uSize;)
            {
                if (m_fnThreadProcStatus != &CISPProc::Thread_ProgramFlash)
                {
                    break;
                }

                unsigned long uLen;
                unsigned int uRetry = 10;

                while (uRetry)
                {
                    m_ISPDev.UpdateFlash(uAddr, uSize, uAddr + i,
                                           (const char *)(pBuffer + i),
                                           &uLen);

                    break;
                }

                i += uLen;

                if (MainHWND != NULL) {
                    PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_WRITE_STATUS, (i * 100) / uSize);
                }
            }
        }

        if (m_bProgram_LDROM)
        {
            uAddr = m_uLDROM_Addr;
            uSize = m_sFileInfo[1].st_size;
            pBuffer = vector_ptr(m_sFileInfo[1].vbuf);

            if (m_uLDROM_Size < uSize) {
                m_eProcSts = EPS_ERR_SIZE;
                Set_ThreadAction(&CISPProc::Thread_CheckDisconnect);
                return;
            }

            if (MainHWND != NULL) {
                PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_WRITE_STATUS, 0);
            }

            for (unsigned long i = 0; i < uSize;)
            {
                if (m_fnThreadProcStatus != &CISPProc::Thread_ProgramFlash) {
                    break;
                }

                unsigned long uLen;
                unsigned int uRetry = 10;

                while (uRetry)
                {
                    m_ISPDev.UpdateFlash(uAddr, uSize, uAddr + i,
                                         (const char *)(pBuffer + i),
                                         &uLen);

                    break;
                }

                i += uLen;

                if (MainHWND != NULL) {
                    PostMessage(*MainHWND, MSG_USER_EVENT, MSG_UPDATE_WRITE_STATUS, (i * 100) / uSize);
                }
            }
        }

        if (m_bRunAPROM)
        {
            if (m_uReBootSrc) {
                m_ISPDev.Cmd_REBOOT_SOURCE(m_uReBootSrc, m_uVecMap);
            }

            m_eProcSts = EPS_OK;
            time_t end = time(NULL);
            m_uProgramTime = unsigned int(end - start);
            CString str;
            str.Format(_T("Programming flash, OK! Run to APROM (%d secs)"), m_uProgramTime);

            if (MainHWND != NULL) {
                MessageBox(*MainHWND, str, _T(""), MB_ICONINFORMATION);
            }

            // For Virtual Com Port device, it takes more than 5ms to convert USB signals to UART signals. (64 * 10 * 1000 / 115200 )
            // After sending Reset Command by PC Tool, we must wait for a little time to make sure the target device will receive Reset Command.
            // Without this latency, the Close Port operation will cancel the transmission immediately.
            Sleep(20);
            Set_ThreadAction(&CISPProc::Thread_Idle);
            return;
        }

        if (m_bSet_USBDISP)
        {
            m_ISPDev.Cmd_GOTO_USBDISP();
            m_eProcSts = EPS_OK;
            time_t end = time(NULL);
            m_uProgramTime = unsigned int(end - start);
            CString str;
            //str.Format(_T("Set USBDISP OK! Reconnect using USBD (not supported yet). (%d secs)"), m_uProgramTime);
			str.Format(_T("Set USBDISP OK! (%d secs)"), m_uProgramTime);

            if (MainHWND != NULL) {
                MessageBox(*MainHWND, str, _T(""), MB_ICONINFORMATION);
            }

            // For Virtual Com Port device, it takes more than 5ms to convert USB signals to UART signals. (64 * 10 * 1000 / 115200 )
            // After sending Reset Command by PC Tool, we must wait for a little time to make sure the target device will receive Reset Command.
            // Without this latency, the Close Port operation will cancel the transmission immediately.
            Sleep(20);
            Set_ThreadAction(&CISPProc::Thread_Idle);
            return;
        }

        if (m_bSet_CLOCK)
        {
            m_ISPDev.Cmd_SET_UART_SPEED(m_uClock);
            m_eProcSts = EPS_OK;
            time_t end = time(NULL);
            m_uProgramTime = unsigned int(end - start);
            CString str;
            str.Format(_T("Set Clock (%d) OK! Reconnect using new clock setting. (%d secs)"), m_uClock, m_uProgramTime);

            if (MainHWND != NULL) {
                MessageBox(*MainHWND, str, _T(""), MB_ICONINFORMATION);
            }

            // For Virtual Com Port device, it takes more than 5ms to convert USB signals to UART signals. (64 * 10 * 1000 / 115200 )
            // After sending Reset Command by PC Tool, we must wait for a little time to make sure the target device will receive Reset Command.
            // Without this latency, the Close Port operation will cancel the transmission immediately.
            Sleep(20);
            Set_ThreadAction(&CISPProc::Thread_Idle);
            return;
        }

        if (m_fnThreadProcStatus == &CISPProc::Thread_ProgramFlash)
        {
            time_t end = time(NULL);
            m_uProgramTime = unsigned int(end - start);
            m_eProcSts = EPS_PROG_DONE;
            Set_ThreadAction(&CISPProc::Thread_CheckDisconnect);
        }
    }
    catch (...)
    {
        if (MainHWND != NULL) {
            MessageBox(*MainHWND, _T("Lost connection!!!"), NULL, MB_ICONSTOP);
        }

        Set_ThreadAction(&CISPProc::Thread_Idle);
    }
}

void CISPProc::LockGUI()
{
    ::WaitForSingleObject(m_hThreadMutex, INFINITE);
}

void CISPProc::UnlockGUI()
{
    ::ReleaseMutex(m_hThreadMutex);
}

bool CISPProc::UpdateSizeInfo(unsigned int uPDID, unsigned int uAPROM_TotalSize, unsigned int uLDROM_Size, unsigned int uCONFIG[])
{
	if (GetChipDynamicInfo(uPDID, uAPROM_TotalSize, uLDROM_Size, uCONFIG))
	{
		m_uAPROM_Size	= gsChipCfgInfo.uAPROM_Size;
		m_uNVM_Addr		= gsChipCfgInfo.uNVM_Addr;
		m_uNVM_Size		= gsChipCfgInfo.uNVM_Size;

		m_uLDROM_Addr	= gsChipCfgInfo.uLDROM_Addr;
		m_uCONFIG_Addr	= gsChipCfgInfo.uCONFIG_Addr;

		return true;
	}

	return false;
}
