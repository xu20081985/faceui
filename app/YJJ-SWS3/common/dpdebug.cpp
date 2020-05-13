#include "roomlib.h"

static DWORD printlevel = 0xE0000000;
static StaticLock g_DebugCS;

//======================================================
//** 函数名称: InitDebugLeven
//** 功能描述: 初始化debug级别
//** 输　入: level
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void InitDebugLeven(DWORD level)
{
	printlevel |= level;
}

//======================================================
//** 函数名称: DBGMSG
//** 功能描述: debug消息
//** 输　入: level format ...
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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



