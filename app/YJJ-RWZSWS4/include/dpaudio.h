#pragma once
typedef BOOL (*Audiofun) (DWORD userData, char * pdata, int dlen);

// 创建音频播放对象
//   返回   音频播放对象指针
//   参数
HANDLE AudioPlayCreate(void);

// 销毁音频播放对象
//   返回   
//   参数   hAudio     -- 音频播放对象指针
void AudioPlayDestroy(HANDLE hAudio);

// 开始播放
//   返回   
//   参数   hAudio		-- 音频播放对象指针
//          simplerate	-- 采样率
//          nchannel	-- 通道数
void AudioPlayStart(HANDLE hAudio, int simplerate, int nchannel);

// 停止播放
//   返回   
//   参数   hAudio		-- 音频播放对象指针
void AudioPlayStoped(HANDLE hAudio);

// 设置音量
//   返回   
//   参数   hAudio	-- 音频播放对象指针
//          Volume	-- 范围0 - 0xFFFFFFFF
void AudioPlaySetVolume(HANDLE hAudio, DWORD Volume);

// 添加音频数据播放
//   返回   
//   参数   hAudio	-- 音频播放对象指针
//          lpdata	-- 数据
//          dlen	-- 数据长度
void AudioPlayAddPlay(HANDLE hAudio, char * lpdata, int dlen);


// 创建录音对象
//   返回   录音对象指针
//   参数
HANDLE AudioRecCreate(void);

// 开始录音
//   返回   
//   参数   hAudio			-- 录音对象指针
//          simplerate		-- 采样率
//          nchannel		-- 通道数
//          dwInCallback	-- 回调函数
//          userData		-- 回调函数参数
void AudioRecStart(HANDLE hAudio, int simplerate, int nchannel, Audiofun dwInCallback, DWORD userData);

// 停止录音
//   返回   
//   参数   hAudio			-- 录音对象指针
void AudioRecStop(HANDLE hAudio);

// 销毁录音对象
//   返回   
//   参数   hAudio			-- 录音对象指针
void AudioRecDestroy(HANDLE hAudio);

// 开始录音（同步模式）
//   注意   调用此函数后，会阻塞在此函数中，直到调用 AudioRecStopEx 才会跳出此函数
//   返回   
//   参数   simplerate	-- 采样率
//          nchannel		-- 通道数
//          dwInCallback	-- 回调函数
BOOL AudioRecStartEx(int simplerate, int nchannel, Audiofun dwInCallback);

// 停止录音（同步模式）
//   返回   
BOOL AudioRecStopEx();

void SetSystemVolume(DWORD vol);
void GetSystemVolume(DWORD* vol);

// 回声消除 /FlashDev/AECPARA.INF
void DPSetAecMode(BOOL onoff);
