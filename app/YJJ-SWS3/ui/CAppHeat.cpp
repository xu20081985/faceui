
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "CCtrlModules.h"
#include "SmartConfig.h"
int Onof_Heat = 0; // 地暖状态标志位
int Q = 0;

int D_Temp = 0;
int Cal_Heat;           // 温度校准参数
int Heat_Flag = 0;
int Heat_compare = 0;   // 温度校准相关

int HEAT_FLAG = 0;      // 2018.1.11添加，用于区分二级界面点下，抬起时图标的状态 

extern DWORD Tem_Cal_Heat;
extern int Ctl_Flag;    // 2018.2.24添加，用于修改APP控制问题 
extern unsigned int G_Temp;

//ADC采集电压值
const unsigned short NTC_5K_Tab[60] = 
{
	3121,3083,3044,3003,2963, 2922,2881,2838,2795,2752,    // 0 ~9	
	2708,2664,2619,2574,2529, 2484,2438,2392,2347,2301,    // 10~19	
	2255,2209,2164,2118,2072, 2028,1983,1938,1895,1851,    // 20~29	
	1807,1765,1722,1681,1640, 1599,1559,1519,1481,1442,    // 30~39	
	1405,1368,1332,1300,1262, 1228,1195,1162,1130 ,1099 ,  // 40~49	
	1069 ,1039 ,1010 ,982 ,954 , 928 ,901 ,874 ,851 ,826   // 50~59
};

//ADC采集电压对应温度值
const unsigned short NTC_5K_Tem[60] = 
{
	0,1,2,3,4,5,6,7,8,9, // 0 ~9	
	10,11,12,13,14, 15,16,17,18,19,  // 10~19	
	20,21,22,23,24, 25,26,27,28,29,  // 20~29	
	30,31,32,33,34, 35,36,37,38,39,  // 30~39	
	40,41,42,43,44, 45,46,47,48,49 , // 40~49	
	50,51,52,53,54, 55,56,57,58,59   // 50~59
};

TEMPTR TEMP;

/*
 * 功能:获取当前室温温度值
 */

void detect_cur_temp(const unsigned short value)
{
	int i;


	for(i = 0; i<60; i++) {

		if(NTC_5K_Tab[i]<value) {

			if(Tem_Cal_Heat != 0) {

				if(NTC_5K_Tem[i-2] > Tem_Cal_Heat) {

					Cal_Heat = NTC_5K_Tem[i-2]-Tem_Cal_Heat;
					Heat_compare = 1;
				}
				
				else  {

					Cal_Heat = Tem_Cal_Heat - NTC_5K_Tem[i-2];
					Heat_compare = 2;
				}	
				
				Tem_Cal_Heat = 0;
			}

			if(Heat_compare == 1)
				TEMP.CurVal = NTC_5K_Tem[i-2-Cal_Heat];
			else if(Heat_compare == 2)
				TEMP.CurVal = NTC_5K_Tem[i-2+Cal_Heat];
			else if(Heat_compare == 0)
				TEMP.CurVal = NTC_5K_Tem[i-2];

			break;
		}
	}
}


/*
 * 功能:读取当前ADC电压值
 */

int Show_Temp()
{
	int fd;
	int res;
	unsigned short data = 0;
	fd = open("/dev/tpx1",O_RDWR);      // 文件读写操作

	if(fd <0) {

		printf("open /dev/tpx1 error\n");
		return -1;
	}

	res = read(fd,&data,2);
	detect_cur_temp(data);

	close(fd);

//	printf("read res = %d,data=%d,tem=%d\n",res,data,TEMP.CurVal);
}


class CHeatApp : public CAppBase
{
public:
	CHeatApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CHeatApp()
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

						// 这一块专门处理地暖液晶面板发送的00保持不变信息
						if((zParam >> 8) == 0) {

							m_pSmartDev->status &= 0xff00;
							zParam |= m_pSmartDev->status;
						}
							
						m_pSmartDev->status = zParam;
						m_pSmartDev->param1 = (m_pSmartDev->status & 0x001F);//m_pHeat->temp;//(m_pSmartDev->status & 0x001F); // 温度参数
						m_pSmartDev->param1 -= 6;
						
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
			if(wParam == m_idBack)
			{
				HEAT_FLAG  = 0;
				Ctl_Flag = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}

			// 温度+ 按键
			else if(wParam == m_idAdd) {

				char buf[128];
			//	m_pHeat->onoff = AC_STATUS_ON;
				if(m_pSmartDev->param1 == AC_TEMPRATRUE_MAX - 2)

					m_pSmartDev->param1 = 0;

				else if(m_pSmartDev->param1 < AC_TEMPRATRUE_MAX - 2)
				{
					m_pSmartDev->param1++;
					// 设置温度
					sprintf(buf, "%d", m_pSmartDev->param1 + 1 + HEAT_TEMPRATURE_INDEX);
					m_pSetTemp->SetSrc(buf);
					m_pSetTemp->Show(TRUE);

					SendTempCmd(m_pSmartDev->param1 + 6);
				}
			}
            // 温度- 按键
			else if(wParam == m_idSub) {

				char buf[128];
			//	m_pHeat->onoff = AC_STATUS_ON;
				if(m_pSmartDev->param1 == 0)

					m_pSmartDev->param1 = AC_TEMPRATRUE_MAX - 2;

				else if(m_pSmartDev->param1 > 0)
				{
					m_pSmartDev->param1--;
					// 设置温度
					sprintf(buf, "%d", m_pSmartDev->param1 + 1 + HEAT_TEMPRATURE_INDEX);
					m_pSetTemp->SetSrc(buf);
					m_pSetTemp->Show(TRUE);

					SendTempCmd(m_pSmartDev->param1 + 6);
				}
			}
			else if(wParam == m_idOnOff)
			{
				if(m_pHeat->onoff == AC_STATUS_ON) {

					m_pHeat->onoff = AC_STATUS_OFF;	// 关闭
				}
					
				else {

					m_pHeat->onoff = AC_STATUS_ON;	// 开启
				}
				SendTempCmd(m_pSmartDev->param1 + 6);
			//	SendSmartCmd(m_pSmartDev->addr, SCMD_AC, m_pSmartDev->status);
			}

			break;
		}

		if(m_pSmartDev->addr == 0xffff || (G_Temp < 2)) {

			Show_Temp();
			D_Temp = TEMP.CurVal;
			ShowAircStatus();
		}

		else {

			D_Temp =  G_Temp;
			ShowAircStatus();
		}
		return TRUE;	
	}

	void SendTempCmd(int temp)
	{
		WORD status = m_pSmartDev->status;
		AC_DATA *pHeat = (AC_DATA *)&status;
		pHeat->temp = temp;

		SendSmartCmd(m_pSmartDev->addr, SCMD_AC, status);
	}

	void ShowAircStatus()
	{
			
		char buf[128];	
		
		// 设置温度 地暖最低温度为15℃
		sprintf(buf, "%d" , m_pSmartDev->param1 + 1 + HEAT_TEMPRATURE_INDEX);
		m_pSetTemp->SetSrc(buf);
		m_pSetTemp->Show(TRUE);

		// 当前温度
		sprintf(buf, "%d", D_Temp);
		m_pCurTemp->SetSrc(buf);
		m_pCurTemp->Show(TRUE);	

		if((m_pSmartDev->status >> 8) != 0) {

			if(m_pHeat->onoff == AC_STATUS_ON || (m_pSmartDev->status & 0x8000) == 0)
				Onof_Heat = 0; 
			else if(m_pHeat->onoff == AC_STATUS_OFF || (m_pSmartDev->status & 0x8000) >0)
				Onof_Heat = 1;
		}
			
		m_pOnOff->SetSrcpng(GetSmartPngOnOff(Onof_Heat));
		m_pOnOff->Show(STATUS_NORMAL);
		
	}

	void OnCreate(SmartDev* pSmartDev, DWORD status)
	{
		char buf[128];		
#ifdef DPCE
		pSmartDev->status = 0x4551;
#endif
		// 显示名称
		m_pTitle->SetSrc(pSmartDev->name);
		m_pTitle->Show(TRUE);

		m_pSmartDev = pSmartDev;
		m_pHeat = (AC_DATA *)&pSmartDev->status;
		m_pHeat->mode = 1;

		// 设置温度 地暖最低温度为15℃
		sprintf(buf, "%d" , m_pSmartDev->param1 + 1 + HEAT_TEMPRATURE_INDEX);
		m_pSetTemp->SetSrc(buf);
		m_pSetTemp->Show(TRUE);
		
		m_pDu->Show(STATUS_NORMAL);
		m_pDu1->Show(STATUS_NORMAL);

		SmartGetStatus_Air(pSmartDev->addr);

		if(status != 0)  
			pSmartDev->status = status;
		
		ShowAircStatus();
		
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("heat.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);                      // 返回按键  
		GetCtrlByName("btn_add", &m_idAdd);                    // 温度+
		GetCtrlByName("btn_sub", &m_idSub);                    // 温度-
		GetCtrlByName("btn_onoff", &m_idOnOff);                // 开/关机
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");        // 标题         

		m_pDu  = (CDPButton *)GetCtrlByName("btn_shezhidu", &m_idDu);
		m_pDu1 = (CDPButton *)GetCtrlByName("btn_dangqiandu", &m_idDu1);
		m_pSetTemp = (CDPStatic *)GetCtrlByName("set_temp");   // 设置温度
		m_pCurTemp = (CDPStatic *)GetCtrlByName("cur_temp");   // 当前温度
		m_pOnOff = (CDPButton *)GetCtrlByName("btn_onoff", &m_idOnOff);// 开关控制 
		OnCreate((SmartDev*)lParam, zParam);                            
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idMode;
	DWORD m_idAdd;
	DWORD m_idSub;
	DWORD m_idOnOff;
	DWORD m_idDu;
	DWORD m_idDu1;
	CDPStatic* m_pTitle;
	CDPStatic* m_pMode;
	CDPStatic* m_pSetTemp;
	CDPStatic* m_pCurTemp;
	CDPButton* m_pOnOff;

	CDPButton* m_pDu;
	CDPButton* m_pDu1;
	
	SmartDev* m_pSmartDev;
	AC_DATA* m_pHeat;
};

CAppBase* CreateHeatApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CHeatApp* pApp = new CHeatApp(wParam);
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
