#ifndef INC__ISP_CMD_H__
#define INC__ISP_CMD_H__
#pragma once


// type
#define TYPE_LDROM		(0)
#define TYPE_BOOTLOADER	(1)
#define TYPE_APROM_BL2	(2)

// interface
#define INTF_HID		(1)
#define INTF_UART		(2)
// OpenNuvoton/NuLink2_ISP_Bridge
// https://github.com/OpenNuvoton/NuLink2_ISP_Bridge
#define INTF_SPI		(3)
#define INTF_I2C		(4)
#define INTF_CAN		(5)


#include "CScopedMutex.hpp"
#include "Interface\CHidIO2.h"
#include "Interface\CUartIO.h"
#define HID_MAX_PACKET_SIZE_EP 64
class CUartIO;

class ISPCMD
{
protected:
	CHAR			m_acBuffer[HID_MAX_PACKET_SIZE_EP + 1];
	CHAR			m_acPattern[HID_MAX_PACKET_SIZE_EP + 1];
	unsigned int	m_uCmdIndex;
	USHORT			m_usCheckSum;

	// Interface
	ULONG			m_uInterface;
	ULONG			m_uUSB_PID;		// for compatibility
	CString			m_strComNum;
	CHidIO2			m_hidIO;
	CUartIO			m_comIO;
	BOOL			m_bOpenPort;
	CMutex2			m_Mutex;
	//
	DWORD			m_dwClock;

	BOOL ReadFile(char *pcBuffer, size_t szMaxLen, DWORD dwMilliseconds, BOOL bCheckIndex = TRUE);
	BOOL WriteFile(unsigned long uCmd, const char *pcBuffer = NULL, DWORD dwLen = 0, DWORD dwMilliseconds = 20/*USBCMD_TIMEOUT*/);

	// For ISP Protocol 2
	BOOL ReadFile2(char *pcBuffer, size_t szMaxLen, DWORD dwMilliseconds, BOOL bCheckIndex = TRUE);
	BOOL WriteFile2(unsigned long uCmd, const char *pcBuffer = NULL, DWORD dwLen = 0, DWORD dwMilliseconds = 20/*USBCMD_TIMEOUT*/);

public:
	int m_iIspType;

	ISPCMD();
	virtual ~ISPCMD();

	bool Check_USB_Link();
	bool Open_Port();
	void Close_Port();
	void ReOpen_Port(BOOL bForce = FALSE);

	unsigned short Checksum(const unsigned char *buf, int len)
	{
		int i;
		unsigned short c;

		for (c = 0, i = 0; i < len; i++)
		{
			c += buf[i];
		}

		return (c);
	}

	BOOL CMD_UART_Connect(unsigned int uResetOption, DWORD dwMilliseconds = 10);
	BOOL CMD_Connect(unsigned int uResetOption, DWORD dwMilliseconds = 30);

	unsigned int GetFirmwareVersion();
	unsigned int GetDevicePDID();
	unsigned int GetChipInfo(unsigned int uIndex);
	void ReadConfig(unsigned int config[]);
	void UpdateConfig(unsigned int addr, unsigned int config[], unsigned int response[]);

	void UpdateFlash(unsigned long start_addr,
					 unsigned long total_len,
					 unsigned long cur_addr,
					 const char *buffer,
					 unsigned long *update_len);

	void UpdateROTPK(unsigned long uIndex,
					 unsigned long uAttribute,
					 unsigned long uLen,
					 const char *buffer);

	BOOL EraseAll();

	// it = 1 for HID, str is ignored.
	// it = 2 for UART, str as "COM5".
	void SetInterface(unsigned int it, CString str, DWORD dwClock = 115200)
	{
		m_uInterface = it;
		m_strComNum = str;
		m_dwClock = dwClock;
	};

	CString m_strDevPathName;

	enum
	{
		/* short ack command - 32-bits x 3:
			[ checksum + cmd_id ] + [ packet number ] + [ result ]
			  [31:16]    [15:0]          [31:0]           [31:0]
		 */
		ISPCMD2_REBOOT_SOURCE			= 0x00B0,	// por, cpu, sys, specify base(vecmap+vtor)
		ISPCMD2_WRITE_DATA				= 0x00D0,   // write directly
		ISPCMD2_WRITE_DATA_EXT			= 0x00D1,	// erase then write

		ISPCMD2_WRITE_ROTPK				= 0x00D3,	// Write ROTPK

		ISPCMD2_GOTO_USBDISP			= 0x00C0,   // change to USB mode
		ISPCMD2_SET_UART_SPEED			= 0x00C1,   // 115200, 230400, 460800, 921600
		ISPCMD2_SET_CAN_SPEED			= 0x00C2,   // 250k, 500k, 750k, 1000k

		ISPCMD2_ERASE_PAGE				= 0x00E0,   // easre according address and page counts
		ISPCMD2_ERASE_ALL_FLASH			= 0x00E1,   // erase APROM, LDROM

		/* long ack command - 32-bits x 16:
			[ checksum + cmd_id ] + [ packet number ] + [ result ] + [ word-0 ~ word-12 ]
			  [31:16]    [15:0]          [31:0]           [31:0]
		 */
		ISPCMD2_GET_CHIP_INFO			= 0x00A0,	// connect, the first command
		ISPCMD2_READ_DATA				= 0x00D2,

		ISPCMD2_BRIDGE_CONNECT_TARGET	= 0x005A,	// Connect to Target. For Tool to Bridge usage.
	};

	BOOL Cmd_SET_UART_SPEED(DWORD dwClock);
	BOOL Cmd_SET_CAN_SPEED(DWORD dwClock);
	BOOL Cmd_REBOOT_SOURCE(DWORD rebootSrc, DWORD address);
	BOOL Cmd_GOTO_USBDISP(void);
	BOOL Cmd_ERASE_ALL_FLASH(void);
	BOOL Cmd_ERASE_PAGE(DWORD address, DWORD page_cnt);
	BOOL Cmd_GET_CHIP_INFO(void);
	BOOL Cmd_BRIDGE_CONNECT_TARGET(DWORD dwClock, DWORD dwTxData, DWORD dwRxData, DWORD dwTryCount, DWORD dwOption);

	DWORD m_uChipInfo[13];	// 0: PDID
							// 1: Version
							// 2: APROM Size
							// 3: LDROM Size
							// 4: Reserved
							// 5: CONFIG0
							// 6: CONFIG1
							// 7: CONFIG2
							// 8: CONFIG3
							// 9~11: Reserved
							// 12: Constant

	BOOL Cmd_WRITE_DATA_EXT(DWORD address, DWORD length, DWORD *data);

	BOOL Cmd_WRITE_ROTPK(unsigned short usKeyIndex, unsigned short usKeyAttribute, unsigned long uLen, const char *buffer);
};


#define CMD_RST_SRC_CHIP            (0x1ul)
#define CMD_RST_SRC_CPU             (0x2ul) // + vecmap addr (512-bytes alignment)
#define CMD_RST_SRC_SYS             (0x4ul) // + vecmap addr (512-bytes alignment)
#define CMD_EXEC_ADDR               (0x8ul) // + vrot addr (1024-bytes alignment)

#endif
