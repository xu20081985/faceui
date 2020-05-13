
#include "port.h"
#include "sip_sal.h"
#include "sal_eXosip2.h"
#include "smartUac.h"
#include "private.h"




instantMessageSave* findInstantMessageSave(smartUACCore* core ,SalOp* op)
{
	MSList *elem;
	instantMessageSave* tim;
	if(core == NULL || op == NULL)
		return NULL;
	for (elem=core->msgList;elem!=NULL;elem=elem->next){
			tim = (instantMessageSave*)elem->data;
			if(tim && (tim->op == op)){
					return tim;
			}
	}
	return NULL;

}

void instantMessageSaveDestroy(instantMessageSave* im)
{
	 ms_free(im);
}


void instantMessageSaveRemove(smartUACCore* core, SalOp* op)
{
	instantMessageSave *r = NULL;
	if(core == NULL)
		return;
	r=(instantMessageSave*)findInstantMessageSave(core,op);
	if (r){
		core->msgList=ms_list_remove(core->msgList,r);
		/*printf("len=%i newlen=%i\n",len,newlen);*/
		instantMessageSaveDestroy(r);
	}
}

void instantMessageSaveClear(smartUACCore* core)
{
	MSList *elem = NULL;
	int i;
	if(core == NULL)
		return;
	for(i=0,elem=core->msgList;elem!=NULL;elem=ms_list_next(elem),i++){
		instantMessageSave *info=(instantMessageSave*)elem->data;
		instantMessageSaveDestroy(info);
	}
	ms_list_free(core->msgList);
	core->msgList=NULL;
}