#include "CCtrlModules.h"

static BOOL m_auto = TRUE;

class CPrjSetDateApp : public CAppBase
{
public:
    CPrjSetDateApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetDateApp()
    {
    }

    DWORD GetMonthStatus()
    {
        DWORD status;
        memset(m_str, 0, sizeof(m_str));
        tm.wMonth = atoi(strncpy(m_str, m_time + 7, 2));
        switch (tm.wMonth)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                status = 1;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                status = 2;
                break;
            case 2:
                status = 3;
                break;
            default:
                status = 1;
                break;
        }
        return status;
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
                if (m_auto == TRUE)
                {
                    m_pEditPwd->ShowFlick(FALSE);
                    onCreate();
                }
                else
                {
                    m_pEditPwd->FlickSet();
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
                else if (wParam == m_idButton)
                {
                    m_auto = (m_auto == TRUE) ? FALSE : TRUE;
                    setTimeAuto(m_auto);
                    if (m_auto == TRUE)
                        SmartTimeSync();
                }
                else if (wParam == m_idOK)
                {
                    if (m_auto == TRUE)
                        break;
                    memset(m_str, 0, sizeof(m_str));
                    tm.wYear = atoi(strncpy(m_str, m_time, 4));
                    memset(m_str, 0, sizeof(m_str));
                    tm.wMonth = atoi(strncpy(m_str, m_time + 7, 2));
                    memset(m_str, 0, sizeof(m_str));
                    tm.wDay = atoi(strncpy(m_str, m_time + 12, 2));
                    memset(m_str, 0, sizeof(m_str));
                    tm.wHour = atoi(strncpy(m_str, m_time + 20, 2));
                    memset(m_str, 0, sizeof(m_str));
                    tm.wMinute = atoi(strncpy(m_str, m_time + 23, 2));
                    DPSetLocalTime(&tm);
                    DPPostMessage(MSG_START_FROM_ROOT, MECHINE_SET, 0, 0);
                }
                else if(wParam == m_idDelete)
                {
                    if (m_auto == TRUE)
                        break;
                    showDelete();
                    m_pEditPwd->ShowFlick(TRUE);
                }
                break;
            case KBD_MESSAGE:
                m_dwTimeout = 0;
                if (m_auto == TRUE)
                    break;
                if (wParam == KBD_CTRL)
                {
                    switch (m_curPos)
                    {
                        case 0:
                            if (lParam == '2')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 1:
                            if (lParam >= '0' && lParam <= '9')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 2:
                            if (lParam >= '0' && lParam <= '9')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 3:
                            if (lParam >= '0' && lParam <= '9')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 4;
                                m_flickPos += 2;
                            }
                            break;
                        case 4:
                        case 5:
                        case 6:
                            break;
                        case 7:
                            if (lParam == '0')
                            {
                                if (m_time[m_curPos + 1] == '0')
                                    m_time[m_curPos + 1] = '1';
                            }
                            else if (lParam == '1')
                            {
                                if (m_time[m_curPos + 1] > '2')
                                    m_time[m_curPos + 1] = '2';
                            }
                            memset(m_str, 0, sizeof(m_str));
                            tm.wYear = atoi(strncpy(m_str, m_time, 4));
                            memset(m_str, 0, sizeof(m_str));
                            tm.wDay = atoi(strncpy(m_str, m_time + 12, 2));
                            if (GetMonthStatus() == 3)
                            {
                                if ((tm.wYear % 4 == 0 && tm.wYear % 100 != 0) || (tm.wYear % 400 == 0))
                                {
                                    if (tm.wDay > 28)
                                    {
                                        m_time[12] = '2';
                                        m_time[13] = '9';
                                    }
                                }
                                else
                                {
                                    if (tm.wDay > 28)
                                    {
                                        m_time[12] = '2';
                                        m_time[13] = '8';
                                    }
                                }
                            }
                            else if (GetMonthStatus() == 1)
                            {
                                if (tm.wDay > 31)
                                {
                                    m_time[12] = '3';
                                    m_time[12] = '1';
                                }
                            }
                            else if (GetMonthStatus() == 2)
                            {
                                if (tm.wDay > 30)
                                {
                                    m_time[12] = '3';
                                    m_time[13] = '0';
                                }
                            }

                            if (lParam >= '0' && lParam <= '1')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 8:
                            if (m_time[m_curPos - 1] == '0')
                            {
                                if (lParam >= '1' && lParam <= '9')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 4;
                                    m_flickPos += 2;
                                }
                            }
                            else if (m_time[m_curPos - 1] == '1')
                            {
                                if (lParam >= '0' && lParam <= '2')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 4;
                                    m_flickPos += 2;
                                }
                            }

                            memset(m_str, 0, sizeof(m_str));
                            tm.wYear = atoi(strncpy(m_str, m_time, 4));
                            memset(m_str, 0, sizeof(m_str));
                            tm.wDay = atoi(strncpy(m_str, m_time + 12, 2));
                            if (GetMonthStatus() == 3)
                            {
                                if ((tm.wYear % 4 == 0 && tm.wYear % 100 != 0) || (tm.wYear % 400 == 0))
                                {
                                    if (tm.wDay > 28)
                                    {
                                        m_time[12] = '2';
                                        m_time[13] = '9';
                                    }
                                }
                                else
                                {
                                    if (tm.wDay > 28)
                                    {
                                        m_time[12] = '2';
                                        m_time[13] = '8';
                                    }
                                }
                            }
                            else if (GetMonthStatus() == 1)
                            {
                                if (tm.wDay > 31)
                                {
                                    m_time[12] = '3';
                                    m_time[12] = '1';
                                }
                            }
                            else if (GetMonthStatus() == 2)
                            {
                                if (tm.wDay > 30)
                                {
                                    m_time[12] = '3';
                                    m_time[13] = '0';
                                }
                            }
                            break;
                        case 9:
                        case 10:
                        case 11:
                            break;
                        case 12:
                            if (GetMonthStatus() == 3)
                            {
                                if (lParam == '0')
                                {
                                    if (m_time[m_curPos + 1] == '0')
                                        m_time[m_curPos + 1] = '1';
                                }
                                else if (lParam == '2')
                                {
                                    memset(m_str, 0, sizeof(m_str));
                                    tm.wYear = atoi(strncpy(m_str, m_time, 4));
                                    if ((tm.wYear % 4 == 0 && tm.wYear % 100 != 0) || (tm.wYear % 400 == 0))
                                    {
                                    }
                                    else
                                    {
                                        if (m_time[m_curPos + 1] == '9')
                                            m_time[m_curPos] = '8';
                                    }
                                }

                                if (lParam >= '0' && lParam <= '2')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 1;
                                    m_flickPos += 1;
                                }
                            }
                            else if (GetMonthStatus() == 1)
                            {
                                if (lParam == '0')
                                {
                                    if (m_time[m_curPos + 1] == '0')
                                        m_time[m_curPos + 1] = '1';
                                }
                                else if (lParam == '3')
                                {
                                    if (m_time[m_curPos + 1] > '1')
                                        m_time[m_curPos + 1] = '1';
                                }

                                if (lParam >= '0' && lParam <= '3')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 1;
                                    m_flickPos += 1;
                                }
                            }
                            else if (GetMonthStatus() == 2)
                            {
                                if (lParam == '0')
                                {
                                    if (m_time[m_curPos + 1] == '0')
                                        m_time[m_curPos + 1] = '1';
                                }
                                else if (lParam == '3')
                                {
                                    if (m_time[m_curPos + 1] > '0')
                                        m_time[m_curPos + 1] = '0';
                                }

                                if (lParam >= '0' && lParam <= '3')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 1;
                                    m_flickPos += 1;
                                }
                            }
                            break;
                        case 13:
                            if (GetMonthStatus() == 3)
                            {
                                if (m_time[m_curPos - 1] == '0')
                                {
                                    if (lParam >= '1' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if (m_time[m_curPos - 1] == '1')
                                {
                                    if (lParam >= '0' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if (m_time[m_curPos - 1] == '2')
                                {
                                    memset(m_str, 0, sizeof(m_str));
                                    tm.wYear = atoi(strncpy(m_str, m_time, 4));
                                    if ((tm.wYear % 4 == 0 && tm.wYear % 100 != 0) || (tm.wYear % 400 == 0))
                                    {
                                        if (lParam >= '0' && lParam <= '9')
                                        {
                                            m_time[m_curPos] = lParam;
                                            m_curPos += 7;
                                            m_flickPos += 5;
                                        }
                                    }
                                    else
                                    {
                                        if (lParam >= '0' && lParam <= '8')
                                        {
                                            m_time[m_curPos] = lParam;
                                            m_curPos += 7;
                                            m_flickPos += 5;
                                        }
                                    }
                                }
                            }
                            else if (GetMonthStatus() == 1)
                            {
                                if (m_time[m_curPos - 1] == '0')
                                {
                                    if (lParam >= '1' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if (m_time[m_curPos - 1] == '1'
                                         || m_time[m_curPos - 1] == '2')
                                {
                                    if (lParam >= '0' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if ((m_time[m_curPos - 1] == '3'))
                                {
                                    if (lParam >= '0' && lParam <= '1')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                            }
                            else if (GetMonthStatus() == 2)
                            {
                                if (m_time[m_curPos - 1] == '0')
                                {
                                    if (lParam >= '1' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if (m_time[m_curPos - 1] == '1'
                                         || m_time[m_curPos - 1] == '2')
                                {
                                    if (lParam >= '0' && lParam <= '9')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                                else if ((m_time[m_curPos - 1] == '3'))
                                {
                                    if (lParam == '0')
                                    {
                                        m_time[m_curPos] = lParam;
                                        m_curPos += 7;
                                        m_flickPos += 5;
                                    }
                                }
                            }
                            break;
                        case 20:
                            if (lParam == '2')
                            {
                                if (m_time[m_curPos + 1] > '3')
                                    m_time[m_curPos + 1] = '3';
                            }

                            if (lParam >= '0' && lParam <= '2')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }

                            break;
                        case 21:
                            if (m_time[m_curPos - 1] == '2')
                            {
                                if (lParam >= '0' && lParam <= '3')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 2;
                                    m_flickPos += 2;
                                }
                            }
                            else if (m_time[m_curPos - 1] == '0'
                                     || m_time[m_curPos - 1] == '1')
                            {
                                if (lParam >= '0' && lParam <= '9')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 2;
                                    m_flickPos += 2;
                                }
                            }
                            break;
                        case 23:
                            if (lParam >= '0' && lParam <= '5')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 24:
                            if (lParam >= '0' && lParam <= '9')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos = 0;
                                m_flickPos = 0;
                            }
                            break;
                        default:
                            break;
                    }
                    m_pEditPwd->Input_show(m_time, m_flickPos);
                    m_pEditPwd->ShowFlick(TRUE);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void showDelete()
    {
        if (m_curPos == 1 || m_curPos == 2 || m_curPos == 3 || m_curPos == 8
                || m_curPos == 13 || m_curPos == 21 || m_curPos == 24)
        {
            m_curPos -= 1;
            m_flickPos -= 1;
        }
        else if (m_curPos == 7 || m_curPos == 12)
        {
            m_curPos -= 4;
            m_flickPos -= 2;
        }
        else if (m_curPos == 20)
        {
            m_curPos -= 7;
            m_flickPos -= 5;
        }
        else if (m_curPos == 23)
        {
            m_curPos -= 2;
            m_flickPos -= 2;
        }
        else if (m_curPos == 0)
        {
            m_curPos = 24;
            m_flickPos = 18;
        }
        m_pEditPwd->set_curpos(m_flickPos);
    }

    void setTimeAuto(BOOL status)
    {
        m_pButton->SetSrcpng(GetSmartAutoOnOff(status));
        m_pButton->Show(STATUS_NORMAL);
        m_auto = status;
    }

    void onCreate()
    {
        DPGetLocalTime(&tm);

        sprintf(m_time, "%04d%s%02d%s%02d%s   %02d:%02d",
                tm.wYear,
                GetStringByID(4222),
                tm.wMonth,
                GetStringByID(4223),
                tm.wDay,
                GetStringByID(4224),
                tm.wHour,
                tm.wMinute);

        m_pEditPwd->Input_show(m_time, m_flickPos);

        setTimeAuto(m_auto);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("mechine_clock.xml");
        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("ok", &m_idOK);
        GetCtrlByName("delete", &m_idDelete);
        m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);
        m_pButton = (CDPButton *)GetCtrlByName("auto", &m_idButton);

        m_curPos = 0;
        m_flickPos = 0;
        onCreate();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idOK;
    DWORD m_idDelete;
    DWORD m_idEditPwd;
    CEditBox *m_pEditPwd;
    DWORD m_idButton;
    CDPButton *m_pButton;

    SYSTEMTIME tm;
    char m_time[64];
    char m_str[32];
    DWORD m_curPos;
    DWORD m_flickPos;
};

CAppBase *CreatePrjSetDateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetDateApp *pApp = new CPrjSetDateApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
