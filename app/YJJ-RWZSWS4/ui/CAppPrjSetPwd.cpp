#include "CCtrlModules.h"

enum
{
    STAGE_FIRST,		// ��һ������
    STAGE_AGAIN,		// �ٴ�����
    STAGE_OK			// �޸���ϣ��ȴ��˳�
};

class CPrjSetPwdApp : public CAppBase
{
public:
    CPrjSetPwdApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetPwdApp()
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
                    if ((DPGetTickCount() - m_tick) > 1000
                            && (DPGetTickCount() - m_tick) < 3000)
                    {
                        m_pEditPwd->ShowPwd();
                    }
                }

                if (m_Stage == STAGE_OK
                        && (DPGetTickCount() - m_tick) > 1000)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
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
                else if(wParam == m_idOK)
                {
                    if (m_bShow)
                        break;

                    if (m_Stage == STAGE_FIRST)
                    {
                        m_Stage = STAGE_AGAIN;
                        strcpy(m_szPasswd, m_pEditPwd->GetString());
                        ShowTip(GetStringByID(4212));		   // ���ٴ����������õ�����
                    }
                    else
                    {
                        if (strcmp(m_szPasswd, m_pEditPwd->GetString()) == 0)
                        {
                            m_Stage = STAGE_OK;
                            SetPrjPwd(m_szPasswd);
                            ShowTip(GetStringByID(4213));		// ���������óɹ�,���μ�
                            m_tick = DPGetTickCount();
                        }
                        else
                        {
                            m_Stage = STAGE_FIRST;
                            ShowTip(GetStringByID(4214));		// �����벻һ��,����������
                        }
                    }
                }
                else if(wParam == m_idDelete)
                {
                    if (!m_bShow)
                    {
                        if (m_pEditPwd->GetCurCount() == 0)
                        {
                            if (m_Stage == STAGE_FIRST)
                                ShowTip(GetStringByID(4211));	 // ���������µ�����
                            else
                                ShowTip(GetStringByID(4212));	 // �ٴ������µ�����
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
                    if (m_Stage == STAGE_OK)
                        break;

                    if (m_bShow)
                    {
                        m_pEditPwd->SetString(NULL);
                        m_bShow = FALSE;
                    }

                    if (m_pEditPwd->GetCurCount() < 8)
                    {
                        m_pEditPwd->SetIsPwd(FALSE);
                        m_pEditPwd->Input(lParam);
                        m_pEditPwd->SetIsPwd(TRUE);
                        m_tick = DPGetTickCount();
                    }
                }
                break;
            default:
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
        InitFrame("mechine_pwd.xml");
        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("ok", &m_idOK);
        GetCtrlByName("delete", &m_idDelete);
        m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);

        m_Stage = STAGE_FIRST;
        ShowTip(GetStringByID(4211));	// ����������Ҫ���õ�����
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idOK;
    DWORD m_idDelete;
    DWORD m_idEditPwd;
    CEditBox *m_pEditPwd;

    BOOL m_bShow;
    DWORD m_Stage;
    DWORD m_tick;
    char m_szPasswd[32];
};

CAppBase *CreatePrjSetPwdApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetPwdApp *pApp = new CPrjSetPwdApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}