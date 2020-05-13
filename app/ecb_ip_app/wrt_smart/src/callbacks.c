/*
linphone
Copyright (C) 2010  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//#include <winsock2.h>
#include "port.h"
#include "sip_sal.h"
#include "sal_eXosip2.h"
#include "smartUac.h"
#include "private.h"
#include "auth.h"
#include "message.h"
#include "ortp/b64.h"


static void register_failure(SalOp *op, SalError error, SalReason reason, const char *details);


#if 0
static bool_t is_duplicate_call(LinphoneCore *lc, const LinphoneAddress *from, const LinphoneAddress *to){
	MSList *elem;
	for(elem=lc->calls;elem!=NULL;elem=elem->next){
		LinphoneCall *call=(LinphoneCall*)elem->data;
		if (linphone_address_weak_equal(call->log->from,from) &&
		    linphone_address_weak_equal(call->log->to, to)){
			return TRUE;
		}
	}
	return FALSE;
}
#endif




static void call_received(SalOp *h){
	sal_call_decline(h,SalReasonBusy,NULL);
	sal_op_release(h);
}

static void call_ringing(SalOp* h)
{
}

/*
 * could be reach :
 *  - when the call is accepted
 *  - when a request is accepted (pause, resume)
 */
static void call_accepted(SalOp *op){

}

static void call_ack(SalOp *op){
	
}


/* this callback is called when an incoming re-INVITE modifies the session*/
static void call_updating(SalOp *op){
}

static void call_terminated(SalOp *op, const char *from){
}

static void call_failure(SalOp *op, SalError error, SalReason sr, const char *details, int code){
	
}

static void call_released(SalOp *op){
	if(op)
		sal_op_release(op);
}

static void auth_requested(SalOp *h, const char *realm, const char *username){
	Sal *sal=sal_op_get_sal(h);
	smartUACCore* core = sal_get_user_pointer(sal);
	SalAuthInfo *ai=(SalAuthInfo*)authInfoFind(core,realm,username);
	
	ms_message("auth_requested() for realm=%s, username=%s",realm,username);
	if(ai){
		sal_op_authenticate(h,ai);
	}else
		sal_op_cancel_authentication(h);

}

static void auth_success(SalOp *h, const char *realm, const char *username){
	/*
	LinphoneCore *lc=(LinphoneCore *)sal_get_user_pointer(sal_op_get_sal(h));
	LinphoneAuthInfo *ai=(LinphoneAuthInfo*)linphone_core_find_auth_info(lc,realm,username);
	if (ai){
		ms_message("%s/%s authentication works.",realm,username);
		ai->works=TRUE;
	}
	*/
}

static void register_success(SalOp *op, bool_t registered){

	regState* reg = NULL;
	smartUACCore *core=(smartUACCore *)sal_get_user_pointer(sal_op_get_sal(op));
	core->rError = NOREGERROR;
	core->rstatus = registered?REGSUCCEEDED:UNREGSUCCEEDED;

	reg = ms_new0(regState,1);
	if(reg)
	{
		reg->state = registered?RegistrationOk:RegistrationCleared;
		sprintf(reg->proxy,sal_op_get_proxy(op));
		reg->reason = NOREGERROR; 
		if(core->smartUacCallback)
			core->smartUacCallback(registered?RegistrationOk:RegistrationCleared,reg,sizeof(regState));
		ms_free(reg);
		reg = NULL;
	}
}

static void register_failure(SalOp *op, SalError error, SalReason reason, const char *details){

	regState* reg = NULL;
	smartUACCore *core=(smartUACCore *)sal_get_user_pointer(sal_op_get_sal(op));
	if (error== SalErrorFailure && reason == SalReasonForbidden)
		core->rError = AUTHFAILED;
	else if(error == SalErrorNoResponse)
		core->rError = NORESPONSE;
	core->rstatus = REGFAILED;
	reg = ms_new0(regState,1);
	if(reg)
	{
		reg->state = RegistrationFailed;
		sprintf(reg->proxy,sal_op_get_proxy(op));
		reg->reason = core->rError; 
		if(core->smartUacCallback)
			core->smartUacCallback(RegistrationFailed,reg,sizeof(regState));
		ms_free(reg);
		reg = NULL;
	}
}

static void refer_received(Sal *sal, SalOp *op, const char *referto)
{
}

static void vfu_request(SalOp *op){
#ifdef VIDEO_ENABLED
	LinphoneCall *call=(LinphoneCall*)sal_op_get_user_pointer (op);
	if (call==NULL){
		ms_warning("VFU request but no call !");
		return ;
	}
	if (call->videostream)
		video_stream_send_vfu(call->videostream);
#endif
}

static void dtmf_received(SalOp *op, char dtmf){
#if 0
	LinphoneCore *lc=(LinphoneCore *)sal_get_user_pointer(sal_op_get_sal(op));
	LinphoneCall *call=(LinphoneCall*)sal_op_get_user_pointer(op);
	if (lc->vtable.dtmf_received != NULL)
		lc->vtable.dtmf_received(lc, call, dtmf);
#endif
}



static bool_t is_duplicate_msg(smartUACCore *lc, const char *msg_id){

	MSList *elem=lc->last_recv_msg_ids;
	MSList *tail=NULL;
	int i;
	bool_t is_duplicate=FALSE;
	for(i=0;elem!=NULL;elem=elem->next,i++){
		if (strcmp((const char*)elem->data,msg_id)==0){
			is_duplicate=TRUE;
		}
		tail=elem;
	}
	if (!is_duplicate){
		lc->last_recv_msg_ids=ms_list_prepend(lc->last_recv_msg_ids,ms_strdup(msg_id));
	}
	if (i>=10){
		ms_free(tail->data);
		lc->last_recv_msg_ids=ms_list_remove_link(lc->last_recv_msg_ids,tail);
	}
	return is_duplicate;

}


static void text_received(SalOp *op, const SalMessage *msg){

	smartUACCore *lc=(smartUACCore *)sal_get_user_pointer(sal_op_get_sal(op));
	if (is_duplicate_msg(lc,msg->message_id)==FALSE){
		int len = 0;
		int b64delen = 0;
		int totallen = 0;
		int whole =0;
		int rebytes = 0;
		int declen = 0;
		instantMessage* im  = NULL;
		char* strFrom = NULL;
		SalAddress* fromaddr =NULL;
		if(msg->text == NULL){
				return;
		}
		fromaddr= sal_address_new(msg->from);
		if(fromaddr == NULL)
			return;
		sal_address_clean(fromaddr);
		strFrom = sal_address_as_string(fromaddr);
		sal_address_destroy(fromaddr);
		fromaddr = NULL;
		if(strFrom == NULL)
			return;
	
		len = strlen(msg->text);
		whole = (len /NUM_ENCODED_DATA_BYTES);
		rebytes = (len % NUM_ENCODED_DATA_BYTES);
		b64delen =  (whole + (0 != rebytes))*NUM_PLAIN_DATA_BYTES;


	    im  = (instantMessage*)ms_malloc0(sizeof(instantMessage)+b64delen+1);
		if(im == NULL){
			ms_free(strFrom);
			return;
		}
		if(msg->url)
			strcpy(im->external_url,msg->url);
		strcpy(im->remotename,strFrom);
		//strcpy(im->msg,msg->text);
		declen = b64_decode(msg->text,len,im->msg,b64delen);
		im->msgstatus = (int)NewMessageDeliver;
		im->msglen = declen;
		im->usrdata = 0;

		if(lc->smartUacCallback)
		{
			lc->smartUacCallback(NewMessageDeliver,(void*)im,sizeof(instantMessage)+len+1);
		}
		ms_free(im);
		im = NULL;
		ms_free(strFrom);
		
	}

}

static void notify(SalOp *op, const char *from, const char *msg){
#if 1
	ms_message("get a Event: %s notify from %s ,it is not body!",msg,from);
#endif
}

static void notify_presence(SalOp *op, SalSubscribeStatus ss, SalPresenceStatus status, const char *msg){

}

static void subscribe_received(SalOp *op, const char *from){

	ms_message("receved %s sub ,but I not support!\n",from);
	sal_subscribe_decline(op);

}

static void subscribe_closed(SalOp *op, const char *from){
	sal_op_release(op); //表示收到外部订阅，由于某种原因被关闭。所以释放掉op;
}

static void ping_reply(SalOp *op){

}

static void pub_reply(SalOp *salop,const char* eventName,SalPublishStatus status)
{
	if(status != SalPublishProgress){
		globalState state = PublishFailed;
		smartUACCore* core = sal_op_get_user_pointer(salop);
		if(core){
			if(status == SalPublishOK)
				state = PublishOK;
			if(core->smartUacCallback)
				core->smartUacCallback(state,(void*)eventName,strlen(eventName));
			outPubRemove(core,salop);
		}
	}

}

static void notify_refer(SalOp *op, SalReferStatus status){

}

static globalState chatStatusSal2Linphone(SalTextDeliveryStatus status){

	if(status ==  SalTextDeliveryInProgress)
				return MessageDeliveryInProgress;
	else if(status == SalTextDeliveryDone)
		        return MessageDeliverOk;
	else if(status == SalTextDeliveryFailed)
				return MessageDeliverFailed;
}

#if 0
static int op_equals(LinphoneCall *a, SalOp *b) {
	return a->op !=b; /*return 0 if equals*/
}
#endif
static void text_delivery_update(SalOp *op, SalTextDeliveryStatus status){
	smartUACCore* core = (smartUACCore*)sal_op_get_user_pointer(op);
	if(core && op)
	{
		instantMessageSave* im = findInstantMessageSave(core,op);
		if(im){
			instantMessage* _im = (instantMessage*)ms_malloc0(sizeof(instantMessage)+im->msglen+1);
			if(_im){
				strcpy(_im->external_url ,im->external_url);
				strcpy(_im->remotename,im->remotename);
				strcpy(_im->msg,im->msg);
				_im->msglen = im->msglen;
				_im->usrdata = im->msglen;
				_im->msgstatus = (int)chatStatusSal2Linphone(status);
				if(core->smartUacCallback)
					core->smartUacCallback(chatStatusSal2Linphone(status),(void*)_im,sizeof(instantMessage)+im->msglen+1);
				ms_free(_im);
			}
			instantMessageSaveRemove(core,op);
		}
		sal_op_release(op);
	}

}
static void call_recv_info(SalOp* op,const char *info)
{

}
//2013-12-05 ljw
static void notify_event(SalOp* op,SalSubscribeStatus ss,  const char* body,int len ,const char* eventName){

	smartUACCore* core = (smartUACCore*)sal_op_get_user_pointer(op);
	if(core == NULL)
		return;
	if(ss == SalSubscribeActive && body ){

		notifyState* notify = (notifyState*)ms_malloc0(len+1+sizeof(notifyState));
		if(notify == NULL)
			return;
		strcpy(notify->eventname,eventName);
		notify->status = (int)notifyArrived;
		notify->contentsize = len;
		memcpy(notify->content,body,len);
		if(core->smartUacCallback)
			core->smartUacCallback(notifyArrived,(void*)notify,len+1+sizeof(notifyState));
		ms_free(notify);
		notify = NULL;
			
	}else if(ss == SalSubscribeActive  && body == NULL)
	{
		notifyState* notify = (notifyState*)ms_malloc0(sizeof(notifyState));
		if(notify == NULL)
			return;
		strcpy(notify->eventname,eventName);
		notify->status = (int)SubscribeActive;
		notify->contentsize = 0;
		if(core->smartUacCallback)
			core->smartUacCallback(notifyArrived,(void*)notify,sizeof(notifyState));
		ms_free(notify);
		notify = NULL;
	}else if(ss == SalSubscribeTerminated){
		notifyState* notify = (notifyState*)ms_malloc0(sizeof(notifyState));
		if(notify == NULL)
			return;
		strcpy(notify->eventname,eventName);
		notify->status = (int)SubscribeTerminated;
		notify->contentsize = 0;
		if(core->smartUacCallback)
			core->smartUacCallback(notifyArrived,(void*)notify,sizeof(notifyState));
		ms_free(notify);
		notify = NULL;
		outSubRemove(core,op);
	}

}

SalCallbacks linphone_sal_callbacks={
	call_received,
	call_ringing,
	call_accepted,
	call_ack,
	call_updating,
	call_terminated,
	call_failure,
	call_released,
	auth_requested,
	auth_success,
	register_success,
	register_failure,
	vfu_request,
	dtmf_received,
	refer_received,
	text_received,
	text_delivery_update,
	notify,
	notify_presence,
	notify_refer,
	subscribe_received,
	subscribe_closed,
	ping_reply,
	call_recv_info,
	notify_event, //2013-12-05 ljw
	pub_reply
};


