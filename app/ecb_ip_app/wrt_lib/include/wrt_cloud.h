#ifndef WRT_CLOUD_H_
#define WRT_CLOUD_H_

#include <cds/dstring.h>
#include <cds/sstr.h>
#include <cds/ptr_vector.h>

#ifdef __cplusplus
extern "C"{
#endif

        /************************************************************************/
        /* 注意：通过cloud 系列函数获取的字符串都是UTF-8编码,并且都要主动释放     */
	/*主要封装了curl库的使用*/
	/*依赖库libcds and libjson and libcurl*/
        /************************************************************************/
/**
*PUT,POST,DELETE方法接口
*/
#define BASE		0
#define LOGIN		1
#define RESETPWD	2
#define GETBIND		3
#define UPGRADE		4
#define HOSTLIST	5

typedef struct __wrtServer
{
	str_t name; //"W_TALK"/"W_ALARM"/"W_SMART"/"W_IPC"/"W_MSG" 等等。
	ptr_vector_t domain;// IP+PORT list 保存为char* 类型的数组
}WRTSERVER;

typedef struct __wrtCloud{
	str_t host;//云端主机地址。host格式IP:PORT或者域名
	str_t deviceCode; //设备编号（对讲设备为呼号）
	str_t devPassword; //设备编号密码
	str_t devType;  //设备型号
	int    isHaveCa;  //本地是否有CA文件。
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
	ptr_vector_t serverAddress; //保存结构体为WRTSERVER* 
	
}LOGINRESULT;

typedef struct __bindResult{
	ptr_vector_t userName; //谁绑定了我
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
	str_t deviceCode; //主机的23位物理编号。
	str_t talkID;	//主机23位物理编号对应的talkID.
	
}WRTHOST;

typedef struct _hostlistResult{
	ptr_vector_t hostList; //保存结构体为WRTHOST*
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
*函数名称：device_login_cloud
*函数功能：设备登录云服务器，获取业务获取列表。
*输入参数：
*		WRTCLOUD *
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_login_cloud(WRTCLOUD* _wrtcloud); 

/**
*函数名称：device_modified_password_cloud
*函数功能：设备修改密码（指针对对讲设备）,释放放回结果时：输入type 为：BASE
*输入参数：
*		WRTCLOUD *
		str_t* newPassowrd,新密码。
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_modified_password_cloud(WRTCLOUD* _wrtcloud,const str_t* newPassword);


/**
*函数名称：device_get_bind_list_form_cloud
*函数功能：设备获取绑定列表（谁绑定了我）
*输入参数：
*		WRTCLOUD *
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_get_bind_list_form_cloud(WRTCLOUD* _wrtcloud);


/**
*函数名称：device_is_upgarde
*函数功能：设备是否需要升级
*输入参数：
*		WRTCLOUD *
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_is_upgarde(WRTCLOUD* _wrtcloud);

/**
*函数名称：device_free_cloud_result
*函数功能：释放返回结果的资源。
*输入参数：
*		WRTCLOUDRESULT *
		type 新密码。（BASE，LOGIN,RESETPWD等）
*输出参数：
*		WRTCLOUDRESULT* 
*/
void device_free_cloud_result(WRTCLOUDRESULT* cloudResult,int type);



	
/**
*函数名称：device_get_hostList
*函数功能：获取设备的主机列表。
*输入参数：
*		WRTCLOUD *
		
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_get_hostList(WRTCLOUD* _wrtcloud);	




#ifdef __cplusplus
}
#endif

#endif
