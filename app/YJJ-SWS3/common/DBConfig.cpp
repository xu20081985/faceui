#include "CAppBase.h"

typedef struct
{
	BOOL	bSwitch[SWITCH_MAX];
	DWORD	nFrameVal[FRAME_MAX];
	DWORD	nKeyVal[KEY_VALUE_MAX];
	DWORD	nTextSize[TEXT_SIZE_MAX];
}DPConfig;

static DPConfig* g_pConfig;

//======================================================
//** 函数名称: InitDefaultConfig
//** 功能描述: 初始化默认界面配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void InitDefaultConfig()
{
	memset(g_pConfig, 0, sizeof(DPConfig));

	g_pConfig->nFrameVal[FRAME_WIDTH]			= 320;
	g_pConfig->nFrameVal[FRAME_HEIGHT]			= 240;

	g_pConfig->nFrameVal[FRAME_TIP_WIDTH]		= 284;
	g_pConfig->nFrameVal[FRAME_TIP_HEIGHT]		= 204;

	g_pConfig->nFrameVal[LIGHT_PNG_TOP]			= 40;
	g_pConfig->nFrameVal[LIGHT_TEXT_TOP]		= 170;
	g_pConfig->nFrameVal[LIGHT_TEXT_PNG_TOP]	= 168;

	g_pConfig->nFrameVal[DEV_FAIL_PNG_TOP]		= 50;
	g_pConfig->nFrameVal[DEV_FAIL_TEXT_TOP]		= 156;

	g_pConfig->nFrameVal[SCENE_FAIL_PNG_TOP]	= 54;
	g_pConfig->nFrameVal[SCENE_FAIL_TEXT_TOP1]	= 136;
	g_pConfig->nFrameVal[SCENE_FAIL_TEXT_TOP2]	= 168;

	g_pConfig->nFrameVal[COLOR_TEXT_NORMAL]		= 0xFFFFFFFF;
	g_pConfig->nFrameVal[COLOR_TEXT_FOCUS]		= 0xFFFFFFFF; //0xFFD06108;

	g_pConfig->nFrameVal[TIMER_SELECT_TOP]		= 61;			// 居中，36 + 25
	g_pConfig->nFrameVal[TIMER_SELECT_INTERVAL]	= 50;

	g_pConfig->nTextSize[TS_BUTTON]				= 20;
	g_pConfig->nTextSize[TS_STATIC]				= 24;
	g_pConfig->nTextSize[TS_TEXT]				= 24;
	g_pConfig->nTextSize[TS_DATE]				= 24;
	g_pConfig->nTextSize[TS_WEEK]				= 24;
	g_pConfig->nTextSize[TS_PROGRESS]			= 24;
	g_pConfig->nTextSize[TS_PAGE]				= 26;
	g_pConfig->nTextSize[TS_TITLE]				= 26;
	g_pConfig->nTextSize[TS_KEYBOARD_P]			= 28;
	g_pConfig->nTextSize[TS_EDITBOX]			= 32;
	g_pConfig->nTextSize[TS_KEYBOARD]			= 32;
	g_pConfig->nTextSize[TS_TIME]				= 32;
	g_pConfig->nTextSize[TS_MKEYBOARD]			= 36;
}

//======================================================
//** 函数名称: InitConfig
//** 功能描述: 初始化界面配置信息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void InitConfig(void)
{
	if(g_pConfig)
		return;

	g_pConfig = (DPConfig *)malloc(sizeof(DPConfig));    //malloc申请内存。
	if(NULL == g_pConfig)
	{
		DBGMSG(DPERROR, "InitConfig memreq fail\r\n");
		return;
	}

	InitDefaultConfig();                //进行一些数组成员的初始化。
}

//======================================================
//** 函数名称: GetSwitch
//** 功能描述: 获取选择方式
//** 输　入: index
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
BOOL GetSwitch(DWORD index)
{
	if(index >= SWITCH_MAX)
		return FALSE;

	return g_pConfig->bSwitch[index];
}

//======================================================
//** 函数名称: GetUIConfig
//** 功能描述: 获取UI配置
//** 输　入: index
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
int GetUIConfig(DWORD index)
{
	if(index >= FRAME_MAX)
		return 0;

	return g_pConfig->nFrameVal[index];
}

//======================================================
//** 函数名称: GetKeyConfig
//** 功能描述: 获取按键配置索引
//** 输　入: index
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
int GetKeyConfig(DWORD index)
{
	return index;
}

//======================================================
//** 函数名称: GetTextSize
//** 功能描述: 获取文本大小
//** 输　入: index
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetTextSize(DWORD index)
{
	if(index >= TEXT_SIZE_MAX)
		return 0;

	return g_pConfig->nTextSize[index];
}                 