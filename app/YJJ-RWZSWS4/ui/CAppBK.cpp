#include "CCtrlModules.h"

class CBK : public CAppBase
{
public:
    CBK(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CBK()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if(m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[0])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk1.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_APP, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[1])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk2.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[2])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk3.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[3])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk4.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_APP, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[4])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk5.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[5])
                {
                    if (status == TRUE)
                        break;
                    strcpy(szbkp, "bk6.png");
                    SetPrjbkp(szbkp);
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if (wParam == m_idButton[6])
                {
                    status = (status == TRUE) ? FALSE : TRUE;
                    SetAutoBk(status);
                    m_pButton[6]->SetSrcpng(GetSmartAutoOnOff(status));
                    m_pButton[6]->Show(STATUS_NORMAL);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("mechine_bak.xml");
        GetCtrlByName("back", &m_idBack);
        m_pButton[0] = (CDPButton *)GetCtrlByName("icon1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("icon2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("icon3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("icon4", &m_idButton[3]);
        m_pButton[4] = (CDPButton *)GetCtrlByName("icon5", &m_idButton[4]);
        m_pButton[5] = (CDPButton *)GetCtrlByName("icon6", &m_idButton[5]);
        m_pButton[6] = (CDPButton *)GetCtrlByName("auto",  &m_idButton[6]);

        status = GetAutoBk();
        m_pButton[6]->SetSrcpng(GetSmartAutoOnOff(status));
        m_pButton[6]->Show(STATUS_NORMAL);

        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idButton[7];
    CDPButton *m_pButton[7];
    BYTE status;
    char szbkp[256];
};

CAppBase *CreateBKApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CBK *pApp = new CBK(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
