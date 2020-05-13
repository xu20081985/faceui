#pragma once

enum
{
	ONVIF_ERR_NONE,
	ONVIF_ERR_CONNECT,				// ����ʧ��
	ONVIF_ERR_AUTHENTICATE,			// ��֤ʧ��
	ONVIF_ERR_PARSE					// ����ʧ��
};

typedef void (*IPCFrameFunc)(BYTE* pFrameBuf, DWORD dwFrameLen, BOOL bIFrame);
typedef void (*IPCOffLineFunc)();

int		OpenUrl(int ip, char* user, char* passwd, IPCFrameFunc afterGettingFrame, IPCOffLineFunc afterOffLine);
void	CloseUrl();