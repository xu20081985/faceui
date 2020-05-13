#pragma once
typedef BOOL (*Audiofun) (DWORD userData, char * pdata, int dlen);

// ������Ƶ���Ŷ���
//   ����   ��Ƶ���Ŷ���ָ��
//   ����
HANDLE AudioPlayCreate(void);

// ������Ƶ���Ŷ���
//   ����   
//   ����   hAudio     -- ��Ƶ���Ŷ���ָ��
void AudioPlayDestroy(HANDLE hAudio);

// ��ʼ����
//   ����   
//   ����   hAudio		-- ��Ƶ���Ŷ���ָ��
//          simplerate	-- ������
//          nchannel	-- ͨ����
void AudioPlayStart(HANDLE hAudio, int simplerate, int nchannel);

// ֹͣ����
//   ����   
//   ����   hAudio		-- ��Ƶ���Ŷ���ָ��
void AudioPlayStoped(HANDLE hAudio);

// ��������
//   ����   
//   ����   hAudio	-- ��Ƶ���Ŷ���ָ��
//          Volume	-- ��Χ0 - 0xFFFFFFFF
void AudioPlaySetVolume(HANDLE hAudio, DWORD Volume);

// �����Ƶ���ݲ���
//   ����   
//   ����   hAudio	-- ��Ƶ���Ŷ���ָ��
//          lpdata	-- ����
//          dlen	-- ���ݳ���
void AudioPlayAddPlay(HANDLE hAudio, char * lpdata, int dlen);


// ����¼������
//   ����   ¼������ָ��
//   ����
HANDLE AudioRecCreate(void);

// ��ʼ¼��
//   ����   
//   ����   hAudio			-- ¼������ָ��
//          simplerate		-- ������
//          nchannel		-- ͨ����
//          dwInCallback	-- �ص�����
//          userData		-- �ص���������
void AudioRecStart(HANDLE hAudio, int simplerate, int nchannel, Audiofun dwInCallback, DWORD userData);

// ֹͣ¼��
//   ����   
//   ����   hAudio			-- ¼������ָ��
void AudioRecStop(HANDLE hAudio);

// ����¼������
//   ����   
//   ����   hAudio			-- ¼������ָ��
void AudioRecDestroy(HANDLE hAudio);

// ��ʼ¼����ͬ��ģʽ��
//   ע��   ���ô˺����󣬻������ڴ˺����У�ֱ������ AudioRecStopEx �Ż������˺���
//   ����   
//   ����   simplerate	-- ������
//          nchannel		-- ͨ����
//          dwInCallback	-- �ص�����
BOOL AudioRecStartEx(int simplerate, int nchannel, Audiofun dwInCallback);

// ֹͣ¼����ͬ��ģʽ��
//   ����   
BOOL AudioRecStopEx();

void SetSystemVolume(DWORD vol);
void GetSystemVolume(DWORD* vol);

// �������� /FlashDev/AECPARA.INF
void DPSetAecMode(BOOL onoff);
