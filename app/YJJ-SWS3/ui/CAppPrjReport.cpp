#include "CCtrlModules.h"
#include "SmartConfig.h"

class CPrjReportApp : public CAppBase
{
public:
	CPrjReportApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjReportApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:           								    //�����ǲ���������û�����ݣ�
			if(m_dwTimeout > 0)     								   //�����������˳��ý��档
			{
				m_dwTimeout--;
				if(m_dwTimeout == 0)
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}

			if(m_dwTick > 0)                                       //����Ǹ�Э��û�з���MSG_PRIVATE��Ϣ����ô�����ϱ�ʧ�ܡ�
			{
				if(DPGetTickCount() > m_dwTick)                   //ò���Ƕ೤ʱ��û�з�Ӧ��
				{
					m_pPng->SetSrc("tip_ok_big.png");
					m_pPng->Show(TRUE);

					m_pText->SetSrc(GetStringByID(14001));	   // �ϱ�IDʧ�ܣ������������ӣ�
					m_pText->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeout = TIP_TIMEOUT_FAIL;	
				}
			}
			break;
		case MSG_PRIVATE:
			if(wParam == m_IdBase)
			{
				m_pPng->SetSrc("tip_ok_big.png");
				m_pPng->Show(TRUE);

				m_pText->SetSrc(GetStringByID(14001));		// �ϱ�ID�ɹ���
				m_pText->Show(TRUE);

				m_dwTimeout = TIP_TIMEOUT_OK;
				m_dwTick = 0;
			}
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prj_report.xml");
		m_pPng = (CDPStatic *)GetCtrlByName("png");
		m_pText = (CDPStatic *)GetCtrlByName("text");

		SmartReportID();                 //��һ���ϱ�ID���ܽ����ֱ�ӷ��͸�Э�顣
		m_dwTick = DPGetTickCount() + TIP_TIMEOUT_ACK * 1000;  //��ô�������ˣ�����Ϊʲô����1000��
		return TRUE;
	}

private:
	CDPStatic* m_pPng;
	CDPStatic* m_pText;
	DWORD m_dwTick;
};

CAppBase* CreatePrjReportApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjReportApp* pApp = new CPrjReportApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}