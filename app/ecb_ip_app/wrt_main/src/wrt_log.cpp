#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "wrt_log.h"
#include "wrt_common.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include "wrt_cfg.h"

extern T_SYSTEMINFO* pSystemInfo;

const CMDLIST netCmdArr[256] =
{
	{0x0616, "APP控制情景"},
	{0x0816, "APP控制情景应答"},
	{0x061B, "APP控制设备开/关"},
	{0x081B, "APP控制设备开/关应答"},
	{0x0636, "APP设置一键布撤防"},
	{0x0836, "APP设置一键布撤防应答"},	
	{0x0647, "APP同步网关时间"},
	{0x0847, "APP同步网关时间应答"},				
	{0x0692, "APP直接控制设备"},
	{0x0893, "设备直接通知APP"},
	{0x0620, "APP查询设备开/关状态"},
	{0x0820, "APP查询设备开/关状态应答"},
	{0x0621, "APP查询当前情景"},
	{0x0821, "APP查询当前情景应答"},
	{0x0622, "APP同步组地址"},
	{0x0822, "APP同步组地址应答"},
	{0x0623, "APP查询设备地址"},
	{0x0823, "APP查询设备地址应答"},
	{0x0624, "APP同步情景"},
	{0x0824, "APP同步情景应答"},
	{0x0625, "APP查询情景"},
	{0x0825, "APP查询情景应答"},
	{0x0626, "APP查询单个设备开/关状态"},
	{0x0826, "APP查询单个设备开/关状态应答"},
	{0x0627, "APP查询空调数据"},
	{0x0827, "APP查询空调数据应答"},
	{0x0628, "APP备份信息到网关"},
	{0x0828, "APP备份信息到网关应答"},
	{0x0629, "APP从网关得到信息"},
	{0x0829, "APP从网关得到信息应答"},
	{0x062A, "APP设置设备信息"},
	{0x082A, "APP设置设备信息应答"},
	{0x062B, "APP查询设备信息"},
	{0x082B, "APP查询设备信息应答"},
	{0x062C, "APP搜索设备"},
	{0x082C, "APP搜索设备应答"},
	{0x062D, "APP设置传感器控制方式"},
	{0x082D, "APP设置传感器控制方式应答"}
};

const CMDLIST uartCmdArr[256] = 
{
	{0x0101, "搜索步骤1"},
	{0x0102, "搜索步骤1应答"},
	{0x01A0, "搜索步骤2"},
	{0x01A1, "搜索步骤2应答"},
	{0x0103, "设备上报信息给网关"},
	{0x0104, "设备上报信息给网关应答"},
	{0x0105, "设置器件物理地址"},
	{0x0106, "设置器件物理地址应答"},
	{0x0109, "查询器件物理地址"},
	{0x0110, "查询器件物理地址应答"},
	{0x0113, "设置器件组地址"},
	{0x0114, "设置器件组地址应答"},
	{0x0115, "查询器件组地址"},
	{0x0116, "查询器件组地址应答"},
	{0x0121, "设置器件组地址和控制方式"},
	{0x0122, "设置器件组地址和控制方式应答"},
	{0x0123, "查询器件组地址和控制方式"},
	{0x0124, "查询器件组地址和控制方式应答"},
	{0x0125, "设备被什么场景控制"},
	{0x0126, "设备被什么场景控制应答"},
	{0x0127, "查询设备被什么场景控制"},
	{0x0128, "查询设备被什么场景控制应答"},
	{0x0133, "设置传感器响应时间"},
	{0x0134, "设置传感器响应时间应答"},
	{0x0135, "查询传感器响应时间"},
	{0x0136, "查询传感器响应时间应答"},
	{0x017E, "设置窗帘开关时间"},
	{0x017F, "设置窗帘开关时间应答"},
	{0x0189, "学习红外设备信息"},
	{0x0190, "学习红外设备信息应答"},
	{0x0191, "学习红外设备信息成功"},	
	{0x0192, "设置红外控制方式"},
	{0x0193, "设置红外控制方式应答"},	
	{0x0194, "查询红外控制方式"},
	{0x0195, "查询红外控制方式应答"},	
	{0x018A, "查询窗帘开关时间"},
	{0x018B, "查询窗帘开关时间应答"},	
	{0x018E, "设置传感器控制方式"},
	{0x018F, "设置传感器控制方式应答"},	
	{0x019A, "查询传感器控制方式"},
	{0x019B, "查询传感器控制方式应答"},	
	{0x0196, "设置传感器组地址"},
	{0x0197, "设置传感器组地址应答"},	
	{0x0198, "查询传感器组地址"},
	{0x0199, "查询传感器组地址应答"},	
	{0x019C, "删除传感器组地址"},
	{0x019D, "删除传感器组地址应答"},	
	{0x019E, "设置传感器参数"},
	{0x019F, "设置传感器参数应答"},
	{0x01AA, "查询传感器参数"},
	{0x01AB, "查询传感器参数应答"},
	{0x0201, "设备开/关"},
	{0x0202, "设备开/关应答"},	
	{0x0320, "采集物理地址"},
	{0x0321, "采集物理地址应答"},
	{0x0322, "采集组地址"},
	{0x0323, "采集组地址应答"},
	{0x0330, "设置设备时间"},
	{0x0331, "设置设备时间应答"},
	{0x0350, "传感器触发"},
	{0x0351, "传感器触发应答"},
	{0x0380, "查询设备开/关状态"},
	{0x0381, "查询设备开/关状态应答"},
	{0x0382, "查询空调数据"},
	{0x0383, "查询空调数据应答"},
	{0x0391, "驱动设备触发"},
	{0x0392, "驱动设备触发应答"},
	{0x03A0, "控制驱动器"},
	{0x03A1, "控制驱动器应答"},
	{0x03A2, "控制面板"},
	{0x03A3, "控制面板应答"},
	{0x0549, "设置传感器控制方式"},
	{0x0550, "设置传感器控制方式应答"},
	{0x0551, "查询传感器控制方式"},
	{0x0552, "查询传感器控制方式应答"},
	{0x0901, "查询器件固有信息"},
	{0x0902, "查询器件固有信息应答"},
	{0x0903, "进入测试状态"},
	{0x0904, "进入测试状态应答"},
	{0x010C, "初始化设备"},
	{0x010D, "初始化设备应答"},
	{0x01AE, "初始化设备"},
	{0x01AF, "初始化设备应答"},
	{0x0A01, "初始化所有设备"},
	{0x0A02, "初始化所有设备应答"},
	{0x010A, "软复位"},
	{0x010B, "软复位应答"},
	{0x01AC, "软复位"},
	{0x01AD, "软复位应答"},
	{0x0A03, "软复位所有器件"},
	{0x0A04, "软复位所有器件应答"}
};

static size_t read_callback1(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode;

/*  in real-world cases, this would probably get this data differently
 *	as this fread() stuff is exactly what the library already would do
 *	by default internally 
 */
	retcode = fread(ptr, size, nmemb, (FILE *)stream);

	fprintf(stderr, "*** We read %d bytes from file\n", retcode);

	return retcode;
}

CWRTLogManage CWRTLogManage::s_logObj;

CWRTLogManage::CWRTLogManage(void)
{
}

CWRTLogManage::~CWRTLogManage(void)
{

}

int CWRTLogManage::getFormatTime(int *year, int *month, int *day, int *week, int *hour, int * minute, int * second)
{
	time_t timep;
	struct tm *p;
	
	time(&timep);
	p = localtime(&timep);
	
	*year = (1900 + p->tm_year);
	*month = (1 + p->tm_mon);
	*day = p->tm_mday;
	*week = p->tm_wday;
	*hour = p->tm_hour;
	*minute = p->tm_min;
	*second = p->tm_sec;

	return 0;
}

int CWRTLogManage::Init(void)
{
	DIR *dirp;
	
	dirp = opendir(WRT_LOG_PATH);
	if ((dirp = opendir(WRT_LOG_PATH)) == NULL)
	{
		char path[BUFF_SIZE] = {0};
		sprintf(path, "mkdir -p %s", WRT_LOG_PATH);
		system(path);
	}
	
	int year, month, day, week, hour, minute, second;
	getFormatTime(&year, &month, &day, &week, &hour, &minute, &second);
	sprintf(m_curUseFile, "%04d%02d%02d.log", year, month, day);
	
	char fileName[BUFF_SIZE] = {0};
	sprintf(fileName, "%s%s", WRT_LOG_PATH, m_curUseFile);

	if ((m_logFd = fopen(fileName, "a+")) == NULL)
	{
		DEBUG_ERROR("open log %s file error\n", fileName);
		return -1;
	}
	fclose(m_logFd);
	m_logFd = NULL;
	
	m_lastUploadTime = time(NULL);
	pthread_mutex_init(&m_fdMutex, NULL);		
	CreateThread(m_logThread, logThread, NULL);
	
	return 0;
}

int CWRTLogManage::DisInit(void)
{
	return 0;
}

int CWRTLogManage::CreateThread(pthread_t thread, funcThread func, void *param)
{
	bool ret;
	pthread_attr_t thread_attr;

	ret = pthread_attr_init(&thread_attr);
	if (ret != 0)
	{
		return -1;
	}	
	ret = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	if (ret != 0)
	{
		(void)pthread_attr_destroy(&thread_attr);
		return -1;
	}
	ret = pthread_create(&thread, &thread_attr, func, param);
	if (ret != 0)
	{
		(void)pthread_attr_destroy(&thread_attr);
		return -1;
	}
	(void)pthread_attr_destroy(&thread_attr);

	return 0;
}

void *CWRTLogManage::logThread(void *)
{
	s_logObj.logThreadProj();
	return NULL;
}
void *CWRTLogManage::logThreadProj()
{
	int flag;
	char FilePath[BUFF_SIZE] = {0};
	while (true)
	{
/*
 *  Upload the server two hours at a time
 */		
		if((time(NULL) - m_lastUploadTime) >= 60 * 60 * 2)
		{			
			pthread_mutex_lock(&m_fdMutex);
			
			strcpy(m_bkFile, m_curUseFile);
			memset(m_curUseFile, 0, sizeof(m_curUseFile));
			int year, month, day, week, hour, minute, second;			
			getFormatTime(&year, &month, &day, &week, &hour, &minute, &second);
			sprintf(m_curUseFile, "%04d%02d%02d.log", year, month, day);
			
			char fileName[BUFF_SIZE] = {0};
			sprintf(fileName, "%s%s", WRT_LOG_PATH, m_curUseFile);
				
			m_logFd = fopen(fileName, "a+");
			if (NULL == m_logFd)
			{
				DEBUG_ERROR("open log %s file error\n", fileName);
				continue;
			}
			fclose(m_logFd);
			m_logFd = NULL;

			pthread_mutex_unlock(&m_fdMutex);
			m_lastUploadTime = time(NULL);
		}
		
		if (strlen(m_bkFile) != 0) 	//have Backup log files
		{
			sprintf(FilePath, "%s%s", WRT_LOG_PATH, m_bkFile);
			if (access(FilePath, F_OK) != 0) //file does not exist
			{
				memset(m_bkFile, 0, sizeof(m_bkFile));
			}
			else
			{
				flag = 5;
				while (flag > 0)
				{
					int ret = uploadFile(m_bkFile);
					if (ret == 0)
					{
						DEBUG_MESSAGE("wrt_log upload ok\n");
						if (strcmp(m_bkFile, m_curUseFile))
						{
							char cmd[BUFF_SIZE];
							sprintf(cmd, "rm -rf %s", FilePath);
							system(cmd);
							DEBUG_MESSAGE("wrt_log del ok\n");//Upload successfully deleted yesterday log			
						}
						memset(m_bkFile, 0, sizeof(m_bkFile));
						break;
					}
					else
					{
						DEBUG_ERROR("wrt_log upload error\n");
						if(1 == flag)
						{
							char cmd1[BUFF_SIZE];
							sprintf(cmd1, "rm -rf %s", FilePath);
							system(cmd1);
							DEBUG_MESSAGE("wrt_log del ok\n");//Upload successfully deleted 
						}
					}
					flag--;
					usleep(1000 * 500);
				}
			}
		}
		
		sleep(60);
	}
	
	return NULL;
}

int CWRTLogManage::uploadFile(char *fileName)
{
	CURL *curl;
	CURLcode res;
	FILE * hd_src ;
	struct stat file_info;
	char *auth = NULL;
	long auth_methods;
	
	char remote_path[256];
	memset( remote_path, 0, sizeof(remote_path) );
	snprintf( remote_path, sizeof(remote_path), "ftp://182.92.195.120/backup/%s/%s", 
		pSystemInfo->DoorSysInfo.gateWayDeviceID, fileName);
	DEBUG_MESSAGE("remote log path:%s\n", remote_path);
	
/* 
 *  get the file size of the local file 
 */
	char filePath[64] = {0};
	sprintf(filePath, "%s%s", WRT_LOG_PATH, fileName);
	stat(filePath, &file_info);

/*  get a FILE * of the same file, could also be made with
 *	fdopen() from the previous descriptor, but hey this is just
 *	an example! 
 */
	hd_src = fopen(filePath, "rb");

/* 
 *	In windows, this will init the winsock stuff 
 */
	curl_global_init(CURL_GLOBAL_ALL);

/* 
 * do authentication 
 */
	const int m_size = 128;
	auth = (char *)malloc(m_size);
	if (!auth) 
		return -1;
	snprintf(auth, m_size, "%s:%s", "wrt", "wrt");

	static CURLSH *share_handle = NULL;
	if(NULL == share_handle)
	{
		share_handle = curl_share_init();
		curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
	}
	
	auth_methods = CURLAUTH_BASIC | CURLAUTH_DIGEST;
/* 
 * get a curl handle 
 */
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_SHARE, share_handle);
		/* we want to use our own read function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback1);

		/* enable uploading */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* HTTP PUT please */
		//curl_easy_setopt(curl, CURLOPT_PUT, 1L);

		curl_easy_setopt(curl ,CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);


		/* specify target URL, and note that this URL should include a file
		name, not only a directory */
		curl_easy_setopt(curl, CURLOPT_URL, remote_path);

		/* now specify which file to upload */
		curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

		/* auth */
		//curl_easy_setopt(curl, CURLOPT_HTTPAUTH, auth_methods); /* TODO possibility of selection */
		//curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth);

		/* provide the size of the upload, we specicially typecast the value
		to curl_off_t since we must be sure to use the correct data size */
		curl_easy_setopt(curl, CURLOPT_INFILESIZE,
			(int)file_info.st_size);

		/* Now run off and do what you've been told! */
		res = curl_easy_perform(curl);
		
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	fclose(hd_src); /* close the local file */
	if (auth) 
		free(auth);
	auth = NULL;
	curl_global_cleanup();
	
	return res;
}

int CWRTLogManage::getFileSize(const char *fileName)
{
	int size;
	struct stat statbuf;
	char path[BUFF_SIZE] = {0};
	
	sprintf(path, "%s%s", WRT_LOG_PATH, fileName);
	if ((access(path, F_OK)) != 0)
	{
		DEBUG_ERROR("access path error\n");
		return -1;
	}

	stat(path, &statbuf);
	size = statbuf.st_size;
	
	return size;
}

int CWRTLogManage::logWrite(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char filePath[BUFF_SIZE] = {0};

	pthread_mutex_lock(&m_fdMutex);
	
	sprintf(filePath, "%s%s", WRT_LOG_PATH, m_curUseFile);
	if(NULL != (m_logFd = fopen(filePath, "a+w")))
	{
		vfprintf(m_logFd, format, args);
		fflush(m_logFd);
		fclose(m_logFd);
	}
	
	pthread_mutex_unlock(&m_fdMutex);
	va_end (args);

	return 0;
}

/*
 *  Function: convert 16 hexadecimal data into string, which is easy to read
 */
int CWRTLogManage::ecbLogDec2Hex(const unsigned char *inStr, char *outStr, int len)
{	
	int index;
	int length;
	char *pdata = NULL;

	length = len * 2 + 4;
	pdata = (char *)malloc(length);
	if (NULL == pdata)
	{
		DEBUG_ERROR("malloc pdata error\n");
		return -1;
	}
	
	memset(pdata, 0, length);

	for(index = 0; index < len; index++)
	{
		sprintf(pdata + index * 2, "%02X", *(inStr + index));
	}

	if (length < 1024)
		memcpy(outStr, pdata, length);
	else
		memcpy(outStr, pdata, 1023);
	
	if (pdata)
		free(pdata);

	return 0;
}

int CWRTLogManage::getTimeYmdms(char *str)
{
	time_t t = 0;
	struct tm * tm;
	struct timeval tv;
	char year[BUFF_SIZE] = {0};
	char days[BUFF_SIZE] = {0};
	char usec[BUFF_SIZE] = {0};

	time(&t);
	tm = localtime(&t);
	gettimeofday(&tv, NULL);

	/* Format the date and time */
	sprintf(year, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	sprintf(days, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	sprintf(usec, "%03d", (tv.tv_usec / 1000) % 1000);
	sprintf(str, "%s %s %s", year, days, usec);

	return 0;
}


