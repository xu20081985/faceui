#include "port.h"
#include "sip_sal.h"
#include "sal_eXosip2.h"
#include "smartUac.h"
#include "private.h"
#include "subpub.h"


SalOp* findOutSub(smartUACCore* core ,SalOp* op)
{
	MSList *elem;
	SalOp* op1;
	if(core == NULL || op == NULL)
		return NULL;
	for (elem=core->outSubList;elem!=NULL;elem=elem->next){
			op1 = (SalOp*)elem->data;
			if(op1 && (op1 == op)){
					return op1;
			}
	}
	return NULL;

}

void outSubDestroy(SalOp* op)
{
	sal_op_release(op);
}


void outSubRemove(smartUACCore* core, SalOp* op)
{
	SalOp *r = NULL;
	if(core == NULL)
		return;
	r=(SalOp*)findOutSub(core,op);
	if (r){
		core->outSubList=ms_list_remove(core->outSubList,r);
		/*printf("len=%i newlen=%i\n",len,newlen);*/
		outSubDestroy(r);
	}
}

void outSubClear(smartUACCore* core)
{
	MSList *elem = NULL;
	int i;
	if(core == NULL)
		return;
	for(i=0,elem=core->outSubList;elem!=NULL;elem=ms_list_next(elem),i++){
		SalOp *info=(SalOp*)elem->data;
		outSubDestroy(info);
	}
	ms_list_free(core->outSubList);
	core->outSubList=NULL;
}

//---------------------------------------------------------------------------

SalOp* findOutPub(smartUACCore* core ,SalOp* op)
{
	MSList *elem;
	SalOp* op1;
	if(core == NULL || op == NULL)
		return NULL;
	for (elem=core->outPubList;elem!=NULL;elem=elem->next){
			op1 = (SalOp*)elem->data;
			if(op1 && (op1 == op)){
					return op1;
			}
	}
	return NULL;

}

void outPubDestroy(SalOp* op)
{
	sal_op_release(op);
}


void outPubRemove(smartUACCore* core, SalOp* op)
{
	SalOp *r = NULL;
	if(core == NULL)
		return;
	r=(SalOp*)findOutPub(core,op);
	if (r){
		core->outPubList=ms_list_remove(core->outPubList,r);
		/*printf("len=%i newlen=%i\n",len,newlen);*/
		outPubDestroy(r);
	}
}

void outPubClear(smartUACCore* core)
{
	MSList *elem = NULL;
	int i;
	if(core == NULL)
		return;
	for(i=0,elem=core->outPubList;elem!=NULL;elem=ms_list_next(elem),i++){
		SalOp *info=(SalOp*)elem->data;
		outPubDestroy(info);
	}
	ms_list_free(core->outPubList);
	core->outPubList=NULL;
}


void eventNameAdd(smartUACCore* core,const char* eventName)
{
	core->eventNameList = ms_list_append(core->eventNameList,ms_strdup(eventName));
}

void eventNameDestroy(const char* eventName)
{
	ms_free(eventName);

}
const char* eventNameFind(smartUACCore* core ,const char* eventName)
{
	MSList *elem;
	char* _event;
	if(core == NULL || eventName == NULL)
		return NULL;
	for (elem=core->eventNameList;elem!=NULL;elem=elem->next){
			_event = (char*)elem->data;
			if(_event && (strcasecmp(_event,eventName) == 0)){
					return _event;
			}
	}
	return NULL;

}

void eventNameRemove(smartUACCore* core, const char* eventName)
{
	const char *r = NULL;
	if(core == NULL)
		return;
	r=(const char *)eventNameFind(core,eventName);
	if (r){
		core->eventNameList=ms_list_remove(core->eventNameList,r);
		/*printf("len=%i newlen=%i\n",len,newlen);*/
		eventNameDestroy(r);
	}
}

void eventNameClear(smartUACCore* core)
{
	MSList *elem = NULL;
	int i;
	if(core == NULL)
		return;
	for(i=0,elem=core->eventNameList;elem!=NULL;elem=ms_list_next(elem),i++){
		const char *info=(const char*)elem->data;
		eventNameDestroy(info);
	}
	ms_list_free(core->eventNameList);
	core->eventNameList=NULL;
}