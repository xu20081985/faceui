#include "roomlib.h"

void DPDeleteFile(const char* pfile)
{
	wchar_t pwfile[64];
	swprintf(pwfile, L"%S", pfile);
	DeleteFile(pwfile);
}

void DPMoveFile(const char* dstfile, const char* srcfile)
{
	wchar_t pwdst[64];
	wchar_t pwsrc[64];
	swprintf(pwdst, L"%S", dstfile);
	swprintf(pwsrc, L"%S", srcfile);
	MoveFile(pwsrc, pwdst);
}

void DPCopyFile(const char* dstfile, const char* srcfile)
{
	wchar_t pwdst[64];
	wchar_t pwsrc[64];
	swprintf(pwdst, L"%S", dstfile);
	swprintf(pwsrc, L"%S", srcfile);
	CopyFile(pwsrc, pwdst, TRUE);
}

HANDLE DPFindFirstFile(char* dir, char* pfile)
{
	wchar_t szFind[64];
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	int i;

	swprintf(szFind, L"%S\\*", dir);
	hFind = FindFirstFile(szFind, &FindFileData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		unicode2utf8((BYTE*)pfile, FindFileData.cFileName);
		for(i = 0; i < strlen(pfile); i++)
		{
			if((pfile[i] >= 0x41)
				&& (pfile[i] <= 0x5a))
				pfile[i] = pfile[i] | 0x20;
		}
	}
	return hFind;
}

BOOL DPFindNextFile(HANDLE hFind, char* pfile)
{
	WIN32_FIND_DATA FindFileData;
	int i;
	if(FindNextFile(hFind, &FindFileData))
	{
		unicode2utf8((BYTE*)pfile, FindFileData.cFileName);
		for(i = 0; i < strlen(pfile); i++)
		{
			if((pfile[i] >= 0x41)
				&& (pfile[i] <= 0x5a))
				pfile[i] = pfile[i] | 0x20;
		}
		return TRUE;
	}
	return FALSE;
}

void DPFindClose(HANDLE hFind)
{
	::FindClose(hFind);
}

int DPGetFileAttributes(const char* filename)
{
	WCHAR wcFileName[128];
	utf82unicode((WORD*)wcFileName, (BYTE*)filename);
	return GetFileAttributes(wcFileName);
}

BOOL CheckAndCreateDir(const char *cDir)
{
	WIN32_FIND_DATA fd;
	HANDLE hFile;
	wchar_t wcDir[64];

	swprintf(wcDir, L"%S", cDir);

	hFile=::FindFirstFile(wcDir,&fd);

	if(hFile==INVALID_HANDLE_VALUE)
	{
		DBGMSG(DPINFO, "[APP]not find the path:%S \n",wcDir);
		int nTem=::CreateDirectory(wcDir,NULL);	
		if(nTem==0)
		{
			printf("[APP]Create Directory DIR:%S err:%u \n",wcDir,GetLastError());
			return FALSE;
		}
		hFile=::FindFirstFile(wcDir,&fd);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			printf("[APP]Create Directory DIR:%S err:%u \n",wcDir,GetLastError());
			return FALSE;
		}
	}
	::FindClose(hFile);
	return TRUE;
}

UINT64 CheckSpareSpace(const char* dir)
{
	ULARGE_INTEGER  freespace, totalspace, tspace;

	if(strstr(dir, "Windows")!=NULL)
		GetDiskFreeSpaceEx(L"\\Windows\\", &freespace, &totalspace, &tspace);
	else if(strstr(dir, "FlashDev")!=NULL)
		GetDiskFreeSpaceEx(L"\\FlashDev\\", &freespace, &totalspace, &tspace);
	else if(strstr(dir, "UserDev")!=NULL)
		GetDiskFreeSpaceEx(L"\\UserDev\\", &freespace, &totalspace, &tspace);
	else
		return 0;

	printf("%s total=%d free=%d \r\n",dir, (int)(totalspace.QuadPart/1024), (int)(freespace.QuadPart/1024));
	return (UINT64)freespace.QuadPart;
}

void StopLogo(void)
{
	HANDLE hstoplogo = CreateEvent(NULL,FALSE,FALSE,_T("STOPPLAYLOGO"));
	SetEvent(hstoplogo);
	CloseHandle(hstoplogo);
}

DWORD DumpMemory(DWORD* total, DWORD* used)
{
	MEMORYSTATUS m_status;
	GlobalMemoryStatus(&m_status);
	*total = m_status.dwTotalPhys;
	*used = m_status.dwAvailPhys;
	return m_status.dwMemoryLoad;
}

void DumpPhysicalMemory()
{
#ifndef _DEBUG
	CloseHandle(CreateFile(_T("DIS1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL));
#endif
}

typedef DWORD (*SETSYSTEMMEMORYDIVISION)(DWORD);
typedef BOOL  (*GETSYSTEMMEMORYDIVISION)(LPDWORD,LPDWORD,LPDWORD);

DWORD StMemorySpace_GetObjecWF(int nSize,DWORD dwNewPage)
{
	//从动态库中获取函数地址以设置对象存储区的大小.
	DWORD dwOldPage = 0;
	HINSTANCE hDll = LoadLibrary(_T("Coredll.dll"));
	SETSYSTEMMEMORYDIVISION    SetSystemMemoryDivisionProc = NULL;  
	GETSYSTEMMEMORYDIVISION    GetSystemMemoryDivisionProc = NULL;  
	if( NULL != hDll )
	{
		//获取导出函数地址
		SetSystemMemoryDivisionProc = (SETSYSTEMMEMORYDIVISION)GetProcAddress(hDll,_T("SetSystemMemoryDivision"));
		GetSystemMemoryDivisionProc = (GETSYSTEMMEMORYDIVISION)GetProcAddress(hDll,_T("GetSystemMemoryDivision"));
		if(SetSystemMemoryDivisionProc && GetSystemMemoryDivisionProc)
		{
			DWORD dwStorePages = 0;
			DWORD dwRamPages = 0; 
			DWORD dwPageSize = 0; 
			BOOL bRet = (*GetSystemMemoryDivisionProc)(&dwStorePages,&dwRamPages,&dwPageSize);
			if(bRet)
			{
				dwOldPage = dwStorePages;

				if (dwNewPage != 0)
				{
					(*SetSystemMemoryDivisionProc)(dwNewPage);
				}
				else
				{
					int nPageCount = nSize/dwPageSize;
					if(nSize%dwPageSize != 0)
					{
						nPageCount++;
					}

					(*SetSystemMemoryDivisionProc)(nPageCount);
				}
			}
		}

		SetSystemMemoryDivisionProc = NULL;
		GetSystemMemoryDivisionProc = NULL;
		FreeLibrary(hDll);
		hDll = NULL;  
	}
	return dwOldPage;
}

void DPUnloadFile(const char* pfile)
{
	
}

BOOL DPWriteFile(const char* filename, char* pdata, int len)
{
	FILE* pFile = fopen(filename, "wb");
	if(pFile)
	{
		fwrite(pdata, 1, len, pFile);
		fclose(pFile);
	}
	return TRUE;
}