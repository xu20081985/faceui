#include "CCtrlModules.h"
#include "SmartConfig.h"

BOOL ON_OFF;          // ���ػ�״̬��־λ

// �������
WORD m_TVOPEN;		   // ���ӿ��������
WORD m_TVCLOSE;		   // ���ӹغ������
WORD m_TVOL; 		   // ���ӿ�/�غ������
WORD m_CHANNEL1; 	   // Ƶ��+�������
WORD m_CHANNEL2; 	   // Ƶ��-�������
WORD m_VOICE1;		   // ����+�������
WORD m_VOICE2;		   // ����-�������
WORD m_SWITCH;		   // TV/AV�л�������� 

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

			else if(wParam == m_idOnOff) { // ���ػ�����ִ��
				
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_TVOL);
			}

			else if(wParam == m_idTvAV) {  // TV/AVת������ִ��

				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_SWITCH);
			}

			else if(wParam == m_idAdd) {   // Ƶ��+����ִ

				// �����������
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_CHANNEL1);
			}

			else if(wParam == m_idSub) {   // Ƶ��-����ִ��

				// �����������
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_CHANNEL2);
			}

			else if(wParam == m_idVoicep) { // ����+����ִ��

				// �����������
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_VOICE1);
			}

			else if(wParam == m_idVoice) {  // ����-����ִ��

				// �����������
				SendSmartCmd(m_pSmartDev->addr, SCDM_INFRARED, m_VOICE2);
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

		// ��ʾ��������(�������)
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
		GetCtrlByName("back", &m_idBack);         // ���ذ���
		GetCtrlByName("btn_onoff", &m_idOnOff);
		GetCtrlByName("btn_tvav", &m_idTvAV);
		GetCtrlByName("btn_channelp", &m_idAdd);
		GetCtrlByName("btn_channel", &m_idSub);
		GetCtrlByName("btn_voicep", &m_idVoicep);
		GetCtrlByName("btn_voice", &m_idVoice);
		
		m_pTitle = (CDPStatic *)GetCtrlByName("title");                     // ����ͼ��	
		m_pCh1   = (CDPStatic *)GetCtrlByName("cha");
		m_pCh2   = (CDPStatic *)GetCtrlByName("chs");
		m_pVo1   = (CDPStatic *)GetCtrlByName("vca");
		m_pVo2   = (CDPStatic *)GetCtrlByName("vcs");
		m_pTVAV  = (CDPStatic *)GetCtrlByName("tvav");

		// ��ú�����ӵĺ�����Ʊ���

#if 1
		m_TVOL     = GetIR_TV_CODE(m_TVOL,0);
		m_CHANNEL1 = GetIR_TV_CODE(m_CHANNEL1,1);
		m_CHANNEL2 = GetIR_TV_CODE(m_CHANNEL2,2);
		m_VOICE1   = GetIR_TV_CODE(m_VOICE1,3);
		m_VOICE2   = GetIR_TV_CODE(m_VOICE2,4);
		m_SWITCH   = GetIR_TV_CODE(m_SWITCH,5); 
#endif 
		OnCreate((SmartDev*)lParam); /*����Ϊ������豸*/


		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idAdd;          // Ƶ��+����id
	DWORD m_idSub;          // Ƶ��-����id
	DWORD m_idVoicep;       // ����+����id
	DWORD m_idVoice;        // ����-����id
	DWORD m_idOnOff;        // ��/�ػ�����id
	DWORD m_idTvAV;         // TV/AV�л�����id         

	CDPStatic* m_pTitle;    // ���⾲̬����
	CDPStatic* m_pCh1;
	CDPStatic* m_pCh2;
	CDPStatic* m_pVo1;
	
	CDPStatic* m_pVo2;
	
	CDPStatic* m_pTVAV;
	
/*	CDPButton* m_pOnOff;    // ���ػ�ͼ��ָ��
	CDPButton* m_pTVAV;     // TV/AVͼ��ָ��
	CDPButton* m_pCh1;      // Ƶ��+ͼ�����ָ��
	CDPButton* m_pCh2;      // Ƶ��-ͼ�����ָ��
	CDPButton* m_pVo1;      // ����+ͼ�����ָ��
	CDPButton* m_pVo2;      // ����-ͼ�����ָ��
	CDPButton* m_pOk;       // Okͼ��ָ�� 
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


