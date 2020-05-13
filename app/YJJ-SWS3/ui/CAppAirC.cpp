
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "CCtrlModules.h"
#include "SmartConfig.h"
int Onoff_Air = 0;
int T = 0;

int K_Temp = 0;
extern DWORD Tem_Cal_Air;
int Cal_Air;

int Air_Compaire = 0;
int AIR_FLAG = 0;          // 2018.1.11��ӣ��������ֶ���������£�̧��ʱͼ���״̬

int AIR_RESET = 0;
extern int Ctl_Flag;       // 2018.2.24��ӣ������޸�APP�������� 
extern unsigned int G_Temp;
//ADC�ɼ���ѹֵ
const unsigned short NTC_5K_Tab[60] = 
{
	3121,3083,3044,3003,2963, 2922,2881,2838,2795,2752,    // 0 ~9	
	2708,2664,2619,2574,2529, 2484,2438,2392,2347,2301,    // 10~19	
	2255,2209,2164,2118,2072, 2028,1983,1938,1895,1851,    // 20~29	
	1807,1765,1722,1681,1640, 1599,1559,1519,1481,1442,    // 30~39	
	1405,1368,1332,1300,1262, 1228,1195,1162,1130 ,1099 ,  // 40~49	
	1069 ,1039 ,1010 ,982 ,954 , 928 ,901 ,874 ,851 ,826   // 50~59
};

//ADC�ɼ���ѹ��Ӧ�¶�ֵ
const unsigned short NTC_5K_Tem[60] = 
{
	0,1,2,3,4,5,6,7,8,9, // 0 ~9	
	10,11,12,13,14, 15,16,17,18,19, // 10~19	
	20,21,22,23,24, 25,26,27,28,29, // 20~29	
	30,31,32,33,34, 35,36,37,38,39, // 30~39	
	40,41,42,43,44, 45,46,47,48 ,49 , // 40~49	
	50 ,51 ,52 ,53 ,54 , 55 ,56 ,57 ,58 ,59   // 50~59
};

TEMPTR TEM;

/*
 * ����:��ȡ��ǰ�����¶�ֵ
 */

void detect_cur_tem(const unsigned short value)
{
	int i;

	for(i = 0; i<60; i++) {

		if(NTC_5K_Tab[i]<value) {

			if(Tem_Cal_Air != 0) {

				if(NTC_5K_Tem[i-2] > Tem_Cal_Air) {
					Cal_Air = NTC_5K_Tem[i-2]-Tem_Cal_Air;
					Air_Compaire = 1;
				}
				
				else  {
					Cal_Air = Tem_Cal_Air - NTC_5K_Tem[i-2];
					Air_Compaire = 2;
				}	
				
				Tem_Cal_Air = 0;
			}

			if(Air_Compaire == 1)
				TEM.CurVal = NTC_5K_Tem[i-2-Cal_Air];
			else if(Air_Compaire == 2)
				TEM.CurVal = NTC_5K_Tem[i-2+Cal_Air];
			else if(Air_Compaire == 0)
				TEM.CurVal = NTC_5K_Tem[i-2];

			break;
		}
	}

	
}


/*
 * ����:��ȡ��ǰADC��ѹֵ
 */

int Show_Temper()
{
	int fd;
	int res;
	unsigned short data = 0;
	fd = open("/dev/tpx1",O_RDWR);      // �ļ���д����

	if(fd <0) {

		printf("open /dev/tpx1 error\n");
		return -1;
	}

	res = read(fd,&data,2);
	detect_cur_tem(data);

	close(fd);
}


class CAirCApp : public CAppBase
{
public:
	CAirCApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CAirCApp()
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
			
					AIR_FLAG = 0;
					Ctl_Flag = 0;
					m_dwTimeout = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case MSG_BROADCAST:
			if(lParam == m_pSmartDev->addr)
			{
				if(wParam == SMART_STATUS_ACK)
				{
					// �����ڴ���ʾ����ʧ��
				}
				else if(wParam == SMART_STATUS_SYNC)
				{
					Ctl_Flag = 0;
					if(zParam != 0) {

						m_pSmartDev->status = zParam;
						m_pSmartDev->param1 = (m_pSmartDev->status & 0x001F); // �¶Ȳ���
						m_pSmartDev->param1 -= 6;

					//	m_pAirC->speed = (m_pSmartDev->status>>5)&0x0007;
							
						ShowAircStatus();

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
					ShowAircStatus();
				}  
			}
			break;
		case TOUCH_MESSAGE:

			m_dwTimeout = 0;
			if(wParam == m_idBack)           // ����
			{
				AIR_FLAG = 0;
				Ctl_Flag = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idMode)
			{
				char buf[128];               // ģʽ
				m_pAirC->mode = (m_pAirC->mode + 1) % AC_MODE_MAX;
				if(m_pAirC->mode == 0)
				{
					// ȥ��0ֵ
					m_pAirC->mode = 1;
				}
				    // ģʽ
				sprintf(buf, "%s: %s", GetStringByID(10000), GetStringByID(10040 + m_pAirC->mode));		
				m_pMode->SetSrc(buf);
				m_pMode->Show(TRUE);
				
				m_pAirC->onoff = AC_STATUS_ON;
				SendTempCmd(m_pSmartDev->param1 + 6);
			//	SendSmartCmd(m_pSmartDev->addr, SCMD_AC, m_pSmartDev->status);
			}
			else if(wParam == m_idSpeed)      // ���� 
			{
				char buf[128];
				m_pAirC->speed = (m_pAirC->speed + 1) % AC_SPEED_MAX;
				if(m_pAirC->speed == 0)        
				{
					// ȥ��0ֵ
					m_pAirC->speed = 1;
				}
				    // ����
				sprintf(buf, "%s: %s", GetStringByID(10001), GetStringByID(10080 + m_pAirC->speed));
				m_pSpeed->SetSrc(buf);
				m_pSpeed->Show(TRUE);
				
				m_pAirC->onoff = AC_STATUS_ON;
				SendTempCmd(m_pSmartDev->param1 + 6);
			//	SendSmartCmd(m_pSmartDev->addr, SCMD_AC, m_pSmartDev->status);
			}

			else if(wParam == m_idOnOff)      // ���ػ�״̬
			{
				if(m_pAirC->onoff == AC_STATUS_OFF)
				{
					m_pAirC->onoff = AC_STATUS_ON;	// ����
					
				}
				else
				{
					m_pAirC->onoff = AC_STATUS_OFF;	// ֹͣ		
				}
				SendTempCmd(m_pSmartDev->param1 + 6);
			//	SendSmartCmd(m_pSmartDev->addr, SCMD_AC, m_pSmartDev->status);  
			}
			else if(wParam == m_idSub)          //�¶�-
			{
				char buf[128];
			//	m_pAirC->onoff = AC_STATUS_ON;
				if(m_pSmartDev->param1 == 0)

					m_pSmartDev->param1 =  AC_TEMPRATRUE_MAX - 2;

				else if(m_pSmartDev->param1 > 0)
				{
					m_pSmartDev->param1--;
					// �����¶�
					sprintf(buf, "%d",m_pSmartDev->param1 + 1 + AC_TEMPRATURE_INDEX);
					m_pSetTemp->SetSrc(buf);
					m_pSetTemp->Show(TRUE);
					
					SendTempCmd(m_pSmartDev->param1 + 6);
				}
			}
			else if(wParam == m_idAdd)          //�¶�+
			{
				char buf[128];

			//	m_pAirC->onoff = AC_STATUS_ON;
				if(m_pSmartDev->param1 ==  AC_TEMPRATRUE_MAX - 2)
				{
						m_pSmartDev->param1 = 0;
				}


			   else if(m_pSmartDev->param1 < AC_TEMPRATRUE_MAX - 2)
				{
					m_pSmartDev->param1++;
					// �����¶�
					sprintf(buf, "%d", m_pSmartDev->param1 + 1 + AC_TEMPRATURE_INDEX);
					m_pSetTemp->SetSrc(buf);
					m_pSetTemp->Show(TRUE);
					
					
					SendTempCmd(m_pSmartDev->param1 + 6);
				}

				
			}
			// ���ͽ�������ָ��
			break;
		}

		if(m_pSmartDev->addr == 0xffff || (G_Temp < 2)) {

			Show_Temper();
			K_Temp = TEM.CurVal;
			ShowAircStatus();                             // ʵʱ��ʾ
		}

		else {

			K_Temp = G_Temp;
			ShowAircStatus(); 
		}
		
		return TRUE;	
	}

	void SendTempCmd(int temp)
	{
		WORD status = m_pSmartDev->status;
		AC_DATA *pAirC = (AC_DATA *)&status;
		pAirC->temp = temp;
	
		SendSmartCmd(m_pSmartDev->addr, SCMD_AC, status);
	}

	void ShowAircStatus()
	{
		char buf[128];
		// ģʽ
		if(m_pAirC->mode == 0) {

			sprintf(buf, "%s: %s", GetStringByID(10000), GetStringByID(10041)); 
		}
		else
			sprintf(buf, "%s: %s", GetStringByID(10000), GetStringByID(10040 + m_pAirC->mode));       		
		m_pMode->SetSrc(buf);
		
		m_pMode->Show(TRUE);
			// ����
		if(m_pAirC->speed == 0)
			sprintf(buf, "%s: %s", GetStringByID(10001), GetStringByID(10081));
		else 
			sprintf(buf, "%s: %s", GetStringByID(10001), GetStringByID(10080 + m_pAirC->speed));
		
		m_pSpeed->SetSrc(buf);
		m_pSpeed->Show(TRUE);
		
		// �����¶�
		sprintf(buf, "%d",m_pSmartDev->param1 + 1 + AC_TEMPRATURE_INDEX);
		m_pSetTemp->SetSrc(buf);
		m_pSetTemp->Show(TRUE);
		// ��ǰ�¶�
		sprintf(buf, "%d", K_Temp);
		m_pCurTemp->SetSrc(buf);
		m_pCurTemp->Show(TRUE);
		
		if(m_pAirC->onoff == AC_STATUS_ON || (m_pSmartDev->status & 0x8000) == 0)
			Onoff_Air = 0;
		else if(m_pAirC->onoff == AC_STATUS_OFF || (m_pSmartDev->status & 0x8000) >0)
			Onoff_Air = 1;
		
		m_pOnOff->SetSrcpng(GetSmartPngOnOff(Onoff_Air));
		m_pOnOff->Show(STATUS_NORMAL);	
	}

	void OnCreate(SmartDev* pSmartDev,DWORD status)
	{

#ifdef DPCE
		pSmartDev->status = 0x4551;
#endif
		// ��ʾ����
		m_pTitle->SetSrc(pSmartDev->name);
		m_pTitle->Show(TRUE);	

		m_pSmartDev = pSmartDev;
		m_pAirC = (AC_DATA *)&pSmartDev->status;              // ��ַ����
		
		SmartGetStatus_Air(pSmartDev->addr);
		
		m_pAirC->onoff == AC_STATUS_OFF;

		m_pDu->Show(STATUS_NORMAL);
		m_pcc->Show(STATUS_NORMAL);
		
		ShowAircStatus();
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("airc.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("btn_mode", &m_idMode);     // ģʽ
		GetCtrlByName("btn_speed", &m_idSpeed);   // ����
		GetCtrlByName("sub", &m_idSub);           // ����-
		GetCtrlByName("add", &m_idAdd);           // ����+

		m_pOnOff = (CDPButton *)GetCtrlByName("btn_onoff", &m_idOnOff); // ��/�ػ�

		m_pTitle    = (CDPStatic *)GetCtrlByName("title");
		m_pMode     = (CDPStatic *)GetCtrlByName("mode");
		m_pSpeed    = (CDPStatic *)GetCtrlByName("speed");
		m_pSetTemp  = (CDPStatic *)GetCtrlByName("set_temp");
		m_pCurTemp  = (CDPStatic *)GetCtrlByName("cur_temp");

		m_pDu       = (CDPButton *)GetCtrlByName("btn_dangqiandu", &m_idDu);
		m_pcc      = (CDPButton *)GetCtrlByName("btn_xiaowendu", &m_idDu1);
		
		OnCreate((SmartDev*)lParam,zParam);              
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idMode;           // �յ�ģʽ
	DWORD m_idSpeed;          // �յ�����
	DWORD m_idOnOff;          // �յ�����
	DWORD m_idSub;            // �¶�-
	DWORD m_idAdd;            // �¶�+
//	DWORD m_idProgress;       // ������

	DWORD m_idDu;
	DWORD m_idDu1;
	CDPButton* m_pDu;
	CDPButton* m_pcc;
	
	CDPStatic* m_pTitle;
	CDPStatic* m_pMode;
	CDPStatic* m_pSpeed;
	CDPStatic* m_pSetTemp;
	CDPStatic* m_pCurTemp;
	CDPButton* m_pOnOff;
	CDPProgress* m_pProgress;

	SmartDev* m_pSmartDev;
	AC_DATA* m_pAirC;          // �յ�����
};

CAppBase* CreateAirCApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CAirCApp* pApp = new CAirCApp(wParam);
	AIR_FLAG = 1;
	Ctl_Flag = 0;             // ���һ��������ƶ�����ı���
	G_Temp = 0;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}