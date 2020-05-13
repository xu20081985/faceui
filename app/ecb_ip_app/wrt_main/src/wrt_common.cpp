/*
 * wrt_common.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_common.cpp 	2017/06/20   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 * 	This file stores some common functions and functions, 
 *  including macros, definitions, structures, and so on
 */
 
/********************************* Includes ***********************************/

#include <pthread.h>
#include "wrt_common.h"
#include "wrt_devHandler.h"
#include "wrt_network.h"
#include "queue.h"

/********************************* Defines ************************************/

static uint CRC32[BUFF_SIZE];
static char init = 0;

/********************************** code *************************************/
/*
 *   init_table crc32
 */
//======================================================
//** 函数名称: init_table
//** 功能描述: 初始化crc32表
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
static void init_table()
{
	int i;
	int j;
	unsigned int crc;

	for (i = 0; i < BUFF_SIZE; i++)
	{
		crc = i;
		for (j = 0; j < RANDOM_ID_LEN; j++)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ 0xEDB88320;
			else
				crc = crc >> 1;
		}
		CRC32[i] = crc;	
	}
}

/******************************************************************************/
/*
 *   CRC32 implementation function
 */
//======================================================
//** 函数名称: crc32
//** 功能描述: crc32算法
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
unsigned int crc32(unsigned char *buf, int len)
{
	unsigned int ret = 0xFFFFFFFF;
	int i;
	
	if (!init)
	{
		init_table();
		init = 1;
	}
	
	for (i = 0; i< len; i++)
	{
		ret = CRC32[((ret & 0xFF) ^ (buf[i]))] ^ (ret >> RANDOM_ID_LEN);
	}

	ret = ~ret;

	return ret;
}

/******************************************************************************/
/*
 *   Function: memory release
 */

//======================================================
//** 函数名称: wrt_free_memory
//** 功能描述: 内存释放
//** 输　入: data_ptr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
void wrt_free_memory(void *data_ptr)
{
	if (NULL != data_ptr)
	{
		free(data_ptr);
		data_ptr = NULL;
	}
}

/******************************************************************************/
/*
 *	Thread creation
 */

//======================================================
//** 函数名称: pthread_create_t
//** 功能描述: 线程创建
//** 输　入: func
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int pthread_create_t(void *(* func)(void *))
{
	pthread_t id;
	pthread_attr_t attr;
	pthread_attr_init(&attr); 
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&id, &attr, func, NULL);
	pthread_attr_destroy(&attr);
	pthread_detach(id);
	return 0;
}

/******************************************************************************/
/*
 *  Gets N bytes of data
 */

//======================================================
//** 函数名称: getDataNaBytes
//** 功能描述: 数据帧获取数据
//** 输　入: str pos num
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
unsigned int getDataNaBytes(const unsigned char *str, int pos, int num)
{	
	int index;
	unsigned int tmp = 0;
    	
	for (index = 0; index < num; index++)
	{
		tmp |= str[pos + index];
		if (index < num - 1)
		{
			tmp <<= 8;
		}
	}
		
	return tmp;
}

/******************************************************************************/
/*
 *Generate send data checksum
 */
//======================================================
//** 函数名称: uartCheck
//** 功能描述: 串口校验
//** 输　入: str
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
unsigned char uartCheck(unsigned char *str)
{
	unsigned char checkSum;
	int length;
	int index;
	if (NULL == str)
	{
		return 0;
	}

	length = (*str);
	if (length >= 128)
	{
		return 0;
	}
	checkSum = (*str ^ (*(str+1))); 					 
	for (index = 2; index < (length); index++) 
	{ 
	   checkSum ^= (*(str+index)); 
	} 
	(*(str + index)) = checkSum;

	return checkSum;
}

/******************************************************************************/
/*
 *	Get gateway version file
 */

//======================================================
//** 函数名称: get_version_file
//** 功能描述: 获取网关文件
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================
int get_version_file()
{
    FILE *cfg_fd = NULL;
    int bytes_read;	
	int file_size;
	char *tmp;
	
    if ((cfg_fd = fopen(VER_FILE, "rb")) == NULL)
    {
        DEBUG_ERROR("version file does not exist\n");
		return -1;
    }
    else
    {
        fseek(cfg_fd, 0, SEEK_END);
		file_size = ftell(cfg_fd);
		fseek(cfg_fd , 0 , SEEK_SET);
		tmp = (char *)malloc(file_size * sizeof(char) + 1);
		bytes_read = fread(tmp, sizeof(char), file_size, cfg_fd);
        if (bytes_read != file_size)
        {
            fclose(cfg_fd);
            cfg_fd = NULL;
			free(tmp);
			tmp = NULL;
            return -1;
        }
		get_version_info(tmp, file_size);
        fclose(cfg_fd);
        cfg_fd = NULL;
		free(tmp);
		tmp= NULL;
    }
	return 0;
}


/******************************************************************************/
/*
 *	Get gateway version
 */
//======================================================
//** 函数名称: get_gateway_version
//** 功能描述: 获取网关版本
//** 输　入: str mcuVer
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int get_gateway_version(const char *str, int *mcuVer)
{
	if (!strlen(str)) 
	{
		DEBUG_ERROR("get_version_number str is null!\n");
		return -1;
	}
	
	char version[3][8];
	int index = 0;
	memset( version, 0, sizeof(version) );

	while (index < 3) 
	{
		const char *find = strchr( str, '.' );
		if ( NULL == find ) 
		{
			DEBUG_ERROR("nothing find . break!\n");
			break;
		} 
		else 
		{
			memcpy( version[index], str, find-str );
			str = find + 1;
			index++;
			if ( 2 == index )
			{
				memcpy( version[2], str, 2 );
				break;
			}
		}
	}
	
	mcuVer[0] = atoi(version[0]);
	mcuVer[1] = atoi(version[1]);
	mcuVer[2] = atoi(version[2]);

	DEBUG_MESSAGE("version:%d.%02d.%02d\n", mcuVer[0], mcuVer[1], mcuVer[2]);
	
	return 0;
}

/******************************************************************************/
/*
 *  Get formatting time
 */
//======================================================
//** 函数名称: getFormatTime
//** 功能描述: 获取格式化时间
//** 输　入: data
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int getFormatTime(unsigned char *data)
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	char buf[24] = {0};

	time_t timep;
	struct tm *p;
	
	time(&timep);
	p = localtime(&timep);
	
	year = (unsigned short)(1900 + p->tm_year);
	month = (unsigned char)(1 + p->tm_mon);
	day = (unsigned char)(p->tm_mday);
	week = (unsigned char)(p->tm_wday);
	hour = (unsigned char)(p->tm_hour) + 8;
	minute = (unsigned char)(p->tm_min);
	second = (unsigned char) (p->tm_sec);

	sprintf(buf, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);

	data[0] = ((buf[0] << 4) & 0xF0) | (buf[1] & 0x0F);
	data[1] = ((buf[2] << 4) & 0xF0) | (buf[3] & 0x0F);
	data[2] = ((buf[4] << 4) & 0xF0) | (buf[5] & 0x0F);
	data[3] = ((buf[6] << 4) & 0xF0) | (buf[7] & 0x0F);
	data[4] = ((buf[8] << 4) & 0xF0) | (buf[9] & 0x0F);
	data[5] = ((buf[10] << 4) & 0xF0) | (buf[11] & 0x0F);
	data[6] = ((buf[12] << 4) & 0xF0) | (buf[13] & 0x0F);
	//DEBUG_ERROR("data %02x %02x %02x %02x %02x %02x %02x\n", 
	//		data[0], data[1],data[2],data[3],data[4],data[5],data[6]);
	return 0;
}

/******************************************************************************/
/*
 *  16 Decimal Conversion
 */
//======================================================
//** 函数名称: dec2Hex
//** 功能描述: 字符串转换
//** 输　入: inStr outStr len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================   
int dec2Hex(const unsigned char *inStr, char *outStr, int len)
{
	if (NULL == inStr || NULL == outStr || len > 38)
		return -1;
 
    char *caTmp = NULL;
	unsigned char tmp = 0;
	
    caTmp = (char*)malloc(len + 1);
	CHECK_PTR(caTmp, "caTmp malloc error");

    memset(caTmp, 0, len + 1);

    for(int index = 0; index < len; index++)
    {
        tmp = inStr[index];
        sprintf(caTmp + index, "%x", tmp);
    }
    strncpy(outStr, caTmp, 12);
	wrt_free_memory(caTmp);
	
	return 0;
}



