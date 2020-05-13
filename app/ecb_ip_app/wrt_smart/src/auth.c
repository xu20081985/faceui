#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sal_eXosip2.h"
#include "smartUac.h"
#include "private.h"
#include "auth.h"


SalAuthInfo* authInfoNew(const char* username,const char* passwd)
{
	SalAuthInfo* ai =sal_auth_info_new();
	if(username && (strlen(username) > 0)){
		ai->username = ms_strdup(username);
		ai->userid = ms_strdup(username);
	}
	if(passwd && (strlen(passwd) > 0))
		ai->password = ms_strdup(passwd);
	return ai;
}
void authInfoDestroy(SalAuthInfo* ai)
{
	if(ai==NULL)
		return;
	if(ai->username)
		ms_free(ai->username);
	ai->username = NULL;
	if(ai->userid)
		ms_free(ai->userid);
	ai->userid = NULL;
	if(ai->password)
		ms_free(ai->password);
	ai->password = NULL;
	if(ai->realm)
		ms_free(ai->realm);
	ai->realm = NULL;


	ms_free(ai);
	
}

static bool_t key_match(const char* tmp1,const char* tmp2)
{
	if(tmp1 == NULL && tmp2 == NULL) return TRUE;
	if(tmp1 != NULL && tmp2 != NULL && strcmp(tmp1,tmp2) == 0) return TRUE;
	return FALSE;
}

static char * remove_quotes(char * input){
	char *tmp;
	if (*input=='"') input++;
	tmp=strchr(input,'"');
	if (tmp) *tmp='\0';
	return input;
}

static int realm_match(const char *realm1, const char *realm2){
	if (realm1==NULL && realm2==NULL) return TRUE;
	if (realm1!=NULL && realm2!=NULL){
		if (strcmp(realm1,realm2)==0) return TRUE;
		else{
			char tmp1[128];
			char tmp2[128];
			char *p1,*p2;
			strncpy(tmp1,realm1,sizeof(tmp1)-1);
			strncpy(tmp2,realm2,sizeof(tmp2)-1);
			p1=remove_quotes(tmp1);
			p2=remove_quotes(tmp2);
			return strcmp(p1,p2)==0;
		}
	}
	return FALSE;
}

void authInfoAdd(smartUACCore* core,SalAuthInfo* ai)
{
	SalAuthInfo *ai2;
	MSList *elem;
	MSList *l;
	if(core->sal == NULL)
		return;
	/* find if we are attempting to modify an existing auth info */
	ai2=(SalAuthInfo*)authInfoFind(core,ai->realm,ai->username);
	if (ai2!=NULL){
		core->authList=ms_list_remove(core->authList,ai2);
		authInfoDestroy(ai2);
	}
	core->authList=ms_list_append(core->authList,ai);
	/* retry pending authentication operations */
	for(l=elem=sal_get_pending_auths(core->sal);elem!=NULL;elem=elem->next){
		const char *username,*realm;
		SalOp *op=(SalOp*)elem->data;
		SalAuthInfo *ai1;
		sal_op_get_auth_requested(op,&realm,&username);
		ai1=(SalAuthInfo*)authInfoFind(core,realm,username);
		if (ai1){
			SalAuthInfo sai;
			sai.username=ai->username;
			sai.userid=ai->userid;
			sai.realm=ai->realm;
			sai.password=ai->password;
			sal_op_authenticate(op,&sai);
		}
	}
	ms_list_free(l);
}
SalAuthInfo* authInfoFind(smartUACCore* core,const char* realm,const char* username)
{
	MSList *elem;
	SalAuthInfo *ret=NULL,*candidate=NULL;
	if(core->sal == NULL)
		return NULL;
	for (elem=core->authList;elem!=NULL;elem=elem->next){
		SalAuthInfo *pinfo=(SalAuthInfo*)elem->data;
		if (realm==NULL){
			/*return the authinfo for any realm provided that there is only one for that username*/
			if (key_match(pinfo->username,username)){
				if (ret!=NULL){
					ms_warning("There are several auth info for username '%s'",username);
					return NULL;
				}
				ret=pinfo;
			}
		}else{
			/*return the exact authinfo, or an authinfo for which realm was not supplied yet*/
			if (pinfo->realm!=NULL){
				if (realm_match(pinfo->realm,realm) 
					&& key_match(pinfo->username,username))
					ret=pinfo;
			}else{
				if (key_match(pinfo->username,username))
					candidate=pinfo;
			}
		}
	}
	if (ret==NULL && candidate!=NULL)
		ret=candidate;
	return ret;
}

void authInfoRemove(smartUACCore* core,SalAuthInfo* ai)
{
	SalAuthInfo *r = NULL;
	if(core->sal == NULL)
		return;
	r=(SalAuthInfo*)authInfoFind(core,ai->realm,ai->username);
	if (r){
		core->authList=ms_list_remove(core->authList,r);
		/*printf("len=%i newlen=%i\n",len,newlen);*/
		authInfoDestroy(r);
	}
}
void authInfoClear(smartUACCore* core)
{
	MSList *elem = NULL;
	int i;
	if(core->sal == NULL)
		return;
	for(i=0,elem=core->authList;elem!=NULL;elem=ms_list_next(elem),i++){
		SalAuthInfo *info=(SalAuthInfo*)elem->data;
		authInfoDestroy(info);
	}
	ms_list_free(core->authList);
	core->authList=NULL;
}