
#include "CCtrlModules.h"
#include "SmartConfig.h"

int pause_flag = 0;       // ��ͣ/���ŵ�һ��״̬��־λ
int Onoff_Music = 0;      // �������ֿ���״̬��־λ
int P = 0;

int MUSIC_FALG = 0;
extern int Ctl_Flag;      // 2018.2.24��ӣ������޸�APP�������� 

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

				// Ӧ�����龰ˢ״̬
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
			
			if(wParam == m_idBack)               // ����
			{
				MUSIC_FALG = 0;
				Ctl_Flag = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			
			else if(wParam == m_idLastsong)      // ��һ��
			{	
				m_pMusic->func = MUSIC_STATUS_LAST;
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();	
			}
			
			else if(wParam == m_idPause)         // ��ͣ 
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
			
			else if(wParam == m_idNextsong)      // ��һ��
			{
		  		m_pMusic->func = MUSIC_STATUS_NEXT;
			//	m_pMusic->status = MUSIC_STATUS_ON;	
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
				ShowAircStatus();
			}

			else if(wParam == m_idOnOff)                        // ���ػ�����
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
				m_pMusic->func = 0;              // Ҫ�������ܱ��ֲ���
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);
			}	
					
			else if(wParam == m_idProgress)      // ���������� 
			{              
				char buf[32];

				if (lParam > 28)
					lParam = 31;
				if (lParam >= 25 && lParam <= 28)
					lParam += rand() % 4; 
				
				m_pMusic->status = MUSIC_STATUS_ON;	
				m_pMusic->voice = lParam;
				m_pProgress->SetProgressCur(m_pMusic->voice);     // ��ʾ������	
				m_pProgress->Show();
		
				m_pMusic->func = 0;              // Ҫ�������ܱ��ֲ���
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);			
				ShowAircStatus();
				
			}

			else if(wParam == m_idMusicSource) {  // ��Դѡ��(���ð�һ���л���һ��Դģʽ�ķ�ʽ)

				char buf[128];                    // ģʽ
				m_pMusic->source = (m_pMusic->source + 1) % 8;
				if(m_pMusic->source == 0)
				{
					// ȥ��0ֵ
					m_pMusic->source = 1;
				}
				// ��ʾ��ǰ��Դ
				sprintf(buf, "%s", GetStringByID(10500 + m_pMusic->source));		
				m_psource->SetSrc(buf);
				m_psource->Show(TRUE); 
				m_pMusic->status = MUSIC_STATUS_ON;	
				m_pMusic->func = 0;       // Ҫ�������ܱ��ֲ���  
				SendSmartCmd(m_pSmartDev->addr, SCMD_MUSIC, m_pSmartDev->status);  // ��������
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

				m_pMusic->func = 0;  // Ҫ�������ܱ��ֲ���
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

					m_pMusic->func = 0;  // Ҫ�������ܱ��ֲ���
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
		
		m_psource->SetSrc(buf);     // �л���ʾ��Դ
		m_psource->Show(TRUE);

		m_pProgress->SetProgressCur(m_pMusic->voice);         // ��ʾ������	
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
		// ��ʾ����
		m_pTitle->SetSrc(pSmartDev->name);                    // �������ֱ���
		m_pTitle->Show(TRUE);
	
		m_pSmartDev = pSmartDev;                              // ��ַ��ֵ

		m_pProgress->SetProgressTotal(MUSIC_VOICE_MAX);
		
		m_pMusic = (MUSIC_DATA *)&pSmartDev->status;          // ��ַ����
		
#if 0
		sprintf(buf, "%d%%", m_pSmartDev->param1);
		m_pPercent->SetSrc(buf);
		m_pPercent->Show(TRUE);
#endif
		SmartGetStatus(pSmartDev->addr);
		m_pMusic->status == MUSIC_STATUS_OFF;

		// 2018.2.24��ӣ������޸�APP��������
		if(status != 0)  
			pSmartDev->status = status;

		ShowAircStatus();

	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("music.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);                // ���ذ���
		GetCtrlByName("btn_lastsong", &m_idLastsong);    // ��һ��
	//	GetCtrlByName("btn_pause", &m_idPause);          // ��ͣ
		GetCtrlByName("btn_nextsong", &m_idNextsong);    // ��һ��
		GetCtrlByName("btn_choice", &m_idMusicSource);   // ��Դѡ�񰴼� 
		GetCtrlByName("add", &m_idAdd);
		GetCtrlByName("sub", &m_idSub);
		
		m_pPause    = (CDPButton *)GetCtrlByName("btn_pause", &m_idPause);	
			
		m_pTitle    = (CDPStatic *)GetCtrlByName("title");                          // ������ʾ
		m_pOnOff    = (CDPButton *)GetCtrlByName("btn_onoff", &m_idOnOff);          // ���ذ���
		m_psource   = (CDPStatic *)GetCtrlByName("source");                         // ��Դ��ʾ 
		m_pPercent  = (CDPStatic *)GetCtrlByName("percent");
		m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);      // ����������
		
		OnCreate((SmartDev*)lParam, zParam);                                            
		
		return TRUE;
	}

private:
	DWORD m_idBack;         // ���ذ���ID
	DWORD m_idLastsong;     // ��һ�װ���ID
	DWORD m_idPause;        // ��ͣ����ID
	DWORD m_idNextsong;     // ��һ�װ���ID
	DWORD m_idProgress;     // ������ID
	DWORD m_idOnOff;        // ���ػ�����ID
	DWORD m_idMusicSource;  // ����Դѡ�񰴼�ID
	DWORD m_idAdd;
	DWORD m_idSub;

	CDPStatic* m_pTitle;    // ����  
	CDPButton* m_pOnOff;    // ����
	CDPButton* m_pPause;    // ��ͣ
	CDPStatic* m_psource;   // ��Դ��ʾ
	CDPStatic* m_pPercent;  // �ٷֱ�
	CDPProgress* m_pProgress;

	SmartDev* m_pSmartDev;
	MUSIC_DATA* m_pMusic;   // ������������Э�����
};

CAppBase* CreateMUSICApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMusicApp* pApp = new CMusicApp(wParam);
	MUSIC_FALG = 1;
	Ctl_Flag = 0;           // ���һ��������ƶ�����ı���
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}










