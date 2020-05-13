#include <windows.h>
#include "dpcom.h"

HANDLE OpenComm(const char* szComName, DWORD dwBAUD_RATE, BYTE bufDataByte, DWORD timeOut)
{
	DCB dcb;
	WCHAR p_fname[64];
	swprintf(p_fname, L"%S", szComName);

	HANDLE hCom = CreateFile(p_fname,                       // �ļ���   
		GENERIC_READ   |   GENERIC_WRITE,					// �������д     
		0,                                                  // ��ռ��ʽ   
		NULL,     
		OPEN_EXISTING,										// �򿪶����Ǵ���   
		0,
		0   
		);   

	if(hCom == INVALID_HANDLE_VALUE)   
	{
		printf("OpenCom %s fail\r\n", szComName);
		return NULL;
	}

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = MAXDWORD;     
	TimeOuts.ReadTotalTimeoutMultiplier = 0;     
	TimeOuts.ReadTotalTimeoutConstant = timeOut;     
	TimeOuts.WriteTotalTimeoutMultiplier = 0;     
	TimeOuts.WriteTotalTimeoutConstant = 0;   
	SetCommTimeouts(hCom, &TimeOuts);

	dcb.DCBlength = sizeof(DCB);
	GetCommState(hCom, &dcb);
	dcb.BaudRate = dwBAUD_RATE;   //   ������
	dcb.ByteSize = bufDataByte;   //   ÿ���ַ���8λ   
	//dcb.ByteSize = 8;   //   ÿ���ַ���8λ   
	dcb.Parity = NOPARITY;   //��У��   
	dcb.StopBits = ONESTOPBIT;   //һ��ֹͣλ   
	if(!SetCommState(hCom, &dcb))
	{
		DWORD dwError = GetLastError();
		CloseHandle(hCom);
		printf("Open comm port fail 2 : dwError = %d\n", dwError);
		return NULL;
	}

	return hCom;
}

void CloseComm(HANDLE hCom)
{
	if(hCom)
	{
		CloseHandle(hCom);
	}
}

int ReadComm(HANDLE hCom, char* buf, int len)
{
	if(NULL == hCom) 
	{
		printf("ReadComm fail\r\n");
		return -1;
	}

	DWORD dwRead;
	if(0 == ReadFile(hCom, buf, len, &dwRead,0))
	{
		printf("ReadComm Fail,error is %d\r\n",GetLastError());
		return -1;
	}

	return dwRead;
}

int SendComm(HANDLE hCom, char* buf, int len)
{
	if(NULL == hCom) 
	{
		printf("SendComm fail\r\n");
		return 0;
	}

	DWORD dwWrite;
	if(0 == WriteFile(hCom, buf, len, &dwWrite, NULL))
	{
		printf("SendComm Fail,error is %d\r\n",GetLastError());
		return -1;
	}

	return dwWrite;
}
