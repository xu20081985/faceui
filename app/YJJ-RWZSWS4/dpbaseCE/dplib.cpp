#include "roomlib.h"

#pragma comment(lib, "Mmtimer.lib")

static DWORD TimeEvent(LPVOID pParam)
{
	HANDLE hEvent;
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	timeSetEvent(1000, 10, (LPTIMECALLBACK)hEvent, 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
	while(1)	//-V776
	{
		WaitForSingleObject(hEvent, INFINITE);
		DPPostMessage(TIME_MESSAGE, 0, 0, 0, MSG_TIME_TYPE);
	}
}

static DWORD TouchEvent(LPVOID pParam)
{
	HANDLE m_hTouch;
	MSGQUEUEOPTIONS opt1;
	TOUCHDATA tch;
	DWORD dwRead;
	DWORD dwFlag;

	opt1.dwSize = sizeof(MSGQUEUEOPTIONS);
	opt1.cbMaxMessage = sizeof(TOUCHDATA);
	opt1.dwMaxMessages = 100;
	opt1.dwFlags =  MSGQUEUE_ALLOW_BROKEN ;
	opt1.bReadAccess = TRUE; 
	m_hTouch = CreateMsgQueue(_T("TouchMessage"), &opt1);
	if(m_hTouch == NULL)
		return FALSE;
	while(1)
	{
		if(ReadMsgQueue(m_hTouch, &tch, sizeof(TOUCHDATA), &dwRead, INFINITE, &dwFlag))
			DPPostMessage(TOUCH_RAW_MESSAGE, tch.x, tch.y, tch.flag, MSG_TOUCH_TYPE);
	}
	return 0;
}

static DWORD KeybdThread(LPVOID pParam)
{
	MSGQUEUEOPTIONS opt1;
	KBDDATA tch;
	DWORD dwRead;
	DWORD dwFlag;

	opt1.dwSize = sizeof(MSGQUEUEOPTIONS);
	opt1.cbMaxMessage = sizeof(KBDDATA);
	opt1.dwMaxMessages = 100;
	opt1.dwFlags = 0;
	opt1.bReadAccess = TRUE; 
	HANDLE hKey = CreateMsgQueue(_T("KeybdMessage"), &opt1);

	while(1)
	{
		if(ReadMsgQueue(hKey, &tch, sizeof(TOUCHDATA), &dwRead, INFINITE, &dwFlag))
			DPPostMessage(HARDKBD_MESSAGE, tch.key, tch.flag, 0, MSG_KEY_TYPE);
	}
	return 0;
}

void DPCreateTimeEvent(void)
{
	CloseHandle(CreateThread(NULL, 0x1000, TimeEvent, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL));
}

void DPCreateTouchEvent()
{
	CloseHandle(CreateThread(NULL, 0x1000, TouchEvent, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL));
}

void DPCreateKeyEvent()
{
	CloseHandle(CreateThread(NULL, 0x1000, KeybdThread, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL));
}

void DPBacktrace()
{
	
}