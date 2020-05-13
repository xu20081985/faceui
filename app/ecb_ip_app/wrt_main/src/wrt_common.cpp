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
//** ��������: init_table
//** ��������: ��ʼ��crc32��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: crc32
//** ��������: crc32�㷨
//** �䡡��: buf len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: wrt_free_memory
//** ��������: �ڴ��ͷ�
//** �䡡��: data_ptr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: pthread_create_t
//** ��������: �̴߳���
//** �䡡��: func
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: getDataNaBytes
//** ��������: ����֡��ȡ����
//** �䡡��: str pos num
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: uartCheck
//** ��������: ����У��
//** �䡡��: str
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: get_version_file
//** ��������: ��ȡ�����ļ�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: get_gateway_version
//** ��������: ��ȡ���ذ汾
//** �䡡��: str mcuVer
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: getFormatTime
//** ��������: ��ȡ��ʽ��ʱ��
//** �䡡��: data
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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
//** ��������: dec2Hex
//** ��������: �ַ���ת��
//** �䡡��: inStr outStr len
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��02��14��
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



