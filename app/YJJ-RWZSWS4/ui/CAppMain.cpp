#include "CCtrlModules.h"
#include "SmartConfig.h"
#include <roomlib.h>

class CMainApp : public CAppBase
{
public:
    CMainApp(DWORD hWnd) : CAppBase(hWnd)
    {

    }

    ~CMainApp()
    {

    }

    void ResumeAck(void)
    {
        m_dwTimeout = 0;
        OnPage(m_mapPage[m_nPage]);
        return CAppBase::ResumeAck();
    }

    void ScreenoffShow()
    {
        // ������++, ����(m_screenoff = 0)
        if (++m_dwTimeout == m_screenoff)
        {
            DPPostMessage(MSG_START_FROM_ROOT, CLOCK_APPID, 0, 0);
        }

		#if 1
		if (m_bright == TRUE)
		{
			SetScreenOnOff(TRUE);
			m_bright = FALSE;
		}
		#endif
    }

    void TimeShow()
    {
        static int minute = -1;
		static int hour = -1;
        static int day = -1;
		static int month = -1;
		static int year = -1;

        DPGetLocalTime(&m_tm);
        sprintf(m_buf, "%02d:%02d", m_tm.wHour, m_tm.wMinute);

        if (m_tm.wMinute != minute || m_tm.wHour != hour || showTime == TRUE)
        {
            m_pTitle[0]->Show(FALSE);
            m_pTitle[2]->Show(FALSE);
            m_pTitle[2]->SetSrc(m_buf);
            m_pTitle[2]->Show(TRUE);
            m_pTitle[0]->RefreshBak();
            m_pTitle[0]->SetSrc(m_buf);
            m_pTitle[0]->Show(TRUE);
            minute = m_tm.wMinute;
			hour = m_tm.wHour;
        }

        if (m_tm.wDay != day || m_tm.wMonth != month || m_tm.wYear != year || showTime == TRUE)
        {
            sprintf(m_buf, "%04d/%02d/%02d  %s", m_tm.wYear, m_tm.wMonth, m_tm.wDay, GetStringByID(1000 + m_tm.wDayOfWeek));
            m_pTitle[1]->Show(FALSE);
            m_pTitle[3]->Show(FALSE);
            m_pTitle[3]->SetSrc(m_buf);
            m_pTitle[3]->Show(TRUE);
            m_pTitle[1]->RefreshBak();
            m_pTitle[1]->SetSrc(m_buf);
            m_pTitle[1]->Show(TRUE);
            day = m_tm.wDay;
			month = m_tm.wMonth;
			year = m_tm.wYear;
        }

        showTime = FALSE;
    }

    void CfgTipShow()
    {
        static DWORD index = 0;

        if (GetCfgStatus() == TRUE)
        {
            m_pTitle[4]->SetSrc(GetStringByID(9000 + index));
            m_pTitle[4]->Show(TRUE);
            if (index++ == 5)
                index = 0;
        }
        else
        {
            m_pTitle[4]->Show(FALSE);
            m_pTitle[4]->RefreshBak();
        }
    }

    void ShowDevStatus(SmartDev *pSmartDev)
    {
        pSmartDev->cmd = (pSmartDev->cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;
        SetStatusBySync(&pSmartDev->device, pSmartDev->cmd, 0);
        if (pSmartDev->device.id == GetDevID()
                && pSmartDev->device.type == GetDevType())
        {
            if (pSmartDev->cmd == SCMD_OPEN)
                SetLightGpioVal(pSmartDev->device.channel, FALSE);
            else
                SetLightGpioVal(pSmartDev->device.channel, TRUE);
            SendStatusCmd(&pSmartDev->device, pSmartDev->cmd, 0);
        }
        else
        {
            SendSmartCmd(&pSmartDev->device, pSmartDev->cmd, 0);
        }
    }

    void ShowSceneStatus(SmartDev *pSmartDev)
    {
        pSmartDev->cmd = SCMD_OPEN;
        SendSmartCmd(&pSmartDev->device, SCMD_SCENE, pSmartDev->scene);
        SetStatusByScene(pSmartDev->scene);
        DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                TimeShow();											// ˢ��ʱ��
                ScreenoffShow();								  	// ������ʾ
                CfgTipShow();										// ������ʾ
                break;
            case MSG_BROADCAST:									  	// �յ��㲥��Ϣ����ҳ��״̬
                OnPage(m_mapPage[m_nPage]);
                break;
            case TOUCH_SLIDE:                                    		// ִ�еĻ������������ݡ�
                m_dwTimeout = 0;
                switch (wParam)
                {
                    case SLIDE_LEFT:
                        if (m_nPage < m_lPage - 1)
                        {
                            DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_nPage + 1, 0);
                        }
                        else
                        {
                            DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                        }
                        break;
                    case SLIDE_RIGHT:
                        if (m_nPage > 0)
                        {
                            DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_nPage - 1, 0);
                        }
                        else
                        {
                            DPPostMessage(MSG_START_APP, MAIN_APPID, m_lPage - 1, 0);
                        }
                        break;
                    case SLIDE_UPSIDE:
                        DPPostMessage(MSG_START_FROM_ROOT, PWD_INPUT_APPID, 0, 0);
                        break;
                    case SLIDE_DOWN:
                        //DPPostMessage(MSG_START_FROM_ROOT, PROJECT_APPID, 0, 0);
                        //DPPostMessage(MSG_START_APP, SCENE_APPID, 0, 0);
                        //DPPostMessage(MSG_START_APP, CLOCK_APPID, 0, 0);
                        break;
                }
                break;
            case MSG_PRIVATE:
                if (wParam == m_IdBase)
                {
                    if (lParam == MSG_SMART_UPDATE)
                    {
                        //OnPage(m_mapPage[m_nPage]);
                        DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_nPage, 0);
                    }
                }
                break;
            case TOUCH_MESSAGE:                                // ������Ϣ
                m_dwTimeout = 0;
                if ((wParam >= m_idButton[0]) && (wParam <= m_idButton[m_nCount - 1]))
                {
                    int index = wParam - m_idButton[0];
                    SmartDev *pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM + m_mapButton[index]];
                    switch (pSmartDev->type)
                    {
                        case ST_LIGHT_A ... ST_LIGHT_D:
                            ShowDevStatus(pSmartDev);
                            //DPPostMessage(MSG_START_FROM_OVER, LIGHT_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_DIMMER_A ... ST_DIMMER_D:                        //����
                            DPPostMessage(MSG_START_FROM_OVER, DIMMER_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_CURTAIN_A ... ST_CURTAIN_D:                      //����
                            DPPostMessage(MSG_START_FROM_OVER, CURTAIN_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_AC_A ... ST_AC_D:                              //�յ�
                            DPPostMessage(MSG_START_FROM_OVER, AIRC_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_IRAIR_A ... ST_IRAIR_D:						  //����յ�
                            DPPostMessage(MSG_START_FROM_OVER, IR_AIR_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_HEAT_A ... ST_HEAT_D:                           //��ů
                            DPPostMessage(MSG_START_FROM_OVER, HEAT_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_WINDOW_A ... ST_WINDOW_D:                        //����
                            DPPostMessage(MSG_START_FROM_OVER, WINDOW_APPID, (DWORD)pSmartDev, 0);
                            break;
						case ST_WIND_A ... ST_WIND_D: 						  // �·�
                            DPPostMessage(MSG_START_FROM_OVER, WIND_APPID, (DWORD)pSmartDev, m_nPage);
                            break;	
                        case ST_OUTLET_A ... ST_OUTLET_D:                        //����
                            ShowDevStatus(pSmartDev);
                            //DPPostMessage(MSG_START_FROM_OVER, OUTLET_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_SCENE_A ... ST_SCENE_Z:	 						//�龰
                            ShowSceneStatus(pSmartDev);
                            //DPPostMessage(MSG_START_FROM_OVER, SCENE_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_TV_A ... ST_TV_D:    								// �������
                            DPPostMessage(MSG_START_FROM_OVER, TV_APPID, (DWORD)pSmartDev, 0);
                            break;
                        case ST_MUSIC_A ... ST_MUSIC_D: 							// ��������
                            DPPostMessage(MSG_START_FROM_ROOT, MUSIC_APPID, (DWORD)pSmartDev, m_nPage);
                            break;	
                        case ST_LOCK_A ... ST_LOCK_D:
                            ShowDevStatus(pSmartDev);
                            break;
                        default:
                            break;
                    }
                }
                OnPage(m_mapPage[m_nPage]);
                break;
            case TOUCH_ACTIVE:
                if ((wParam >= m_idButton[0]) && (wParam <= m_idButton[m_nCount - 1]))
                {
                    int index = wParam - m_idButton[0];
                    SmartDev *pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM + m_mapButton[index]];
                    if (pSmartDev->type >= ST_SCENE_A && pSmartDev->type <= ST_SCENE_Z)
                        break;
                    DPPostMessage(MSG_START_FROM_OVER, UI_APPID, (DWORD)pSmartDev, 0);
                }
                break;
        }

        return TRUE;
    }

    void OnPage(DWORD nPage)
    {
        // ��ʾҳ����״̬
        for (DWORD i = 0; i < m_lPage; i++)
        {
            if (m_mapPage[i] == nPage)
            {
                m_ppoint[m_nPoint + i]->SetSrcpng(GetSmartPngPoint(1));
                m_ppoint[m_nPoint + i]->Show(STATUS_NORMAL);
            }
            else
            {
                m_ppoint[m_nPoint + i]->SetSrcpng(GetSmartPngPoint(0));
                m_ppoint[m_nPoint + i]->Show(STATUS_NORMAL);
            }
        }

        // ͼ��״̬�Լ��ı�����ʾ
        SmartDev *pSmartDev = &m_pSmartDev[nPage * MAX_ICON_NUM];
        for (DWORD k = 0; k < m_nCount; k++)
        {
            int i = m_mapButton[k];
            //����յ��ͺ������
            if ((pSmartDev[i].type >= ST_IRAIR_A && pSmartDev[i].type <= ST_IRAIR_D) ||
                    (pSmartDev[i].type >= ST_TV_A && pSmartDev[i].type <= ST_TV_D))
            {
                m_pButton[k]->SetSrcpng(STATUS_NORMAL, GetSmartPng(pSmartDev[i].type, ICON_ON_NORMAL));
                m_pButton[k]->SetSrcpng(STATUS_PRESSED, GetSmartPng(pSmartDev[i].type, ICON_ON_LITTLE));
            }
            else
            {
                if (pSmartDev[i].cmd == SCMD_OPEN)
                {
                    m_pButton[k]->SetSrcpng(STATUS_NORMAL, GetSmartPng(pSmartDev[i].type, ICON_ON_NORMAL));
                    m_pButton[k]->SetSrcpng(STATUS_PRESSED, GetSmartPng(pSmartDev[i].type, ICON_ON_LITTLE));
                }
                else
                {
					pSmartDev[i].cmd = SCMD_CLOSE;
                    m_pButton[k]->SetSrcpng(STATUS_NORMAL, GetSmartPng(pSmartDev[i].type, ICON_OFF_NORMAL));
                    m_pButton[k]->SetSrcpng(STATUS_PRESSED, GetSmartPng(pSmartDev[i].type, ICON_OFF_LITTLE));
                }
            }
            m_pButton[k]->SetSrcText(pSmartDev[i].name);
            m_pButton[k]->Show(STATUS_NORMAL);
        }
    }

    BOOL Create(DWORD lParam, DWORD zParam)          // lParam�ǵ�ǰ�ڼ�ҳ
    {
        m_nCount = 0;
        m_lPage = 0;
        showTime = TRUE;
		m_bright = TRUE;

        if (lParam >= 0 && lParam < MAX_PAGE_NUM)
            m_nPage = lParam;
        else
            m_nPage = 0;

        m_pSmartDev = GetSmartDev(&m_dwCount);

        // ���ҳ��
        for (DWORD k = 0; k < MAX_PAGE_NUM; k++)
        {
            SmartDev *pSmartDev = &m_pSmartDev[k * MAX_ICON_NUM];
            for (DWORD i = 0; i < MAX_ICON_NUM; i++)
            {
                if (pSmartDev[i].exist)
                {
                    m_mapPage[m_lPage++] = k;
                    break;
                }
            }
        }

        // ��ǰҳ�豸����
        for (DWORD i = 0; i < MAX_ICON_NUM; i++)
        {
            SmartDev *pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM];
            if (pSmartDev[i].exist)
            {
                m_mapButton[m_nCount++] = i;
            }
        }

        // ����XML�ļ�
        if (m_nCount > 0 && m_nCount < 10)
        {
            sprintf(m_buf, "main%d.xml", m_nCount);
            InitFrame(m_buf);
        }
        else
            InitFrame("main.xml");

        /* ����ͼ��λ�� */
        for (DWORD i = 0; i < m_nCount; i++)
        {
            sprintf(m_buf, "button%d", i + 1);
            m_pButton[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idButton[i]);
        }

        /* ����ҳ����λ�� */
        m_nPoint = MAX_PAGE_NUM / 2 - (m_lPage + 1) / 2;
        for (DWORD i = 0; i < m_lPage; i++)
        {
            sprintf(m_buf, "point%d", m_nPoint + i + 1);
            m_ppoint[m_nPoint + i]   = (CDPButton *)GetCtrlByName(m_buf, &m_idpoint[m_nPoint + i]);
        }

        /* �����ı�λ�� */
        for (int i = 0; i < 5; i++)
        {
            sprintf(m_buf, "title%d", i + 1);
            m_pTitle[i] = (CDPStatic *)GetCtrlByName(m_buf);
        }
			
        OnPage(m_mapPage[m_nPage]);

        m_screenoff = GetPrjShow();
        // ʱ����ʾ
        TimeShow();
        // ������ʾ
        CfgTipShow();

        return TRUE;
    }

private:
    DWORD m_nPage;									  // ��ǰҳ��
    DWORD m_lPage;									  // ���ҳ��
    DWORD m_nPoint;									  // ��ʼ����
    DWORD m_nCount;									  // ��ǰҳ�豸����

    DWORD m_dwCount;                                  // �豸����
    SmartDev *m_pSmartDev;    						  // �豸�б�

    DWORD m_idButton[MAX_ICON_NUM];					  // ͼ��id
    DWORD m_idpoint[MAX_PAGE_NUM]; 					  // ҳ����id
    CDPButton *m_pButton[MAX_ICON_NUM];				  // ͼ����-thisָ��
    CDPButton *m_ppoint[MAX_PAGE_NUM];               	  // ҳ������-thisָ��
    CDPStatic *m_pTitle[5];							  // �ı����-thisָ��
    BYTE m_mapPage[MAX_PAGE_NUM];					  	  // ҳӳ��(��ʾ���洢)
    BYTE m_mapButton[MAX_ICON_NUM];					  // ͼ��ӳ��(��ʾ���洢)

    SYSTEMTIME m_tm;                                  // ϵͳʱ��
    BOOL showTime;
    char m_buf[64];
	BOOL m_bright;
};

CAppBase *CreateMainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CMainApp *pApp = new CMainApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
