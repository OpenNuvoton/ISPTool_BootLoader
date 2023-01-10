#include "stdafx.h"
#include "ISPCMD.h"
#include <stdio.h>

#define USBCMD_TIMEOUT			5000
#define USBCMD_TIMEOUT_LONG		25000
//#define USBCMD_TIMEOUT_LONG		30000

#define printf(...)

ISPCMD::ISPCMD()
	: m_bOpenPort(FALSE)
	, m_uCmdIndex(18)	// Do not use 0 to avoid firmware already has index 0 occasionally.
#if defined(_BL)
	, m_iIspType(TYPE_BOOTLOADER)
#elif defined(_BL2)
	, m_iIspType(TYPE_APROM_BL2)
#endif
{
	memset(m_acPattern, 'a', sizeof(m_acPattern));
}

ISPCMD::~ISPCMD()
{
}

bool ISPCMD::Open_Port()
{
    if (m_bOpenPort) {
        return true;
    }

    m_uUSB_PID = 0;
    m_strDevPathName = _T("");
    ScopedMutex scopedLock(m_Mutex);

    switch (m_uInterface) {
        case INTF_HID:
            if (m_hidIO.OpenDevice(0x0416, 0x2000, -1)) {
                m_uUSB_PID = 0x2000;
            } else {
                return false;
            }

            m_strDevPathName = m_hidIO.GetDevicePath();
            break;

        case INTF_UART:
            if (!m_comIO.OpenDevice(m_strComNum, m_dwClock)) {
                return false;
            }

            break;

        case INTF_SPI:
        case INTF_I2C:
        case INTF_CAN:
#if 0
            if (m_hidIO.OpenDevice(0x0416, 0x5201, 5)) {	// Nu-Link2 with ISP-Bridge
                m_uUSB_PID = 0x5201;
            } else if (m_hidIO.OpenDevice(0x0416, 0x5203, 5)) {	// Nu-Link2 with ISP-Bridge
                m_uUSB_PID = 0x5203;
            } else if (m_hidIO.OpenDevice(0x0416, 0x3F10, -1)) {	// ISP-Bridge
                m_uUSB_PID = 0x3F10;
            } else {
                return false;
            }
#else
			if (m_hidIO.OpenDevice(0x0416, 0x2007, 4))		// ISP-Bridge
			{
				m_uUSB_PID = 0x2007;
			}
			else
			{
				return false;
			}
#endif

            m_strDevPathName = m_hidIO.GetDevicePath();
            break;

        default:
            return false;
    }

    m_bOpenPort = TRUE;
    return true;
}

void ISPCMD::Close_Port()
{
    m_strDevPathName = _T("");
    ScopedMutex scopedLock(m_Mutex);

    if (!m_bOpenPort) {
        return;
    }

    m_bOpenPort = FALSE;

    switch (m_uInterface) {
        case INTF_HID:
            m_hidIO.CloseDevice();
            break;

        case INTF_UART:
            m_comIO.CloseDevice();
            break;

        case INTF_SPI:
        case INTF_I2C:
        case INTF_CAN:
            m_hidIO.CloseDevice();
            break;

        default:
            break;
    }
}

void ISPCMD::ReOpen_Port(BOOL bForce)
{
    // Re-Open COM Port to clear previous status
    if (bForce || (m_uInterface == INTF_UART)) {
        Close_Port();
        Open_Port();
    }
}

bool ISPCMD::Check_USB_Link()
{
    ScopedMutex scopedLock(m_Mutex);
    return m_bOpenPort ? true : false;
}

unsigned int ISPCMD::GetFirmwareVersion()
{
	return m_uChipInfo[1];
}

unsigned int ISPCMD::GetDevicePDID()
{
	return m_uChipInfo[0];
}

unsigned int ISPCMD::GetChipInfo(unsigned int uIndex)
{
	if (uIndex > (_countof(m_uChipInfo) - 1))
	{
		return 0;
	}

	return m_uChipInfo[uIndex];
}

void ISPCMD::ReadConfig(unsigned int config[])
{
	if (Cmd_GET_CHIP_INFO())
	{
		memcpy(config, &m_uChipInfo[5], 16);
	}
}

void ISPCMD::UpdateConfig(unsigned int addr, unsigned int config[], unsigned int response[])
{
	if (Cmd_WRITE_DATA_EXT(addr, 16, (DWORD *)config))
	{
		memcpy(response, config, 16);
	}
}

void ISPCMD::UpdateFlash(unsigned long start_addr,
						 unsigned long total_len,
						 unsigned long cur_addr,
						 const char *buffer,
						 unsigned long *update_len)
{
	unsigned long write_len = total_len - (cur_addr - start_addr);
	char acBuffer[HID_MAX_PACKET_SIZE_EP - 8 /* cmd, index */];

	if (start_addr == cur_addr)
	{
		if (write_len > sizeof(acBuffer) - 8/*start_addr, total_len*/)
		{
			write_len = sizeof(acBuffer) - 8/*start_addr, total_len*/;
		}

		//unsigned long uCmd = ISPCMD2_WRITE_DATA;
		unsigned long uCmd = ISPCMD2_WRITE_DATA_EXT;

		memcpy(&acBuffer[0], &start_addr, 4);
		memcpy(&acBuffer[4], &total_len, 4);
		memcpy(&acBuffer[8], buffer, write_len);

		WriteFile(
			uCmd,
			acBuffer,
			write_len + 8/*start_addr, total_len*/,
			USBCMD_TIMEOUT_LONG);

		//if (uCmd == ISPCMD2_WRITE_DATA_EXT)
		//	Sleep(2700);

		//if (uCmd == ISPCMD2_WRITE_DATA_EXT)
		//	Sleep(60);

		/* First block need erase the chip, need long timeout */
		ReadFile(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);

		//if (uCmd == ISPCMD2_WRITE_DATA_EXT)
		//	Sleep(70);
	}
	else
	{
		if (write_len > sizeof(acBuffer))
		{
			write_len = sizeof(acBuffer);
		}

		WriteFile(
			0,
			buffer,
			write_len,
			USBCMD_TIMEOUT_LONG);
		ReadFile(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	if (update_len != NULL)
	{
		*update_len = write_len;
	}
}

BOOL ISPCMD::EraseAll()
{
	if (m_iIspType == TYPE_BOOTLOADER)
	{
		return Cmd_ERASE_ALL_FLASH();
	}
	else if (m_iIspType == TYPE_APROM_BL2)
	{
	}

	return FALSE;
}

BOOL ISPCMD::CMD_UART_Connect(unsigned int uResetOption, DWORD dwMilliseconds)
{
	if (((m_iIspType != TYPE_BOOTLOADER) && (m_iIspType != TYPE_APROM_BL2)) || (m_uInterface != INTF_UART)) {
		return TRUE; // Always Pass
	}

	if (!m_bOpenPort) {
		throw _T("There is no Nu-Link connected to a USB port.");
	}

	// BOOL ISPCMD::WriteFile
	DWORD dwLength;

	if (uResetOption)
		m_comIO.SendBreak(5);

	if (m_comIO.WriteFile(m_acPattern, 62, &dwLength, dwMilliseconds))
	{
		if (!m_comIO.ReadFile(m_acBuffer, 1, &dwLength, dwMilliseconds))
		{
			printf("NG in m_comIO.ReadFile\n");
			return FALSE;
		}

		if (m_acBuffer[0] == 'A')
		{
			return TRUE;
		}
	}
	else
	{
		Close_Port();
	}

	return FALSE;
}

BOOL ISPCMD::CMD_Connect(unsigned int uResetOption, DWORD dwMilliseconds)
{
	if ((m_iIspType == TYPE_BOOTLOADER) || (m_iIspType == TYPE_APROM_BL2))
	{
		if (m_uInterface == INTF_SPI)
		{
			if (uResetOption)
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(4000000, 0x0A0A0A0A, 0x50505050, 200, 1))
					return FALSE;
			}
			else
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(4000000, 0x0A0A0A0A, 0x50505050, 12000, 0))
					return FALSE;
			}
		}

		if (m_uInterface == INTF_I2C)
		{
			if (uResetOption)
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(400000, 0xAA, 0xAA, 600, 1))
					return FALSE;
			}
			else
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(400000, 0xAA, 0xAA, 40000, 0))
					return FALSE;
			}
		}

		if (m_uInterface == INTF_CAN)
		{
			if (uResetOption)
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(250000, 0x7812, 0xAA, 0, 1))
					return FALSE;
			}
			else
			{
				if (!Cmd_BRIDGE_CONNECT_TARGET(250000, 0x7812, 0xAA, 0, 0))
					return FALSE;
			}
		}

		return Cmd_GET_CHIP_INFO();
	}

	return FALSE;
}

BOOL ISPCMD::ReadFile2(char *pcBuffer, size_t szMaxLen, DWORD dwMilliseconds, BOOL bCheckIndex)
{
	size_t szPacket = (szMaxLen == 0) ? 12 : 64;

	while (1)
	{
		if (!m_bOpenPort)
			throw _T("There is no Nu-Link connected to a USB port.");

		DWORD dwLength;

		switch (m_uInterface)
		{
			case INTF_HID:
				if (!m_hidIO.ReadFile(m_acBuffer, 65, &dwLength, dwMilliseconds)) {
					return FALSE;
				}

				break;

			case INTF_UART:
				if (!m_comIO.ReadFile(m_acBuffer + 1, szPacket, &dwLength, dwMilliseconds)) {
					printf("NG in m_comIO.ReadFile\n");
					return FALSE;
				}

				break;

			case INTF_SPI:
			case INTF_I2C:
			case INTF_CAN:
				if (!m_hidIO.ReadFile(m_acBuffer, 65, &dwLength, dwMilliseconds)) {
					return FALSE;
				}

				break;

			default:
				return FALSE;
		}

		/* Check if correct package index was read */
		//m_acBuffer[0];	//For HID internal usage, ignored.
		USHORT usCheckSum	= *((USHORT *)&m_acBuffer[3]);
		ULONG uCmdIndex		= *((ULONG *) &m_acBuffer[5]);
		ULONG uCmdResult	= *((ULONG *) &m_acBuffer[9]);

		if ((dwLength >= 12) && (!bCheckIndex || (uCmdIndex == (m_uCmdIndex - 1))))
		{
			if (szMaxLen > dwLength - 12)
			{
				szMaxLen = dwLength - 12;
			}

			if ((pcBuffer != NULL) && (szMaxLen > 0))
			{
				memcpy(pcBuffer, m_acBuffer + 13, szMaxLen);
			}

			return TRUE;
		}
		else
		{
			printf("dwLength = %d, uCmdIndex = %d, %d, usCheckSum = %d, %d\n", dwLength, uCmdIndex, m_uCmdIndex, usCheckSum, m_usCheckSum);
			break;
		}
	}

	return FALSE;
}

BOOL ISPCMD::WriteFile2(unsigned long uCmd, const char *pcBuffer, DWORD dwLen, DWORD dwMilliseconds)
{
	if (!m_bOpenPort)
	{
		throw _T("There is no Nu-Link connected to a USB port.");
	}

	/* Set new package index value */
	DWORD dwCmdLength = dwLen;

	if (dwCmdLength > (sizeof(m_acBuffer) - 9))
	{
		dwCmdLength = (sizeof(m_acBuffer) - 9);
	}

	memset(m_acBuffer, 0, sizeof(m_acBuffer));
	//m_acBuffer[0] = 0x00;	//Always 0x00
	*((ULONG *)&m_acBuffer[1]) = uCmd;
	*((ULONG *)&m_acBuffer[5]) = m_uCmdIndex;

	if ((pcBuffer != NULL) && (dwCmdLength > 0))
	{
		memcpy(m_acBuffer + 9, pcBuffer, dwCmdLength);
	}

	m_usCheckSum = Checksum((unsigned char *)&m_acBuffer[5], sizeof(m_acBuffer) - 5);
	// 2021.05.19 new check sum should include the command id.
	m_usCheckSum += (USHORT)uCmd;
	*((unsigned short *)&m_acBuffer[3]) = m_usCheckSum;
	DWORD dwLength;
	BOOL bRet = FALSE;

	switch (m_uInterface)
	{
		case INTF_HID:
			bRet = m_hidIO.WriteFile(m_acBuffer, 65, &dwLength, dwMilliseconds);
			break;

		case INTF_UART:
			bRet = m_comIO.WriteFile(m_acBuffer + 1, 64, &dwLength, dwMilliseconds);
			break;

		case INTF_SPI:
		case INTF_I2C:
		case INTF_CAN:
			m_acBuffer[2] = static_cast<CHAR>(m_uInterface);
			bRet = m_hidIO.WriteFile(m_acBuffer, 65, &dwLength, dwMilliseconds);
			break;

		default:
			break;
	}

	if (bRet != FALSE)
	{
		m_uCmdIndex += 2;
	}
	else
	{
		Close_Port();
	}

	printf("Write Cmd : %X\n", uCmd);
	return bRet;
}

BOOL ISPCMD::ReadFile(char *pcBuffer, size_t szMaxLen, DWORD dwMilliseconds, BOOL bCheckIndex)
{
	return ReadFile2(pcBuffer, szMaxLen, dwMilliseconds, bCheckIndex);
}

BOOL ISPCMD::WriteFile(unsigned long uCmd, const char *pcBuffer, DWORD dwLen, DWORD dwMilliseconds)
{
	return WriteFile2(uCmd, pcBuffer, dwLen, dwMilliseconds);
}

/* short ack command - 32-bits x 3:
    [ checksum + cmd_id ] + [ packet number ] + [ result ]
      [31:16]    [15:0]          [31:0]           [31:0]
*/

BOOL ISPCMD::Cmd_SET_UART_SPEED(DWORD dwClock)
{
	if (WriteFile2(ISPCMD2_SET_UART_SPEED, (const char *)(&dwClock), 4))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_SET_CAN_SPEED(DWORD dwClock)
{
	if (WriteFile2(ISPCMD2_SET_CAN_SPEED, (const char *)(&dwClock), 4))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_REBOOT_SOURCE(DWORD rebootSrc, DWORD address)
{
	DWORD Input[] = { rebootSrc, address };

	return WriteFile2(ISPCMD2_REBOOT_SOURCE, (const char *)(Input), 8);
}

BOOL ISPCMD::Cmd_GOTO_USBDISP(void)
{
	if (WriteFile2(ISPCMD2_GOTO_USBDISP))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_ERASE_ALL_FLASH(void)
{
	if (WriteFile2(ISPCMD2_ERASE_ALL_FLASH))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_ERASE_PAGE(DWORD address, DWORD page_cnt)
{
	DWORD Input[] = { address, page_cnt };

	if (WriteFile2(ISPCMD2_ERASE_PAGE, (const char *)(Input), 8))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_WRITE_DATA_EXT(DWORD address, DWORD byte_length, DWORD *data)
{
	size_t len = (byte_length < 48) ? byte_length : 48;
	DWORD Input[16] = { address, byte_length };
	memcpy(&Input[2], data, len);

	if (WriteFile2(ISPCMD2_WRITE_DATA_EXT, (const char *)(Input), len + 16))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_WRITE_ROTPK(unsigned short usKeyIndex,
							 unsigned short usKeyAttribute,
							 unsigned long uLen,
							 const char *buffer)
{
	char Input[36];

	memcpy(&Input[0], &usKeyIndex, 2);
	memcpy(&Input[2], &usKeyAttribute, 2);
	memcpy(&Input[4], buffer, uLen);

	if (WriteFile2(ISPCMD2_WRITE_ROTPK, (const char *)(Input), 36))
	{
		return ReadFile2(NULL, 0, USBCMD_TIMEOUT_LONG, TRUE);
	}

	return FALSE;
}

/* long ack command - 32-bits x 16:
   [ checksum + cmd_id ] + [ packet number ] + [ result ] + [ word-0 ~ word-12 ]
     [31:16]    [15:0]          [31:0]           [31:0]
*/

BOOL ISPCMD::Cmd_GET_CHIP_INFO(void)
{
	memset(m_uChipInfo, 0x00, sizeof(m_uChipInfo));

	if (WriteFile2(ISPCMD2_GET_CHIP_INFO))
	{
		// don't check index
		if (ReadFile2((char *)(m_uChipInfo), 52, USBCMD_TIMEOUT_LONG, FALSE))
		{
			if (m_uChipInfo[12] == 0x13572468)
			{
				m_uCmdIndex = 3;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL ISPCMD::Cmd_BRIDGE_CONNECT_TARGET(DWORD dwClock, DWORD dwTxData, DWORD dwRxData, DWORD dwTryCount, DWORD dwOption)
{
	DWORD Input[] = {dwClock, dwTxData, dwRxData, dwTryCount, dwOption};

	if (WriteFile2(ISPCMD2_BRIDGE_CONNECT_TARGET, (const char *)(Input), 20))
	{
		DWORD dwResult;

		if (!ReadFile2((char *)&dwResult, 4, USBCMD_TIMEOUT, FALSE))
			return FALSE;

		if (dwResult == dwRxData)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}
