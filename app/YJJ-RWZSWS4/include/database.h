#pragma once

#define LANGUAGE_CH				0	// 中文
#define LANGUAGE_EN				1	// 英文

#define DEFAULT_RING_MP3			"/FlashDev/ring/musicring.mp3"
#define DEFAULT_BK_JPG				"/Windows/bk.jpg"

#define DELAY_SCREEN			0	// 屏保延时
// DBSet.cpp
void InitSystemSet(void);
void InitTimerFILE();

void ResetSystemSet(void);

void InitDefaultSystemSet(void);       	// 配置初始密码及背景等

void GetPrjbkp(char* szbkp);					// 获取背景图片
void SetPrjbkp(char* szbkp);            		// 设置背景图片

BYTE GetAutoBk();								// 获取自动背景
void SetAutoBk(BYTE status);					// 设置自动背景

void GetPrjPwd(char* szPasswd);				// 获取管理密码
void SetPrjPwd(char* szPasswd);				// 设置管理密码

DWORD GetPrjShow();							// 获取屏保时间
void SetPrjShow(DWORD show);				// 设置屏保时间

DWORD GetShowBright();						// 获取屏幕亮度
void SetShowBright(DWORD bright);			// 设置屏幕亮度

WORD GetIR_TV_CODE(WORD Status,int i);  // 获得TV红外编码
void SetIR_TV_CODE(WORD Status,int i);  // 红外编码写入到文件当中

WORD GetIR_AIR_CODE(WORD Status,int i); // 获得IR_AIR红外编码
void SetIR_AIR_CODE(WORD Status,int i); // 红外编码写入到文件当中

DWORD GetRingVol(void);
DWORD GetDelay(DWORD type);
BOOL SetScreenOnOff(BOOL bOn);
