#include "CCtrlModules.h"
#include "SmartConfig.h"
int BK1 = 0;
int BK2 = 0;
class CBK : public CAppBase
{
public:
	CBK(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CBK()
	{
	}

	void ResumeAck(void)
	{
		m_dwTimeout = 0;
	//	OnPage(); 
		return CAppBase::ResumeAck();
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
					m_dwTimeout = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;					
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == m_idButton[0])   {   //加载图片1

				BK1 = 1;
				strcpy(szbkp, "/FlashDev/wallpaper/3.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				strcpy(szbkp, "/FlashDev/wallpaper/3.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();

				strcpy(szbkp, "/FlashDev/wallpaper/3.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);	
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idButton[1]) {  //加载                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               图片2

                BK2 = 1;
				strcpy(szbkp, "/FlashDev/wallpaper/2.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				strcpy(szbkp, "/FlashDev/wallpaper/2.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();

				strcpy(szbkp, "/FlashDev/wallpaper/2.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);  
			} 

			
			else if(wParam == m_idButton[2]) {	//加载																																																																																																																																																																																																																																																																																																																																																																																																																																																																												 图片2
			
				BK2 = 1;
				strcpy(szbkp, "/FlashDev/wallpaper/1.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				strcpy(szbkp, "/FlashDev/wallpaper/1.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				
				strcpy(szbkp, "/FlashDev/wallpaper/1.jpg");
				SetPrjbkp(szbkp);
				InitSystemSet();
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);	
			} 

			else if(wParam == m_idButton[4]) {  //返回按键

				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("background.xml");
		m_pButton[0] = (CDPButton *)GetCtrlByName("button1", &m_idButton[0]);
		m_pButton[1] = (CDPButton *)GetCtrlByName("button2", &m_idButton[1]);
		m_pButton[2] = (CDPButton *)GetCtrlByName("button3", &m_idButton[2]);
	//	m_pButton[3] = (CDPButton *)GetCtrlByName("button4", &m_idButton[3]);
		m_pButton[4] = (CDPButton *)GetCtrlByName("back", &m_idButton[4]);                                                                                              

		return TRUE;
	}

private:
	DWORD m_dwCount;
	DWORD m_idButton[MAX_ICON_NUM];
	CDPButton* m_pButton[MAX_ICON_NUM];
	char szbkp[256];                     //设置背景界面的数字
};

CAppBase* CreateBKApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CBK* pApp = new CBK(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
