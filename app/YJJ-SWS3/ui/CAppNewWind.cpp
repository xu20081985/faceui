
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "CCtrlModules.h"
#include "SmartConfig.h"


extern int D_Temp ;
extern int Heat_Flag ;
extern int HEAT_FLAG ;      // 2018.1.11��ӣ��������ֶ���������£�̧��ʱͼ���״̬ 

extern int Ctl_Flag;    // 2018.2.24��ӣ������޸�APP�������� 
extern unsigned int G_Temp;

//ADC�ɼ���ѹֵ

extern TEMPTR TEMP;
extern void detect_cur_temp(const unsigned short value);
extern int Show_Temp();
extern WORD CO2 ;			//���ڴ���·��CO2
extern WORD PM25 ; 			//���ڴ���·��PM24



class CNewWindApp : public CAppBase
{
public:
	CNewWindApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CNewWindApp()
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
			
					HEAT_FLAG  = 0;
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
					if(zParam != 0) {

						// ��һ��ר�Ŵ����ůҺ����巢�͵�00���ֲ�����Ϣ
						if((zParam >> 8) == 0) {

							m_pSmartDev->status &= 0xff00;
							zParam |= m_pSmartDev->status;
						}
							
						m_pSmartDev->status = zParam;
						m_pSmartDev->param1 = (m_pSmartDev->status & 0x001F);//m_pHeat->temp;//(m_pSmartDev->status & 0x001F); // �¶Ȳ���
						m_pSmartDev->param1 -= 6;
						
						this->uGetCo2ValueFlag = TRUE;
						ShowNewwindStatus();
					}
				
				}

				else if(wParam == SMART_STATUS_S) {

					if(zParam == 0x01) {
							
						m_pSmartDev->status &= 0x7fff;
						m_pSmartDev->status |= 0x4000;
					}

					else if(zParam == 0x03) {
			
						m_pSmartDev->status &= 0xbfff;
						m_pSmartDev->status |= 0x8000;
					}
					Ctl_Flag = 0;
					ShowNewwindStatus();
				}  
			}
			break;
		case TOUCH_MESSAGE:

			m_dwTimeout = 0;
			if(wParam == m_idBack)
			{
				HEAT_FLAG  = 0;
				Ctl_Flag = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOnOff)
			{
				if(m_pNewwind->onoff == AC_STATUS_ON) {

					m_pNewwind->onoff = AC_STATUS_OFF;	// �ر�
					SendSmartCmd(m_pSmartDev->addr,SCMD_CLOSE ,0);
				}
					
				else {

					m_pNewwind->onoff = AC_STATUS_ON;	// ����
					SendSmartCmd(m_pSmartDev->addr,SCMD_OPEN,0);
				}
				//SendTempCmd(m_pSmartDev->param1 + 6);
				
			//	SendSmartCmd(m_pSmartDev->addr, SCMD_AC, m_pSmartDev->status);
			}

			break;
		}

		if(m_pSmartDev->addr == 0xffff || (G_Temp < 2)) {

			Show_Temp();
			D_Temp = TEMP.CurVal;
			ShowNewwindStatus();
		}

		else {

			D_Temp =  G_Temp;
			ShowNewwindStatus();
		}
		return TRUE;	
	}


	void ShowNewwindStatus()
	{
			
		char buf[128];	
		int NewWindStatu =0; 
		// ��ǰ�¶�
		sprintf(buf, "%d", D_Temp);
		m_pCurTemp->SetSrc(buf);
		m_pCurTemp->Show(TRUE);	
		
		if(this->uGetCo2ValueFlag == TRUE) //���·��л�ȡ���¶�
		{
			sprintf(buf, "%d" , CO2); //��λCO2
//			if(CO2<1000)
//			{
//				m_pAir->SetStart(125,120) ;
//			}
//			else
//			{
//				m_pAir->SetStart(95,120) ;
//			}
			m_pAir->SetSrc(buf);
			m_pAir->Show(TRUE);	
		}
		else
		{
			sprintf(buf, "%s" ,"-----");
			m_pAir->SetSrc(buf);
			m_pAir->Show(TRUE);	
		}

		if((m_pSmartDev->status >> 8) != 0) {

			if(m_pNewwind->onoff == AC_STATUS_ON || (m_pSmartDev->status & 0x8000) == 0)
				NewWindStatu = 0; 
			else if(m_pNewwind->onoff == AC_STATUS_OFF || (m_pSmartDev->status & 0x8000) >0)
				NewWindStatu = 1;
		}
			
		m_pOnOff->SetSrcpng(GetSmartPngOnOff(NewWindStatu));
		m_pOnOff->Show(STATUS_NORMAL);
		
	}

	void OnCreate(SmartDev* pSmartDev, DWORD status)
	{
		char buf[128];		
#ifdef DPCE
		pSmartDev->status = 0x4551;
#endif
		// ��ʾ����
		m_pTitle->SetSrc(pSmartDev->name);
		m_pTitle->Show(TRUE);

	
		
		m_pSmartDev = pSmartDev;
		m_pNewwind = (AC_DATA *)&pSmartDev->status;
		m_pNewwind->mode = 1;

		CO2 = m_pSmartDev->param1 + 0+NEWWIND_CO2;
		//CO2 = 845;
//		sprintf(buf, "%d" , m_pSmartDev->param1 + 1+NEWWIND_CO2);
//		m_pAir->SetSrc(buf);
//		m_pAir->Show(TRUE);

		//m_pDu->Show(STATUS_NORMAL);
		m_pDu1->Show(STATUS_NORMAL);

		SmartGetStatus_Air(pSmartDev->addr);

		if(status != 0)  
			pSmartDev->status = status;
		
		ShowNewwindStatus();
		
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{	
		InitFrame("newwind.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);                      // ���ذ���  
		GetCtrlByName("btn_onoff", &m_idOnOff);                // ��/�ػ�
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");        // ����         

		m_pAir = (CDPStatic *)GetCtrlByName("get_air");        // ��������
		
		//m_pDu  = (CDPButton *)GetCtrlByName("air_denwei", &m_idDu);
		m_pDu1 = (CDPButton *)GetCtrlByName("btn_dangqiandu", &m_idDu1);
		//m_pSetTemp = (CDPStatic *)GetCtrlByName("set_temp");   // �����¶�
		m_pCurTemp = (CDPStatic *)GetCtrlByName("cur_temp");   // ��ǰ�¶�
		m_pOnOff = (CDPButton *)GetCtrlByName("btn_onoff", &m_idOnOff);// ���ؿ��� 

		this->uGetCo2ValueFlag = FALSE;

		OnCreate((SmartDev*)lParam, zParam);                            
		return TRUE;


		
   }
private:
	bool uGetCo2ValueFlag;
	DWORD m_idBack;
	DWORD m_idMode;

	DWORD m_idOnOff;
	DWORD m_idDu;
	DWORD m_idDu1;
	CDPStatic* m_pTitle;
	CDPStatic* m_pMode;
	CDPStatic* m_pAir;

	CDPStatic* m_pCurTemp;
	CDPButton* m_pOnOff;

	//CDPButton* m_pDu;
	CDPButton* m_pDu1;
	
	SmartDev* m_pSmartDev;
	AC_DATA* m_pNewwind;
	
};

CAppBase* CreateNewWindApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CNewWindApp* pApp = new CNewWindApp(wParam);
	HEAT_FLAG = 1;
	Ctl_Flag = 0;
	G_Temp = 0;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}

