#include "roomlib.h"
#include "AudioCore.h"
#include "MP3dec.h"

#define AUDIO_SYNC_MODE   false  
//true  ͬ��ģʽ��ֱ�ӵ��ö�д���������ܻ�������������������ʱ�����ƣ�
//false �첽ģʽ��ʹ�ûص���������ע���̼߳�ͬ����

#define	BLOCKTIME         40     //ÿ���������ݿ��ʱ�������룩
#define	PLAYBLOCKS        4      //������󻺳����ݿ���
#define	RECORDBLOCKS      4      //¼����󻺳����ݿ���

class CDPAudio
{
public:
	CDPAudio()
	{
		pwavOutBuf = NULL;
		pwavInBuf = NULL;
		dwPlyBuflen = 0;
		dwPlyBufPos = 0;
		hWaveOut = 0;
		hWaveIn  = 0;
		m_fdwRecCallback = NULL;
	}
	~CDPAudio()
	{
		if(hWaveIn)
			RecStop();
		if(hWaveOut)
			PlyStop();
		if(pwavOutBuf){
			free(pwavOutBuf);
			pwavOutBuf =NULL;
		}
	}

	static int waveInProc(char *DataBuffer, int DataSize, void *pContext)
	{
		CDPAudio *mAudio =(CDPAudio *)pContext;
		if(mAudio!=NULL){
			if(mAudio->m_fdwRecCallback !=NULL){
				mAudio->dwRecMuteCnt -= DataSize;
				if (mAudio->dwRecMuteCnt < 0)
					mAudio->dwRecMuteCnt = 0;
				else
					memset(DataBuffer, 0, DataSize);
				mAudio->m_fdwRecCallback(mAudio->m_uRData,DataBuffer,DataSize);
			}
		}
		return DataSize;
	}

	static int waveOutProc(char *DataBuffer, int DataSize, void *pContext)
	{
		CDPAudio *mAudio = (CDPAudio *) pContext;
		if (mAudio!=NULL && mAudio->hWaveOut && mAudio->pwavOutBuf!=NULL && mAudio->dwPlyBufPos>=DataSize)
		{
			memcpy(DataBuffer, mAudio->pwavOutBuf+0, DataSize);
			mAudio->dwPlyBufPos -= DataSize;
			memcpy(mAudio->pwavOutBuf+0, mAudio->pwavOutBuf+DataSize, mAudio->dwPlyBufPos);
		}
		else
		{
			//DBGMSG(DPINFO, "waveOutProc Underflow\n");
			memset(DataBuffer, 0x00, DataSize);
		}
		return DataSize;
	}

	BOOL	RecStar(DWORD channal, DWORD samprate,Audiofun dwCallback,DWORD userData)
	{
		//DBGMSG(DPINFO, "######## RecStar channal:%d, samprate:%d !!!\n", channal, samprate);
		hWaveIn = 0;

		int  StreamID = 0;
		int  Channels = channal;
		int  SampleBits = 16;
		int  SampleRate = samprate;
		int  Blocks = RECORDBLOCKS;
		int  SamplesPerBlock = SampleRate*BLOCKTIME/1000;

		StreamID = Audio_Recorder_Open();
		if (StreamID == 0)
			return FALSE;

		dwRecMuteCnt = SampleRate * Channels * (SampleBits/8) * 3/2;

		m_fdwRecCallback = dwCallback;
		m_uRData = userData;

		Audio_Recorder_SetDevice(StreamID, "default");
		Audio_Recorder_SetFormat(StreamID, Channels, SampleBits, SampleRate);
		Audio_Recorder_SetCache (StreamID, Blocks, SamplesPerBlock);
		Audio_Recorder_SetMode	(StreamID, AUDIO_SYNC_MODE, waveInProc, this);

		if (Audio_Recorder_Start(StreamID) == false)
		{
			Audio_Recorder_Close(StreamID);
			StreamID = 0;
			DBGMSG(DPINFO, "Audio_Recorder_Start FAILED !!!\n");
			return FALSE;
		}

		//DBGMSG(DPINFO, "######### RecStar OK !!!\n");
		hWaveIn = StreamID;
		return TRUE;
	}

	BOOL	RecStop()
	{
		if (hWaveIn)
		{
			int StreamID = hWaveIn;
			Audio_Recorder_Stop(StreamID);
			Audio_Recorder_Close(StreamID);
			hWaveIn = 0;
			return TRUE;
		}
		return FALSE;
	}

	BOOL PlyStar(DWORD channal, DWORD samprate)
	{
		//DBGMSG(DPINFO, "######## PlyStar channal:%d, samprate:%d !!!\n", channal, samprate);
		hWaveOut = 0;

		int  Volume = 100;
		int  StreamID = 0;
		int  Channels = channal;
		int  SampleBits = 16;
		int  SampleRate = samprate;
		int  Blocks = PLAYBLOCKS;
		int  SamplesPerBlock = SampleRate*BLOCKTIME/1000;

		StreamID = Audio_Player_Open();
		if (StreamID == 0)
			return FALSE;

		dwPlyBufPos = 0;
		dwPlyBuflen = Blocks * 2 * (SamplesPerBlock * Channels * (SampleBits / 8));
		pwavOutBuf = (char*) malloc(dwPlyBuflen);

		Audio_Player_SetDevice(StreamID, "default");
		Audio_Player_SetFormat(StreamID, Channels, SampleBits, SampleRate);
		Audio_Player_SetCache (StreamID, Blocks, SamplesPerBlock);
		Audio_Player_SetMode  (StreamID, AUDIO_SYNC_MODE, waveOutProc, this);
		Audio_Player_SetVolume(StreamID, Volume);

		if (Audio_Player_Start(StreamID) == false)
		{
			Audio_Player_Close(StreamID);
			StreamID = 0;
			DBGMSG(DPINFO, "Audio_Player_Start FAILED !!!\n");
			return FALSE;
		}

		//DBGMSG(DPINFO, "######### PlyStar OK !!!\n");
		hWaveOut = StreamID;
		return TRUE;
	}

	BOOL	PlyStop()
	{
		if (hWaveOut)
		{
			int StreamID = hWaveOut;
			Audio_Player_Stop(StreamID);
			Audio_Player_Close(StreamID);
			hWaveOut = 0;
			if(pwavOutBuf){
				free(pwavOutBuf);
				pwavOutBuf =NULL;
			}
			return TRUE;
		}
		return FALSE;
	}
	BOOL	AddPlay(char * lpdata,int dlen)
	{
		//ͬ��ģʽ��ֱ�ӵ���д������ÿ��д��һ������
		if (pwavOutBuf!=NULL && lpdata!=NULL && dlen>0)
		{
			DBGMSG(DEV_MOD, "CDPAudio::AddPlay %d\r\n", dlen);
			if (dlen > (dwPlyBuflen-dwPlyBufPos))
				dlen =	dwPlyBuflen-dwPlyBufPos;
			memcpy(pwavOutBuf+dwPlyBufPos, lpdata, dlen);
			dwPlyBufPos += dlen;
			return TRUE;
		}
		return FALSE;
	}
	void	SetVol(DWORD dwVol)
	{
		if (hWaveOut)
		{
			int percent = (dwVol>>16) * 100 / 0x0ffff;
			DBGMSG(DPINFO, "CDPAudio::SetVol 0x%08X -> %d%%\r\n", dwVol, percent);
			int StreamID = hWaveOut;
			Audio_Player_SetVolume(StreamID, percent);
		}
	}
	int		dwPlyBuflen;
	int		dwRecBuflen;

	//protected:
	Audiofun m_fdwRecCallback;		//¼���ص�����
	//��Ƶ
	char * pwavInBuf;	//¼�������� 
	char * pwavOutBuf;	//����������
	DWORD m_uRData;

	int hWaveIn;
	int hWaveOut;

	int	dwPlyBufPos;
	int dwRecMuteCnt;
};

HANDLE AudioPlayCreate(void)
{
	return new CDPAudio();
}

void AudioPlayDestroy(HANDLE hAudio)
{
	CDPAudio* pPlay = (CDPAudio*)hAudio;
	delete pPlay;
}

void AudioPlayStart(HANDLE hAudio, int simplerate, int nchannel)
{
	CDPAudio* pPlay = (CDPAudio*)hAudio;
	pPlay->PlyStar(nchannel, simplerate);
}

void AudioPlayStoped(HANDLE hAudio)
{
	CDPAudio* pPlay = (CDPAudio*)hAudio;
	pPlay->PlyStop();
}

void AudioPlaySetVolume(HANDLE hAudio, DWORD Volume)
{
	CDPAudio* pPlay = (CDPAudio*)hAudio;
	pPlay->SetVol(Volume);
}

void AudioPlayAddPlay(HANDLE hAudio, char * lpdata, int dlen)
{
	CDPAudio* pPlay = (CDPAudio*)hAudio;
	pPlay->AddPlay(lpdata, dlen);
}

HANDLE AudioRecCreate(void)
{
	return new CDPAudio();
}

void AudioRecStart(HANDLE hAudio, int simplerate, int nchannel, Audiofun dwInCallback, DWORD userData)
{
	CDPAudio* pRec = (CDPAudio*)hAudio;
	pRec->RecStar(nchannel, simplerate, dwInCallback, userData);
}

void AudioRecStop(HANDLE hAudio)
{
	CDPAudio* pRec = (CDPAudio*)hAudio;
	pRec->RecStop();
}

void AudioRecDestroy(HANDLE hAudio)
{
	CDPAudio* pRec = (CDPAudio*)hAudio;
	delete pRec;
}

static DWORD dwStaticVol;
void SetSystemVolume(DWORD vol)
{
	dwStaticVol = vol;		// ��ϵͳ�������õ�������
	AudioCore_SetPlayVolume((vol & 0xffff) * 100/0xffff);
}

void GetSystemVolume(DWORD* vol)
{
	*vol = dwStaticVol;
}

void DPSetAecMode(BOOL onoff)
{
	if(onoff == TRUE)
		AudioCore_SetAecMode(true);
	else
		AudioCore_SetAecMode(false);
}

/*****************************************************************************/
static BOOL g_bRecStart;
BOOL AudioRecStartEx(int simplerate, int nchannel, Audiofun dwInCallback)
{
	printf("AudioRecStartEx Start\r\n");
	int StreamID = Audio_Recorder_Open();
	if(StreamID == 0)
	{
		printf("AudioRecStartEx Open fail\r\n");
		return FALSE;
	}

	int Channels = nchannel;
	int SampleBits = 16;
	int SampleRate = simplerate;
	int Blocks = RECORDBLOCKS;
	int SamplesPerBlock = SampleRate*BLOCKTIME/1000;

	Audio_Recorder_SetDevice(StreamID, "default");
	Audio_Recorder_SetFormat(StreamID, Channels, SampleBits, SampleRate);
	Audio_Recorder_SetCache (StreamID, Blocks, SamplesPerBlock);
	Audio_Recorder_SetMode  (StreamID, true, NULL, NULL);

	if(Audio_Recorder_Start(StreamID) == false)
	{
		printf("AudioRecStartEx Start fail\r\n");
		Audio_Recorder_Close(StreamID);
		return FALSE;
	}

	int nSize = SamplesPerBlock * Channels * (SampleBits / 8);
	char* pbuf = (char*)malloc(nSize * RECORDBLOCKS); 

	g_bRecStart = TRUE;
	while(g_bRecStart)
	{
		if(false == Audio_Recorder_Read(StreamID, pbuf, nSize, BLOCKTIME*RECORDBLOCKS))
		{
			printf("AudioRecStartEx Read fail\r\n");
		}
		else
		{
			dwInCallback(0, pbuf, nSize);
		}
	}

	Audio_Recorder_Stop(StreamID);
	Audio_Recorder_Close(StreamID);
	free(pbuf);
	printf("AudioRecStartEx End\r\n");
	return TRUE;
}

BOOL AudioRecStopEx()
{
	g_bRecStart = FALSE;
}