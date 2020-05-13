#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/input.h>
#include "SmartConfig.h"

static void* TouchEvent(void* pParam)
{
	int ret;
	int fd;
	static int xydata[2];
	static BOOL flag = FALSE;	
	static BOOL curdown = FALSE;
	struct input_event buf;

	if (GetSwitch(SWITCH_KEY))
		fd = open("/dev/input/event1", O_RDONLY);
	else
		fd = open("/dev/input/event0", O_RDONLY);

	if (fd < 0) {
		perror("open rtp event fail!\n");
		return NULL;
	}

	while (1) {
		memset(&buf, 0, sizeof(buf));
		ret = read(fd, &buf, sizeof(struct input_event));
		printf("TP %d %d %d\r\n", buf.type, buf.code, buf.value);

		// 读取坐标点，先读X坐标，后读Y坐标
		if (buf.type == EV_REL)	{				// #define EV_REL			0x02
			if (buf.code == REL_X) {			// #define REL_X			0x00
				xydata[0] = buf.value;
			} else if (buf.code == REL_Y) {  	// #define REL_Y	    	0x01
				xydata[1] = buf.value;
			}
		}

		// 按下: 1  松开: 0
		if (buf.type == EV_KEY)	{
			if (buf.code == BTN_TOUCH) {		// #define BTN_TOUCH		0x14a
				if (buf.value == 0) {
					flag = FALSE;
				} else {
					flag = TRUE;
				}
			}
		}
		
		// 	同步
		if (buf.type == EV_SYN)	{				// #define EV_SYN			0x00
			if (flag) {
				if (curdown) {
					DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_VALID, MSG_TOUCH_TYPE);
				} else {
					DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_DOWN, MSG_TOUCH_TYPE);
					curdown = TRUE;
				}
			} else {
				DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_UP, MSG_TOUCH_TYPE);
				curdown = FALSE;
			}
		}	
	}
	
	close(fd);
	return NULL;
}


static void* KeybdThread(void* pParam)
{
	int fd;
	struct input_event buf;

	fd = open("/dev/input/event0", O_RDONLY);
	if(fd < 0)
	{
		printf("open key event fail!\n");
		return NULL;
	}

	while(read(fd, &buf, sizeof(struct input_event)))
	{
		if(buf.type == 1)
		{
			if(buf.value == 1)
			{
				DPPostMessage(HARDKBD_MESSAGE, KBD_DOWN, buf.code, 0, MSG_KEY_TYPE);
			}
			else if(buf.value == 0)
			{
				DPPostMessage(HARDKBD_MESSAGE, KBD_UP, buf.code, 0, MSG_KEY_TYPE);
			}
		}
	}
	close(fd);
	return NULL;
}

static void TimeThread(union sigval v)  
{
	DPPostMessage(TIME_MESSAGE, 0, 0, 0, MSG_TIME_TYPE);   //定时发送一次时间消息。
}  

void DPCreateTimeEvent(void)
{
	timer_t timeridd;  
	struct sigevent evp;  
	memset(&evp, 0, sizeof(struct sigevent));

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = TimeThread;

	int ret = timer_create(CLOCK_REALTIME, &evp, &timeridd);
	if (ret)
		perror("DPCreateTimeEvent fail\r\n");
	else
	{
		struct itimerspec ts;  
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		ret = timer_settime(timeridd, 0, &ts, NULL);
		if (ret)
			perror("timer_settime");
	}
}


void DPCreateTouchEvent()
{
	pthread_t pid0;
	pthread_create(&pid0, NULL, TouchEvent, NULL);
}

void DPCreateKeyEvent()
{
	if(GetSwitch(SWITCH_KEY))
	{
		pthread_t pid0;
		pthread_create(&pid0, NULL, KeybdThread, NULL);
	}
}

void DPCreateTimerEvent()
{
	pthread_t pid0;
	pthread_create(&pid0, NULL, SmartTimerThread, NULL);
}
