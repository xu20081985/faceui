
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wrt_cloud.h"
#include <curl/curl.h>
#include "json.h"
#include <cds/logger.h>

static int isserver = 0;

static int ishostlist = 0;

//HTTP摘要认证用户名和密码
#define WRT_USR "wrt"
#define WRT_USR_PW "wrt12"

static void traversal_json(struct json_object* obj,void* userdata);
static void process_json_array(const char* key,struct json_object* obj,void* userdata);
static void fillresult_string(const char* key,const char* value,WRTCLOUDRESULT* cloudResult);
static void fillresult_int(const char* key,int value,WRTCLOUDRESULT* cloudResult);

static size_t write_json_data_func(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int s = size * nmemb;
	/*	TRACE_LOG("%d bytes writen\n", s);*/
	if (s != 0) {
		if (dstr_append((dstring_t*)stream, ptr, s) != 0) {
			ERROR_LOG("can't append %d bytes into data buffer\n", s);
			return 0;
		}
		//printf("str: %s \n",dstr_get_data((dstring_t*)stream->str);
	}
	return s;
}


static   size_t read_json_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	static int used = 0;
	dstring_t *p = (dstring_t*)stream;
	str_t mydata;
	dstr_get_str(p,&mydata);
	if(used == mydata.len){
		used = 0;
		return 0;
	}
	memcpy(ptr,mydata.s,mydata.len);
	used += mydata.len;
	return mydata.len;
	//return dstr_get_data_length((dstring_t*)stream);
}

int json_put( const char* url,const char* buf,int bufsize,dstring_t* result)
{
	CURLcode res = -1;
	static CURL *handle = NULL;
	char *auth = NULL;
	int i;
	long auth_methods;
	dstring_t data;
	if (!url) {
		ERROR_LOG("BUG: no uri given\n");
		return -1;
	}
	if (!buf) {
		ERROR_LOG("BUG: no buf given\n");
		return -1;
	}
	
	i=0;
	i += strlen(WRT_USR);
	i += strlen(WRT_USR_PW);
	/* do authentication */
	auth = (char *)cds_malloc(i + 2);
	if (!auth) return -1;
	sprintf(auth, "%s:%s", "wrt","wrt12");
	
	
	auth_methods = CURLAUTH_BASIC | CURLAUTH_DIGEST;
	
	if (!handle) handle = curl_easy_init(); 
	if (handle) {
		struct curl_slist *chunk = NULL;
		dstr_init(&data,bufsize);
		dstr_append(&data,buf,bufsize);
		curl_easy_setopt(handle, CURLOPT_READFUNCTION, read_json_callback);
	
		/* enable uploading */
		curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
	
		/* HTTP PUT please */
		curl_easy_setopt(handle, CURLOPT_PUT, 1L);
	
		/* specify target URL, and note that this URL should include a file
		name, not only a directory */
		curl_easy_setopt(handle, CURLOPT_URL, url);
	
		/* now specify which file to upload */
		curl_easy_setopt(handle, CURLOPT_READDATA, &data);
	
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_json_data_func);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, result);
	
		
	
		chunk = curl_slist_append(chunk, "content-Type: text/json");
		chunk = curl_slist_append(chunk, "content-Encoding: UTF-8");
		curl_easy_setopt(handle,CURLOPT_HTTPHEADER,chunk);
		/* auth */
		curl_easy_setopt(handle, CURLOPT_HTTPAUTH, auth_methods); /* TODO possibility of selection */
		curl_easy_setopt(handle, CURLOPT_NETRC, CURL_NETRC_IGNORED);
		curl_easy_setopt(handle, CURLOPT_USERPWD, auth);
	
		/* SSL */
		/*if (params) {
		 if (params->enable_unverified_ssl_peer) {
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
		 }
		}*/
	
		/* provide the size of the upload, we specicially typecast the value
		to curl_off_t since we must be sure to use the correct data size */
		curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,
				 (curl_off_t)bufsize);
	
		/* Now run off and do what you've been told! */
		res = curl_easy_perform(handle);
		dstr_destroy(&data);
	
		/* always cleanup */
		/*curl_easy_cleanup(handle);*/
	}
	
	if (auth) cds_free(auth);
	return res;	
}
int json_post( const char* url,const char* buf,int bufsize,dstring_t* result)
{
	CURLcode res = -1;
	static CURL *handle = NULL;
	
	char *auth = NULL;
	char* resultstr = NULL;
	int i;
	long auth_methods;
	
	if (!url) {
		ERROR_LOG("BUG: no uri given\n");
		return -1;
	}
	if (!buf) {
		ERROR_LOG("BUG: no buf given\n");
		return -1;
	}
	curl_global_init(CURL_GLOBAL_ALL);
	i = 0;
	i +=strlen(WRT_USR);
	i += strlen(WRT_USR_PW);
	
	if (i > 0) {
		/* do authentication */
		auth = (char *)cds_malloc(i + 2);
		if (!auth) return -1;
		sprintf(auth, "%s:%s", "wrt","wrt12");
	}
	
	auth_methods = CURLAUTH_BASIC | CURLAUTH_DIGEST;
	if (!handle) handle = curl_easy_init(); 
	if (handle) {
		struct curl_slist *chunk = NULL;

	
		/* HTTP POST please */
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_json_data_func);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, result);
	
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, buf);
		curl_easy_setopt(handle,CURLOPT_POSTFIELDSIZE,bufsize);
	
		/* specify target URL, and note that this URL should include a file
		name, not only a directory */
		curl_easy_setopt(handle, CURLOPT_URL, url);
	
		/* now specify which file to upload */
		//curl_easy_setopt(handle, CURLOPT_READDATA, &data);
	
		
		
		chunk = curl_slist_append(chunk, "content-Type: text/json");
	
		chunk = curl_slist_append(chunk, "content-Encoding: UTF-8");
		curl_easy_setopt(handle,CURLOPT_HTTPHEADER,chunk);
		/* auth */
		curl_easy_setopt(handle, CURLOPT_HTTPAUTH, auth_methods); /* TODO possibility of selection */
		curl_easy_setopt(handle, CURLOPT_NETRC, CURL_NETRC_IGNORED);
		curl_easy_setopt(handle, CURLOPT_USERPWD, auth);
	
		/* SSL */
		/*if (params) {
		 if (params->enable_unverified_ssl_peer) {
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
		 }
		}*/
	
		/* provide the size of the upload, we specicially typecast the value
		to curl_off_t since we must be sure to use the correct data size */
		//curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,
		//	(curl_off_t)size);
	
		/* Now run off and do what you've been told! */
		res = curl_easy_perform(handle);
	
		/* always cleanup */
		/*curl_easy_cleanup(handle);*/
	}
	
	if (auth) cds_free(auth);
	curl_global_cleanup();
	return res;	
}
int json_del( const char* url,const char* buf,int bufsize,dstring_t* result)
{
	CURLcode res = -1;
	static CURL *handle = NULL;
	char *auth = NULL;
	int i;
	long auth_methods;
	
	if (!url) {
		ERROR_LOG("BUG: no uri given\n");
		return -1;
	}
	
	i=0;
	i += strlen(WRT_USR);
	i += strlen(WRT_USR_PW);
	/* do authentication */
	auth = (char *)cds_malloc(i + 2);
	if (!auth) return -1;
	sprintf(auth, "%s:%s", "wrt","wrt12");
	
	auth_methods = CURLAUTH_BASIC | CURLAUTH_DIGEST;
	
	if (!handle) handle = curl_easy_init(); 
	if (handle) {
		struct curl_slist *chunk = NULL;
		
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_json_data_func);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, result);
	
		curl_easy_setopt(handle,CURLOPT_CUSTOMREQUEST,"DELETE");
	
		/* specify target URL, and note that this URL should include a file
		name, not only a directory */
		curl_easy_setopt(handle, CURLOPT_URL, url);
	
		/* now specify which file to upload */
		//curl_easy_setopt(handle, CURLOPT_READDATA, &data);
		chunk = curl_slist_append(chunk, buf);
		chunk = curl_slist_append(chunk, "content-Type: text/json");
		chunk = curl_slist_append(chunk, "content-Encoding: UTF-8");
		
		curl_easy_setopt(handle,CURLOPT_HTTPHEADER,chunk);
		/* auth */
		curl_easy_setopt(handle, CURLOPT_HTTPAUTH, auth_methods); /* TODO possibility of selection */
		curl_easy_setopt(handle, CURLOPT_NETRC, CURL_NETRC_IGNORED);
		curl_easy_setopt(handle, CURLOPT_USERPWD, auth);
	
		/* SSL */
		/*if (params) {
		 if (params->enable_unverified_ssl_peer) {
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
		 }
		}*/
	
		/* provide the size of the upload, we specicially typecast the value
		to curl_off_t since we must be sure to use the correct data size */
		//curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,
		//	(curl_off_t)size);
	
		/* Now run off and do what you've been told! */
		res = curl_easy_perform(handle);
		
		/* always cleanup */
		/*curl_easy_cleanup(handle);*/
	}
	
	if (auth) cds_free(auth);
	return res;

}
	
static int check_param(WRTCLOUD* _wrtcloud)
{
	int len = 0;
	if(_wrtcloud == NULL)
		return -1;
	if(str_len(&_wrtcloud->host) == 0 )
		return -1;
	if(str_len(&_wrtcloud->deviceCode) == 0)
		return -1;
	if(str_len(&_wrtcloud->devPassword) == 0)
		return -1;
	if(str_len(&_wrtcloud->devType) == 0)
		return -1;	
	return str_len(&_wrtcloud->deviceCode)+str_len(&_wrtcloud->devPassword)+str_len(&_wrtcloud->devType)+1;
}



static void process_json_array(const char* key,struct json_object* obj,void* userdata)
{
	struct json_object* temp_obj = NULL;
	int i = 0;
	
	
      	 for(i=0 ; i<json_object_array_length(obj) ; i++ ){ 
                temp_obj = json_object_array_get_idx(obj, i );
                if(is_error(temp_obj)){	
                	continue;
               }
               switch( json_object_get_type( temp_obj ) ){  
               		case json_type_object:
               			traversal_json(temp_obj,userdata);
               			break;
                       case json_type_string:  
                       		fillresult_string(key, json_object_get_string(temp_obj),userdata);
                        	TRACE_LOG("value: %s \n", json_object_get_string(temp_obj) )  ;
                      		 break;  
                       
                       case json_type_int:
                       		fillresult_int(key, json_object_get_int(temp_obj),userdata);
                        	 TRACE_LOG("value: %d \n", json_object_get_int(temp_obj) )  ;
                       		break;  
                       case  json_type_array:
			     	//process_json_array(temp_obj,userdata);
				break;
                       default: 
                       	break;  
               }                                        
       }
   
	
}

static void traversal_json(struct json_object* obj,void* userdata)
{
	struct json_object_iter iter;
	json_object_object_foreachC(obj,iter)
	{
		if(strcmp(iter.key,"serverAddress") == 0){
			isserver = 1;
		}else if(strcmp(iter.key,"hostList") == 0){
			ishostlist = 1;
		}
		
		switch(json_object_get_type(iter.val)){
		case json_type_string:
			fillresult_string(iter.key,json_object_get_string(iter.val),(WRTCLOUDRESULT*)userdata);
			break;
		case json_type_array:
			process_json_array(iter.key,iter.val,userdata);
			break;
		case json_type_int:
			fillresult_int(iter.key,json_object_get_int(iter.val),(WRTCLOUDRESULT*)userdata);
			break;
		 case json_type_object:
			traversal_json(iter.val, userdata);
			break;
		default:
			TRACE_LOG("other type no process \n");
			break;
		}
		if(strcmp(iter.key,"serverAddress") == 0){
			isserver = 0;
		}else if(strcmp(iter.key,"hostList") == 0){
			ishostlist = 0;
		}
		
	}
}

static void fillhost_list(const char* key,const char* value,WRTCLOUDRESULT* cloudResult)
{
	static WRTHOST* h = NULL;
	if(strcmp(key,"deviceCode") == 0){
		h =(WRTHOST*)cds_malloc(sizeof(WRTHOST));
		if(h){
			str_dup_zt(&h->deviceCode,value);
		}
				
	}else if(strcmp(key,"talkId") == 0){
		if(h){
			int size = 0;
			str_dup_zt(&h->talkID,value);
			if((size = vector_size(&cloudResult->result.hostListResult.hostList)) == 0){
				ptr_vector_init(&cloudResult->result.hostListResult.hostList,32);
			}
			ptr_vector_add(&cloudResult->result.hostListResult.hostList,h);
			h = NULL;
		}else
			TRACE_LOG("fillhost_list is error \n");
	}
}

static void fillresult_string(const char* key,const char* value,WRTCLOUDRESULT* cloudResult)
{
	TRACE_LOG("key = %s,value= %s\n",key,value);
	if(ishostlist){
		return fillhost_list(key,value,cloudResult);
	}
			
	if(isserver)
	{
		if(key != NULL){
			int size = 0;
			if((size = vector_size(&cloudResult->result.loginResult.serverAddress)) == 0){
				WRTSERVER* s  = NULL;
				ptr_vector_init(&cloudResult->result.loginResult.serverAddress,16);
				s = (WRTSERVER*)cds_malloc(sizeof(WRTSERVER));
				if(s)
				{
					char* str =  zt_strdup(value);
					str_dup_zt(&s->name,key);
					ptr_vector_init(&s->domain,16);
					ptr_vector_add(&s->domain,str);
					ptr_vector_add(&cloudResult->result.loginResult.serverAddress,s);
					
				}
			}else{
				int i =0;
				for(i =0; i<size; i++)
				{
					WRTSERVER* s = (WRTSERVER*)ptr_vector_get(&cloudResult->result.loginResult.serverAddress,i);
					if(s){
						if(str_cmp_zt(&s->name,key) == 0){
							break;
						}
					}
				}
				if( i == size){
					WRTSERVER* s = (WRTSERVER*)cds_malloc(sizeof(WRTSERVER));
					if(s)
					{
						char* str =  zt_strdup(value);
						str_dup_zt(&s->name,key);
						ptr_vector_init(&s->domain,16);
						if(str)
							ptr_vector_add(&s->domain,str);
						ptr_vector_add(&cloudResult->result.loginResult.serverAddress,s);
					}	
				}else{
					WRTSERVER* s = (WRTSERVER*)ptr_vector_get(&cloudResult->result.loginResult.serverAddress,i);
					if(s){
						char* str =  zt_strdup(value);
						if(str)
							ptr_vector_add(&s->domain,str);
					}
				}
					
			}
			
		}
		return;
	}
	if(strcmp(key,"result") == 0){
		str_dup_zt(&cloudResult->reason,value);
		
	}else if(strcmp(key,"updateAddress") == 0){
		str_dup_zt(&cloudResult->result.loginResult.updateAddress,value);

	}else if(strcmp(key,"md5") == 0){
		str_dup_zt(&cloudResult->result.loginResult.md5,value);

	}else if(strcmp(key,"caAddress") == 0){
		str_dup_zt(&cloudResult->result.loginResult.caAddress,value);

	}else if(strcmp(key,"md5Ca") == 0){
		str_dup_zt(&cloudResult->result.loginResult.md5Ca,value);

	}else if(strcmp(key,"currentTime") == 0){
		str_dup_zt(&cloudResult->result.loginResult.currentTime,value);

	}else if(strcmp(key,"devNum") == 0){
		str_dup_zt(&cloudResult->result.loginResult.devNum,value);

	}else if(strcmp(key,"callnum") == 0){
		str_dup_zt(&cloudResult->result.loginResult.callNum,value);

	}else if(strcmp(key,"userName") == 0){
		char* userName = zt_strdup(value);
		if(userName){
			if(vector_size(&cloudResult->result.bindResult.userName) == 0)
				ptr_vector_init(&cloudResult->result.bindResult.userName,4);
			ptr_vector_add(&cloudResult->result.bindResult.userName,userName);
		}
	}else if(strcmp(key,"filePath") == 0){
		str_dup_zt(&cloudResult->result.upgradeResult.filePath,value);
	}else if(strcmp(key,"md5Upgrade") == 0){
		str_dup_zt(&cloudResult->result.upgradeResult.md5Upgrade,value);
	}else if(strcmp(key,"version") == 0){
		str_dup_zt(&cloudResult->result.upgradeResult.version,value);
	}else if(strcmp(key,"remark") == 0){
		str_dup_zt(&cloudResult->result.upgradeResult.remark,value);
	}
}

static void fillresult_int(const char* key,int value,WRTCLOUDRESULT* cloudResult)
{
	if(strcmp(key,"status") == 0){
		cloudResult->yes = value;
	}else if(strcmp(key,"update") == 0){
		cloudResult->result.loginResult.update =value;
	}else if(strcmp(key,"ca") == 0){
		cloudResult->result.loginResult.ca  = value;	
	}else if(strcmp(key,"upgrade") == 0){
		cloudResult->result.upgradeResult.upgrade = value;
	}else if(strcmp(key,"forceUpgrade") == 0){
		cloudResult->result.upgradeResult.forceUpgrade = value;
	}
}

static WRTCLOUDRESULT* parse_result(dstring_t* data)
{
	struct json_object *obj,*obj1,*obj2;
	int ret = 0;
	int i;
	int processed = 0;
	WRTCLOUDRESULT* cloudResult = NULL;
	int len = dstr_get_data_length(data);
	char* buf = (char*)cds_malloc(len+1);
	if(buf == NULL)
		return NULL;
	cloudResult = (WRTCLOUDRESULT*)cds_malloc(sizeof(WRTCLOUDRESULT));
	if(cloudResult == NULL){
		cds_free(buf);
		return NULL;
	}
	memset(cloudResult,0,sizeof(WRTCLOUDRESULT));
	memset(buf,0,len+1);
	dstr_get_data(data,buf);
	
	obj = obj1 = NULL;
	obj = json_tokener_parse(buf);
	if(is_error(obj)){
		ERROR_LOG("json parser Error: %s \n",buf);
		cloudResult->yes = 0;
		str_dup_zt(&cloudResult->reason,"json parser error.");
		cds_free(buf);
		return cloudResult;
	}
	traversal_json(obj,cloudResult);
	json_object_put(obj);
	cds_free(buf);
	return cloudResult;

}

WRTCLOUDRESULT* device_login_cloud(WRTCLOUD* _wrtcloud)
{
	dstring_t data;
	int ret;
	dstring_t url;
	char* curl = NULL;
	char* param = NULL;
	int paramlen = 0;
	char deviceCode[11];
	char devicePassword[16];
	char devType[256];
	WRTCLOUDRESULT* cloudResult = NULL;
	char* buf="{\"login\":{\"userName\":\"%s\",\"password\":\"%s\",\"dev-info\":{\"dev\":\"%s\",\"ca\":%d}}}";
	if((paramlen = check_param(_wrtcloud)) == -1)
		return NULL;
	
	TRACE_LOG("device_login_cloud\n");
	paramlen += strlen(buf)+1;
	param = (char*)cds_malloc(paramlen);
	TRACE_LOG("paramlen: %d \n",paramlen);
	if(param == NULL){
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(param,0,paramlen);
	memset(deviceCode,0,11);
	memset(devicePassword,0,16);
	memset(devType,0,256);
	memcpy(deviceCode,_wrtcloud->deviceCode.s,_wrtcloud->deviceCode.len);
	memcpy(devicePassword,_wrtcloud->devPassword.s,_wrtcloud->devPassword.len);
	memcpy(devType,_wrtcloud->devType.s,_wrtcloud->devType.len);
	sprintf(param,buf,deviceCode,devicePassword,\
		devType,_wrtcloud->isHaveCa);
	dstr_init(&url,256);
	dstr_append_zt(&url,"http://");
	TRACE_LOG("host: %s \n",_wrtcloud->host.s);
	dstr_append_str(&url,&_wrtcloud->host);
	dstr_append_zt(&url,"/tnserver/user");
	curl = (char*)cds_malloc(url.len+1);
	if(curl == NULL){
		cds_free(param);
		dstr_destroy(&url);
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(curl,0,url.len+1);
	dstr_get_data(&url,curl);
	dstr_init(&data,256);
	TRACE_LOG("post: %s \n",curl);
	TRACE_LOG("param: %s \n",param);
	ret = json_post(curl,param,strlen(param),&data);
	if(ret == 0 ){
		cloudResult = parse_result(&data);
	}
	cds_free(curl);
	cds_free(param);
	dstr_destroy(&url);
	dstr_destroy(&data);
	return cloudResult;
	
}

/**
*函数名称：device_is_upgarde
*函数功能：设备是否需要升级
*输入参数：
*		WRTCLOUD *
*输出参数：
*		WRTCLOUDRESULT* 
*/
WRTCLOUDRESULT* device_is_upgarde(WRTCLOUD* _wrtcloud)
{
	int paramlen = 0;
	char* param = NULL;
	dstring_t url;
	dstring_t data; //,\"deviceCode\":\"%s\"
	int ret;
	char* curl = NULL;
	char devType[256];
	char deviceCode[128];
	WRTCLOUDRESULT* cloudResult = NULL;
	char* buf = "{\"upgrade\":{\"dev\":\"%s\",\"deviceCode\":\"%s\"}}";
	if(check_param(_wrtcloud) == -1)
		return  NULL;
	paramlen = strlen(buf)+1 +str_len(&_wrtcloud->devType)+str_len(&_wrtcloud->deviceCode);
	
	param = (char*)cds_malloc(paramlen);
	
	TRACE_LOG("paramlen: %d \n",paramlen);
	if(param == NULL){
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(param,0,paramlen);
	memset(devType,0,256);
	memset(deviceCode,0,128);
	memcpy(devType,_wrtcloud->devType.s,_wrtcloud->devType.len);
	memcpy(deviceCode,_wrtcloud->deviceCode.s,_wrtcloud->deviceCode.len);
	sprintf(param,buf,devType,deviceCode);
	dstr_init(&url,256);
	dstr_append_zt(&url,"http://");
	TRACE_LOG("host: %s \n",_wrtcloud->host.s);
	dstr_append_str(&url,&_wrtcloud->host);
	dstr_append_zt(&url,"/tnserver/app");
	curl = (char*)cds_malloc(url.len+1);
	if(curl == NULL){
		cds_free(param);
		dstr_destroy(&url);
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(curl,0,url.len+1);
	dstr_get_data(&url,curl);
	dstr_init(&data,256);
	TRACE_LOG("post: %s \n",curl);
	TRACE_LOG("param: %s \n",param);
	ret = json_post(curl,param,strlen(param),&data);
	if(ret == 0 ){
		cloudResult = parse_result(&data);
	}
	cds_free(curl);
	cds_free(param);
	dstr_destroy(&url);
	dstr_destroy(&data);
	return cloudResult;	
}

//获取都有谁绑定了我。
WRTCLOUDRESULT* device_get_bind_list_form_cloud(WRTCLOUD* _wrtcloud)
{
	int paramlen = 0;
	char* param = NULL;
	dstring_t url;
	dstring_t data;
	int ret;
	char* curl = NULL;
	char deviceCode[11];
	char devicePassword[16];
	WRTCLOUDRESULT* cloudResult = NULL;
	char* buf = "{\"getBind\":{\"deviceCode\":\"%s\",\"password\":\"%s\"}}";
	if(check_param(_wrtcloud) == -1)
		return  NULL;
	paramlen = strlen(buf)+1 +str_len(&_wrtcloud->deviceCode)+str_len(&_wrtcloud->devPassword);
	
	param = (char*)cds_malloc(paramlen);
	
	TRACE_LOG("paramlen: %d \n",paramlen);
	if(param == NULL){
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(param,0,paramlen);
	memset(deviceCode,0,11);
	memset(devicePassword,0,16);
	memcpy(deviceCode,_wrtcloud->deviceCode.s,_wrtcloud->deviceCode.len);
	memcpy(devicePassword,_wrtcloud->devPassword.s,_wrtcloud->devPassword.len);
	sprintf(param,buf,deviceCode,devicePassword);
	dstr_init(&url,256);
	dstr_append_zt(&url,"http://");
	TRACE_LOG("host: %s \n",_wrtcloud->host.s);
	dstr_append_str(&url,&_wrtcloud->host);
	dstr_append_zt(&url,"/tnserver/device/user");
	curl = (char*)cds_malloc(url.len+1);
	if(curl == NULL){
		cds_free(param);
		dstr_destroy(&url);
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(curl,0,url.len+1);
	dstr_get_data(&url,curl);
	dstr_init(&data,256);
	TRACE_LOG("post: %s \n",curl);
	TRACE_LOG("param: %s \n",param);
	ret = json_post(curl,param,strlen(param),&data);
	if(ret == 0 ){
		cloudResult = parse_result(&data);
	}
	cds_free(curl);
	cds_free(param);
	dstr_destroy(&url);
	dstr_destroy(&data);
	return cloudResult;
						
	
}	
WRTCLOUDRESULT* device_modified_password_cloud(WRTCLOUD* _wrtcloud,const str_t* newPassword)
{
	int paramlen = 0;
	char* param = NULL;
	dstring_t url;
	dstring_t data;
	int ret;
	char* curl = NULL;
	char* buf = NULL;
	WRTCLOUDRESULT* cloudResult = NULL;	
	if(check_param(_wrtcloud) == -1)
		return NULL;
	if(str_len(newPassword) == 0)
		return NULL;
		
	 buf= "{\"passwordUpdate\":{\"deviceCode\":\"%.*s\",\"password\":\"%.*s\",\"newpwd\":\"%.*s\"}}";
	if(check_param(_wrtcloud) == -1)
		return  NULL;
	paramlen = strlen(buf)+1 +str_len(&_wrtcloud->deviceCode)+str_len(&_wrtcloud->devPassword)+str_len(newPassword);
	
	param = (char*)cds_malloc(paramlen);
	
	TRACE_LOG("paramlen: %d \n",paramlen);
	if(param == NULL){
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(param,0,paramlen);

	sprintf(param,buf,_wrtcloud->deviceCode.len,_wrtcloud->deviceCode.s, \
			_wrtcloud->devPassword.len,_wrtcloud->devPassword.s,\
			newPassword->len,newPassword->s);
	dstr_init(&url,256);
	dstr_append_zt(&url,"http://");
	TRACE_LOG("host: %.*s \n",_wrtcloud->host.len,_wrtcloud->host.s);
	dstr_append_str(&url,&_wrtcloud->host);
	dstr_append_zt(&url,"/tnserver/device/password");
	curl = (char*)cds_malloc(url.len+1);
	if(curl == NULL){
		cds_free(param);
		dstr_destroy(&url);
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(curl,0,url.len+1);
	dstr_get_data(&url,curl);
	dstr_init(&data,256);
	TRACE_LOG("post: %s \n",curl);
	TRACE_LOG("param: %s \n",param);
	ret = json_post(curl,param,strlen(param),&data);
	if(ret == 0 ){
		cloudResult = parse_result(&data);
	}
	cds_free(curl);
	cds_free(param);
	dstr_destroy(&url);
	dstr_destroy(&data);
	return cloudResult;		
	
}

WRTCLOUDRESULT* device_get_hostList(WRTCLOUD* _wrtcloud)
{
	int paramlen = 0;
	char* param = NULL;
	dstring_t url;
	dstring_t data;
	int ret;
	char* curl = NULL;
	char* buf = NULL;
	WRTCLOUDRESULT* cloudResult = NULL;	
	if((paramlen = check_param(_wrtcloud)) == -1)
		return NULL;
		
	 buf= "{\"getHostList\":{\"dev\":\"%.*s\",\"userName\":\"%.*s\",\"password\":\"%.*s\"}}";
	if(check_param(_wrtcloud) == -1)
		return  NULL;
	paramlen = strlen(buf)+1 +str_len(&_wrtcloud->deviceCode)+str_len(&_wrtcloud->devPassword)+str_len(&_wrtcloud->devType);
	
	param = (char*)cds_malloc(paramlen);
	
	TRACE_LOG("paramlen: %d \n",paramlen);
	if(param == NULL){
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(param,0,paramlen);

	sprintf(param,buf,_wrtcloud->devType.len,_wrtcloud->devType.s,\
		_wrtcloud->deviceCode.len,_wrtcloud->deviceCode.s, \
			_wrtcloud->devPassword.len,_wrtcloud->devPassword.s);
	dstr_init(&url,256);
	dstr_append_zt(&url,"http://");
	dstr_append_str(&url,&_wrtcloud->host);
	dstr_append_zt(&url,"/tnserver/user/device");
	curl = (char*)cds_malloc(url.len+1);
	if(curl == NULL){
		cds_free(param);
		dstr_destroy(&url);
		ERROR_LOG("cds_malloc faile %s:%d\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(curl,0,url.len+1);
	dstr_get_data(&url,curl);
	dstr_init(&data,256);
	TRACE_LOG("post: %s \n",curl);
	TRACE_LOG("param: %s \n",param);
	ret = json_put(curl,param,strlen(param),&data);
	if(ret == 0 ){
		cloudResult = parse_result(&data);
	}
	cds_free(curl);
	cds_free(param);
	dstr_destroy(&url);
	dstr_destroy(&data);
	return cloudResult;			
}

static void free_vector_ptr(ptr_vector_t* _v)
{
	int i = 0;
	if(_v == NULL)
		return;
	for(i = 0; i< vector_size(_v); i++){
		char* tmp = (char*)ptr_vector_get(_v,i);
		if(tmp){
			cds_free(tmp);
		}
	}
}

static void free_serverAddress(ptr_vector_t* _v)
{
	int i = 0;
	if(_v == NULL)
		return ;
	for(i = 0; i< vector_size(_v); i++){
		WRTSERVER* s = ptr_vector_get(_v,i);
		if(s){
			str_free_content(&s->name);
			free_vector_ptr(&s->domain);
			vector_destroy(&s->domain);
			cds_free(s);
		}
	}
}

static void free_hostList(ptr_vector_t* _v)
{
		int i = 0;
	if(_v == NULL)
		return ;
	for(i = 0; i< vector_size(_v); i++){
		WRTHOST* h = ptr_vector_get(_v,i);
		if(h){
			str_free_content(&h->deviceCode);
			str_free_content(&h->talkID);
			cds_free(h);
		}
	}
}

void device_free_cloud_result(WRTCLOUDRESULT* cloudResult,int type)
{
	if(cloudResult == NULL)
		return;
	str_free_content(&cloudResult->reason);
	if(type == LOGIN){
		str_free_content(&cloudResult->result.loginResult.updateAddress);
		str_free_content(&cloudResult->result.loginResult.md5);
		str_free_content(&cloudResult->result.loginResult.caAddress);
		str_free_content(&cloudResult->result.loginResult.md5Ca);
		str_free_content(&cloudResult->result.loginResult.currentTime);
		str_free_content(&cloudResult->result.loginResult.devNum);
		str_free_content(&cloudResult->result.loginResult.callNum);
		free_serverAddress(&cloudResult->result.loginResult.serverAddress);
		ptr_vector_destroy(&cloudResult->result.loginResult.serverAddress);
	}else if(type == GETBIND){
		free_vector_ptr(&cloudResult->result.bindResult.userName);
		ptr_vector_destroy(&cloudResult->result.bindResult.userName);
	}else if(type == UPGRADE){
		str_free_content(&cloudResult->result.upgradeResult.filePath);
		str_free_content(&cloudResult->result.upgradeResult.md5Upgrade);
		str_free_content(&cloudResult->result.upgradeResult.version);
		str_free_content(&cloudResult->result.upgradeResult.remark);		
	}else if(type == HOSTLIST){
		free_hostList(&cloudResult->result.hostListResult.hostList);
		ptr_vector_destroy(&cloudResult->result.hostListResult.hostList);
	}
	cds_free(cloudResult);
}





int curl_test_get( const char* url)
{
	CURLcode res = -1;
	static CURL *handle = NULL;
	
	if (!url) {
		ERROR_LOG("BUG: no uri given\n");
		return -1;
	}
	if (!handle) handle = curl_easy_init(); 
	if (handle) {
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle,CURLOPT_CONNECTTIMEOUT,5000);
	//	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
	
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		 curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);

		/* Now run off and do what you've been told! */
		res = curl_easy_perform(handle);
		printf("res = %d \n",res);
	
	}

	return res;	
}