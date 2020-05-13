#include <RoomLib.h>
#include <ndis.h>
#include <nuiouser.h>
#include "Iphlpapi.h"
#include <ws2tcpip.h>

#pragma comment(lib, "Iphlpapi.lib")

static int m_iNetState = 0;
static HANDLE h_NetServer = NULL;
static BOOL IsNetServerStart = FALSE;
BOOL g_isConflict = FALSE;
static void
IPAddrToStr(LPTSTR szStr, DWORD IPAddr)
{
	wsprintf(szStr, TEXT("%d.%d.%d.%d"), IPAddr & 0xFF,(IPAddr >> 8) & 0xFF,
		(IPAddr >> 16) & 0xFF,(IPAddr >> 24) & 0xFF);
}

static BOOL ReBindNdis(wchar_t * wszNDName)
{
	if(!wszNDName||wcscmp(wszNDName,L"")==0)
	{
		DBGMSG(DPERROR, "NDName NULL or empty,ReBindNdis false\n");
		return FALSE;
	}
	HANDLE hNdis = CreateFile(L"NDS0:", 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) INVALID_HANDLE_VALUE);
	if( hNdis == INVALID_HANDLE_VALUE )
	{
		DBGMSG(DPERROR, "CreateFile err,ReBindNdis false\n");
		return FALSE;
	}
	wszNDName[wcslen(wszNDName)+1] = 0x00;

	// Send the device command.
	if (!DeviceIoControl(hNdis,
		IOCTL_NDIS_REBIND_ADAPTER,
		wszNDName,
		(_tcslen( wszNDName)+2) * sizeof( WCHAR ),    // buf contains the name of the
		NULL,
		0,
		NULL,
		NULL))
	{
		DWORD dret = ::GetLastError();
		DBGMSG(DPERROR, "ReBindNdis GetLastError:%x\n",dret);
		CloseHandle( hNdis );
		return FALSE;
	}

	CloseHandle( hNdis );
	return TRUE;
}
/*
*	@param: szNDName Buffer to store NetCard Name
*/
static HANDLE GetEthNDName(wchar_t * wszNDName)
{
	DWORD   DesiredAccess = 0x0;
	PTCHAR  ptcDeviceName = NULL;
	PTCHAR  g_ptcDriverName = (PTCHAR)NDISUIO_DEVICE_NAME;

	HANDLE g_hNdisUio = CreateFile(
		g_ptcDriverName,                                  //    Object name.
		DesiredAccess,                                    //    Desired access.
		0x00,                                             //    Share Mode.
		NULL,                                             //    Security Attr
		OPEN_EXISTING,                                    //    Creation Disposition.
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,     //    Flag and Attributes..
		(HANDLE)INVALID_HANDLE_VALUE);    

	if (g_hNdisUio != INVALID_HANDLE_VALUE)
	{
		DWORD                     dwBytesWritten = 0;
		CHAR                     Buf[1040];
		DWORD                    i = 0;
		DWORD                    dwBufLength = sizeof(Buf);
		PNDISUIO_QUERY_BINDING    pQueryBinding;

		pQueryBinding = (PNDISUIO_QUERY_BINDING)Buf;

		// Iterate through each device
		for(pQueryBinding->BindingIndex = i; /*Nothing*/; pQueryBinding->BindingIndex = ++i)
		{
			// Get device name
			if (DeviceIoControl(
				g_hNdisUio,
				IOCTL_NDISUIO_QUERY_BINDING,
				pQueryBinding,
				sizeof(NDISUIO_QUERY_BINDING),
				Buf,
				dwBufLength,
				&dwBytesWritten,
				NULL))
			{
				// Get Device name
				ptcDeviceName = (PTCHAR)((PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset);        
				wcscpy(wszNDName,ptcDeviceName);
				//				DBGMSG(DPINFO, L"GetEthNDName	Ethernet device name is %s\r\n", wszNDName);
			}
			else
			{
				break;
			}
		}
	}
	return g_hNdisUio;
}

DWORD GetIpAddress(void)
{
	DWORD				dwRetVal			= NO_ERROR;
	PIP_ADAPTER_INFO	pAdapterInfo		= NULL;
	PIP_ADAPTER_INFO	pAdapterInfoHead	= NULL;
	ULONG				ulBufferSize		= 0;
	TCHAR*				pszAdapterName		= NULL;
	DWORD				cdwAdapterName		= 0;
	DWORD retip = 0;

	// Get IP Adapters Info from IPhelper
	dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
	if(dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)LocalAlloc(LMEM_FIXED, ulBufferSize);
		if(pAdapterInfo == NULL)
		{
			dwRetVal = ERROR_OUTOFMEMORY;
			goto exit;
		}

		// Set a pointer to the head of the list
		pAdapterInfoHead = pAdapterInfo;

		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
		if(dwRetVal != NO_ERROR)
		{
			goto exit;
		}
	}
	else if(dwRetVal != NO_ERROR)
	{
		goto exit;
	}

	// If no adapter interface is present
	if(ulBufferSize == 0)
	{
		dwRetVal = ERROR_INVALID_DATA;
		goto exit;
	}	 

	// Check if the Public Interface Adapter is connected
	do
	{
		DWORD cdwCurrAdapterName = strlen(pAdapterInfo->AdapterName)+1;
		if(cdwCurrAdapterName > cdwAdapterName)
		{
			// Free the existing buffer (if it exists) 
			if(pszAdapterName)
			{
				LocalFree(pszAdapterName);
				pszAdapterName = NULL;
			}

			// Allocate some memory for the adapter name string.  Make this large enough so 
			// that we probably won't have to reallocate more later.
			cdwAdapterName	   = cdwCurrAdapterName + 10;
			pszAdapterName	  = (WCHAR*)LocalAlloc(LMEM_FIXED, cdwAdapterName*sizeof(TCHAR));
			if(pszAdapterName == NULL)
			{
				dwRetVal = ERROR_OUTOFMEMORY;
				goto exit;
			}
		}

		mbstowcs(pszAdapterName, pAdapterInfo->AdapterName, cdwCurrAdapterName);
		//		DBGMSG(DPERROR, L"%s UpdateConnectionStatus \r\n", pszAdapterName);

		retip = inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
	}	while(pAdapterInfo != NULL);
exit:
	if(pszAdapterName)
	{
		LocalFree(pszAdapterName);
	}

	if(pAdapterInfoHead)
	{
		LocalFree(pAdapterInfoHead);
	}

	return retip;
}

BOOL SetNetDhcp(void)
{
	WCHAR Names[50] = L"";
	WCHAR wszKeyName[255] = L"";
	HKEY   hkey; 
	DWORD   value = 1;
	HANDLE hndis;

	hndis = GetEthNDName(Names);
	if(hndis == INVALID_HANDLE_VALUE)
	{
		DBGMSG(DPERROR, "GetEthNDName error!\r\n");
		return FALSE;
	}
	CloseHandle(hndis);
	wsprintf(wszKeyName,L"Comm\\%s\\Parms\\TCPIP",Names);

	//打开注册表对网卡IP信息对应子健进行修改
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszKeyName, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress Open reg error\r\n");
		return FALSE;
	}

	//set   EnableDHCP   
	if(RegSetValueEx(hkey, TEXT("EnableDHCP"), 0, REG_DWORD, (const unsigned char*)&value, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress Dhcp\r\n");
		return FALSE;
	}

	RegFlushKey(hkey);
	RegCloseKey(hkey); 

	return ReBindNdis(Names); 
}

BOOL 
SetIPAddress(wchar_t *wszIPAddr, wchar_t *wszMask, wchar_t *wszGateway)
{
	WCHAR Names[50] = L"";
	WCHAR wszKeyName[255] = L"";
	HKEY   hkey; 
	DWORD   value = 0;
	HANDLE hndis;

	DBGMSG(DPINFO, "SetIPAddress %S %S %S\r\n", wszIPAddr, wszMask, wszGateway);
	hndis = GetEthNDName(Names);
	if(hndis == INVALID_HANDLE_VALUE)
	{
		DBGMSG(DPERROR, "GetEthNDName error!\r\n");
		return FALSE;
	}
	CloseHandle(hndis);
	wsprintf(wszKeyName,L"Comm\\%s\\Parms\\TCPIP",Names);

	//打开注册表对网卡IP信息对应子健进行修改
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszKeyName, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress Open reg error\r\n");
		return FALSE;
	}

	//set   EnableDHCP   
	if(RegSetValueEx(hkey, TEXT("EnableDHCP"), 0, REG_DWORD, (const unsigned char*)&value, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress Dhcp\r\n");
		return FALSE;
	}

	//set   ip   address   	
	if(RegSetValueEx(hkey, TEXT("IpAddress"), 0, REG_MULTI_SZ, (const unsigned char*)wszIPAddr, lstrlen(wszIPAddr)*2 + 2) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress ip error");
		return FALSE;
	}

	//set   subnet   mask 	
	if(RegSetValueEx(hkey, TEXT("SubnetMask"), 0, REG_MULTI_SZ, (const unsigned char *)wszMask, lstrlen(wszMask)*2+2) != ERROR_SUCCESS)
	{
		DBGMSG(DPERROR, "SetIPAddress mask error\r\n");
		return FALSE;
	}

	//set   gateway
	if(RegSetValueEx(hkey, TEXT("DefaultGateway"), 0, REG_MULTI_SZ, (const unsigned char*)wszGateway, lstrlen(wszGateway)*2+2) != ERROR_SUCCESS)   
	{
		DBGMSG(DPERROR, "SetIPAddress gw error\r\n");
		return FALSE;
	}

	RegFlushKey(hkey);
	RegCloseKey(hkey); 

	return ReBindNdis(Names); 
}

BOOL 
SetIPAddress(int ip, int mask, int gw)
{
	wchar_t IpAddr[20], MaskAddr[20], GwAddr[20];
	IPAddrToStr(IpAddr, ip);
	IPAddrToStr(MaskAddr, mask);
	IPAddrToStr(GwAddr, gw);
	return SetIPAddress(IpAddr, MaskAddr, GwAddr);
}

BOOL GetNetworkcardInfo(unsigned char * pMac)
{
	BOOL bget = FALSE;
	PIP_ADAPTER_INFO pAdapterInfo = NULL; 
	ULONG uLen = 0; 
	GetAdaptersInfo(pAdapterInfo, &uLen);  
	pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, uLen);  
	if(ERROR_SUCCESS == GetAdaptersInfo(pAdapterInfo,&uLen))
	{  
		//解析适配器结构体输出适配器信息   
		if(pAdapterInfo)  
		{
			memcpy(pMac,pAdapterInfo->Address,6);	//-V512_UNDERFLOW_OFF
			bget = TRUE;
		}  
	}  
	GlobalFree(pAdapterInfo);
	return bget;
}

static BOOL UpdateConnectionStatus(PTCHAR ptcDeviceName)
{
	DWORD               dwRetVal            = NO_ERROR;
	PIP_ADAPTER_INFO    pAdapterInfo        = NULL;
	PIP_ADAPTER_INFO    pAdapterInfoHead    = NULL;
	ULONG               ulBufferSize        = 0;
	TCHAR*              pszAdapterName      = NULL;
	DWORD               cdwAdapterName      = 0;
	BOOL                fDisConnected       = TRUE;
	BOOL ret = TRUE;

	// Get IP Adapters Info from IPhelper
	dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
	if(dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)LocalAlloc(LMEM_FIXED, ulBufferSize);
		if(pAdapterInfo == NULL)
		{
			dwRetVal = ERROR_OUTOFMEMORY;
			goto exit;
		}

		// Set a pointer to the head of the list
		pAdapterInfoHead = pAdapterInfo;

		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
		if(dwRetVal != NO_ERROR)
		{
			goto exit;
		}
	}
	else if(dwRetVal != NO_ERROR)
	{
		goto exit;
	}

	// If no adapter interface is present
	if(ulBufferSize == 0)
	{
		dwRetVal = ERROR_INVALID_DATA;
		goto exit;
	}    

	// Check if the Public Interface Adapter is connected
	do
	{
		DWORD cdwCurrAdapterName = strlen(pAdapterInfo->AdapterName)+1;
		if(cdwCurrAdapterName > cdwAdapterName)
		{
			// Free the existing buffer (if it exists) 
			if(pszAdapterName)
			{
				LocalFree(pszAdapterName);
				pszAdapterName = NULL;
			}

			// Allocate some memory for the adapter name string.  Make this large enough so 
			// that we probably won't have to reallocate more later.
			cdwAdapterName     = cdwCurrAdapterName + 10;
			pszAdapterName    = (WCHAR*)LocalAlloc(LMEM_FIXED, cdwAdapterName*sizeof(TCHAR));
			if(pszAdapterName == NULL)
			{
				dwRetVal = ERROR_OUTOFMEMORY;
				goto exit;
			}
		}

		mbstowcs(pszAdapterName, pAdapterInfo->AdapterName, cdwCurrAdapterName);

		if(_tcscmp(ptcDeviceName, pszAdapterName) == 0)
		{
			if(strcmp(pAdapterInfo->IpAddressList.IpAddress.String, "0.0.0.0") == 0)
			{
				DBGMSG(DPERROR, "Change IP address fail,error is %d \r\n",GetLastError());
				if((m_iNetState & WIRE_PLUGIN)!= 0)
				{
					g_isConflict = TRUE;
					m_iNetState &= ~ADDRESS_SET;
					DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
				}
				else
				{
					//当网线拔出，程序会到这里，但此时不是ip冲突
				}
			}
			else
			{
				DBGMSG(DPERROR, "Change IP address OK!\r\n");
				m_iNetState = WIRE_PLUGIN | ADDRESS_SET;
				DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
			}

			dwRetVal            = NO_ERROR;
			fDisConnected        = FALSE;
			break;
		}

		pAdapterInfo = pAdapterInfo->Next;
	}
	while(pAdapterInfo != NULL);

exit:

	if(pszAdapterName)
	{
		LocalFree(pszAdapterName);
	}

	if(pAdapterInfoHead)
	{
		LocalFree(pAdapterInfoHead);
	}

	if(fDisConnected)
	{
		dwRetVal = ERROR_DEV_NOT_EXIST;
	}

	if(dwRetVal != NO_ERROR)
	{
		RETAILMSG(1, (TEXT("ETHMAN: Error updating connection status: %d\r\n"), GetLastError()));
		ret = FALSE;
	}
	return ret;
}


static DWORD NetStateThread(HANDLE param)
{
	TCHAR	szDeviceName[50] = {0};
	HANDLE	hndis;
	HANDLE	hQueue[4];
	SOCKET	AddrChangeSock = INVALID_SOCKET;
	WSAOVERLAPPED   WSAOverlapped;
	int				Status;
	DWORD dwRetVal;
	HANDLE hWnd = (HANDLE)param;

begin:
	DBGMSG(DPINFO, "NetStateThread start\r\n");

	hQueue[0] = NULL;
	hQueue[1] = NULL;
	hQueue[2] = CreateEvent(NULL,FALSE,FALSE,_T("USB_NET_ERR_REBOOT"));
	hndis = GetEthNDName(szDeviceName);
	wprintf(L"------GetEthNDName is %s-----\r\n", szDeviceName);
	if(hndis == INVALID_HANDLE_VALUE)
	{
		DBGMSG(DPERROR, "GetEthNDName error!\r\n");
		return 0;
	}

	// Request Notify of NDISUIO 
	{
		MSGQUEUEOPTIONS 			   sOptions;
		NDISUIO_REQUEST_NOTIFICATION   sRequestNotification;
		//
		//    First stop create the msg queue..
		//
		sOptions.dwSize            = sizeof(MSGQUEUEOPTIONS);
		sOptions.dwFlags           = 0;
		sOptions.dwMaxMessages     = 4;
		sOptions.cbMaxMessage      = sizeof(NDISUIO_DEVICE_NOTIFICATION);
		sOptions.bReadAccess       = TRUE;

		hQueue[0] = CreateMsgQueue(NULL, &sOptions);
		if (hQueue[0] == NULL)
		{
			DBGMSG(DPERROR, "Could not create message queue for Ndis notifications\r\n");
			goto Fail;
		}

		//
		//    Queue created successfully, tell NDISUIO about it..
		//    Oh ya, we want all type of notifications :)..
		//

		sRequestNotification.hMsgQueue              = hQueue[0];
		sRequestNotification.dwNotificationTypes    = 0xffffffff;

		if (!DeviceIoControl(hndis,
			IOCTL_NDISUIO_REQUEST_NOTIFICATION,
			&sRequestNotification,
			sizeof(NDISUIO_REQUEST_NOTIFICATION),
			NULL,
			0x00,
			NULL,
			NULL))
		{
			DBGMSG(DPERROR, "Error invoking Ndis Ioctl method\r\n");
			goto Fail;
		}
	}
	{
		AddrChangeSock = socket(AF_INET, SOCK_STREAM, 0);
		if (AddrChangeSock == INVALID_SOCKET)
		{
			DBGMSG(DPERROR, "Failed socket() call.\r\n");
			return FALSE;
		}

		hQueue[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hQueue[1] == NULL)
		{
			DBGMSG(DPERROR, "Failed CreateEvent() in IPNotificationThread().\r\n");
			goto Fail;
		}  

		//
		//  Now request for the notification..
		//
		memset (&WSAOverlapped, 0x00, sizeof(WSAOVERLAPPED));
		WSAOverlapped.hEvent = hQueue[1];    

		Status = WSAIoctl(AddrChangeSock, 
			SIO_ADDRESS_LIST_CHANGE, 
			NULL, 
			0, 
			NULL, 
			0, 
			NULL, 
			&WSAOverlapped, 
			NULL);   
		if (Status != ERROR_SUCCESS && GetLastError() != ERROR_IO_PENDING)
		{
			DBGMSG(DPERROR, "Failed WSAIoctl() for SIO_ADDRESS_LIST_CHANGE.\r\n");
			goto Fail;
		}
	}
	while(IsNetServerStart)
	{
		dwRetVal = WaitForMultipleObjects(3, hQueue, FALSE, 1000);
		if(dwRetVal == WAIT_OBJECT_0 + 1)
		{
			Status = WSAIoctl(
				AddrChangeSock, 
				SIO_ADDRESS_LIST_CHANGE, 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL, 
				&WSAOverlapped, 
				NULL);
			if (Status != ERROR_SUCCESS && GetLastError() != ERROR_IO_PENDING)
				break;
			if(!UpdateConnectionStatus(szDeviceName))
			{
			}
		}
		else if(dwRetVal == WAIT_OBJECT_0)
		{
			NDISUIO_DEVICE_NOTIFICATION    sDeviceNotification;
			DWORD						   dwBytesReturned;
			DWORD						   dwFlags;
			if(ReadMsgQueue(hQueue[0], &sDeviceNotification, sizeof(NDISUIO_DEVICE_NOTIFICATION), &dwBytesReturned,	0, &dwFlags))
			{
				PTCHAR     ptcDeviceName = NULL;

				// The device name should be in uppercase to query NdisUIO
				ptcDeviceName = _wcsdup(sDeviceNotification.ptcDeviceName);
				if (!ptcDeviceName)
				{
					DBGMSG(DPERROR, "Could not allocate memory for string \"%s\"\r\n",  sDeviceNotification.ptcDeviceName);
					continue;
				}

				_wcsupr(ptcDeviceName);

				switch(sDeviceNotification.dwNotificationType)
				{
				case NDISUIO_NOTIFICATION_MEDIA_CONNECT:
					DBGMSG(DPERROR, "===================>>>>>> Net cable is inserted!\r\n");
					m_iNetState |= WIRE_PLUGIN;
					DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
					break;
				case NDISUIO_NOTIFICATION_MEDIA_DISCONNECT:
					DBGMSG(DPERROR, "===================>>>>>> Net cable is not inserted!\r\n");
					m_iNetState &= ~WIRE_PLUGIN;
					DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
					break;
				}

				LocalFree(ptcDeviceName);
			}    
		}   
		else if(dwRetVal == WAIT_OBJECT_0 + 2)
		{
			DBGMSG(DPERROR, "Fine usb net work error.\r\n");
			DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
		}
	}
Fail:
	if (AddrChangeSock != INVALID_SOCKET)
		closesocket(AddrChangeSock);

	if (hQueue[1])
		CloseHandle(hQueue[1]);

	if(hndis != INVALID_HANDLE_VALUE)
		CloseHandle(hndis);

	if(hQueue[0])
		CloseHandle(hQueue[0]);    

	DBGMSG(DPINFO, "NetStateThread end\r\n");
	return 0;
}

BOOL StartNetMonitor(void)
{
	WCHAR Names[50] = L"";
	DBGMSG(DPINFO, "StartNetMonitor start\r\n");
#ifndef _DEBUG
	while(1)
	{
		HANDLE hndis;
		// 等待网络驱动初始化成功
		hndis = GetEthNDName(Names);
		if(hndis == INVALID_HANDLE_VALUE)
		{
			DBGMSG(DPERROR, "GetEthNDName error!\r\n");
			continue;
		}
		CloseHandle(hndis);
		if(wcslen(Names) > 0)
		{
			wprintf(L"------GetEthNDName is %s-----\r\n", Names);
			break;
		}
	}
#endif	
	IsNetServerStart = TRUE;
	h_NetServer = CreateThread(NULL, NULL, NetStateThread, NULL, 0, NULL);
	DBGMSG(DPINFO, "StartNetMonitor end\r\n");

	return TRUE;
}

void StopNetMonitor(void)
{
	DBGMSG(DPINFO, "StopNetMonitor start\r\n");
	IsNetServerStart = FALSE;
	while(1)
	{
		if(WaitForSingleObject(h_NetServer, 1000) == WAIT_OBJECT_0)
			break;
	}
	CloseHandle(h_NetServer);
	DBGMSG(DPINFO, "StopNetMonitor end\r\n");
}

int GetNetState()
{
	return m_iNetState;
}

void SetNetState(BOOL bConflict)
{
	
}

int GetMyselfIP(void)  
{  
	PIP_ADAPTER_INFO pAdapterInfo = NULL; 
	ULONG uLen = 0; 
	GetAdaptersInfo(pAdapterInfo, &uLen);  
	pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, uLen); 
	int ip = 0;
	if(ERROR_SUCCESS == GetAdaptersInfo(pAdapterInfo,&uLen))
	{  
		//解析适配器结构体输出适配器信息   
		if(pAdapterInfo)  
		{
			fprintf(stderr, "\r\nHost itself IP:%s\r\n", pAdapterInfo->IpAddressList.IpAddress.String);
			ip = inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
		}  
	}  
	GlobalFree(pAdapterInfo);  
	return ip;
}  

DWORD GetIpAddressEx(char* szName)
{
	return 0;
}