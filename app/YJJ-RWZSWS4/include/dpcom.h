#pragma once

HANDLE OpenComm(const char* szComName, DWORD dwBAUD_RATE, BYTE bufDataByte, DWORD timeOut);
void CloseComm(HANDLE hCom);
int ReadComm(HANDLE hCom, char* buf, int len);
int SendComm(HANDLE hCom, char* buf, int len);