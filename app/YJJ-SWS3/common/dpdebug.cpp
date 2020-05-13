#include "roomlib.h"

static DWORD printlevel = 0xE0000000;
static StaticLock g_DebugCS;

//======================================================
//** ��������: InitDebugLeven
//** ��������: ��ʼ��debug����
//** �䡡��: level
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void InitDebugLeven(DWORD level)
{
	printlevel |= level;
}

//======================================================
//** ��������: DBGMSG
//** ��������: debug��Ϣ
//** �䡡��: level format ...
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
int DBGMSG(DWORD level, const char * format, ...)
{
	char tszInfo[512];
	if(((1 << level) & printlevel) == 0)
		return 0;
	va_list va;
	g_DebugCS.lockon();
	va_start(va, format);
	vsprintf(&tszInfo[9], format, va);
	va_end(va);

	SYSTEMTIME cursystem;
	DPGetLocalTime(&cursystem);
	sprintf(tszInfo, "%02d:%02d:%02d", cursystem.wHour, cursystem.wMinute, cursystem.wSecond);
	tszInfo[8] = 0x20;
	printf("%s", tszInfo);
	g_DebugCS.lockoff();
	return 0;
}



