#pragma once

typedef struct
{
	DWORD dwRIFF;
	DWORD dwFileLen;
	DWORD dwWAVE;
	DWORD dw_fmt;
	DWORD dwFmtLen;
	WORD  wDataType;
	WORD  wNChannels;		// ������
	DWORD dwSamplingRate;	// ������
	DWORD dwNBytesPerSec;	// ÿ���ֽ���
	WORD  wAlignment;
	WORD  wNBitsPerSam;		// ����λ�� 8 16
	DWORD dwdata;
	DWORD dwDataLen;
} WaveSound;


HANDLE PlayMp3(char *filename,int times = 1);
void StopMp3(HANDLE hMp3);
void SetMp3Volume(HANDLE hMp3,DWORD dVol);

void PlayWav(DWORD wavindex, DWORD vol);

void StopAlarmWav(HANDLE hdev);
HANDLE StartAlarmWav(DWORD index);
void SetAlarmVolume(DWORD volume);
void StartLiuYan(void* pbuf, DWORD len);
void StopLiuYan(void);

#define	KEYPAD_INDEX		0
#define	DUDU_INDEX			1
#define	LONGPRESS_INDEX		2
#define	STARTUP_INDEX		3
#define CALL_BUSY  			4
#define MESSAGE_INDEX		5
#define OK_INDEX			6

#define PHOTO_MP3			1	// ����˲��
#define VOLUME_MP3			2	// ��������
#define DOOR_DELL			3	// ����

void StartPlayMp3(DWORD index, DWORD vol, DWORD times);
void StopPlayMp3(DWORD ring);
void ChangeVolume(DWORD vol);

