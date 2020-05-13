#include "CCtrlModules.h"

class CClockApp : public CAppBase
{
public:
    CClockApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CClockApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
			   if (m_dwTimeout <= 60)
                    m_pTime->UpdataDateTime();                           //实时更新时间
               if (m_dwTimeout++ == 60)
					SetScreenOnOff(FALSE);
                break;
            case TOUCH_MESSAGE:
                if (wParam == m_idEmpty)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case MSG_BROADCAST:
                break;
        }
        return TRUE;
    }

    void showAutoBk()
    {
        DWORD count;
        char str[256] = {0};
        status = GetAutoBk();
        if (status == TRUE)
        {
            GetPrjbkp(szbkp);
            srand(time(NULL));
            while (1)
            {
                count = (rand() % 6) + 1;
                sprintf(str, "bk%d.png", count);
                if (strcmp(str, szbkp))
                {
                    SetPrjbkp(str);
                    break;
                }
            }
        }
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("clock.xml");
        GetCtrlByName("ebutton", &m_idEmpty);
        m_pTime = (CTimeDate *)GetCtrlByName("time");
        showAutoBk();
        return TRUE;
    }

private:
    DWORD m_idEmpty;
    BYTE status;
    char szbkp[256];
    CTimeDate *m_pTime;
};

CAppBase *CreateClockApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CClockApp *pApp = new CClockApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}