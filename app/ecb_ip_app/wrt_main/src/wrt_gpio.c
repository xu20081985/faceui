/*
 * wrt_gpio.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_gpio.cpp 	2017/07/05   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *  The module is mainly to achieve the initialization of the X1 chip GPIO, 
 *  by calling the underlying interface function for GPIO and other related operations, 
 *  including buttons, led, watchdog and other functions of GPIO
 */
 
/********************************* Includes ***********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <errno.h>
#include <sys/ioctl.h>
#include "wrt_gpio.h"

int g_hWatchDog;

/******************************************************************************/
/*
 *   This function initializes GPIO and opens the GPIO device
 */
int InitGpio(int *fd, GPIO_CFG gpio_cfg)
{	
	if (*fd > 0) 
	{
		printf("InitGpio  error\n");
		return -1;
	}
	*fd = open("/dev/gpio", O_RDWR, 0);
	if (*fd < 0) 
	{
		printf("open /dev/gpio error\n");
		return -1;
	}
	
	if (ioctl(*fd, IOCTRL_GPIO_INIT, &gpio_cfg))
	{
		printf("IOCTRL_GPIO_INIT error is %d\n");
		return -1;
	}
	
	return 0;
}

/******************************************************************************/
/*
 *   Set the status of GPIO via IOCTL
 */
int SetGpioVal(int fd, unsigned char data)
{
	if (ioctl(fd, IOCTRL_SET_GPIO_VALUE, &data))
	{
		printf("IOCTRL_SET_GPIO_VALUE  fail error is %d\n");
		return -1;
	}
	else
		return 0;
}

/******************************************************************************/
/*
 *   Get the status of GPIO via IOCTL
 */
int GetGpioVal(int fd, unsigned char*data)
{
	if (ioctl(fd, IOCTRL_GET_GPIO_VALUE, data))
	{
		printf("IOCTRL_SET_GPIO_VALUE  fail error is %d\n");
		return -1;
	}
	else
		return 0;
}

/******************************************************************************/
/*
 *   Start the watchdog task by initializing watchdog GPIO,
 *   watchdog timeout is 16S, more than the time system restart
 */
int StartWatchDog()
{
	unsigned int timeout = 16000;
	
	if ((g_hWatchDog = open("/dev/watchdog", O_RDWR, 0)) <= 0) 
	{
		printf("open /dev/watchdog error\n");
		return -1;
	}
	
	if (ioctl(g_hWatchDog, IOCTL_START_WATCHDOG))
	{
		printf("IOCTL_START_WATCHDOG error\n");
		close(g_hWatchDog);
		return -1;
	}
	
	if (ioctl(g_hWatchDog, IOCTL_REFRESH_WATCHDOG, &timeout))
	{
		printf("IOCTL_START_WATCHDOG error\n");
		ioctl(g_hWatchDog, IOCTL_STOP_WATCHDOG);
		close(g_hWatchDog);
		return -1;
	}
	
	return 0;
}




