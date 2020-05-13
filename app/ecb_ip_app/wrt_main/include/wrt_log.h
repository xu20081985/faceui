#ifndef _WRT_LOG_H_
#define _WRT_LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WRT_LOG_PATH "/UserDev/log/"

typedef struct CMDLIST
{
	unsigned short cmd;
	char ch[40];
}CMDLIST;

extern const CMDLIST netCmdArr[256];
extern const CMDLIST uartCmdArr[256];



class CWRTLogManage
{
private:
	time_t m_lastUploadTime;
	FILE *m_logFd;
	char m_curUseFile[32];
	char m_bkFile[32];
	char m_delFile[32];
	static CWRTLogManage s_logObj; 
	///< 互斥量
	pthread_mutex_t m_fdMutex;
	pthread_t m_logThread;

private:
	typedef void *(*funcThread)(void*);
	/// @brief 创建线程
	int CreateThread(pthread_t thread, funcThread func, void *param);
	static void *logThread(void *);
	void * logThreadProj();
	

private:
	int FtpSend(void);
	void ClearnSnapPicBuff();


public:
	CWRTLogManage(void);
	virtual ~CWRTLogManage(void);

	static CWRTLogManage &WRTLogManage() { return s_logObj;};
	int Init(void);
	int DisInit(void);
	int getFormatTime(int *year, int *month, int *day, int *week, int *hour, int * minute, int * second);
	int getFileSize(const char *path);	
	int logWrite(const char *format, ...);
	int ecbLogDec2Hex(const unsigned char *inStr, char *outStr, int len);
	int getTimeYmdms(char *str);
	int uploadFile(char *uploadFilePath);
};

#endif /*_WRT_LOG_H_*/

