#include "roomlib.h"
#include <Pkfuncs.h>
#include <Diskio.h>
#include "DPGpio.h"

static HANDLE m_hWatchDog = INVALID_HANDLE_VALUE ;
static DWORD refreshtime = 16000;

BOOL AdjustScreen(DWORD bright, DWORD contrast, DWORD saturation)
{
	BklParam curbkl;
	curbkl.bright = bright;
	curbkl.contrast = contrast;
	curbkl.saturation = saturation;
	printf("Set Screen %u %u %u\r\n",bright, contrast, saturation);
	HANDLE hdis = CreateFile(_T("DIS1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hdis == INVALID_HANDLE_VALUE)
	{	
		printf("Open DIS1 fail error is %u\r\n",GetLastError());
		return FALSE;
	}

	if(!DeviceIoControl(hdis,IOCTL_DISP_ADJUST_BKL,&curbkl,sizeof(BklParam),NULL,0,NULL,NULL))
	{
		printf("Get BKL fail error is %u\r\n",GetLastError());
		return FALSE;
	}
	CloseHandle(hdis);
	return TRUE;
}

BOOL AdjustCscparam(DWORD bright, DWORD contrast, DWORD saturation, DWORD hue)
{
	CscParam param;
	param.bright = bright;
	param.contrast = contrast;
	param.saturation = saturation;
	param.hue = hue;
	HANDLE hdis = CreateFile(_T("DIS1:"),GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(hdis == INVALID_HANDLE_VALUE)
	{
		printf("Open DIS1 fail error is %u\r\n",GetLastError());
		return FALSE;
	}
	if(!DeviceIoControl(hdis, IOCTL_DISP_ADJUST_FE, &param, sizeof(CscParam), NULL, 0, NULL, NULL))
	{
		printf("Set cscparam fail error is %u\r\n", GetLastError());
		return FALSE;
	}
	CloseHandle(hdis);
	return TRUE;
}

void SetGpioVal(HANDLE hdev, UINT8 value)
{
	DWORD dwRet;
	if(hdev == NULL)
		return;
	if(!DeviceIoControl(hdev,IOCTRL_SET_GPIO_VALUE,(LPVOID)&value,1,NULL,0,&dwRet,NULL))
	{
		DBGMSG(DPWARNING, "IOCTRL_SET_GPIO_VALUE  fail error is %d\r\n", GetLastError());
	}
}

UINT8 GetGpioVal(HANDLE hdev)
{
	UINT8 data = 0;
	DWORD bytesreturned;

	if(!DeviceIoControl(hdev, IOCTRL_GET_GPIO_VALUE, (LPVOID)&data, 1, NULL, 0, &bytesreturned,NULL))
	{
		DBGMSG(DPWARNING, "IOCTRL_GET_GPIO_VALUE  fail error is %d\r\n",GetLastError());
	}
	return data;
}

HANDLE InitGpio(DWORD pin, DWORD inout, DWORD value)
{
	HANDLE hHandle;
	GPIO_CFG gpio_cfg;
	DWORD dwRet;

	hHandle = CreateFile(_T("PIO1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hHandle == INVALID_HANDLE_VALUE)
	{	
		DBGMSG(DPERROR, "Open PIO1 fail error is %d\r\n",GetLastError());
		return NULL;
	}

	memset(&gpio_cfg,0,sizeof(GPIO_CFG));
	gpio_cfg.gpio_num= (WORD)pin;
	gpio_cfg.gpio_cfg = (BYTE)inout;
	gpio_cfg.gpio_pull = (BYTE)value;
	if(!DeviceIoControl(hHandle, IOCTRL_GPIO_INIT, (LPVOID)&gpio_cfg, sizeof(GPIO_CFG), NULL, 0, &dwRet, NULL))
	{
		DBGMSG(DPERROR, "IOCTRL_GPIO_INIT  %d %d %d fail error is %d\r\n", pin, inout, value, GetLastError());
		CloseHandle(hHandle);
		return NULL;
	}
	return hHandle;
}

void DeinitGpio(HANDLE hGpio)
{
	CloseHandle(hGpio);
}

BOOL StartWatchDog(void)
{
	DWORD bytesreturned;
	if(m_hWatchDog == INVALID_HANDLE_VALUE )
	{
		m_hWatchDog = CreateFile(_T("RTC1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if(m_hWatchDog == INVALID_HANDLE_VALUE)
		{
			DBGMSG(DPERROR, "open watchdog device fail\r\n");
			return FALSE;
		}
	}
	if(!DeviceIoControl(m_hWatchDog,IOCTL_START_WATCHDOG,NULL,NULL,NULL,0,&bytesreturned,NULL))
	{
		DBGMSG(DPERROR, "IOCTL_START_WATCHDOG  fail error is %d\r\n",GetLastError());
		goto end;
	}

	if(!DeviceIoControl(m_hWatchDog,IOCTL_REFRESH_WATCHDOG,(LPVOID)&refreshtime,sizeof(DWORD),NULL,0,&bytesreturned,NULL))
	{
		DBGMSG(DPERROR, "IOCTL_REFRESH_WATCHDOG  fail error is %d\r\n",GetLastError());
		goto end;
	}
	return TRUE;
end:
	DeviceIoControl(m_hWatchDog,IOCTL_STOP_WATCHDOG,NULL,0,NULL,0,&bytesreturned,NULL);
	CloseHandle(m_hWatchDog);
	m_hWatchDog = INVALID_HANDLE_VALUE ;
	return FALSE;
}

void StopWatchDog(void)
{
	DWORD bytesreturned;
	if(m_hWatchDog != INVALID_HANDLE_VALUE )
	{
		DeviceIoControl(m_hWatchDog,IOCTL_STOP_WATCHDOG,NULL,0,NULL,0,&bytesreturned,NULL);
		CloseHandle(m_hWatchDog);
		m_hWatchDog = NULL;
	}
}

void FeedWatchDog(void)
{
	DWORD bytesreturned;
	if(m_hWatchDog != INVALID_HANDLE_VALUE )
		DeviceIoControl(m_hWatchDog,IOCTL_REFRESH_WATCHDOG,(LPVOID)&refreshtime,sizeof(DWORD),NULL,0,&bytesreturned,NULL);
}

void RebootSystem(void)
{
	DWORD	bytesreturned;
	KernelIoControl(IOCTL_HAL_REBOOT, NULL,0,NULL,0,&bytesreturned);
}

void ScanDisk(void)
{
	HANDLE hVolume;
	DWORD lpBytesReturned;
	HANDLE	hFile;
	WIN32_FIND_DATA   FindData;
	wchar_t tmpFile[255];

	hVolume = CreateFile(TEXT("\\UserDev\\Vol:"), 	GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); 
	DeviceIoControl(hVolume, IOCTL_DISK_SCAN_VOLUME, NULL, 0, NULL, 0, &lpBytesReturned, NULL);
	CloseHandle(hVolume);

	hFile = FindFirstFile(L"\\UserDev\\*.chk", &FindData);
	if (hFile != NULL)
	{
		do{
			wsprintf(tmpFile, L"\\UserDev\\%s", FindData.cFileName);
			DeleteFile(tmpFile);
		}while(FindNextFile(hFile, &FindData));
		FindClose(hFile);
	}
}

HANDLE InitSpr(void)
{
	HANDLE h_hspr;
	DWORD bg = 0xC2BCBE;
	h_hspr = CreateFile(_T("SPR1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(h_hspr == INVALID_HANDLE_VALUE)
	{
		printf("CreateSprFile error\n");
		return NULL;
	}
	DeviceIoControl(h_hspr, IOCTL_SET_BKGRD, &bg, 4, NULL, NULL, NULL, NULL);
	return h_hspr;
}

void DeinitSpr(HANDLE h_hspr)
{
	CloseHandle(h_hspr);
}

void DPSprCtrl(HANDLE h_hspr, DWORD ioctl, void* data, DWORD len)
{
	DeviceIoControl(h_hspr, ioctl, data, len, NULL, NULL, NULL, NULL);
}

void* DPVirtualAlloc(DWORD size)
{
	return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,PAGE_READWRITE);
}

void DPVirtualFree(void* pMem)
{
	VirtualFree(pMem, 0, MEM_RELEASE);
}

BOOL DPPhisycalAlloc(HANDLE h_hspr, Block_Req* frame, DWORD memsize, DWORD prop)
{
	BOOL bret;
	frame->viraddr = (DWORD*)VirtualAlloc(0, memsize, MEM_RESERVE,PAGE_NOACCESS);
	if(frame->viraddr == NULL)
	{
		printf("SprReqRollBar VirtualAlloc %u fail\r\n", memsize);
		return FALSE;
	}

	if(prop == 0)
		bret = DeviceIoControl(h_hspr, IOCTL_BAR_REQUEST, frame, sizeof(Block_Req), NULL, NULL, NULL, NULL);
	else if(prop == 1)
		bret = DeviceIoControl(h_hspr, IOCTL_FRM_REQUEST, frame, sizeof(Block_Req), NULL, NULL, NULL, NULL);
	else 
		bret = DeviceIoControl(h_hspr, IOCTL_SPR_REQUEST, frame, sizeof(Block_Req), NULL, NULL, NULL, NULL);
	if(!bret)
	{
		VirtualFree(frame->viraddr, 0, MEM_RELEASE);
		return FALSE;
	}
	return TRUE;
}

void DPPhisycalFree(HANDLE h_hspr, Block_Req* frame)
{
	DeviceIoControl(h_hspr, IOCTL_BLK_REMOVE, frame, sizeof(Block_Req), NULL, NULL, NULL, NULL);
	VirtualFree(frame->viraddr, 0, MEM_RELEASE);
}


HANDLE OpenSafeDev(void)
{
	HANDLE hSafeDev = CreateFile(_T("SAF1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hSafeDev == INVALID_HANDLE_VALUE)
		printf("SafeDev Open File fail:%u\r\n", GetLastError());
	return hSafeDev;
}

BOOL ReadSafeDev(HANDLE hSafe, BYTE* data)
{
	DWORD bytesreturned;
	return DeviceIoControl(hSafe, 0x12345678, NULL, 0, (LPVOID)&data, 1, &bytesreturned, NULL);
}

void CloseSafeDev(HANDLE hSafe)
{
	if(hSafe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSafe);
	}
}

DWORD ReadCpuID()
{
	return 0x12345678;
}

// Upgrade
static BOOL existRead;
static DWORD ReadProcess(HANDLE hd)
{
	HANDLE m_hMsgQueue;
	DWORD dwRead;
	DWORD dwFlag;
	DWORD readdata;

	MSGQUEUEOPTIONS opt1;

	opt1.dwSize = sizeof(MSGQUEUEOPTIONS);
	opt1.cbMaxMessage = 4;
	opt1.dwMaxMessages = 1;
	opt1.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	opt1.bReadAccess = TRUE; 
	m_hMsgQueue = CreateMsgQueue(_T("SPIUPDATE"), &opt1);

	if(m_hMsgQueue == NULL)
	{
		return FALSE;
	}

	DBGMSG(DPINFO, "ReadProcess Start\n");
	while(!existRead)
	{
		if(ReadMsgQueue(m_hMsgQueue, &readdata, 4, &dwRead, 100, &dwFlag))
		{
			DBGMSG(DPINFO, "ReadProcess percent %d\r\n",readdata);
			DPPostMessage(MSG_PRIVATE, (DWORD)hd, 1, readdata);
			if(readdata == 100)
				break;
		}
	}
	CloseHandle(m_hMsgQueue);
	DBGMSG(DPINFO, "ReadProcess Exit\n");
	return 0;
}

void StartUpdate(char* filename, DWORD id)
{
	HANDLE hfile;
	DWORD bytesreturned;
	BYTE buf;
	HANDLE hReadProgress;

	hfile =  CreateFile(_T("UPT1:"),GENERIC_READ|GENERIC_WRITE,0, NULL,OPEN_EXISTING,NULL,NULL);
	if(hfile == NULL)
	{
		DBGMSG(DPINFO, "Open UPT device error ,error is %d\r\n",GetLastError());
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
		return;
	}
	existRead = FALSE;
	hReadProgress = CreateThread(NULL, 0, ReadProcess, (HANDLE)id, 0, NULL);
	if(DeviceIoControl(hfile, IOCTL_NK_UPDATA, filename, sizeof(char*), &buf,sizeof(buf),&bytesreturned,NULL))
	{
		DPPostMessage(MSG_PRIVATE, id, 0, buf);
	}
	else
	{
		DBGMSG(DPINFO, "DeviceIoCtrol  Fail,error is %d\r\n",GetLastError());
	}
	CloseHandle(hfile);
	existRead = TRUE;
	if(WAIT_OBJECT_0 != WaitForSingleObject(hReadProgress, 1000))
	{
		DBGMSG(DPERROR, "UpdateApp WaitThread error:%d\n", GetLastError());
	}
	CloseHandle(hReadProgress);
	return;
}


