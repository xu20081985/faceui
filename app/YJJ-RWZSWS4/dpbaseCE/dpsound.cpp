#include "roomlib.h"

#pragma comment(lib, "mp3play.lib")
HANDLE AudioPlayStar(wchar_t *filename,int times);
void AudioPlayStop(HANDLE hPlay);
void AudioSetVolume(HANDLE hPlay,DWORD dVol);

static StaticLock g_SoundCS;

class CDPWavePlay
{
public:
	CDPWavePlay()
	{
		pwavOutBuf = NULL;
		m_bisPlay = FALSE;
		InitializeCriticalSection(&cs);
	}
	~CDPWavePlay()
	{
		DeleteCriticalSection(&cs);
	}

	BOOL Start(void* data, DWORD datalen, DWORD volume, BOOL isloop)
	{
		WAVEFORMATEX waveform;

		//∑≈“Ù
		waveform.wFormatTag = WAVE_FORMAT_PCM;
		waveform.nChannels = 1;
		waveform.wBitsPerSample = 16;	// »± °Œ™16 bit
		waveform.nBlockAlign = 2;
		waveform.nSamplesPerSec = 8000;
		waveform.nAvgBytesPerSec = waveform.nSamplesPerSec*waveform.nBlockAlign;
		waveform.cbSize = 0;
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)waveOutProcStub, (DWORD)this, CALLBACK_FUNCTION);
		waveOutSetVolume(hWaveOut, volume);
		pwavOutBuf = (BYTE*)data;
		//		DBGMSG(DPINFO, "start\r\n");

		memset(&pWaveOutHdr, 0, sizeof(WAVEHDR));
		pWaveOutHdr.lpData			=  (char *)data;
		pWaveOutHdr.dwBufferLength	=  datalen;
		pWaveOutHdr.dwBytesRecorded	= 0;
		pWaveOutHdr.dwUser			= 0;
		if(isloop)
		{
			pWaveOutHdr.dwFlags 	= WHDR_BEGINLOOP|WHDR_ENDLOOP;
			pWaveOutHdr.dwLoops		= 0x7fffffff;
		}
		else
		{
			pWaveOutHdr.dwFlags 	= 0;
			pWaveOutHdr.dwLoops		= 1;
		}
		pWaveOutHdr.lpNext			= NULL;
		pWaveOutHdr.reserved		= 0;
		waveOutPrepareHeader(hWaveOut, &pWaveOutHdr, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, &pWaveOutHdr, sizeof (WAVEHDR));
		m_bisPlay = TRUE;
		return TRUE;	
	}

	void SetVolume(DWORD volume)
	{
		if(m_bisPlay)
		{
			waveOutSetVolume(hWaveOut, volume);
		}
	}

	BOOL Stop()
	{
		if(m_bisPlay)
		{
			waveOutReset(hWaveOut);
			waveOutUnprepareHeader(hWaveOut,&pWaveOutHdr,sizeof(WAVEHDR));
			waveOutClose(hWaveOut);
			hWaveOut = NULL;
			if(pwavOutBuf != NULL)
				free(pwavOutBuf);
			pwavOutBuf = NULL;
			m_bisPlay = FALSE;
		}
		return TRUE;
	}

	static void CALLBACK waveOutProcStub(HWAVEOUT hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
	{
		//		DBGMSG(DPINFO, "end\r\n");
	}

private:
	//“Ù∆µ
	BYTE * pwavOutBuf;	//∑≈“Ùª∫≥Â«¯
	WAVEHDR pWaveOutHdr;	
	HWAVEOUT hWaveOut;
	CRITICAL_SECTION cs;
	BOOL m_bisPlay;
};

static CDPWavePlay m_PlayWav;
static CDPWavePlay m_PlayAlarm;
static CDPWavePlay m_PlayLiuyan;

void PlayWav(DWORD wavindex, DWORD vol)
{
	wchar_t filename[64];
	FILE* fd;
	char* pbuf;
	WaveSound fmt;

	switch(wavindex)
	{
	case STARTUP_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\startup.wav");
		break;
	case KEYPAD_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\KEYVOL.wav");
		break;
	case OK_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\timeover10.wav");
		break;
	case DUDU_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\timeover15.wav");
		break;
	case LONGPRESS_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\timeover20.wav");
		break;
	case CALL_BUSY:
		wcscpy(filename, L"\\FlashDev\\SOUND\\BUSY.wav");
		break;
	case MESSAGE_INDEX:
		wcscpy(filename, L"\\FlashDev\\SOUND\\msgring2.wav");
		break;
	}

	fd = _wfopen(filename, L"rb");
	if(fd != NULL)
	{
		fread(&fmt, 1, sizeof(WaveSound), fd);
		pbuf = (char*)malloc(fmt.dwDataLen);
		if(NULL == pbuf)
		{
			fclose(fd);
			return;
		}
		fread(pbuf, 1, fmt.dwDataLen, fd);
		fclose(fd);
		g_SoundCS.lockon();
		m_PlayWav.Stop();
		m_PlayWav.Start(pbuf, fmt.dwDataLen, vol * 0x11111111, FALSE);
		g_SoundCS.lockoff();
	}
}

static DWORD m_AlarmVol = 0xffffffff;
HANDLE StartAlarmWav(DWORD index)
{
	FILE* fd;
	char* pbuf;
	WaveSound fmt;

	if(index == 0)
		fd = _wfopen(L"\\FlashDev\\Sound\\timeover20.wav", L"rb");
	else
		fd = _wfopen(L"\\FlashDev\\Sound\\AlarmS.wav", L"rb");
	if(fd != NULL)
	{
		fread(&fmt, 1, sizeof(WaveSound), fd);
		pbuf = (char*)malloc(fmt.dwDataLen);
		if(NULL == pbuf)
		{
			fclose(fd);
			return NULL;
		}
		fread(pbuf, 1, fmt.dwDataLen, fd);
		fclose(fd);
		m_PlayAlarm.Start(pbuf, fmt.dwDataLen, m_AlarmVol, TRUE);
	}
	return &m_PlayAlarm;
}

void StopAlarmWav(HANDLE hdev)
{
	m_PlayAlarm.Stop();
}

void SetAlarmVolume(DWORD volume)
{
	m_PlayAlarm.SetVolume(volume);
	m_AlarmVol = volume;
}

static HANDLE m_hMp3Play;		// ¡Â…˘≤•∑≈æ‰±˙
static DWORD lastring = 0;

void StartPlayMp3(DWORD index, DWORD vol, DWORD times)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		AudioPlayStop(m_hMp3Play);
		m_hMp3Play = NULL;
	}
	if(m_hMp3Play == NULL)
	{
		wchar_t mp3File[128];
		switch(index)
		{
		case PHOTO_MP3:
			wcscpy(mp3File, L"\\FlashDev\\sound\\photoshutter.mp3");
			break;
		case VOLUME_MP3:
			wcscpy(mp3File, L"\\FlashDev\\ring\\musicring.mp3");
			break;
		case DOOR_DELL:
			wcscpy(mp3File, L"\\FlashDev\\sound\\doorbell.mp3");
			break;
		default:
			mp3File[0] = 0;
			break;
		}
		m_hMp3Play = AudioPlayStar(mp3File, times);
		AudioSetVolume(m_hMp3Play, vol * 0x11111111);
	}
	g_SoundCS.lockoff();
}

void ChangeVolume(DWORD vol)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		AudioSetVolume(m_hMp3Play, vol * 0x11111111);
	}
	g_SoundCS.lockoff();
}

void StopPlayMp3(DWORD ring)
{
	g_SoundCS.lockon();
	if(m_hMp3Play != NULL)
	{
		AudioPlayStop(m_hMp3Play);
		m_hMp3Play = NULL;
	}
	g_SoundCS.lockoff();
}

void StartLiuYan(void* pbuf, DWORD len)
{
	DBGMSG(DPINFO, "StartLiuYan\r\n");
	short* pvoice = (short*)pbuf;
	for(int i = 0; i < len/2; i++)
		pvoice[i] = pvoice[i];
	m_PlayLiuyan.Start(pbuf, len, 0xffffffff, FALSE);
}

void StopLiuYan(void)
{
	DBGMSG(DPINFO, "StopLiuYan\r\n");
	m_PlayLiuyan.Stop();
}

HANDLE PlayMp3(char *filename,int times)
{
	wchar_t wname[64];
	utf82unicode((WORD*)wname, (BYTE*)filename);
	return AudioPlayStar(wname, times);
}

void StopMp3(HANDLE hMp3)
{
	if(hMp3)
	{
		AudioPlayStop(hMp3);
	}
}

void SetMp3Volume(HANDLE hMp3,DWORD dVol)
{
	if(hMp3)
	{
		AudioSetVolume(hMp3, dVol);
	}
}
