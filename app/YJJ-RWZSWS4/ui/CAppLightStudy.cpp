#include "CCtrlModules.h"

enum
{
    LIGHT_STUDY_INIT = 0,
    LIGHT_STUDY_SINGLE = 1,
    LIGHT_STUDY_DOUBLE = 2
};

class CLightStudy : public CAppBase
{
public:
    CLightStudy(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CLightStudy()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (GetLightStudy() == TRUE)
                    Show();
                if (m_dwTimeout++ == 30)
                {
                    ShowTip(FALSE);
                    DPSleep(2000);
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                break;
            case MSG_BROADCAST:
                if (wParam == SMART_LIGHT_STUDY)
                {
                    if ((lParam == LIGHT_STUDY_SINGLE && zParam == 0)
                            || (lParam == LIGHT_STUDY_DOUBLE && zParam == 0))
                    {
                        ShowTip(TRUE);
                        DPSleep(2000);
                        DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                    }
                    else if ((lParam == LIGHT_STUDY_SINGLE && zParam == 1)
                             || (lParam == LIGHT_STUDY_DOUBLE && zParam == 1))
                    {
                        ShowTip3();
                        DPSleep(1000);
                    }
                    else if (lParam == LIGHT_STUDY_SINGLE && zParam == 2)
                    {
                        ShowTip2(0);
                        DPSleep(1000);
                    }
                    else if (lParam == LIGHT_STUDY_DOUBLE && zParam == 2)
                    {
                        ShowTip2(1);
                        DPSleep(1000);
                    }
                    else
                    {
                        ShowTip(FALSE);
                        DPSleep(2000);
                        DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                    }
                }
                break;
            case TOUCH_SLIDE:
                break;
            case TOUCH_MESSAGE:
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void Show()
    {
        char buf[32] = {0};
        if (m_studyType == LIGHT_STUDY_SINGLE)
        {
            m_pStatic[0]->SetSrc(GetStringByID(4270 + m_studyChan));
            m_pStatic[0]->Show(TRUE);
        }
        else
        {
            m_pStatic[0]->SetSrc(GetStringByID(4280 + m_studyChan));
            m_pStatic[0]->Show(TRUE);
        }

        m_pStatic[1]->SetSrc(GetStringByID(4256));
        m_pStatic[1]->Show(TRUE);

        sprintf(buf, "%d", 30 - m_dwTimeout);
        m_pStatic[2]->SetSrc(buf);
        m_pStatic[2]->Show(TRUE);
    }

    void ShowTip(BOOL status)
    {
        m_pStatic[0]->Show(FALSE);
        m_pStatic[1]->SetTextSize(42);
        if (status == TRUE)
            m_pStatic[1]->SetSrc(GetStringByID(4257));
        else
            m_pStatic[1]->SetSrc(GetStringByID(4258));
        m_pStatic[1]->Show(TRUE);
        m_pStatic[2]->Show(FALSE);
        SetLightStudy(FALSE);
        SetStudyNum(0);
    }

    void ShowTip2(DWORD status)
    {
        if (status == 1)
            m_pStatic[0]->SetSrc(GetStringByID(4290));
        else
            m_pStatic[0]->SetSrc(GetStringByID(4291));
        m_pStatic[0]->Show(TRUE);

        m_pStatic[1]->SetSrc(GetStringByID(4292));
        m_pStatic[1]->Show(TRUE);

        m_pStatic[2]->SetSrc(GetStringByID(4293));
        m_pStatic[2]->Show(TRUE);
    }

    void ShowTip3()
    {
        m_pStatic[0]->SetSrc(GetStringByID(4296));
        m_pStatic[0]->Show(TRUE);

        m_pStatic[1]->SetSrc(GetStringByID(4297));
        m_pStatic[1]->Show(TRUE);

        m_pStatic[2]->SetSrc(GetStringByID(4293));
        m_pStatic[2]->Show(TRUE);
    }

    void OnCreate(DWORD i, DWORD status)
    {
        LightStudy *plightStudy = GetLightStudy(i);

        if (status == 0)
        {
            m_pStatic[0]->SetSrc(GetStringByID(4270 + i));
            m_pStatic[0]->Show(TRUE);
            plightStudy->type = LIGHT_STUDY_SINGLE;
            SetStudyNum(1);
        }
        else
        {
            m_pStatic[0]->SetSrc(GetStringByID(4280 + i));
            m_pStatic[0]->Show(TRUE);
            plightStudy->type = LIGHT_STUDY_DOUBLE;
            SetStudyNum(2);
        }
        m_studyType = plightStudy->type;
        m_studyChan = i;
        Show();
        SetLightStudy(TRUE);
        SetStudyChan(i);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("light_study.xml");

        GetCtrlByName("back", &m_idBack);
        m_pStatic[0] = (CDPStatic *)GetCtrlByName("text1");
        m_pStatic[1] = (CDPStatic *)GetCtrlByName("text2");
        m_pStatic[2] = (CDPStatic *)GetCtrlByName("text3");

        OnCreate(lParam, zParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    CDPStatic *m_pStatic[3];
    BYTE m_studyType;
    BYTE m_studyChan;
};

CAppBase *CreateLightStudyApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CLightStudy *pApp = new CLightStudy(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}