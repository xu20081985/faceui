#include "CCtrlModules.h"

class CPrjSetTemApp : public CAppBase
{
public:
    CPrjSetTemApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetTemApp()
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
            case KBD_MESSAGE:
                break;
        }
        return TRUE;
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("temp.xml");

        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idOK;
    DWORD m_idDelete;
    DWORD m_idEditPwd;
    CEditBox *m_pEditPwd;

};

CAppBase *CreatePrjSetTempApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetTemApp *pApp = new CPrjSetTemApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}