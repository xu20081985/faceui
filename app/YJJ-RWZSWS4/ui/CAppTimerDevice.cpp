#include "CCtrlModules.h"

class CTimerDeviceApp : public CAppBase
{
public:
    CTimerDeviceApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerDeviceApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                break;
            case TOUCH_MESSAGE:
                if(wParam == m_idBack)
                {
                }
                else if(wParam == m_idDevice_1)
                {
                }
                OnPage(0);
                break;
        }
        return TRUE;
    }

    void OnPage(DWORD dwPage)
    {

        m_dwPage = dwPage;
    }

    void OnCreate()
    {
        OnPage(0);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_device.xml");
        GetCtrlByName("ctl_device1", &m_idDevice_1);
        GetCtrlByName("back", &m_idBack);
        m_pDevice[0] = (CDPStatic *)GetCtrlByName("dev_1");
        m_pDevice[1] = (CDPStatic *)GetCtrlByName("dev_2");
        m_pDevice[2] = (CDPStatic *)GetCtrlByName("dev_3");
        m_pDevice[3] = (CDPStatic *)GetCtrlByName("dev_4");
        m_pSelect = (CDPStatic *)GetCtrlByName("select");

        OnCreate();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idDevice_1;                        //设备选择控制变量
    CDPStatic *m_pDevice[4];
    CDPStatic *m_pSelect;

    DWORD m_dwPage;
};

CAppBase *CreateTimerDeviceApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerDeviceApp *pApp = new CTimerDeviceApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
