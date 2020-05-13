#pragma once

#include "SmartConfig.h"

// ServiceFile
BOOL InitFileServer(void);
void DeinitFileServer(void);
BOOL WriteServerFile(const char* name, int len, void* buf);
void DeleteServerFile(const char* name);

// ServiceSmart
void StartSmartServer();
void StopSmartServer();
void SendSmartCmd(DEVICE *device, BYTE cmd, WORD param);

DWORD GetDevID();
WORD GetVersion();
WORD GetDevType();
DWORD GetSoftVer();
int GetEnvTemp();
DWORD GetTimeSync();
void SetTimeSync(DWORD status);
DWORD GetCfgStatus();
void SetCfgStatus(DWORD status);
DWORD GetCfgTime();
void SetCfgTime(DWORD time);
DWORD GetLightStudy();
void SetLightStudy(DWORD status);
void SetSmartUi();

void SmartReportID();
void SmartSyncTime();
void SmartGetStatus(DEVICE *device);

// µ˜ ‘”√
void StartPCServer(void);
void StopPCServer(void);

// zigbee
void InitSmartZigbee();
void SmartZigbeeProc(const void *data, const int len);
void SendZigbeeCmd(char *data, const int len);
WORD GetZigbeeVer();

void SendSceneCmd(DEVICE *device, BYTE cmd, WORD param);
void SendStatusCmd(DEVICE *device, BYTE cmd, WORD param);

