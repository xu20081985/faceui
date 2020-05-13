#include <windows.h>
#include <dpgpio.h>
#include "dpvideo.h"

HANDLE g_hVideoDec = INVALID_HANDLE_VALUE;

BOOL JPGDecStart(int left, int top, int width, int height)
{
	// jpeg图片不能大于 800 * 479
	Vdec_Info decinfo;
	if(g_hVideoDec == INVALID_HANDLE_VALUE)
	{
		memset(&decinfo, 0, sizeof(Vdec_Info));

		decinfo.decode.m_winl = left;
		decinfo.decode.m_wint = top;
		decinfo.decode.m_winw = width;
		decinfo.decode.m_winh = height;
		decinfo.decode.m_dectype = DECODE_JPEG;
		decinfo.decode.m_outtype = DECODE_OUTDIR;

		g_hVideoDec = CreateFile(_T("DEC1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if(g_hVideoDec == INVALID_HANDLE_VALUE)
		{
			printf("CreateDecFile error\n");
			return FALSE;
		}

		if(!DeviceIoControl(g_hVideoDec,IOCTL_DECODE_START, (LPVOID)&decinfo, sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
		{
			printf("IOCTL_DECODE_START error\n");
			CloseHandle(g_hVideoDec);
			g_hVideoDec = INVALID_HANDLE_VALUE;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL H264DecStart(int left, int top, int width, int height)
{
	Vdec_Info decinfo;
	if(g_hVideoDec == INVALID_HANDLE_VALUE)
	{
		memset(&decinfo, 0, sizeof(Vdec_Info));

		decinfo.decode.m_winl = left;
		decinfo.decode.m_wint = top;
		decinfo.decode.m_winw = width;
		decinfo.decode.m_winh = height;
		decinfo.decode.m_dectype = DECODE_H264;
		decinfo.decode.m_outtype = DECODE_OUTDIR;

		g_hVideoDec = CreateFile(_T("DEC1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if(g_hVideoDec == INVALID_HANDLE_VALUE)
		{
			printf("CreateDecFile error\n");
			return FALSE;
		}

		if(!DeviceIoControl(g_hVideoDec,IOCTL_DECODE_START, (LPVOID)&decinfo, sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
		{
			printf("IOCTL_DECODE_START error\n");
			CloseHandle(g_hVideoDec);
			g_hVideoDec = INVALID_HANDLE_VALUE;
			return FALSE;
		}
	}

	return TRUE;
}

void H264WriteData(char * pdata,DWORD dlen, DWORD ptr)
{
	Vdec_Info video_info;
	if(g_hVideoDec != INVALID_HANDLE_VALUE)
	{
		memset(&video_info, 0, sizeof(Vdec_Info));
		video_info.wbuf.dwsize = dlen;
		video_info.wbuf.buf = (BYTE*)pdata;
		video_info.wbuf.dec_pts = ptr;
		DeviceIoControl(g_hVideoDec,IOCTL_WRITE_BUF,(LPVOID)&video_info,sizeof(Vdec_Info),NULL,NULL,NULL,NULL);
	}
}

typedef struct
{
	union
	{
		Preview_Info	preview;
		Decode_Info		decode;
		Encode_Info		encode;
		READ_INFO		rbuf;
		WRITE_INFO		wbuf;
		Sensor_Prop		spop;
		Sensor_Set		sset;
	} params;
} Video_Info;

void H264SetDisplayRect(int left, int top, int width, int height)
{
	Video_Info video_info;
	if(g_hVideoDec != INVALID_HANDLE_VALUE)
	{
		memset(&video_info, 0, sizeof(Video_Info));
		video_info.params.decode.m_winl = left;
		video_info.params.decode.m_wint = top;
		video_info.params.decode.m_winw = width;
		video_info.params.decode.m_winh = height;
		DeviceIoControl(g_hVideoDec,IOCTL_DECODE_SET,(LPVOID)&video_info,sizeof(Vdec_Info),NULL,NULL,NULL,NULL);
	}
}

void H264DecStop(void)
{
	if(g_hVideoDec != INVALID_HANDLE_VALUE)
	{
		DeviceIoControl(g_hVideoDec, IOCTL_DECODE_STOP, NULL, 0, NULL, NULL, NULL, NULL);
		CloseHandle(g_hVideoDec);
		g_hVideoDec = INVALID_HANDLE_VALUE;
	}
}

BOOL CheckH264DecStop()
{
	return (g_hVideoDec == INVALID_HANDLE_VALUE);
}

HANDLE VideoEncStart(DWORD format, DWORD width, DWORD height, DWORD qu)
{
	HANDLE hHandle;
	Encode_Info encinfo;
	hHandle = CreateFile(_T("DEC1:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hHandle == INVALID_HANDLE_VALUE)
	{
		printf("CreateEncFile error\n");
		return hHandle;
	}
	memset(&encinfo, 0, sizeof(Encode_Info));

	encinfo.m_enctype = format;//ENCODE_JPEG;//ENCODE_H264;
	encinfo.m_encw = width;
	encinfo.m_ench = height;

	if(format == ENCODE_H264)
	{
		encinfo.m_quality = qu;
		encinfo.property = ENCODE_QUALITY|ENCODE_REALTIME;
	}
	encinfo.m_maxkey = 25;
	if(!DeviceIoControl(hHandle, IOCTL_ENCODE_START, (LPVOID)&encinfo, sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
	{
		printf("IOCTL_ENCODE_START fail\n");
		CloseHandle(hHandle);
		return INVALID_HANDLE_VALUE;
	}
	return hHandle;
}

void VideoEncSetQuality(HANDLE hHandle, DWORD qu)
{
	Encode_Info encinfo;
	encinfo.m_quality = qu;
	encinfo.property = ENCODE_QUALITY;
	if(!DeviceIoControl(hHandle, IOCTL_ENCODE_SET, (LPVOID)&encinfo, sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
	{
		printf("IOCTL_ENCODE_SET Quality fail\n");
	}
}

void VideoEncEnable(HANDLE hHandle, BOOL bOn)
{
	Encode_Info encinfo = {0};
	if(bOn)
		encinfo.property = ENCODE_BLANK_OFF;
	else
		encinfo.property = ENCODE_BLANK_ON;
	encinfo.m_bcolor = 0x808080;
	if(!DeviceIoControl(hHandle, IOCTL_ENCODE_SET, (LPVOID)&encinfo, sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
	{
		printf("IOCTL_ENCODE_SET Enable fail\n");
	}
}

DWORD VideoEncRead(HANDLE hHandle, BYTE* pData, DWORD len, DWORD* property)
{
	Vdec_Info vinfo;

	memset(&vinfo, 0, sizeof(Vdec_Info));
	vinfo.rbuf.dwsize = len;
	vinfo.rbuf.buf = pData;
	vinfo.rbuf.timeout = 200;
	vinfo.rbuf.property = *property;
	if(!DeviceIoControl(hHandle,IOCTL_READ_BUF,(LPVOID)&vinfo,sizeof(Vdec_Info),NULL,NULL,NULL,NULL))
	{
		printf("enc error %u\r\n", vinfo.rbuf.property);
		return 0;
	}
	//	printf("enc size %d %d\r\n",vinfo.rbuf.dwsize, vinfo.rbuf.property); 
	*property = vinfo.rbuf.property;
	return vinfo.rbuf.dwsize;
}

void VideoEncStop(HANDLE hHandle)
{
	if(hHandle != INVALID_HANDLE_VALUE)
	{
		DeviceIoControl(hHandle, IOCTL_ENCODE_STOP, NULL, 0, NULL, NULL, NULL, NULL);
		CloseHandle(hHandle);
	}
}

// 后面再添加
HANDLE VideoReEncStart(DWORD dec_format, int decw, int dech, DWORD enc_format, int encw, int ench)
{
	return INVALID_HANDLE_VALUE;
}
void VideoReEncStop(HANDLE hHandle)
{
}
void VideoReEncWrite(HANDLE hVideoEnc, BYTE* pdata, DWORD len)
{
}
DWORD VideoReEncRead(HANDLE hVideoEnc, BYTE* pdata, DWORD len, DWORD* property)
{
	return 0;
}