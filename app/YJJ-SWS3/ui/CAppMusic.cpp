
#include "CCtrlModules.h"
#include "SmartConfig.h"

int pause_flag = 0;       // 暂停/播放的一个状态标志位
int Onoff_Music = 0;      // 背景音乐开关状态标志位
int P = 0;

int MUSIC_FALG = 0;
extern int Ctl_Flag;      // 2018.2.24添加，用于修改APP控制问题 

extern int G_MUSIC_PAUSE_FLAG;

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
		switch(uMsg)
		{
		
		case TIME_MESSAGE:

			if(m_dwTimeout < 30)
			{
				m_dwTimeout++;
				if(m_dwTimeout == 30)
				{
			
					MUSIC_FALG = 0;
					Ctl_Flag = 0;
					m_dwTimeout = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}	
			break;

		case MSG_BROADCAST:
		
			if(lParam == m_pSmartDev->addr) {
			
				if(wParam == SMART_STATUS_SYNC) {

					Ctl_Flag = 0;
					m_pSmartDev->status = zParam;
					
					if(m_pMusic->func == MUSIC_STATUS_ON) {

						pause_flag = 1;

						m_pPause->SetSrcpng(GetSmartPngMusic(pause_flag));
						m_pPause->Show(STATUS_NORMAL);
					}

					else if(m_pMusic->func == MUSIC_STATUS_OFF) {

						pause_flag = 0;
                        
						m_pPause->SetSrcpng(GetSmartPngMusic(pause_flag));
						m_pPause->Show(STATUS_NORMAL);

						if(m_pMusic->status == MUSIC_STATUS_ON ) {

							ShowAircStatus();
							break;
						}
					}

					else if(m_pMusic->func == MUSIC_STATUS_PAUSE) {

						if(pause_flag == 0)
							pause_flag = 1;
						else if(pause_flag == 1)
							pause_flag = 0;
					}

					if(m_pMusic->status == MUSIC_STATUS_OFF) {

						pause_flag = 0;
					}

					else if(m_pMusic->status == MUSIC_STATUS_ON ) {

						if(G_MUSIC_PAUSE_FLAG == 0)
							pause_flag = 1;
					}
			
					ShowAircStatus();	
				}

				// 应该是情景刷状态
				else if(wParam == SMART_STATUS_S) {

					if(zParam == 0x01) {
							
						pause_flag = 1;
						m_pSmartDev->status &= 0x7fff;
						m_pSmartDev->status |= 0x4000;
					}

					else if(zParam == 0x03) {
			
						pause_flag = 0;
						m_pSmartDev->status &= 0xbfff;
						m_pSmartDev->status |= 0x8000;
					}
					Ctl_Flag = 0;
					ShowAircStatus();
				}  
			}

			else if(wParam == SMART_STATUS_SCENE) {
			
				if(m_pMusic->status == MUSIC_STATUS_OFF) {

						pause_flag = 0;
				}

				else if(m_pMusic->status == MUSIC_STATUS_ON ) {

						pause_flag = 1;
				}
				ShowAircStatus();
			}

			G_MUSIC_PAUSE_FLAG = 0;
			break;
			
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			if(wParam == SLIDE_LEFT) {

				if(m_pMusic->voice>0) {
					
					if(m_pMusic->voice>2)
						m_pMusic->voice-=2;

					else if(m_pMusic->voice<=2)
						m_pMusic->voice-=1;
				}
	
				m_pProgress->SetProgressCur(m_pMusic->voice);         	
				m_pProgress->Show();
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, m_pMusic->voice, 0);
			}

			else if(wParam == SLIDE_RIGHT) {

				if(m_pMusic->voice<31) {

					if(m_pMusic->voice<29)
						m_pMusic->voice+=2;

					else if(m_pMusic->voice>=29)
						m_pMusic->voice+=1;
				}	
				m_pProgress->SetProgressCur(m_pMusic->voice);         	
				m_pProgress->Show();
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, m_pMusic->voice, 0);	
			}		
			break;
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			
			if(wParam == m_idBack)               // 返回
			{
				MUSIC_FALG = 0;
				Ctl_Flag = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			
			else if(wParam == m_idLastsong)      // 上一首
			{	
				m_pMusic->func = MUSIC_STATUS_LAST;
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();	
			}
			
			else if(wParam == m_idPause)         // 暂停 
			{
			//	m_pMusic->func = MUSIC_STATUS_PAUSE;

				if(pause_flag == 1) {

					pause_flag = 0;
					m_pMusic->func = MUSIC_STATUS_OFF;
				}
					
				else if(pause_flag == 0) {

					m_pMusic->func = MUSIC_STATUS_ON;
					pause_flag = 1;
				}
			
				m_pPause->SetSrcpng(GetSmartPngMusic(pause_flag));
				m_pPause->Show(STATUS_NORMAL);
								
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();
			}
			
			else if(wParam == m_idNextsong)      // 下一首
			{
		  		m_pMusic->func = MUSIC_STATUS_NEXT;
			//	m_pMusic->status = MUSIC_STATUS_ON;	
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();
			}

			else if(wParam == m_idOnOff)                        // 开关机按键
			{
				if(m_pMusic->status == MUSIC_STATUS_OFF)
				{
					pause_flag = 1;
					m_pMusic->status = MUSIC_STATUS_ON;	
				}
				else
				{
					pause_flag = 0;
					m_pMusic->status = MUSIC_STATUS_OFF;
				}
				ShowAircStatus();
				m_pMusic->func = 0;              // 要做到功能保持不变
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
			}	
					
			else if(wParam == m_idProgress)      // 音量进度条 
			{              
				char buf[32];

				if (lParam > 28)
					lParam = 31;
				if (lParam >= 25 && lParam <= 28)
					lParam += rand() % 4; 
				
				m_pMusic->status = MUSIC_STATUS_ON;	
				m_pMusic->voice = lParam;
				m_pProgress->SetProgressCur(m_pMusic->voice);     // 显示进度条	
				m_pProgress->Show();
		
				m_pMusic->func = 0;              // 要做到功能保持不变
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);			
				ShowAircStatus();
				
			}

			else if(wParam == m_idMusicSource) {  // 音源选择(采用按一下切换下一音源模式的方式)

				char buf[128];                    // 模式
				m_pMusic->source = (m_pMusic->source + 1) % 8;
				if(m_pMusic->source == 0)
				{
					// 去掉0值
					m_pMusic->source = 1;
				}
				// 显示当前音源
				sprintf(buf, "%s", GetStringByID(10500 + m_pMusic->source));		
				m_psource->SetSrc(buf);
				m_psource->Show(TRUE); 
				m_pMusic->status = MUSIC_STATUS_ON;	
				m_pMusic->func = 0;       // 要做到功能保持不变  
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);  // 发送命令
				ShowAircStatus();
			}

			else if(wParam == m_idAdd) {

				char buf[32];
				m_pMusic->status = MUSIC_STATUS_ON;	
				if(m_pMusic->voice < MUSIC_VOICE_MAX && m_pMusic->voice >= 0) {
	
					m_pMusic->voice++;
					m_pProgress->SetProgressCur(m_pMusic->voice);
					m_pProgress->Show();

				}

				m_pMusic->func = 0;  // 要做到功能保持不变
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();
			}

			else if(wParam == m_idSub) {

					char buf[32];		
					m_pMusic->status = MUSIC_STATUS_ON;	
					if(m_pMusic->voice <= MUSIC_VOICE_MAX && m_pMusic->voice > 0) {

						m_pMusic->voice--;
						m_pProgress->SetProgressCur(m_pMusic->voice);
						m_pProgress->Show();
						
					}

					m_pMusic->func = 0;  // 要做到功能保持不变
					SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
					ShowAircStatus();
			}

			break;
		}
	
		return TRUE;	
	}

	void ShowAircStatus()
	{
		char buf[128];

		if(m_pMusic->source == 0)
			sprintf(buf, "%s",GetStringByID(10501));
		else
			sprintf(buf, "%s",GetStringByID(10500+m_pMusic->source));
		
		m_psource->SetSrc(buf);     // 切换显示音源
		m_psource->Show(TRUE);

		m_pProgress->SetProgressCur(m_pMusic->voice);         // 显示进度条	
		m_pProgress->Show();

		if(m_pMusic->voice <= 30) {

			if(m_pMusic->voice >1)
				sprintf(buf, "%d%%", 3*(m_pMusic->voice+3));
			else if(m_pMusic->voice == 0)
				sprintf(buf, "%d%%", 0);
			
		}

		else if(m_pMusic->voice == 31) {

			sprintf(buf, "%d%%", 100);
		}
	
		m_pPause->SetSrcpng(GetSmartPngMusic(pause_flag));
		m_pPause->Show(STATUS_NORMAL);	

		if(m_pMusic->status == MUSIC_STATUS_ON || (m_pSmartDev->status & 0x8000) == 0)
			Onoff_Music = 0;
		else if(m_pMusic->status == MUSIC_STATUS_OFF || (m_pSmartDev->status & 0x8000) >0)
			Onoff_Music = 1;
		
		m_pOnOff->SetSrcpng(GetSmartPngOnOff(Onoff_Music));
		m_pOnOff->Show(STATUS_NORMAL);	
	}
	
	void OnCreate(SmartDev* pSmartDev, DWORD status)
	{

		char buf[32];
#ifdef DPCE
		pSmartDev->status = 0x4551;
#endif
		// 显示名称
		m_pTitle->SetSrc(pSmartDev->name);                    // 背景音乐标题
		m_pTitle->Show(TRUE);
	
		m_pSmartDev = pSmartDev;                              // 地址赋值

		m_pProgress->SetProgressTotal(MUSIC_VOICE_MAX);
		
		m_pMusic = (MUSIC_DATA *)&pSmartDev->status;          // 地址传送
		
#if 0
		sprintf(buf, "%d%%", m_pSmartDev->param1);
		m_pPercent->SetSrc(buf);
		m_pPercent->Show(TRUE);
#endif
		SmartGetStatus(pSmartDev->addr);
		m_pMusic->status == MUSIC_STATUS_OFF;

		// 2018.2.24添加，用于修改APP控制问题
		if(status != 0)  
			pSmartDev->status = status;

		ShowAircStatus();

	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("music.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);                // 返回按键
		GetCtrlByName("btn_lastsong", &m_idLastsong);    // 上一首
	//	GetCtrlByName("btn_pause", &m_idPause);          // 暂停
		GetCtrlByName("btn_nextsong", &m_idNextsong);    // 下一首
		GetCtrlByName("btn_choice", &m_idMusicSource);   // 音源选择按键 
		GetCtrlByName("add", &m_idAdd);
		GetCtrlByName("sub", &m_idSub);
		
		m_pPause    = (CDPButton *)GetCtrlByName("btn_pause", &m_idPause);	
			
		m_pTitle    = (CDPStatic *)GetCtrlByName("title");                          // 标题显示
		m_pOnOff    = (CDPButton *)GetCtrlByName("btn_onoff", &m_idOnOff);          // 开关按键
		m_psource   = (CDPStatic *)GetCtrlByName("source");                         // 音源显示 
		m_pPercent  = (CDPStatic *)GetCtrlByName("percent");
		m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);      // 音量进度条
		
		OnCreate((SmartDev*)lParam, zParam);                                            
		
		return TRUE;
	}

private:
	DWORD m_idBack;         // 返回按键ID
	DWORD m_idLastsong;     // 上一首按键ID
	DWORD m_idPause;        // 暂停按键ID
	DWORD m_idNextsong;     // 下一首按键ID
	DWORD m_idProgress;     // 进度条ID
	DWORD m_idOnOff;        // 开关机按键ID
	DWORD m_idMusicSource;  // 音乐源选择按键ID
	DWORD m_idAdd;
	DWORD m_idSub;

	CDPStatic* m_pTitle;    // 标题  
	CDPButton* m_pOnOff;    // 开关
	CDPButton* m_pPause;    // 暂停
	CDPStatic* m_psource;   // 音源显示
	CDPStatic* m_pPercent;  // 百分比
	CDPProgress* m_pProgress;

	SmartDev* m_pSmartDev;
	MUSIC_DATA* m_pMusic;   // 背景音乐设置协议参数
};

CAppBase* CreateMUSICApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMusicApp* pApp = new CMusicApp(wParam);
	MUSIC_FALG = 1;
	Ctl_Flag = 0;           // 解决一级界面控制而定义的变量
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}










