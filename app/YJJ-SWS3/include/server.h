#pragma once

// ServiceFile
BOOL InitFileServer(void);
void DeinitFileServer(void);
BOOL WriteServerFile(const char* name, int len, void* buf);
void DeleteServerFile(const char* name);

// ServiceSmart
void StartSmartServer();
void StopSmartServer();
void SendSmartCmd(WORD addr, BYTE cmd, WORD param);

DWORD GetVersion();
DWORD GetDevType();
DWORD GetSoftVer();

void SmartReportID();
void SmartSyncTime();
void SmartGetStatus(WORD addr);
void SmartGetStatus_Air(WORD addr);   // 2018.2.26ÃÌº”

// µ˜ ‘”√
void StartPCServer(void);
void StopPCServer(void);
