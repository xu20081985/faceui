#ifndef WRT_CLOUD_H_
#define WRT_CLOUD_H_

#include <cds/dstring.h>
#include <cds/sstr.h>
#include <cds/ptr_vector.h>

#ifdef __cplusplus
extern "C"{
#endif

        /************************************************************************/
        /* ע�⣺ͨ��cloud ϵ�к�����ȡ���ַ�������UTF-8����,���Ҷ�Ҫ�����ͷ�     */
	/*��Ҫ��װ��curl���ʹ��*/
	/*������libcds and libjson and libcurl*/
        /************************************************************************/
/**
*PUT,POST,DELETE�����ӿ�
*/
#define BASE		0
#define LOGIN		1
#define RESETPWD	2
#define GETBIND		3
#define UPGRADE		4
#define HOSTLIST	5

typedef struct __wrtServer
{
	str_t name; //"W_TALK"/"W_ALARM"/"W_SMART"/"W_IPC"/"W_MSG" �ȵȡ�
	ptr_vector_t domain;// IP+PORT list ����Ϊchar* ���͵�����
}WRTSERVER;

typedef struct __wrtCloud{
	str_t host;//�ƶ�������ַ��host��ʽIP:PORT��������
	str_t deviceCode; //�豸��ţ��Խ��豸Ϊ���ţ�
	str_t devPassword; //�豸�������
	str_t devType;  //�豸�ͺ�
	int    isHaveCa;  //�����Ƿ���CA�ļ���
}WRTCLOUD;

typedef struct __loginResult{
	int update;
	str_t updateAddress;
	str_t md5;
	int 	ca;
	str_t caAddress;
	str_t md5Ca;
	str_t currentTime;
	str_t devNum;
	str_t callNum;
	str_t appID;
	ptr_vector_t serverAddress; //����ṹ��ΪWRTSERVER* 
	
}LOGINRESULT;

typedef struct __bindResult{
	ptr_vector_t userName; //˭������
}BINDRESULT;

typedef struct _upgradeResult{
	int upgrade;
	int forceUpgrade;
	str_t filePath;
	str_t md5Upgrade;
	str_t version;
	str_t remark;
}UPGRADERESULT;

typedef struct _wrthost{
	str_t deviceCode; //������23λ�����š�
	str_t talkID;	//����23λ�����Ŷ�Ӧ��talkID.
	
}WRTHOST;

typedef struct _hostlistResult{
	ptr_vector_t hostList; //����ṹ��ΪWRTHOST*
}HOSTLISTRESULT;

typedef struct __wrtCloudResult
{
	int yes;
	str_t reason;
	union {
		LOGINRESULT  	loginResult;
		BINDRESULT	bindResult;
		UPGRADERESULT   upgradeResult;
		HOSTLISTRESULT  hostListResult;
	}result;
}WRTCLOUDRESULT;

int json_put( const char* url,const char* buf,int bufsize,dstring_t* result);
int json_post( const char* url,const char* buf,int bufsize,dstring_t* result);
int json_del( const char* url,const char* buf,int bufsize,dstring_t* result);
int curl_test_get( const char* url);
	
/**
*�������ƣ�device_login_cloud
*�������ܣ��豸��¼�Ʒ���������ȡҵ���ȡ�б�
*���������
*		WRTCLOUD *
*���������
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_login_cloud(WRTCLOUD* _wrtcloud); 

/**
*�������ƣ�device_modified_password_cloud
*�������ܣ��豸�޸����루ָ��ԶԽ��豸��,�ͷŷŻؽ��ʱ������type Ϊ��BASE
*���������
*		WRTCLOUD *
		str_t* newPassowrd,�����롣
*���������
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_modified_password_cloud(WRTCLOUD* _wrtcloud,const str_t* newPassword);


/**
*�������ƣ�device_get_bind_list_form_cloud
*�������ܣ��豸��ȡ���б�˭�����ң�
*���������
*		WRTCLOUD *
*���������
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_get_bind_list_form_cloud(WRTCLOUD* _wrtcloud);


/**
*�������ƣ�device_is_upgarde
*�������ܣ��豸�Ƿ���Ҫ����
*���������
*		WRTCLOUD *
*���������
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_is_upgarde(WRTCLOUD* _wrtcloud);

/**
*�������ƣ�device_free_cloud_result
*�������ܣ��ͷŷ��ؽ������Դ��
*���������
*		WRTCLOUDRESULT *
		type �����롣��BASE��LOGIN,RESETPWD�ȣ�
*���������
*		WRTCLOUDRESULT* 
*/
void device_free_cloud_result(WRTCLOUDRESULT* cloudResult,int type);



	
/**
*�������ƣ�device_get_hostList
*�������ܣ���ȡ�豸�������б�
*���������
*		WRTCLOUD *
		
*���������
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_get_hostList(WRTCLOUD* _wrtcloud);	




#ifdef __cplusplus
}
#endif

#endif
