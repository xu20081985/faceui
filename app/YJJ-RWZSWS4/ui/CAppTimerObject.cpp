#include "CCtrlModules.h"
#include <string.h>


class CTimerObjectApp : public CAppBase
{
public:
    CTimerObjectApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerObjectApp()
    {
    }
    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                break;
            case TOUCH_MESSAGE:

                break;
        }
        return TRUE;
    }


    void OnCreate()
    {
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_object.xml");

        OnCreate();
        return TRUE;
    }

private:
    DWORD m_idSave;
    DWORD m_idTime;
    DWORD m_idDate;
    DWORD m_idCancel;
    DWORD m_idObject;
    DWORD m_idAction;

    CDPStatic *m_pTime;
    CDPStatic *m_pDate;
    CDPStatic *m_pObject;
    CDPStatic *m_pAction;


};

CAppBase *CreateTimerObjectApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerObjectApp *pApp = new CTimerObjectApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}