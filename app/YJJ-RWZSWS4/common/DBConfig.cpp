#include "CAppBase.h"

typedef struct
{
    BOOL	bSwitch[SWITCH_MAX];
    DWORD	nFrameVal[FRAME_MAX];
    DWORD	nKeyVal[KEY_VALUE_MAX];
    DWORD	nTextSize[TEXT_SIZE_MAX];
} DPConfig;

static DPConfig *g_pConfig;

static void InitDefaultConfig()
{
    memset(g_pConfig, 0, sizeof(DPConfig));

    g_pConfig->nFrameVal[FRAME_WIDTH]				= 720;
    g_pConfig->nFrameVal[FRAME_HEIGHT]			= 720;

    g_pConfig->nFrameVal[FRAME_TIP_WIDTH]			= 284;
    g_pConfig->nFrameVal[FRAME_TIP_HEIGHT]		= 204;

    g_pConfig->nFrameVal[LIGHT_PNG_TOP]			= 40;
    g_pConfig->nFrameVal[LIGHT_TEXT_TOP]			= 170;
    g_pConfig->nFrameVal[LIGHT_TEXT_PNG_TOP]		= 168;

    g_pConfig->nFrameVal[DEV_FAIL_PNG_TOP]		= 50;
    g_pConfig->nFrameVal[DEV_FAIL_TEXT_TOP]		= 156;

    g_pConfig->nFrameVal[SCENE_FAIL_PNG_TOP]		= 54;
    g_pConfig->nFrameVal[SCENE_FAIL_TEXT_TOP1]	= 136;
    g_pConfig->nFrameVal[SCENE_FAIL_TEXT_TOP2]	= 168;

    g_pConfig->nFrameVal[COLOR_TEXT_NORMAL]		= 0xFFFFFFFF;
    g_pConfig->nFrameVal[COLOR_TEXT_FOCUS]		= 0xFFFFFFFF; //0xFFD06108;

    g_pConfig->nFrameVal[TIMER_SELECT_TOP]		= 61;			// ¾ÓÖÐ£¬36 + 25
    g_pConfig->nFrameVal[TIMER_SELECT_INTERVAL]	= 50;
#if 0
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
#endif
    g_pConfig->nTextSize[TS_BUTTON]				= 32;
    g_pConfig->nTextSize[TS_STATIC]				= 32;
    g_pConfig->nTextSize[TS_TEXT]				= 32;
    g_pConfig->nTextSize[TS_DATE]				= 32;
    g_pConfig->nTextSize[TS_WEEK]				= 32;
    g_pConfig->nTextSize[TS_PROGRESS]				= 32;
    g_pConfig->nTextSize[TS_PAGE]				= 32;
    g_pConfig->nTextSize[TS_TITLE]				= 32;
    g_pConfig->nTextSize[TS_KEYBOARD_P]			= 32;
    g_pConfig->nTextSize[TS_EDITBOX]				= 32;
    g_pConfig->nTextSize[TS_KEYBOARD]				= 32;
    g_pConfig->nTextSize[TS_TIME]				= 32;
    g_pConfig->nTextSize[TS_MKEYBOARD]			= 32;
}

void InitConfig(void)
{
    if (g_pConfig)
        return;

    g_pConfig = (DPConfig *)malloc(sizeof(DPConfig));
    if (NULL == g_pConfig)
    {
        DBGMSG(DPERROR, "InitConfig memreq fail\r\n");
        return;
    }

    InitDefaultConfig();
}

BOOL GetSwitch(DWORD index)
{
    if (index >= SWITCH_MAX)
        return FALSE;

    return g_pConfig->bSwitch[index];
}

int GetUIConfig(DWORD index)
{
    if (index >= FRAME_MAX)
        return 0;

    return g_pConfig->nFrameVal[index];
}

int GetKeyConfig(DWORD index)
{
    return index;
}

DWORD GetTextSize(DWORD index)
{
    if (index >= TEXT_SIZE_MAX)
        return 0;

    return g_pConfig->nTextSize[index];
}