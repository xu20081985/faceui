#include "dpplatform.h"

VOID DPInitCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	InitializeCriticalSection(pcsCriticalSection);
}

VOID DPDeleteCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	DeleteCriticalSection(pcsCriticalSection);
}

VOID DPEnterCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	EnterCriticalSection(pcsCriticalSection);
}

VOID DPLeaveCriticalSection(CRITICAL_SECTION* pcsCriticalSection)
{
	LeaveCriticalSection(pcsCriticalSection);
}

HANDLE DPCreateSemaphore(DWORD dwInitCount, DWORD dwMaxCount)
{
	return CreateSemaphore(NULL, dwInitCount, dwMaxCount, NULL);
}

void DPSetSemaphore(HANDLE hSem)
{
	ReleaseSemaphore(hSem, 1, NULL);
}

BOOL DPGetSemaphore(HANDLE hSem, DWORD timeout)
{
	return (WaitForSingleObject(hSem, timeout) == WAIT_OBJECT_0);
}

BOOL DPCreateMsgQueue(const char* name, DWORD vol, DWORD itemsize, HANDLE* pRead, HANDLE* pWrite)
{
	wchar_t wname[32];
	MSGQUEUEOPTIONS option;
	swprintf(wname, L"%S", name);
	option.dwSize = sizeof(MSGQUEUEOPTIONS);
	option.dwMaxMessages  = vol;
	option.cbMaxMessage = itemsize;
	option.dwFlags =  MSGQUEUE_ALLOW_BROKEN ;
	if(pRead != NULL)
	{
		option.bReadAccess = TRUE;
		*pRead = CreateMsgQueue(wname, &option);
	}
	if(pWrite != NULL)
	{
		option.bReadAccess = FALSE;
		*pWrite = CreateMsgQueue(wname, &option);
	}

	return TRUE;
}

BOOL DPWriteMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout)
{
	return WriteMsgQueue(hMsg, data, len, timeout, 0);
}

BOOL DPReadMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout)
{
	DWORD dwRead,dwFlag;
	return ReadMsgQueue(hMsg, data, len, &dwRead, timeout, &dwFlag);
}

void DPCloseMsgQueue(HANDLE hMsg)
{
	CloseHandle(hMsg);
}

static int CE_PPIORITY[8] =
{
	THREAD_PRIORITY_TIME_CRITICAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_ABOVE_IDLE,
	THREAD_PRIORITY_IDLE 
};

HANDLE DPThreadCreate(int stacksize, DWORD (*func) (void *), void *arg, BOOL joinable, DWORD level)
{
	HANDLE hTrd;
	hTrd = CreateThread(NULL, stacksize, (LPTHREAD_START_ROUTINE) func, arg, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
	//SetThreadPriority(hTrd, CE_PPIORITY[level]);
	if(!joinable)
	{
		CloseHandle(hTrd);
		return NULL;
	}
	return hTrd;
}

BOOL DPThreadJoin(HANDLE hThread, DWORD* dPret)
{
	WaitForSingleObject(hThread, INFINITE);
	if(dPret != NULL)
		GetExitCodeThread(hThread, dPret);
	CloseHandle(hThread);
	return TRUE;
}

DWORD DPGetLastError()
{
	return GetLastError();
}