#pragma once

#define LANGUAGE_CH				0	// 中文
#define LANGUAGE_EN				1	// 英文

#define DEFAULT_RING_MP3			"/FlashDev/ring/musicring.mp3"
#define DEFAULT_BK_JPG				"/Windows/bk.jpg"

#define DELAY_SCREEN			0	// 屏保延时
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


void SetPrjbkp(char* szbkp);            // 背景设置
void GetTimercount(int szTimer);        // 获取当前定时事件的个数
void SetTimercount(int szTimer);        // 设置当前定时事件的个数
void InitDefaultSystemSet(void);        // 配置初始密码及背景等

/* 2017.12.08日添加 */
WORD GetIR_TV_CODE(WORD Status,int i);  // 获得TV红外编码
void SetIR_TV_CODE(WORD Status,int i);  // 将ECB总线上的红外编码写入到文件当中

/* 2017.12.18日添加 */ 
WORD GetIR_AIR_CODE(WORD Status,int i); // 获得IR_AIR红外编码
void SetIR_AIR_CODE(WORD Status,int i); // 将ECB总线上的红外编码写入到文件当中



DWORD GetRingVol(void);
DWORD GetDelay(DWORD type);
BOOL SetScreenOnOff(BOOL bOn);
