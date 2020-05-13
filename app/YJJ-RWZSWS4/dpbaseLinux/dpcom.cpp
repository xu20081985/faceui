#include "dpplatform.h"
#include "dpcom.h"
#include <fcntl.h>
#include <termios.h>

HANDLE OpenComm(const char* szComName, DWORD dwBAUD_RATE, BYTE bufDataByte, DWORD timeOut)
{
	int fd = -1;
	do 
	{
		fd = open(szComName, O_RDWR | O_NOCTTY );//| O_NONBLOCK
		if(-1 == fd)
		{
			printf("OpenComm open %s fail:%d\r\n", szComName, errno);
			break;
		}

		struct termios options;
		if(-1 == tcgetattr(fd, &options))
		{
			printf("OpenComm tcgetattr fail:%d\r\n", errno);
			break;
		}

		memset(&options, 0, sizeof(options));
		options.c_cflag |= CLOCAL | CREAD;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;					// 8位
		options.c_cflag &= ~PARENB;				// 无校验
		options.c_cflag &= ~CSTOPB;				// 1位停止位
		cfsetispeed(&options, B115200);			// 波特率
		cfsetospeed(&options, B115200);			// 波特率

		options.c_cc[VTIME] = 1;
		options.c_cc[VMIN] = 100;
		tcflush(fd, TCIFLUSH);
		if(-1 == tcsetattr(fd, TCSANOW, &options))
		{
			printf("OpenComm tcgetattr fail:%d\r\n", errno);
			break;
		}

		return (HANDLE)fd;
	}while(0);

	if(fd != -1)
		close(fd);
	return NULL;
}

void CloseComm(HANDLE hCom)
{
	if(hCom)
		close((int)hCom);
}

int ReadComm(HANDLE hCom, char* buf, int len)
{
	int fd = (int)hCom;
	fd_set fdR;
	FD_ZERO(&fdR);
	FD_SET(fd, &fdR);

	struct timeval tv_out;
	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;

	int ret = select(fd + 1, &fdR, NULL, NULL, &tv_out);
	if(ret < 0)
		printf("ReadComm select fail\r\n");
	if (ret > 0)
	{
		ret = read((int)hCom, buf, len);
		if(ret < 0)
			printf("ReadComm read fail\r\n");
	}
	return ret;
}

int SendComm(HANDLE hCom, char* buf, int len)
{
	if(NULL == hCom)
	{
		printf("SendComm NULL\r\n");
		return -1;
	}

	int ret = write((int)hCom, buf, len);
	if(ret < 0)
	{
		printf("SendComm fail:%d\r\n", errno);
		return -1;
	}
	DPSleep(20);
	return ret;
}