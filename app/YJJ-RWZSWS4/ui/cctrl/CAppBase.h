#pragma once

#include "roomlib.h"
#include "dpgraphic.h"
#include "input.h"

#define	MAX_CTL_ITEM			68
#define	MAX_LAYER_FRAME			10

typedef struct
{
    char name[32];
    DWORD property;
    DWORD IdBegin;
    HANDLE hCtrl;
} CtrlList;

class ContentManage
{
public:
    ContentManage()
    {
        count = 0;
    }
    ~ContentManage() {}
    void Init(char *psrc)
    {
        ExtractParam(psrc);
    }
    char *FindContentByName(const char *name)
    {
        DWORD i;
        for(i = 0; i < count; i++)
        {
            if(strcmp(pname[i], name) == 0)
                return pcontent[i];
        }
        return NULL;
    }
private:
    void ExtractParam(char *psrc)
    {
        count = 0;
        while(1)
        {
            if((psrc = strchr(psrc, '"')) == NULL)
                break;
            psrc++;
            pname[count] = psrc;
            if((psrc = strchr(psrc, '=')) == NULL)
                break;
            *psrc = 0;
            psrc++;
            pcontent[count] = psrc;
            if((psrc = strchr(psrc, '"')) == NULL)
                break;
            *psrc = 0;
            psrc++;
            count++;
        }
    }

    DWORD count;
    char *pname[32];
    char *pcontent[32];
};

class CAppBase;

class CLayOut
{
public:
    CLayOut(CAppBase *pHwnd);
    ~CLayOut(void);
    BOOL InitLay(char *pbuf);
    void DeinitLay(void);
    void ResumeAck(void);
    void PauseAck(void);
    void ShowLay();
    void HideLay();
    BOOL IsHide();
    void ActivateCtrl(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl);
    void DeActivate(HANDLE hctrl);

    // ע��ؼ�
    void RegisterCtrl(DWORD type, char *name, HANDLE, DWORD m_msgid);
    // ע�ᴥ������Ӧ
    void RegisterEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl);
    void RegisterJustEvent(DWORD left, DWORD top, DWORD width, DWORD height, HANDLE hctrl);
    // ע�ᰴ����Ӧ
    void RegisterKeyEvent(int but, HANDLE hctl);
    // ע��ʱ����Ӧ
    void RegisterTickEvent(HANDLE hctl);
    // ��ȡ�ؼ��ľ��
    HANDLE GetCtrlByName(char *name, DWORD *idmap = NULL);
    // ����ؼ���ID
    DWORD RequestMsgId(void);
    // ��ȡͼ������
    char *GetName(void);
    // ��ʾ������ͼ��
    void SwitchLay(BOOL isShow);
    void DoTickProcess(void);

    CDPGraphic *m_pSpr;				// ����֧��
    HANDLE m_frame;					// ͼ���Ӧ��Ӳ��frame
    DWORD m_width;					// ͼ����
    DWORD m_height;					// ͼ��߶�
    DWORD m_left;					// ͼ������Ļ�ϵ�x������ʼ��
    DWORD m_top;					// ͼ������Ļ�ϵ�y�������ʼ��
    DWORD m_isHide;					// ͼ���Ƿ�����,ȱʡΪFALSE,��ʾ
private:
    // ���ڳ�ʼ������
    void InitListFrame(ContentManage *pCm);
    BOOL InitBkFrame(ContentManage *pCm);
    void InitPartFrame(ContentManage *pCm);
    void InitAlphaFrame(ContentManage *pCm);
    char name[32];				// ͼ������,ȱʡΪmainlay

    CAppBase *m_pWin;				// ͼ�����ڴ���ָ��
    DWORD m_layer;					// ͼ������Ӳ����

public:
    DWORD m_isUnack;				// ͼ���Ƿ���Ӧ������
    KeybdMap m_KeyEvent[MAX_CTL_ITEM];	// ������Ӧ�¼�
    DWORD m_KeyCount;					// ������Ӧ�¼�����
    EventMap CtlEvent[MAX_CTL_ITEM];	// ��������Ӧ�¼�
    DWORD m_CtlCount;					// ��������Ӧʱ�����

    HANDLE TickEvent[MAX_CTL_ITEM];
    DWORD m_TickCount;
    CtrlList m_CtrlList[MAX_CTL_ITEM];	// ͼ���ڿؼ�
    DWORD m_CtlListCount;				// ͼ���ڿؼ�����
};

class CAppBase
{
public:
    CAppBase(DWORD pHwnd);
    virtual ~CAppBase(void);
public:
    // ��Ϣ������
    virtual BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam) = 0;
    // ��������
    virtual BOOL Destroy(void);
    // ������ͣ
    virtual BOOL DoPause(void);
    // ���ڼ���
    virtual void DoResume(void);
    // ���ڻָ���Ӧ
    virtual void ResumeAck(void);
    // ����ֹͣ��Ӧ,��������ʾ
    virtual void PauseAck(void);
    static void Show(void);   //��ʾ
    DWORD RequestMsgId(void);

    void InitDisplay();
    BOOL InitFrame(const char *xmlname);
    HANDLE GetCtrlByName(const char *name, DWORD *begin = NULL);
    DWORD m_dwTimeout;		                                      // ���ϴβ������ʱ��

    DWORD m_screenoff;		                                      // ����ʱ��
    BOOL InitLayer(const char *xmlname);
    void DumpImage(char *filename);
public:
    int InitXmlFile(const char *xmlname, char **rpbuf);
    CLayOut *m_layer[MAX_LAYER_FRAME];
    DWORD m_layindex;		// ʹ�õ���ͼ������
    CDPGraphic *m_pSpr;
    DWORD m_IdBase;			// ���ڵ�ID
    DWORD m_IdOrg;			// �ؼ���ID,�ڴ���ID�Ļ��������ϼ�
};

