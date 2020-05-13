#include "CCtrlModules.h"
#include "SmartConfig.h"

#include <roomlib.h>
extern int Ctl_Flag;      // 2018.2.24��ӣ������޸�APP��������
extern int frash_flag;
class CClockApp : public CAppBase
{
public:
	CClockApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CClockApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			m_pTime->UpdataDateTime();                           //ʵʱ����ʱ��
			break;
		case TOUCH_MESSAGE:
			 
			if(wParam == m_idEmpty)
			{
				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;

		case TOUCH_SLIDE:
			
			DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
			DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);		
			break;

		case MSG_BROADCAST:
		#if 1
			if(Ctl_Flag == AIR_TYPE) {

			//	Ctl_Flag = 0;
				SmartDev* pSmartDev = m_pSmartDev;    	  // �յ����͵�ָ�����

				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {      // �����豸�б�
					
					if(lParam == pSmartDev[i].addr) {     // ����Ҫ��״̬����һ��

						pSmartDev[i].status = zParam;

						pSmartDev[i].param1 = (pSmartDev[i].status & 0x001F); // �¶Ȳ���
						pSmartDev[i].param1 -= 6;

					//	break;
					}
				} 
			}
				
			else if(Ctl_Flag == MUSIC_TYPE) {              // ����Ҫ��״̬����һ��

			//	Ctl_Flag = 0;
				SmartDev* pSmartDevm = m_pSmartDev;   	   // �������͵�ָ�����

				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {       // �����豸�б�
				
					if(lParam == pSmartDevm[i].addr) {
								
						pSmartDevm[i].status = zParam;
					//	break;
					}				
				} 
			}

			else if(Ctl_Flag == ALL_TYPE) {

			//	Ctl_Flag = 0;
				SmartDev* pSmartDevm = m_pSmartDev;   	   

				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {       // �����豸�б�
				
					if(lParam == pSmartDevm[i].addr) {
								
						if(zParam == 0x01) {
							
							pSmartDevm[i].status &= 0x7fff;
							pSmartDevm[i].status |= 0x4000;
						}

						else if(zParam == 0x03) {
				
							pSmartDevm[i].status &= 0xbfff;
							pSmartDevm[i].status |= 0x8000;
						}
						
					//	break;
					}				
				} 
			}
#endif			
			Ctl_Flag = 0;
			frash_flag = 0;				
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("clock.xml");
		GetCtrlByName("ebutton", &m_idEmpty);         //�������һ������Ч��
		m_pTime = (CTimeDate *)GetCtrlByName("time");
		m_pSmartDev = GetSmartDev(&m_dwCount);        //��ȡ��ǰ�豸�б�һ���ж��ٸ��豸�Լ��豸���� 
		return TRUE;
	}

private:
	DWORD m_idEmpty;
	CTimeDate* m_pTime;
	DWORD m_dwCount;                                  // һ���ж��ٸ��豸(�豸���)
/*const*/ SmartDev* m_pSmartDev;  
};

CAppBase* CreateClockApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CClockApp* pApp = new CClockApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}