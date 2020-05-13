/*
 * wrt_cfg.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_cfg.cpp 	2017/06/20   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *  This file provides configuration information for the gateway to run
 */
 
/********************************* Includes ***********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wrt_cfg.h"
#include "wrt_common.h"

/********************************* Defines ************************************/

T_SYSTEMINFO  g_GateWayInfo;
T_SYSTEMINFO* pSystemInfo;

/*********************************** Code *************************************/
/*
 *  set some default cfg info for gateway
 */
//======================================================
//** 函数名称: set_default_cfg
//** 功能描述: 设置默认配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
void set_default_cfg()
{
	T_SYSTEMINFO * pGateWayInfo = &g_GateWayInfo;
   
    memset(&pGateWayInfo->DoorSysInfo, 0, sizeof(T_MYSYSINFO_GW));
	memset(&pGateWayInfo->LocalSetting, 0, sizeof(T_PRIVATE_GW));
    memset(&pGateWayInfo->LocalSetting, 0, sizeof(T_PRIVATE_GW));

	snprintf(pGateWayInfo->DoorSysInfo.variableKey, sizeof(pGateWayInfo->DoorSysInfo.variableKey),"%08d%08d%08d%08d%08d%08d%08d%07d",
			rand(),rand(),rand(),rand(),rand(),rand(),rand(),rand());

	snprintf(pGateWayInfo->DoorSysInfo.gateWayDeviceID, sizeof(pGateWayInfo->DoorSysInfo.gateWayDeviceID), "%02d%s", GW_ID_TYPE, "ABCDEFGH");

    pGateWayInfo->LocalSetting.LocalIP   = -1;
 	pGateWayInfo->LocalSetting.SubMaskIP = -1;
	pGateWayInfo->LocalSetting.GateWayIP = -1;
}

/******************************************************************************/
/*
 *  get system info pointer
 */
//======================================================
//** 函数名称: get_system_info
//** 功能描述: 获取系统配置信息指针
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
T_SYSTEMINFO* get_system_info()
{
    return &g_GateWayInfo;
}

/******************************************************************************/
/*
 *   write system info for file
 */
//======================================================
//** 函数名称: write_system_info
//** 功能描述: 写系统配置信息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//======================================================  
int write_system_info()
{
	FILE *fd = NULL;

	if ((fd = fopen(WRT_CFG_FILE, "wb")) == NULL)
	{
		DEBUG_ERROR("fopen write cfg failed\n");
		return -1;
	}

	char* p = (char *)&g_GateWayInfo;
	int totallen = sizeof(g_GateWayInfo);
	int writelen = 0;
	int write_rc = 0;
	
	while (true)
	{
		write_rc = fwrite(p + writelen, 1, totallen - writelen, fd);
		if (write_rc < 0)
		{
			DEBUG_ERROR("fwrite cfg error\n");
			fflush(fd);
			fclose(fd);
			fd = NULL;
			return -1;
		}
		writelen += write_rc;
		if (writelen == totallen)
			break;
	}	
	fflush(fd);
	fclose(fd);
	fd = NULL;
	
	return 0;
}

/******************************************************************************/
/*
 *   read system info for file
 */
//======================================================
//** 函数名称: read_system_info
//** 功能描述: 读系统配置信息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int read_system_info()
{
	FILE *fd = NULL;

	if ((fd = fopen(WRT_CFG_FILE, "rb")) == NULL)
	{
		DEBUG_ERROR("fopen read cfg failed\n");
		return -1;
	}

	char* p = (char*)&g_GateWayInfo;
	int totallen = sizeof(g_GateWayInfo);
	int readlen = 0;
	int read_rc = 0;
	
	while (true)
	{
		read_rc = fread(p + readlen, 1, totallen - readlen, fd);
		if (read_rc < 0)
		{
			DEBUG_ERROR("fread cfg error\n");
			fflush(fd);
			fclose(fd);
			fd = NULL;
			return -1;
		}
		readlen += read_rc;
		if (readlen == totallen)
			break;
	}
	fflush(fd);
	fclose(fd);
	fd = NULL;
	
	return 0;
}

/******************************************************************************/
/*
 *   gateway use file config for init system
 */
//======================================================
//** 函数名称: init_system_info
//** 功能描述: 初始化系统配置信息
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年02月14日
//====================================================== 
int init_system_info()
{
	FILE *fd = NULL;
	if ((fd = fopen(WRT_CFG_FILE, "rb")) == NULL)
	{
		set_default_cfg();
		if (write_system_info())
		{
			DEBUG_ERROR("write system info error\n");
			return -1;
		}	
	}

	if (read_system_info())
	{
		DEBUG_ERROR("read system info error\n");
		return -1;
	}
	pSystemInfo = get_system_info();

	if (fd != NULL)
	{
		fflush(fd);
		fclose(fd);
		fd = NULL;
	}

	return 0;
}




