#include "roomlib.h"

#define	SETFILE0		"SystemSet0.ext"
#define	SETFILE1		"SystemSet1.ext"
#define	SETENDID		0x55595301

/***********************ϵͳ��������*******************************/
typedef struct
{
    char		szPrjPwd[16];				// ��������
    char		szBakJpg[256];				// ����ͼƬ
    BYTE		reserved[128];				// ����λ
    BYTE		autoBak;					// �Զ�����
    DWORD   	screenoff;					// ����ʱ��
    DWORD   	showbright;					// ��Ļ����
    WORD    	TV_Code[6];                 // ������ӵĺ������
    WORD    	IR_Air_Code[8];             // ����յ��ĺ������
    DWORD		VERSION;					// д���ļ���ʶ
    DWORD		Endid;						// ������
} SystemSet_t;
/*******************************************************************/

static StaticLock g_SystemSetCS;
static SystemSet_t *m_gSystemSet;

static void UpdataSystemSet(void)              //ϵͳ����
{
    SystemSet_t *pSet;
    BOOL ret;

    pSet = (SystemSet_t *)malloc(sizeof(SystemSet_t));
    if(NULL == pSet)
    {
        DBGMSG(DPERROR, "UpdataSystemSet malloc fail\r\n");
        return;
    }

    memcpy(pSet, m_gSystemSet, sizeof(SystemSet_t));
    if(m_gSystemSet->VERSION & 1)
        ret = WriteServerFile(SETFILE1, sizeof(SystemSet_t), (char *)pSet);
    else
        ret = WriteServerFile(SETFILE0, sizeof(SystemSet_t), (char *)pSet);
    if(!ret)
        free(pSet);
    else
    {
        m_gSystemSet->VERSION++;
    }
}

void InitDefaultSystemSet(void)
{
    memset(m_gSystemSet, 0, sizeof(SystemSet_t));

    strcpy(m_gSystemSet->szPrjPwd, "12345678");	// ��ʼ��Ĭ������
    strcpy(m_gSystemSet->szBakJpg, "bk1.png");		// ��ʼ��Ĭ�ϱ���
    m_gSystemSet->autoBak = FALSE;					// ��ʼ��Ĭ���Զ��л�������ʶ

    for(int i = 0; i < 6; i++)   					// ��ʼ������������ֵ-����
    {
        m_gSystemSet->TV_Code[i] = 0xffff;
    }

    for(int i = 0; i < 8; i++)  					// ��ʼ������������ֵ-�յ�
    {
        m_gSystemSet->IR_Air_Code[i] = 0xffff;
    }

    m_gSystemSet->screenoff = 60;					// ��ʼ��Ĭ������ʱ��
    m_gSystemSet->showbright = 50;					// ��ʼ��Ĭ����Ļ����
    m_gSystemSet->VERSION = 0;
    m_gSystemSet->Endid = SETENDID;
}

void InitSystemSet(void)
{
    FILE *fd;
    char filename[64];
    SystemSet_t *pSet0 = NULL;
    SystemSet_t *pSet1 = NULL;

    g_SystemSetCS.lockon();
    sprintf(filename, "%s/%s", USERDIR, SETFILE0);
    fd = fopen(filename, "rb");
    if(fd != NULL)
    {
        pSet0 = (SystemSet_t *)malloc(sizeof(SystemSet_t));
        if(pSet0)
        {
            memset(pSet0, 0, sizeof(SystemSet_t));
            int ret = fread(pSet0, 1, sizeof(SystemSet_t), fd);
            if(ret != sizeof(SystemSet_t))
            {
                free(pSet0);
                pSet0 = NULL;
            }
            else
            {
                if(pSet0->Endid != SETENDID)
                {
                    free(pSet0);
                    pSet0 = NULL;
                }
            }
        }
        fclose(fd);

        if(pSet0 == NULL)
        {
            // �ļ����󣬱��浽�����·��
            char bakFile[MAX_PATH];
            sprintf(bakFile, "%s.bak", filename);
            DPMoveFile(bakFile, filename);
        }
    }

    sprintf(filename, "%s/%s", USERDIR, SETFILE1);
    fd = fopen(filename, "rb");
    if(fd != NULL)
    {
        pSet1 = (SystemSet_t *)malloc(sizeof(SystemSet_t));
        if(pSet1)
        {
            memset(pSet1, 0, sizeof(SystemSet_t));
            int ret = fread(pSet1, 1, sizeof(SystemSet_t), fd);
            if(ret != sizeof(SystemSet_t))
            {
                free(pSet1);
                pSet1 = NULL;
            }
            else
            {
                if(pSet1->Endid != SETENDID)
                {
                    free(pSet1);
                    pSet1 = NULL;
                }
            }
        }
        fclose(fd);

        if(pSet1 == NULL)
        {
            // �ļ����󣬱��浽�����·��
            char bakFile[MAX_PATH];
            sprintf(bakFile, "%s.bak", filename);
            DPMoveFile(bakFile, filename);
        }
    }

    if((pSet0 == NULL) && (pSet1 == NULL))
    {
        m_gSystemSet = (SystemSet_t *)malloc(sizeof(SystemSet_t));
        if(m_gSystemSet)
        {
            InitDefaultSystemSet();
            UpdataSystemSet();
        }
    }
    else
    {
        if((pSet0 != NULL) && (pSet1 != NULL))
        {
            if(pSet0->VERSION > pSet1->VERSION)
            {
                m_gSystemSet = pSet0;
                free(pSet1);
            }
            else
            {
                m_gSystemSet = pSet1;
                free(pSet0);
            }
        }
        else if(pSet0 != NULL)
            m_gSystemSet = pSet0;
        else
            m_gSystemSet = pSet1;
        m_gSystemSet->VERSION++;
    }

    g_SystemSetCS.lockoff();
}

void ResetSystemSet(void)
{
    g_SystemSetCS.lockon();
    InitDefaultSystemSet();
    UpdataSystemSet();
    DeleteServerFile(SETFILE1);
    g_SystemSetCS.lockoff();
}

void GetPrjPwd(char *szPasswd)
{
    if(NULL == szPasswd)
        return;

    g_SystemSetCS.lockon();
    strcpy(szPasswd, m_gSystemSet->szPrjPwd);
    g_SystemSetCS.lockoff();
}

void SetIR_TV_CODE(WORD Status, int i)
{
    g_SystemSetCS.lockon();
    m_gSystemSet->TV_Code[i] = Status;
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

WORD GetIR_TV_CODE(WORD Status, int i)
{
    g_SystemSetCS.lockon();
    Status = m_gSystemSet->TV_Code[i];
    g_SystemSetCS.lockoff();
    return Status;
}

WORD GetIR_AIR_CODE(WORD Status, int i)
{
    g_SystemSetCS.lockon();
    Status = m_gSystemSet->IR_Air_Code[i];
    g_SystemSetCS.lockoff();
    return Status;
}

void SetIR_AIR_CODE(WORD Status, int i)
{
    g_SystemSetCS.lockon();
    m_gSystemSet->IR_Air_Code[i] = Status;
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

void SetPrjPwd(char *szPasswd)
{
    if(NULL == szPasswd)
        return;

    g_SystemSetCS.lockon();
    strcpy(m_gSystemSet->szPrjPwd, szPasswd);
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

void SetPrjShow(DWORD show)
{
    g_SystemSetCS.lockon();
    m_gSystemSet->screenoff = show;
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

DWORD GetPrjShow()
{
    DWORD show;
    g_SystemSetCS.lockon();
    show = m_gSystemSet->screenoff;
    g_SystemSetCS.lockoff();
    return show;
}

void SetShowBright(DWORD bright)
{
    g_SystemSetCS.lockon();
    m_gSystemSet->showbright = bright;
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

DWORD GetShowBright()
{
    DWORD bright;
    g_SystemSetCS.lockon();
    bright = m_gSystemSet->showbright;
    g_SystemSetCS.lockoff();
    return bright;
}

void SetPrjbkp(char *szbkp)
{
    if(NULL == szbkp)
        return;

    g_SystemSetCS.lockon();
    strcpy(m_gSystemSet->szBakJpg, szbkp);
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

void GetPrjbkp(char *szbkp)
{
    if(NULL == szbkp)
        return;

    g_SystemSetCS.lockon();
    strcpy(szbkp, m_gSystemSet->szBakJpg);
    g_SystemSetCS.lockoff();
}

void SetAutoBk(BYTE status)
{
    g_SystemSetCS.lockon();
    m_gSystemSet->autoBak = status;
    UpdataSystemSet();
    g_SystemSetCS.lockoff();
}

BYTE GetAutoBk()
{
    BYTE status;
    g_SystemSetCS.lockon();
    status = m_gSystemSet->autoBak;
    g_SystemSetCS.lockoff();
    return status;
}

DWORD GetRingVol(void)
{
    return 0xFFFFFFFF;
}

BOOL SetScreenOnOff(BOOL bOn)
{
    DWORD bright = GetShowBright();

	if(bright < 1)
		bright = 1;
	if(bright > 100)
		bright = 100;
    if(!bOn)
        bright = 0;

    return AdjustScreen(bright, 50, 50);
}

DWORD GetDelay(DWORD type)
{
    return 60;
}
