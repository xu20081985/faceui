#pragma once

// block ��4�֣�
// ��Ӳ����ص���3��
// temp block
typedef struct
{
	DWORD op;
	HANDLE dstframe;
	HANDLE srcframe;
	int dststartx;
	int dststarty;
	int srcstartx;
	int srcstarty;
	int width;
	int height;
	DWORD colorfill;
} Ctrl_Op;

typedef struct
{
	char filename[32];
	Block_Req* pframe; 
} PNG_CACHE;

class CDPGraphic
{
public:
	CDPGraphic();
	~CDPGraphic();
	Block_Req* FreeBlockHead;
	BOOL Inite();
	HANDLE LoadImage(char* name, SIZE* size, BOOL justonec = FALSE);

	HANDLE ReqBakFrame(LONG x, LONG y, LONG w, LONG h);
	HANDLE ReqFrame(LONG x, LONG y, LONG w, LONG h);
	HANDLE ReqCtrl(LONG x, LONG y, LONG w, LONG h);
	HANDLE ReqTempBlk(LONG w, LONG h);
	void SetDisClosed(HANDLE frame, BOOL sys);
	void CloseBlk(HANDLE hframe);

	void MoveBlk(HANDLE hframe, LONG x, LONG y);
	void ShowBlk(HANDLE hframe);
	void HideBlk(HANDLE hframe);
	void Show(void);
	void Hide(void);
	void HideGDI(void);
	void ShowGDI(void);

	void SetBK(HANDLE hframe, DWORD val);
	BOOL BitBlt(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight,HANDLE srcSpr,int nXSrc,int nYSrc);
	void DumpBmp(HANDLE frame, char* filename);
	int DumpBmpEx(HANDLE frame, char** pbuf);
	HANDLE LoadJpg(char* pJpgBuf, int width, int height, int jpgWidth);
	HANDLE LoadBmp(char* filename, SIZE* size);

	BOOL CscBw(HANDLE srcSpr);
	BOOL CscRevert(HANDLE srcSpr);
	BOOL CscColor(HANDLE srcSpr, DWORD color);
	void InitBakRect(HANDLE hframe, DWORD edge, DWORD middle);

	BOOL AlphaBlend(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight,HANDLE srcSpr,int nXSrc,int nYSrc);
	BOOL AlphaBlend(HANDLE dstSpr,int nXDest,int nYDest,int nWidth,int nHeight, DWORD color);
	void FillSolidRect(HANDLE hframe,RECT* lpRect, DWORD clr);
	HANDLE LoadStr(char* cstr, LONG h, COLORREF textColor, DWORD* wide, WORD* pos = NULL);
	HANDLE DupBlock(HANDLE hframe);
	BOOL GetFrameRect(HANDLE hframe, RECT* lpRect);
#ifdef DP3000_CHIP
	HANDLE CscScale(HANDLE srcSpr, DWORD width, DWORD height);
#endif
	BOOL ReloadXml(const char* cstr);
	Block_Req* XmlInitFrame(HANDLE pHandle);
	HANDLE Rotate(HANDLE hpng, SIZE* size);
private:
	HANDLE m_hPng;
	Block_Req* LoadXmlFrame(char* pname);
	void XmlLoadText(Block_Req* pframe, HANDLE pHandle);
	void XmlLoadPng(Block_Req* pframe, HANDLE pHandle);
	void XmlLoadColor(Block_Req* pframe, HANDLE pHandle);
	void XmlLoadJpg(Block_Req* pframe, HANDLE pHandle);
	HANDLE h_hspr;
	Block_Req* HardBlockHead;
	Block_Req* SoftBlockHead;
	CRITICAL_SECTION Sprit_Lock;
	Block_Req* m_strtemp;				// ���ڻ���ʱʹ�ã��Ȼ����ÿ��У��ٸ����ַ����Ŀ�������
	DWORD m_dwCharHeight;
	PNG_CACHE* g_png;					// ���ڻ���pngͼƬ
	DWORD g_pngcount;					// �����pngͼƬ����
	void CacheAllPng();
	BOOL Mix(RGBQUAD * pDst,RGBQUAD * pSrc,RGBQUAD * pSrc2);
	void RGB2BW(RGBQUAD * pDst,RGBQUAD * pSrc);
	void RGB2Color(RGBQUAD * pDst,RGBQUAD * pSrc);
	void RGB2Revert(RGBQUAD * pDst,RGBQUAD * pSrc);
#ifndef	DP3000_CHIP
	LONG convertmatch(DWORD width);
	HANDLE m_hScale;					// ���ڽ���ͼ���������ʹ��
#endif

	int m_width;
	int m_height;
};


