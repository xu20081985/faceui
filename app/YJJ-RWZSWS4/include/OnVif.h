#pragma once

enum
{
	ONVIF_ERR_NONE,
	ONVIF_ERR_CONNECT,				// 连接失败
	ONVIF_ERR_AUTHENTICATE,			// 认证失败
	ONVIF_ERR_PARSE					// 解析失败
};

typedef void (*IPCFrameFunc)(BYTE* pFrameBuf, DWORD dwFrameLen, BOOL bIFrame);
typedef void (*IPCOffLineFunc)();

int		OpenUrl(int ip, char* user, char* passwd, IPCFrameFunc afterGettingFrame, IPCOffLineFunc afterOffLine);
void	CloseUrl();