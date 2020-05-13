#include "CCtrlModules.h"
#include "SmartConfig.h"

BOOL ON_OFF_AIR;           // ���ػ�״̬��־λ

// �������
WORD g_AIRONOFF;	   // ����յ����ذ���  
WORD g_AIRH;		   // ����յ����ٸ߰���
WORD g_AIRM; 		   // ����յ������а��� 
WORD g_AIRL; 	       // ����յ����ٵͰ��� 
WORD g_AIRT; 	       // ����յ�ͨ�簴�� 
WORD g_AIRCODE;		   // ����յ����䰴�� 
WORD g_AIRHOT;		   // ����յ����Ȱ���

int IR_AIR_FLAG = 0;   // ��־λ�����޸Ķ�������Ļ����������߼�


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

			else if(wParam == m_idOn) { // ���ػ�����ִ��
				
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRONOFF);
			}

			else if(wParam == m_idOff) {

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRONOFF);
			}
            // ���ٸ߰���
			else if(wParam == m_idAirh) {  

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRH);
			}
            // �����а���
			else if(wParam == m_idAirm) {   

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRM);
			}
            // ���ٵͰ���
			else if(wParam == m_idAirl) {   

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRL);
			}
            // ͨ�簴��
			else if(wParam == m_idAirt) { 

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRT);
			}
            // ���䰴��
			else if(wParam == m_idCode) {  

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRCODE);
			}
            // ���Ȱ���
			else if(wParam == m_idHot) {     

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, g_AIRHOT);
			}
			
			break;
		}
		
		return TRUE;	
	}

/*
 *����:void OnCreate(SmartDev* pSmartDev)
 *��ڲ���:SmartDev* pSmartDev(�ò����������豸�ĸ�����Ϣ)
 *����:��ʾ��������ӵĽ���
 */
	void OnCreate(SmartDev* pSmartDev)
	{

		// ��ʾ��������(����յ�)
		m_pTitle->SetSrc(pSmartDev->name); 
		m_pTitle->Show(TRUE);

		m_pSmartDev = pSmartDev;
	
	}


	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("ir_airc.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);         // ���ذ���
		GetCtrlByName("btn_airh", &m_idAirh);     // ���ٸ߰���
		GetCtrlByName("btn_airm", &m_idAirm);     // �����а���
		GetCtrlByName("btn_airl", &m_idAirl);     // ���ٵͰ���
		GetCtrlByName("btn_airt", &m_idAirt);     // ͨ�簴�� 
		GetCtrlByName("btn_code", &m_idCode);     // ���䰴��
		GetCtrlByName("btn_hot", &m_idHot);       // ���Ȱ���
		GetCtrlByName("btn_on", &m_idOn);  
		GetCtrlByName("btn_off", &m_idOff); 
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");                     // ����ͼ��	
		
		// ��ú���յ��ĺ�����Ʊ���
#if 1
		g_AIRONOFF = GetIR_AIR_CODE(g_AIRONOFF,0);
		g_AIRHOT   = GetIR_AIR_CODE(g_AIRHOT,1);
		g_AIRCODE  = GetIR_AIR_CODE(g_AIRCODE,2);
		g_AIRT 	   = GetIR_AIR_CODE(g_AIRT,3); 
		g_AIRL 	   = GetIR_AIR_CODE(g_AIRL,4); 
		g_AIRM     = GetIR_AIR_CODE(g_AIRM,5);
		g_AIRH     = GetIR_AIR_CODE(g_AIRH,6);			   				  
#endif 
		OnCreate((SmartDev*)lParam); /*����Ϊ������豸*/

		

		return TRUE;
	}

private:
	DWORD m_idBack;         // ���ذ���id
	DWORD m_idAirh;         // ���ٸ߰���id
	
	
	DWORD m_idAirm;         // �����а���id
	DWORD m_idAirl;         // ���ٵͰ���id
	DWORD m_idAirt;         // ͨ�簴��id
	DWORD m_idCode;         // ���䰴��id
	DWORD m_idHot;          // ���Ȱ���id
	DWORD m_idOn;        	// ���ذ���id          
	DWORD m_idOff; 
	
		
	CDPStatic* m_pTitle;    // ���⾲̬����
	
	SmartDev* m_pSmartDev;
};



CAppBase* CreateIr_AirApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CIr_Air* pApp = new CIr_Air(wParam);
	
	IR_AIR_FLAG = 1;                     // �޸��˻���״̬��־λ��״̬
	
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}








