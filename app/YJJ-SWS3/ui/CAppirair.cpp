#include "CCtrlModules.h"
#include "SmartConfig.h"

BOOL ON_OFF_AIR;           // 开关机状态标志位

// 红外编码
WORD g_AIRONOFF;	   // 红外空调开关按键  
WORD g_AIRH;		   // 红外空调风速高按键
WORD g_AIRM; 		   // 红外空调风速中按键 
WORD g_AIRL; 	       // 红外空调风速低按键 
WORD g_AIRT; 	       // 红外空调通风按键 
WORD g_AIRCODE;		   // 红外空调制冷按键 
WORD g_AIRHOT;		   // 红外空调制热按键

int IR_AIR_FLAG = 0;   // 标志位用于修改二级界面的滑动，触摸逻辑


class CIr_Air : public CAppBase
{
public:
	CIr_Air(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CIr_Air()
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
			
					IR_AIR_FLAG = 0;
					m_dwTimeout = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}	
			break;
		case TOUCH_MESSAGE:

			m_dwTimeout = 0;
			if(wParam == m_idBack)
			{
				IR_AIR_FLAG = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idOn) { // 开关机按键执行
				
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRONOFF);
			}

			else if(wParam == m_idOff) {

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRONOFF);
			}
            // 风速高按键
			else if(wParam == m_idAirh) {  

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRH);
			}
            // 风速中按键
			else if(wParam == m_idAirm) {   

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRM);
			}
            // 风速低按键
			else if(wParam == m_idAirl) {   

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRL);
			}
            // 通风按键
			else if(wParam == m_idAirt) { 

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRT);
			}
            // 制冷按键
			else if(wParam == m_idCode) {  

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRCODE);
			}
            // 制热按键
			else if(wParam == m_idHot) {     

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRHOT);
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

		// 显示标题名称(红外空调)
		m_pTitle->SetSrc(pSmartDev->name); 
		m_pTitle->Show(TRUE);

		m_pSmartDev = pSmartDev;
	
	}


	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("ir_airc.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);         // 返回按键
		GetCtrlByName("btn_airh", &m_idAirh);     // 风速高按键
		GetCtrlByName("btn_airm", &m_idAirm);     // 风速中按键
		GetCtrlByName("btn_airl", &m_idAirl);     // 风速低按键
		GetCtrlByName("btn_airt", &m_idAirt);     // 通风按键 
		GetCtrlByName("btn_code", &m_idCode);     // 制冷按键
		GetCtrlByName("btn_hot", &m_idHot);       // 制热按键
		GetCtrlByName("btn_on", &m_idOn);  
		GetCtrlByName("btn_off", &m_idOff); 
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");                     // 标题图标	
		
		// 获得红外空调的红外控制编码
#if 1
		g_AIRONOFF = GetIR_AIR_CODE(g_AIRONOFF,0);
		g_AIRHOT   = GetIR_AIR_CODE(g_AIRHOT,1);
		g_AIRCODE  = GetIR_AIR_CODE(g_AIRCODE,2);
		g_AIRT 	   = GetIR_AIR_CODE(g_AIRT,3); 
		g_AIRL 	   = GetIR_AIR_CODE(g_AIRL,4); 
		g_AIRM     = GetIR_AIR_CODE(g_AIRM,5);
		g_AIRH     = GetIR_AIR_CODE(g_AIRH,6);			   				  
#endif 
		OnCreate((SmartDev*)lParam); /*参数为点击的设备*/

		

		return TRUE;
	}

private:
	DWORD m_idBack;         // 返回按键id
	DWORD m_idAirh;         // 风速高按键id
	
	
	DWORD m_idAirm;         // 风速中按键id
	DWORD m_idAirl;         // 风速低按键id
	DWORD m_idAirt;         // 通风按键id
	DWORD m_idCode;         // 制冷按键id
	DWORD m_idHot;          // 制热按键id
	DWORD m_idOn;        	// 开关按键id          
	DWORD m_idOff; 
	
		
	CDPStatic* m_pTitle;    // 标题静态变量
	
	SmartDev* m_pSmartDev;
};



CAppBase* CreateIr_AirApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CIr_Air* pApp = new CIr_Air(wParam);
	
	IR_AIR_FLAG = 1;                     // 修改了滑动状态标志位的状态
	
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}








