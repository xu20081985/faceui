#pragma once

#define LANGUAGE_CH				0	// ����
#define LANGUAGE_EN				1	// Ӣ��

#define DEFAULT_RING_MP3			"/FlashDev/ring/musicring.mp3"
#define DEFAULT_BK_JPG				"/Windows/bk.jpg"

#define DELAY_SCREEN			0	// ������ʱ
// DBSet.cpp
void InitSystemSet(void);
void InitTimerFILE();
void UpdatSetTimer();

void FlashStatusTimer(pNode TimerEvent);   // 2018.1.19

void ResetSystemSet(void);
void GetPrjPwd(char* szPasswd);
void SetPrjPwd(char* szPasswd);

void SetPrjShow(BOOL* show);
void GetPrjShow(BOOL* show);


void SetPrjbkp(char* szbkp);            // ��������
void GetTimercount(int szTimer);        // ��ȡ��ǰ��ʱ�¼��ĸ���
void SetTimercount(int szTimer);        // ���õ�ǰ��ʱ�¼��ĸ���
void InitDefaultSystemSet(void);        // ���ó�ʼ���뼰������

/* 2017.12.08����� */
WORD GetIR_TV_CODE(WORD Status,int i);  // ���TV�������
void SetIR_TV_CODE(WORD Status,int i);  // ��ECB�����ϵĺ������д�뵽�ļ�����

/* 2017.12.18����� */ 
WORD GetIR_AIR_CODE(WORD Status,int i); // ���IR_AIR�������
void SetIR_AIR_CODE(WORD Status,int i); // ��ECB�����ϵĺ������д�뵽�ļ�����



DWORD GetRingVol(void);
DWORD GetDelay(DWORD type);
BOOL SetScreenOnOff(BOOL bOn);
