#include "CCtrlModules.h"
#include "SmartConfig.h"

BOOL ON_OFF;          // 开关机状态标志位

// 红外编码
WORD m_TVOPEN;		   // 电视开红外编码
WORD m_TVCLOSE;		   // 电视关红外编码
WORD m_TVOL; 		   // 电视开/关红外编码
WORD m_CHANNEL1; 	   // 频道+红外编码
WORD m_CHANNEL2; 	   // 频道-红外编码
WORD m_VOICE1;		   // 音量+红外编码
WORD m_VOICE2;		   // 音量-红外编码
WORD m_SWITCH;		   // TV/AV切换红外编码 

WORD *test;


int TV_FLAG = 0;

class CTvApp : public CAppBase
{
public:
	CTvApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTvApp()
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
			
					TV_FLAG = 0;
					m_dwTimeout = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}	
			break;
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == m_idBack)
			{

				TV_FLAG = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idOnOff) { // 开关机按键执行
				
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_TVOL);
			}

			else if(wParam == m_idTvAV) {  // TV/AV转换按键执行

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_SWITCH);
			}

			else if(wParam == m_idAdd) {   // 频道+按键执

				// 红外控制类型
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_CHANNEL1);
			}

			else if(wParam == m_idSub) {   // 频道-按键执行

				// 红外控制类型
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_CHANNEL2);
			}

			else if(wParam == m_idVoicep) { // 声音+按键执行

				// 红外控制类型
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_VOICE1);
			}

			else if(wParam == m_idVoice) {  // 声音-按键执行

				// 红外控制类型
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_VOICE2);
			}

			
			break;
		}
		
		return TRUE;	
	}

/*
 *函数:void OnCreate(SmartDev* pSmartDev)
 *入口参数:SmartDev* pSmartDev(该参数附带有设备的各种信息)
 *功能:显示出红外电视的界面
 */
	void OnCreate(SmartDev* pSmartDev)
	{

		// 显示标题名称(红外电视)
		m_pTitle->SetSrc(pSmartDev->name); 
		m_pTitle->Show(TRUE);

	//	m_pTVAV->SetSrc(GetStringByID(10604));
	//	m_pTVAV->Show(TRUE);
		
		m_pSmartDev = pSmartDev;

	
	}


	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("TV.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);         // 返回按键
		GetCtrlByName("btn_onoff", &m_idOnOff);
		GetCtrlByName("btn_tvav", &m_idTvAV);
		GetCtrlByName("btn_channelp", &m_idAdd);
		GetCtrlByName("btn_channel", &m_idSub);
		GetCtrlByName("btn_voicep", &m_idVoicep);
		GetCtrlByName("btn_voice", &m_idVoice);
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");                     // 标题图标	
		m_pCh1   = (CDPStatic *)GetCtrlByName("cha");
		m_pCh2   = (CDPStatic *)GetCtrlByName("chs");
		m_pVo1   = (CDPStatic *)GetCtrlByName("vca");
		m_pVo2   = (CDPStatic *)GetCtrlByName("vcs");
		m_pTVAV  = (CDPStatic *)GetCtrlByName("tvav");

		// 获得红外电视的红外控制编码

#if 1
		m_TVOL     = GetIR_TV_CODE(m_TVOL,0);
		m_CHANNEL1 = GetIR_TV_CODE(m_CHANNEL1,1);
		m_CHANNEL2 = GetIR_TV_CODE(m_CHANNEL2,2);
		m_VOICE1   = GetIR_TV_CODE(m_VOICE1,3);
		m_VOICE2   = GetIR_TV_CODE(m_VOICE2,4);
		m_SWITCH   = GetIR_TV_CODE(m_SWITCH,5); 
#endif 
		OnCreate((SmartDev*)lParam); /*参数为点击的设备*/


		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idAdd;          // 频道+按键id
	DWORD m_idSub;          // 频道-按键id
	DWORD m_idVoicep;       // 音量+按键id
	DWORD m_idVoice;        // 音量-按键id
	DWORD m_idOnOff;        // 开/关机按键id
	DWORD m_idTvAV;         // TV/AV切换按键id         

	CDPStatic* m_pTitle;    // 标题静态变量
	CDPStatic* m_pCh1;
	CDPStatic* m_pCh2;
	CDPStatic* m_pVo1;
	
	CDPStatic* m_pVo2;
	
	CDPStatic* m_pTVAV;
	
/*	CDPButton* m_pOnOff;    // 开关机图标指针
	CDPButton* m_pTVAV;     // TV/AV图标指针
	CDPButton* m_pCh1;      // 频道+图标控制指针
	CDPButton* m_pCh2;      // 频道-图标控制指针
	CDPButton* m_pVo1;      // 音量+图标控制指针
	CDPButton* m_pVo2;      // 音量-图标控制指针
	CDPButton* m_pOk;       // Ok图标指针 
*/
	SmartDev* m_pSmartDev;
};



CAppBase* CreateTVApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTvApp* pApp = new CTvApp(wParam);
	TV_FLAG = 1;
	
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}


