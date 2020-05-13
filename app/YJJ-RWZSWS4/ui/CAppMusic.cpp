#include "CCtrlModules.h"

static BOOL g_is_play = FALSE;       // 暂停播放的一个状态标志位
static BOOL g_init = FALSE; 

class CMusicApp : public CAppBase
{
public:
    CMusicApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CMusicApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, onPage, 0);
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
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, onPage, 0);
                }
                else if (wParam == m_idLastsong)
                {
                    m_pMusic->func = MUSIC_STATUS_LAST;
                    g_is_play = TRUE;
                    m_pMusic->status = MUSIC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    //ShowAircStatus();
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();
                }
                else if (wParam == m_idNextsong)
                {
                    m_pMusic->func = MUSIC_STATUS_NEXT;
                    g_is_play = TRUE;
                    m_pMusic->status = MUSIC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    //ShowAircStatus();
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();
                }
                else if(wParam == m_idPause)
                {
                    if (g_is_play == TRUE)
                    {
                        g_is_play = FALSE;
                        m_pMusic->func = MUSIC_STATUS_OFF;
                    }
                    else if (g_is_play == FALSE)
                    {
                        g_is_play = TRUE;
                        m_pMusic->func = MUSIC_STATUS_ON;
                        m_pMusic->status = MUSIC_STATUS_ON;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    m_pPause->SetSrcpng(GetSmartPngMusic(g_is_play));
                    m_pPause->Show(STATUS_NORMAL);
                    //ShowAircStatus();
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();
                }
                else if (wParam == m_idOnOff)
                {
                    m_pMusic->status = (m_pMusic->status == MUSIC_STATUS_OFF) ? MUSIC_STATUS_ON : MUSIC_STATUS_OFF;
                    if (m_pMusic->status == MUSIC_STATUS_ON)
                    {
                        g_is_play = TRUE;
                        m_pMusic->func = MUSIC_STATUS_ON;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    else
                    {
                        g_is_play = FALSE;
                        m_pMusic->func = MUSIC_STATUS_OFF;
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    }
                    //ShowAircStatus();
                    //m_pMusic->func = 0;
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();         
                }
                else if (wParam == m_idMusicSource)
                {
                    m_pMusic->source = (m_pMusic->source + 1) % 8;
                    if (m_pMusic->source == 0)
                        m_pMusic->source = 1;
                    m_pMusic->status = MUSIC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    //ShowAircStatus();
                    m_pMusic->func = 0;
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();
                }
                else if (wParam == m_idAdd)
                {
                    m_pMusic->status = MUSIC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    if (m_pMusic->voice < MUSIC_VOICE_MAX
                            && m_pMusic->voice >= 0)
                    {
                        m_pMusic->voice++;
                    }
                    //ShowAircStatus();
                    m_pMusic->func = MUSIC_STATUS_ADD;
                    //SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
					SendMusicCmd();
                }
                else if (wParam == m_idSub)
                {
                    m_pMusic->status = MUSIC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    if (m_pMusic->voice <= MUSIC_VOICE_MAX
                            && m_pMusic->voice > 1)
                    {
                        m_pMusic->voice--;
                    }
                    m_pMusic->func = MUSIC_STATUS_SUB;
                    SendMusicCmd();
                }
                else if (wParam == m_idProgress)
                {
                    if (zParam == 1)
                    {
                        m_pProgress->SetProgressCur(lParam);
                        m_pProgress->Show();
                        break;
                    }
                    if (m_pMusic->status == MUSIC_STATUS_OFF)
                    {
                        m_pMusic->status = MUSIC_STATUS_ON;
                        m_pOnOff->SetSrcpng(GetSmartPngOnOff(1));
                        m_pOnOff->Show(STATUS_NORMAL);
                    }
                    m_pSmartDev->cmd = SCMD_OPEN;
					if (lParam)
                    	m_pMusic->voice = lParam;
					else
						m_pMusic->voice = 1;
                    m_pMusic->func = 0;
					SendMusicCmd();
                }
                break;
        }

        return TRUE;
    }

    void ShowAircStatus()
    {
		if (m_pSmartDev->status == 0)
		{
			m_pSmartDev->status = m_pSmartDev->param1;
			if (m_pSmartDev->cmd == SCMD_OPEN)
			{
				m_pMusic->status = MUSIC_STATUS_ON;
				m_pMusic->func == MUSIC_STATUS_ON;
			}
			else
			{
				m_pMusic->status = MUSIC_STATUS_OFF;
				m_pMusic->func == MUSIC_STATUS_OFF;
			}
		}
		else
		{
			m_pSmartDev->param1 = m_pSmartDev->status;
		}

        if (m_pMusic->func == MUSIC_STATUS_ON)
        {
			g_is_play = TRUE;
            m_pPause->SetSrcpng(GetSmartPngMusic(1));
            m_pPause->Show(STATUS_NORMAL);
        }
        else if (m_pMusic->func == MUSIC_STATUS_OFF)
        {
			g_is_play = FALSE;
            m_pPause->SetSrcpng(GetSmartPngMusic(0));
            m_pPause->Show(STATUS_NORMAL);
        }
		else
		{			
			if (m_pMusic->status == MUSIC_STATUS_ON && g_is_play == TRUE)
				m_pPause->SetSrcpng(GetSmartPngMusic(1));
			else
			{
				g_is_play = FALSE;
				m_pPause->SetSrcpng(GetSmartPngMusic(0));
			}
			m_pPause->Show(STATUS_NORMAL);
		}
		
        if (m_pMusic->source > 0)
        {
            sprintf(m_buf, "%s", GetStringByID(10500 + m_pMusic->source));
            m_psource->SetSrc(m_buf);
            m_psource->Show(TRUE);
        }

		if (m_pMusic->voice > 0)
		{
	        if (m_pMusic->voice == 1)
	            m_pProgress->SetProgressCur(0);
	        else
	            m_pProgress->SetProgressCur(m_pMusic->voice);
	       	m_pProgress->Show();
		}

        if (m_pMusic->status == MUSIC_STATUS_ON)
            m_pOnOff->SetSrcpng(GetSmartPngOnOff(1));
        else
            m_pOnOff->SetSrcpng(GetSmartPngOnOff(0));
        m_pOnOff->Show(STATUS_NORMAL);
    }


	void SendMusicCmd()
	{
		m_pSmartDev->param1 = m_pSmartDev->status;
		ShowAircStatus();
		SendSmartCmd(&m_pSmartDev->device, SCMD_MUSIC, m_pSmartDev->status);
	}

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pTitle->SetSrc(pSmartDev->name);                    // 背景音乐标题
        m_pTitle->Show(TRUE);

        m_pSmartDev = pSmartDev;                              // 地址赋值

        m_pProgress->SetProgressTotal(MUSIC_VOICE_MAX);

        m_pMusic = (MUSIC_DATA *)&m_pSmartDev->status;          // 地址传送

        m_pMusic->status = (m_pSmartDev->cmd == SCMD_OPEN) ? MUSIC_STATUS_ON : MUSIC_STATUS_OFF;

		if (g_init == FALSE)
		{
			m_pMusic->voice = 16;
			g_init = TRUE;
		}

        ShowAircStatus();

        SmartGetStatus(&m_pSmartDev->device);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("music.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("last", &m_idLastsong);
        GetCtrlByName("next", &m_idNextsong);
        GetCtrlByName("select", &m_idMusicSource);
        GetCtrlByName("add", &m_idAdd);
        GetCtrlByName("sub", &m_idSub);

        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_psource = (CDPStatic *)GetCtrlByName("source");
        m_pPercent = (CDPStatic *)GetCtrlByName("percent");
        m_pPause = (CDPButton *)GetCtrlByName("run", &m_idPause);
        m_pOnOff = (CDPButton *)GetCtrlByName("power", &m_idOnOff);
        m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);
        OnCreate((SmartDev *)lParam);
        onPage = zParam;

        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idLastsong;
    DWORD m_idPause;
    DWORD m_idNextsong;
    DWORD m_idProgress;
    DWORD m_idOnOff;
    DWORD m_idMusicSource;
    DWORD m_idAdd;
    DWORD m_idSub;
    DWORD onPage;

    CDPStatic *m_pTitle;
    CDPStatic *m_psource;
    CDPStatic *m_pPercent;
    CDPButton *m_pOnOff;
    CDPButton *m_pPause;
    CDPProgress *m_pProgress;

    SmartDev *m_pSmartDev;
    MUSIC_DATA *m_pMusic;
    char m_buf[64];
};

CAppBase *CreateMUSICApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CMusicApp *pApp = new CMusicApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
