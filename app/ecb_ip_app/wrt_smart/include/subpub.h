#ifndef SUBPUB_H_
#define SUBPUB_H_


#ifdef __cplusplus
extern "C"{
#endif

SalOp* findOutSub(smartUACCore* core ,SalOp* op);
void outSubDestroy(SalOp* op);
void outSubRemove(smartUACCore* core, SalOp* op);
void outSubClear(smartUACCore* core);

SalOp* findOutPub(smartUACCore* core ,SalOp* op);
void outPubDestroy(SalOp* op);
void outPubRemove(smartUACCore* core, SalOp* op);
void outPubClear(smartUACCore* core);

void eventNameClear(smartUACCore* core);
void eventNameRemove(smartUACCore* core, const char* eventName);
const char* eventNameFind(smartUACCore* core,const char* eventName);
void eventNameAdd(smartUACCore* core,const char* eventName);
#ifdef __cplusplus
}
#endif


#endif