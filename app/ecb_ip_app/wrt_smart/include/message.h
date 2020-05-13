#ifndef MESSAGE_H_
#define MESSAGE_H_

#ifdef __cplusplus
extern "C"{
#endif

	void instantMessageSaveClear(smartUACCore* core);
	void instantMessageSaveRemove(smartUACCore* core,struct SalOp* op);
	void instantMessageSaveDestroy(instantMessageSave* im);
	instantMessageSave* findInstantMessageSave(smartUACCore* core ,SalOp* op);

#ifdef __cplusplus
}
#endif

#endif