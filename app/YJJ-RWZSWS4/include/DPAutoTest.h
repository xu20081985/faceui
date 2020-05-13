#pragma once

// ×Ô¶¯²âÊÔ

#include "CCtrlModules.h"

enum AUTO_TYPE
{
	AUTO_TALK,
	AUTO_TOUCH,
	AUTO_REBOOT,
};

BOOL GetAutoReboot();
BOOL AutoTestEnable(AUTO_TYPE type);
void AutoTimeProc(CAppBase* pCurApp);
void SetAutoButton(BOOL bTalk, CDPButton* pAccept, CDPButton* pHangUp);
