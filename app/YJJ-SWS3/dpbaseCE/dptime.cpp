#include "dpplatform.h"

DWORD DPGetTickCount(void)
{
	return GetTickCount();
}
void DPSleep(DWORD dwMilliseconds)
{
	Sleep(dwMilliseconds);
}
void DPGetLocalTime(SYSTEMTIME* lpSystemTime)
{
	GetLocalTime(lpSystemTime);
}
BOOL DPSetLocalTime(SYSTEMTIME *lpSystemTime)
{
	return SetLocalTime(lpSystemTime);
}
BOOL DPSystemTimeToFileTime(SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime)
{
	return SystemTimeToFileTime(lpSystemTime, lpFileTime);
} 
BOOL DPFileTimeToSystemTime(FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime)
{
	return FileTimeToSystemTime(lpFileTime, lpSystemTime);
} 