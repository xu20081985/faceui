#include "CCtrlModules.h"

class CAbout : public CAppBase
{
public:
    CAbout(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CAbout()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)    									// 返回
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PROJECT_APPID, 0, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void onCreate()
    {
        char buf[256];

        DWORD softVer = GetSoftVer();
        sprintf(buf, "V%d.%02d.%02d", (softVer >> 24) & 0xFF,
                (softVer >> 16) & 0xFF, (softVer >> 8) & 0xFF);	// 当前版本
        m_pText[0]->SetSrc(buf);
        m_pText[0]->Show(TRUE);

        DWORD devId = GetDevID();
        sprintf(buf, "%08X", htonl(devId));					   // 唯一ID
        m_pText[1]->SetSrc(buf);
        m_pText[1]->Show(TRUE);

		WORD zigVer = GetZigbeeVer();
        sprintf(buf, "V%d.%d", (zigVer >> 4) & 0x0F, zigVer & 0x0F);
		m_pText[2]->SetSrc(buf);
        m_pText[2]->Show(TRUE);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("about.xml");
        GetCtrlByName("back", &m_idBack);
        m_pText[0] = (CDPStatic *)GetCtrlByName("text3");
        m_pText[1] = (CDPStatic *)GetCtrlByName("text4");
		m_pText[2] = (CDPStatic *)GetCtrlByName("text5");
		
        onCreate();

        return TRUE;
    }

private:
    DWORD m_idBack;
    CDPStatic *m_pText[3];
};

CAppBase *CreateAboutApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CAbout *pApp = new CAbout(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}