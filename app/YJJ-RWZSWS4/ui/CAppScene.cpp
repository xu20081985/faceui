#include "CCtrlModules.h"
#include "SmartConfig.h"

class CSceneApp : public CAppBase
{
public:
    CSceneApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CSceneApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if(m_dwTimeout++ == 30)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }
                break;
            case MSG_BROADCAST:
                break;
            case TOUCH_SLIDE:
                DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    if (index == 1)
                        m_pButton->SetBkpng("2_show.png");
                    else if (index == 2)
                        m_pButton->SetBkpng("3_show.png");
                    else if (index == 3)
                        m_pButton->SetBkpng("4_show.png");
                    else if (index == 4)
                        m_pButton->SetBkpng("5_show.png");
                    else if (index == 5)
                    {
                        m_pButton->SetBkpng("1_show.png");
                        index = 0;
                    }
                    m_pButton->Show(STATUS_NORMAL);
                    index++;
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        SendSmartCmd(&m_pSmartDev->device, SCMD_SCENE, m_pSmartDev->scene);
        SetStatusByScene(m_pSmartDev->scene);
        DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("scene.xml");

        m_pButton = (CDPButton *)GetCtrlByName("back", &m_idBack);
        index = 1;
        //OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    SmartDev *m_pSmartDev;
    DWORD m_idBack;
    CDPButton *m_pButton;
    int index;
};

CAppBase *CreateSceneApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CSceneApp *pApp = new CSceneApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}