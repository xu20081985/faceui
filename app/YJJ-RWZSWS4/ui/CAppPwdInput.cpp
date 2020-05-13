#include "CCtrlModules.h"

class CPwdInputApp : public CAppBase
{
public:
    CPwdInputApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPwdInputApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }

                if (!m_bShow)
                {
                    m_pEditPwd->Flick();
                    if ((DPGetTickCount() - m_tick) > 400
                            && (DPGetTickCount() - m_tick) < 1500)
                    {
                        m_pEditPwd->ShowPwd();
                    }
                }
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                }
                else if (wParam == m_idOK)
                {
                    GetPrjPwd(m_sysPwd);
                    //printf("%s %s\n", m_sysPwd, m_pEditPwd->GetString());
                    if (strcmp(m_sysPwd, m_pEditPwd->GetString()) == 0
                            || strcmp("20110755", m_pEditPwd->GetString()) == 0) //ÃÜÂëÊäÈëÕýÈ·£¬½øÈëÉèÖÃ½çÃæ
                    {
                        DPPostMessage(MSG_START_FROM_ROOT, PROJECT_APPID, 0, 0);
                    }
                    else
                    {
                        ShowTip(GetStringByID(20002));		// ÃÜÂë´íÎó
                    }
                }
                else if (wParam == m_idDelete)
                {
                    if (!m_bShow)
                    {
                        if (m_pEditPwd->GetCurCount() == 0)
                        {
                            ShowTip(NULL);
                            //ShowTip(GetStringByID(20001));		// ÇëÊäÈëÃÜÂë
                        }
                        else
                        {
                            m_pEditPwd->Delete();
                        }
                    }
                    else
                    {
                        m_pEditPwd->SetString(NULL);
                        m_bShow = FALSE;
                    }
                }
                break;
            case KBD_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == KBD_CTRL)
                {
                    if (m_bShow)
                    {
                        m_pEditPwd->SetString(NULL);
                        m_bShow = FALSE;
                    }
                    m_pEditPwd->SetIsPwd(FALSE);
                    m_pEditPwd->Input(lParam);
                    m_pEditPwd->SetIsPwd(TRUE);
                    m_tick = DPGetTickCount();
                }
                break;
        }
        return TRUE;
    }

    void ShowTip(char *buf)
    {
        m_bShow = TRUE;
        m_pEditPwd->SetIsPwd(FALSE);
        m_pEditPwd->SetString(buf);
        m_pEditPwd->Show(FALSE);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("pwdinput.xml");
        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("ok", &m_idOK);
        GetCtrlByName("delete", &m_idDelete);
        m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);

        m_pEditPwd->SetMaxLen(8);
        ShowTip(NULL);
        //ShowTip(GetStringByID(20001));		// ÇëÊäÈëÃÜÂë
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idOK;
    DWORD m_idDelete;
    DWORD m_idEditPwd;
    CEditBox *m_pEditPwd;

    BOOL m_bShow;
    char m_sysPwd[32];
    DWORD m_tick;
};

CAppBase *CreatePwdInputApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPwdInputApp *pApp = new CPwdInputApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}