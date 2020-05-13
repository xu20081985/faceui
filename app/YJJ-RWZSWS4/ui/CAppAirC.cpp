#include "CCtrlModules.h"

class CAirCApp : public CAppBase
{
public:
    CAirCApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CAirCApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                if (wParam == SMART_STATUS_SYNC)
                {
                    ShowAircStatus();
                }
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idMode)
                {
                    m_pAirC->mode = (m_pAirC->mode + 1) % AC_MODE_MAX;
                    if (m_pAirC->mode == 0)
                        m_pAirC->mode = 1;

                    sprintf(m_buf, "%s: %s", GetStringByID(10000), GetStringByID(10040 + m_pAirC->mode));
                    m_pMode->SetSrc(m_buf);
                    m_pMode->Show(TRUE);
                    m_pAirC->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
                else if (wParam == m_idSpeed)
                {
                    m_pAirC->speed = (m_pAirC->speed + 1) % AC_SPEED_MAX;
                    if (m_pAirC->speed == 0)
                        m_pAirC->speed = 1;

                    sprintf(m_buf, "%s: %s", GetStringByID(10001), GetStringByID(10080 + m_pAirC->speed));
                    m_pSpeed->SetSrc(m_buf);
                    m_pSpeed->Show(TRUE);
                    m_pAirC->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
                else if (wParam == m_idOnOff)
                {
                    if (m_pAirC->onoff == AC_STATUS_OFF)
                    {
                        m_pAirC->onoff = AC_STATUS_ON;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    else
                    {
                        m_pAirC->onoff = AC_STATUS_OFF;
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    }
                    SendTempCmd(m_pAirC->temp);
                }
                else if (wParam == m_idSub)
                {
                    if (--m_pAirC->temp < 7)
                    {
                        m_pAirC->temp = 25;
                    }
                    sprintf(m_buf, "%d", m_pAirC->temp + 9);
                    m_pSetTemp->SetSrc(m_buf);
                    m_pSetTemp->Show(TRUE);
                    m_pAirC->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
                else if(wParam == m_idAdd)
                {
                    if (++m_pAirC->temp > 25)
                    {
                        m_pAirC->temp = 7;
                    }
                    sprintf(m_buf, "%d", m_pAirC->temp + 9);
                    m_pSetTemp->SetSrc(m_buf);
                    m_pSetTemp->Show(TRUE);
                    m_pAirC->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
				ShowAircStatus();
                break;
            default:
                break;
        }

        return TRUE;
    }

    void SendTempCmd(int temp)
    {
        WORD status = m_pSmartDev->status;
		m_pSmartDev->param1 = m_pSmartDev->status;
        AC_DATA *pAirC = (AC_DATA *)&status;
        pAirC->temp = temp;
        SendSmartCmd(&m_pSmartDev->device, SCMD_AC, status);
    }

    void ShowAircStatus()
    {
		if (m_pSmartDev->status == 0)
		{
			m_pSmartDev->status = m_pSmartDev->param1;
			if (m_pSmartDev->cmd == SCMD_OPEN)
				m_pAirC->onoff = AC_STATUS_ON;
			else
				m_pAirC->onoff = AC_STATUS_OFF;
		}
		else
		{
			m_pSmartDev->param1 = m_pSmartDev->status;
		}
		
        // 模式
        if (m_pAirC->mode > 0)
        {
            sprintf(m_buf, "%s: %s", GetStringByID(10000), GetStringByID(10040 + m_pAirC->mode));
            m_pMode->SetSrc(m_buf);
            m_pMode->Show(TRUE);
        }

        // 风速
        if (m_pAirC->speed > 0)
        {
            sprintf(m_buf, "%s: %s", GetStringByID(10001), GetStringByID(10080 + m_pAirC->speed));
            m_pSpeed->SetSrc(m_buf);
            m_pSpeed->Show(TRUE);
        }

        // 设置温度
        if (m_pAirC->temp > 0)
        {
            sprintf(m_buf, "%02d", m_pAirC->temp + 9);
            m_pSetTemp->SetSrc(m_buf);
            m_pSetTemp->Show(TRUE);
        }

        // 室内温度
        sprintf(m_buf, "%s: %02d%s", GetStringByID(10003), GetEnvTemp(), GetStringByID(10006));
        m_pCurTemp->SetSrc(m_buf);
        m_pCurTemp->Show(TRUE);

        // 开关机
        if (m_pAirC->onoff > 0)
        {
            if (m_pAirC->onoff == AC_STATUS_ON || m_pSmartDev->cmd == SCMD_OPEN)
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(1));
            else
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(0));
            m_pOnOff->Show(STATUS_NORMAL);
        }
    }

    void show()
    {
        if (m_pAirC->onoff == 0)
        {
            if (m_pSmartDev->cmd == SCMD_OPEN)
                m_pAirC->onoff = 1;
            else
                m_pAirC->onoff = 2;
        }

        if (m_pAirC->mode == 0)
        {
            m_pAirC->mode = 2;
        }

        if (m_pAirC->func == 0)
        {
            m_pAirC->func = 1;
        }

        if (m_pAirC->speed == 0)
        {
            m_pAirC->speed = 4;
        }

        if (m_pAirC->temp == 0)
        {
            m_pAirC->temp = 17;
        }

        ShowAircStatus();
    }

    void OnCreate(SmartDev *pSmartDev, DWORD status)
    {
        m_pSmartDev = pSmartDev;

        m_pTitle->SetSrc(m_pSmartDev->name);
        m_pTitle->Show(TRUE);

        m_pAirC = (AC_DATA *)&m_pSmartDev->status;

        SmartGetStatus(&m_pSmartDev->device);

        show();
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("airc.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("run_mode", &m_idMode);
        GetCtrlByName("wind_speed", &m_idSpeed);
        GetCtrlByName("sub", &m_idSub);
        GetCtrlByName("add", &m_idAdd);

        m_pOnOff = (CDPButton *)GetCtrlByName("power", &m_idOnOff);
        m_pTitle    = (CDPStatic *)GetCtrlByName("title");
        m_pMode     = (CDPStatic *)GetCtrlByName("mode");
        m_pSpeed    = (CDPStatic *)GetCtrlByName("speed");
        m_pSetTemp  = (CDPStatic *)GetCtrlByName("set_temp");
        m_pCurTemp  = (CDPStatic *)GetCtrlByName("cur_temp");

        OnCreate((SmartDev *)lParam, zParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idMode;
    DWORD m_idSpeed;
    DWORD m_idOnOff;
    DWORD m_idSub;
    DWORD m_idAdd;

    CDPButton *m_pOnOff;
    CDPStatic *m_pTitle;
    CDPStatic *m_pMode;
    CDPStatic *m_pSpeed;
    CDPStatic *m_pSetTemp;
    CDPStatic *m_pCurTemp;

    SmartDev *m_pSmartDev;
    AC_DATA *m_pAirC;
    char m_buf[64];
};

CAppBase *CreateAirCApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CAirCApp *pApp = new CAirCApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}