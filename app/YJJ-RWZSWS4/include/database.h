#pragma once

#define LANGUAGE_CH				0	// ����
#define LANGUAGE_EN				1	// Ӣ��

#define DEFAULT_RING_MP3			"/FlashDev/ring/musicring.mp3"
#define DEFAULT_BK_JPG				"/Windows/bk.jpg"

#define DELAY_SCREEN			0	// ������ʱ
// DBSet.cpp
void InitSystemSet(void);
void InitTimerFILE();

void ResetSystemSet(void);

void InitDefaultSystemSet(void);       	// ���ó�ʼ���뼰������

void GetPrjbkp(char* szbkp);					// ��ȡ����ͼƬ
void SetPrjbkp(char* szbkp);            		// ���ñ���ͼƬ

BYTE GetAutoBk();								// ��ȡ�Զ�����
void SetAutoBk(BYTE status);					// �����Զ�����

void GetPrjPwd(char* szPasswd);				// ��ȡ��������
void SetPrjPwd(char* szPasswd);				// ���ù�������

DWORD GetPrjShow();							// ��ȡ����ʱ��
void SetPrjShow(DWORD show);				// ��������ʱ��

DWORD GetShowBright();						// ��ȡ��Ļ����
void SetShowBright(DWORD bright);			// ������Ļ����

WORD GetIR_TV_CODE(WORD Status,int i);  // ���TV�������
void SetIR_TV_CODE(WORD Status,int i);  // �������д�뵽�ļ�����

WORD GetIR_AIR_CODE(WORD Status,int i); // ���IR_AIR�������
void SetIR_AIR_CODE(WORD Status,int i); // �������д�뵽�ļ�����

DWORD GetRingVol(void);
DWORD GetDelay(DWORD type);
BOOL SetScreenOnOff(BOOL bOn);
