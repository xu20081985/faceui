#include "windows.h"
#include "Mmsystem.h"
#include "DPAudio.h"
#define		MAX_NUM_SOUNDINBUF		4
#define		MAX_NUM_SOUNDOUTBUF		8
#define		AUDIO_CACHE_SIZE		0x2000

class CDPSndPlay
{
public:
	CDPSndPlay()
	{
		pwavOutBuf = NULL;
		audiocache = audiobuf;
		isPlay = FALSE;
		m_index = 0;
		InitializeCriticalSection(&cs);
	}

	~CDPSndPlay()
	{
		DeleteCriticalSection(&cs);
	}

	static void waveOutProcStub(HWAVEOUT hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
	{
		CDPSndPlay * pAudio = (CDPSndPlay*)dwInstance;
		pAudio->waveOutProc(hwi, uMsg, (WAVEHDR*)dwParam1);
	}

	void waveOutProc(HWAVEOUT hwi, UINT uMsg, WAVEHDR* inHdr)
	{
		DWORD cplen;
		char* pbuf;

		return;

		//if(!isPlay)
		//	return;
		//if(uMsg == WOM_DONE)
		//{
		//	EnterCriticalSection(&cs);
		//	pbuf = inHdr->lpData;
		//	// 如果当前缓存的数据比要求的要少,将不足的部分填0
		//	if(cachelen < inHdr->dwBufferLength)
		//	{
		//		cplen = cachelen;
		//		memset(pbuf + cplen, 0, inHdr->dwBufferLength - cplen);
		//	}
		//	else
		//		cplen = inHdr->dwBufferLength;
		//	cachelen -= cplen;
		//	readptr %= AUDIO_CACHE_SIZE;
		//	if((readptr + cplen) > AUDIO_CACHE_SIZE)
		//	{
		//		memcpy(pbuf, audiocache + readptr, AUDIO_CACHE_SIZE - readptr);
		//		cplen -= AUDIO_CACHE_SIZE - readptr;
		//		pbuf += AUDIO_CACHE_SIZE - readptr;
		//		readptr = 0;
		//	}
		//	memcpy(pbuf, audiocache + readptr, cplen);
		//	readptr += cplen;
		//	LeaveCriticalSection(&cs);
		//	waveOutWrite(hwi, inHdr, sizeof (WAVEHDR));
		//}
	}

	BOOL Start(int simplerate,int nchannel)
	{
		int i;
		WAVEFORMATEX waveform;
		int nSampleLength;

		//放音
		waveform.wFormatTag = WAVE_FORMAT_PCM;
		waveform.nChannels = nchannel;
		waveform.wBitsPerSample = 16;	// 缺省为16 bit
		waveform.nBlockAlign = waveform.wBitsPerSample*waveform.nChannels/8;
		waveform.nSamplesPerSec = simplerate;
		waveform.nAvgBytesPerSec = waveform.nSamplesPerSec*waveform.nBlockAlign;
		waveform.cbSize = 0;
		if(MMSYSERR_NOERROR != waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)waveOutProcStub, (DWORD)this, CALLBACK_FUNCTION))
			return FALSE;

		nSampleLength = 640;//waveform.nAvgBytesPerSec/25;	// 40毫秒

		if(pwavOutBuf)
		{
			delete [] pwavOutBuf;
			pwavOutBuf = NULL;
		}

		pwavOutBuf = new BYTE[MAX_NUM_SOUNDOUTBUF * nSampleLength];
		memset(pwavOutBuf,0,MAX_NUM_SOUNDOUTBUF * nSampleLength);

		for(i = 0; i < MAX_NUM_SOUNDOUTBUF; i++)
		{
			memset(&pWaveOutHdr[i], 0, sizeof(WAVEHDR));
			pWaveOutHdr[i].lpData			=  (char *)(pwavOutBuf + i*nSampleLength);
			pWaveOutHdr[i].dwBufferLength	=  nSampleLength;
			pWaveOutHdr[i].dwBytesRecorded	= 0;
			pWaveOutHdr[i].dwUser			= 0;
			pWaveOutHdr[i].dwFlags			= 0;
			pWaveOutHdr[i].dwLoops			= 1;
			pWaveOutHdr[i].lpNext			= NULL;
			pWaveOutHdr[i].reserved 		= 0;
			waveOutPrepareHeader(hWaveOut, &(pWaveOutHdr[i]), sizeof(WAVEHDR));
		}
		isPlay = TRUE;
		for(i = 0; i < MAX_NUM_SOUNDOUTBUF; i++)
		{
			memset(pWaveOutHdr[i].lpData, 0, nSampleLength);
			pWaveOutHdr[i].dwBufferLength = nSampleLength;
			pWaveOutHdr[i].dwUser = 0;
		}

		return TRUE;
	}

	BOOL Stop()
	{
		int i;

		if(isPlay)
		{
			cachelen = 0;
			waveOutReset(hWaveOut);
			for(i = 0; i < MAX_NUM_SOUNDOUTBUF; i++)
				waveOutUnprepareHeader(hWaveOut,&pWaveOutHdr[i],sizeof(WAVEHDR));

			waveOutClose(hWaveOut);
			hWaveOut = NULL;
			isPlay = FALSE;
		}
		return TRUE;
	}

	void SetVol(DWORD Volume)
	{
		if(hWaveOut == NULL)
			return;
		printf("settalkvol %x\r\n", Volume);
		waveOutSetVolume(hWaveOut,Volume);
	}

	BOOL AddPlay(char * lpdata, int dlen)
	{
		int cplen;
		if(!isPlay)
			return FALSE;
		while(dlen != 0)
		{
			if(dlen >  640)
				cplen = 640;
			else
				cplen = dlen;
			memcpy(pWaveOutHdr[m_index].lpData, lpdata, cplen);
			pWaveOutHdr[m_index].dwBufferLength =  cplen;
			waveOutWrite(hWaveOut, &(pWaveOutHdr[m_index]), sizeof (WAVEHDR));
			m_index++;
			m_index %= MAX_NUM_SOUNDOUTBUF;
			lpdata += cplen;
			dlen -= cplen;
		}
		return TRUE;

		//EnterCriticalSection(&cs);

		////	如果当前写入的数据大小超过了缓存，覆盖掉前面的，并把readptr向后移动
		//if((cachelen + dlen) > AUDIO_CACHE_SIZE)
		//{
		//	readptr += (cachelen + dlen) - AUDIO_CACHE_SIZE;
		//	readptr %= AUDIO_CACHE_SIZE;
		//	cachelen = AUDIO_CACHE_SIZE - dlen;
		//}
		//cachelen += dlen;
		//writeptr %= AUDIO_CACHE_SIZE;
		//if((writeptr + dlen) > AUDIO_CACHE_SIZE)
		//{
		//	memcpy(audiocache + writeptr, lpdata, AUDIO_CACHE_SIZE - writeptr);
		//	lpdata += AUDIO_CACHE_SIZE - writeptr;
		//	dlen -= AUDIO_CACHE_SIZE - writeptr;
		//	writeptr = 0;
		//}
		//memcpy(audiocache + writeptr, lpdata, dlen);
		//writeptr += dlen;
		//LeaveCriticalSection(&cs);
		//return TRUE;
	}

private:

	//音频
	BYTE * pwavOutBuf;	//放音缓冲区
	WAVEHDR pWaveOutHdr[MAX_NUM_SOUNDOUTBUF];	
	HWAVEOUT hWaveOut;

	BYTE audiobuf[AUDIO_CACHE_SIZE];
	BYTE* audiocache;
	DWORD cachelen;
	DWORD readptr;
	DWORD writeptr;
	CRITICAL_SECTION cs;
	BOOL isPlay;
	DWORD m_index;
};



class CDPSndRec
{
public:
	CDPSndRec()
	{
		pwavInBuf = NULL;
		bRec = FALSE;
		InitializeCriticalSection(&cs);
	}

	~CDPSndRec()
	{
		DeleteCriticalSection(&cs);
	}

	static void CALLBACK waveInProcStub(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
	{
		CDPSndRec * pAudio = (CDPSndRec*)dwInstance;
		pAudio->waveInProc(hwi, uMsg, (WAVEHDR*)dwParam1);
	}

	void waveInProc(HWAVEIN hwi, UINT uMsg, WAVEHDR* inHdr)
	{
		EnterCriticalSection(&cs);

		if((bRec)
			&& (uMsg == WIM_DATA))
		{
			m_fdwInCb(m_uData, inHdr->lpData, inHdr->dwBytesRecorded);
			waveInAddBuffer(hWaveIn, inHdr, sizeof (WAVEHDR)) ;
		}
		LeaveCriticalSection(&cs);	
	}

	BOOL Start(int simplerate,int nchannel,Audiofun dwInCallback,DWORD userData)
	{
		int i;
		WAVEFORMATEX waveform;
		int nSampleLength;
		DWORD ret;

		m_fdwInCb = dwInCallback;
		m_uData = userData;

		//录音
		waveform.wFormatTag = WAVE_FORMAT_PCM;
		waveform.nChannels = nchannel;
		waveform.wBitsPerSample = 16;
		waveform.nBlockAlign = waveform.wBitsPerSample*waveform.nChannels/8;
		waveform.nSamplesPerSec = simplerate;
		waveform.nAvgBytesPerSec = waveform.nSamplesPerSec*waveform.nBlockAlign;
		waveform.cbSize = 0;
		if(MMSYSERR_NOERROR != (ret=waveInOpen(&hWaveIn,WAVE_MAPPER,&waveform,(DWORD)waveInProcStub,(DWORD)this,CALLBACK_FUNCTION)))
		{
			printf("voice check wave in open err %u\n",ret);
			return FALSE;
		}

		nSampleLength = 640;//16000 byte total
		pwavInBuf = new BYTE[MAX_NUM_SOUNDINBUF * nSampleLength];
		for(i = 0; i < MAX_NUM_SOUNDINBUF; i++)
		{
			memset(&pWaveInHdr[i], 0, sizeof(WAVEHDR));
			pWaveInHdr[i].lpData				=  (char *)(pwavInBuf + i*nSampleLength);
			pWaveInHdr[i].dwBufferLength		=  nSampleLength;
			pWaveInHdr[i].dwBytesRecorded		= 0;
			pWaveInHdr[i].dwUser				= 0;
			pWaveInHdr[i].dwFlags				= 0;
			pWaveInHdr[i].dwLoops				= 1;
			pWaveInHdr[i].lpNext				= NULL;
			pWaveInHdr[i].reserved				= 0;

			waveInPrepareHeader(hWaveIn,&(pWaveInHdr[i]),sizeof(WAVEHDR));
			if(MMSYSERR_NOERROR != waveInAddBuffer (hWaveIn, &(pWaveInHdr[i]), sizeof (WAVEHDR)) )
			{
				delete [] pwavInBuf;
				pwavInBuf = NULL;
				waveInClose(hWaveIn);
				hWaveIn = NULL;
				return FALSE;
			}
		}

		bRec = TRUE;
		if(MMSYSERR_NOERROR != waveInStart (hWaveIn))
		{
			bRec = FALSE;
			delete [] pwavInBuf;
			pwavInBuf = NULL;
			waveInClose(hWaveIn);
			hWaveIn = NULL;
			return FALSE;
		}

		return TRUE;
	}

	BOOL Stop()
	{
		int i;

		EnterCriticalSection(&cs);
		bRec = FALSE;
		LeaveCriticalSection(&cs);

		waveInReset(hWaveIn);

		for(i = 0; i < MAX_NUM_SOUNDINBUF; i++)
			waveInUnprepareHeader(hWaveIn,&pWaveInHdr[i],sizeof(WAVEHDR));

		waveInClose(hWaveIn);
		hWaveIn = NULL;

		delete [] pwavInBuf;
		pwavInBuf = NULL;

		return TRUE;
	}

protected:
	Audiofun m_fdwInCb;			//录音回调函数
	DWORD m_uData;
	float	m_nSimple;
	//音频
	BYTE * pwavInBuf;	//录音缓冲区 
	WAVEHDR pWaveInHdr[MAX_NUM_SOUNDINBUF];
	HWAVEIN hWaveIn;
	BOOL bRec;
	CRITICAL_SECTION cs;
};

HANDLE AudioPlayCreate(void)
{
	return new CDPSndPlay();
}

void AudioPlayDestroy(HANDLE hAudio)
{
	CDPSndPlay* pPlay = (CDPSndPlay*)hAudio;
	delete pPlay;
}

void AudioPlayStart(HANDLE hAudio, int simplerate, int nchannel)
{
	CDPSndPlay* pPlay = (CDPSndPlay*)hAudio;
	pPlay->Start(simplerate, nchannel);
}

void AudioPlayStoped(HANDLE hAudio)
{
	CDPSndPlay* pPlay = (CDPSndPlay*)hAudio;
	pPlay->Stop();
}

void AudioPlaySetVolume(HANDLE hAudio, DWORD Volume)
{
	CDPSndPlay* pPlay = (CDPSndPlay*)hAudio;
	pPlay->SetVol(Volume);
}

void AudioPlayAddPlay(HANDLE hAudio, char * lpdata, int dlen)
{
	CDPSndPlay* pPlay = (CDPSndPlay*)hAudio;
	pPlay->AddPlay(lpdata, dlen);
}

HANDLE AudioRecCreate(void)
{
	return new CDPSndRec();
}

void AudioRecStart(HANDLE hAudio, int simplerate, int nchannel, Audiofun dwInCallback, DWORD userData)
{
	CDPSndRec* pRec = (CDPSndRec*)hAudio;
	pRec->Start(simplerate, nchannel, dwInCallback, userData);
}

void AudioRecStop(HANDLE hAudio)
{
	CDPSndRec* pRec = (CDPSndRec*)hAudio;
	pRec->Stop();
}

void AudioRecDestroy(HANDLE hAudio)
{
	CDPSndRec* pRec = (CDPSndRec*)hAudio;
	delete pRec;
}

void SetSystemVolume(DWORD vol)
{
	waveOutSetVolume(0, vol);		// 将系统音量设置到静音。
}

void GetSystemVolume(DWORD* vol)
{
	waveOutGetVolume(0, vol);		// 将系统音量设置到静音。
}

static HANDLE g_hAudioRec;
static HANDLE g_hEvent;
BOOL AudioRecStartEx(int simplerate, int nchannel, Audiofun dwInCallback)
{
	if(g_hAudioRec)
		return FALSE;

	g_hAudioRec = AudioRecCreate();
	AudioRecStart(g_hAudioRec, simplerate, nchannel, dwInCallback, 0);

	g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	WaitForSingleObject(g_hEvent, INFINITE);
	CloseHandle(g_hEvent);

	AudioRecStop(g_hAudioRec);
	AudioRecDestroy(g_hAudioRec);
	g_hAudioRec = NULL;

	return TRUE;
}
BOOL AudioRecStopEx()
{
	if(g_hAudioRec)
	{
		SetEvent(g_hEvent);
	}
	return TRUE;
}

void DPSetAecMode(BOOL bOnoff)
{

}


