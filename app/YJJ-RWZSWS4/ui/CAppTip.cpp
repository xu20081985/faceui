#include "CCtrlModules.h"
#include "SmartConfig.h"

class CTipApp: public CAppBase
{
public:
    CTipApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTipApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_LIGHT_CFG_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                if (wParam == SLIDE_UPSIDE)
                {
                    if (m_dwPage == 0)
                    {
                        hide();
                        OnCreate(m_dwChan, 1);
                    }
                    else
                    {
                        hide();
                        OnCreate(m_dwChan, 0);
                    }
                }
                else if (wParam == SLIDE_DOWN)
                {
                    if (m_dwPage == 0)
                    {
                        hide();
                        OnCreate(m_dwChan, 1);
                    }
                    else
                    {
                        hide();
                        OnCreate(m_dwChan, 0);
                    }
                }
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_LIGHT_CFG_APPID, 0, 0);
                }
                break;
        }
        return TRUE;
    }

    void hide()
    {
        for (int i = 0; i < 8; i++)
        {
            m_pStatic[i]->Show(FALSE);
        }
    }

    void OnCreate(DWORD i, DWORD page)
    {
        char buf[64] = {0};
        LightStudy *plightStudy = GetLightStudy(i);
        m_dwChan = i;
        m_dwPage = page;
        if (m_dwPage == 0)
        {
            m_pTitle->SetSrc(GetStringByID(4294));
            m_pTitle->Show(TRUE);
            for (int j = 0; j < plightStudy->singleNum; j++)
            {
                sprintf(buf, "%08X%02X%02X",
                        htonl(plightStudy->senosr[j].id),
                        plightStudy->senosr[j].type,
                        plightStudy->senosr[j].value);
                m_pStatic[j]->SetSrc(buf);
                m_pStatic[j]->Show(TRUE);
            }
        }
        else
        {
            m_pTitle->SetSrc(GetStringByID(4295));
            m_pTitle->Show(TRUE);
            for (int j = 0; j < plightStudy->doubleNum; j++)
            {
                sprintf(buf, "%08X%02X%02X",
                        htonl(plightStudy->senosr[8 + j].id),
                        plightStudy->senosr[8 + j].type,
                        plightStudy->senosr[8 + j].value);
                m_pStatic[j]->SetSrc(buf);
                m_pStatic[j]->Show(TRUE);
            }
        }
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("light_list.xml");
        GetCtrlByName("back", &m_idBack);

        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pStatic[0] = (CDPStatic *)GetCtrlByName("text1");
        m_pStatic[1] = (CDPStatic *)GetCtrlByName("text2");
        m_pStatic[2] = (CDPStatic *)GetCtrlByName("text3");
        m_pStatic[3] = (CDPStatic *)GetCtrlByName("text4");
        m_pStatic[4] = (CDPStatic *)GetCtrlByName("text5");
        m_pStatic[5] = (CDPStatic *)GetCtrlByName("text6");
        m_pStatic[6] = (CDPStatic *)GetCtrlByName("text7");
        m_pStatic[7] = (CDPStatic *)GetCtrlByName("text8");

        OnCreate(lParam, zParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_dwPage;
    DWORD m_dwChan;
    CDPStatic *m_pTitle;
    CDPStatic *m_pStatic[8];
};

CAppBase *CreateTipApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTipApp *pApp = new CTipApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}