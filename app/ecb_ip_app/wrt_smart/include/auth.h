#ifndef AUTH_H_
#define AUTH_H_




SalAuthInfo* authInfoNew(const char* username,const char* passwd);
void authInfoDestroy(SalAuthInfo* ai);

void authInfoAdd(smartUACCore* core,SalAuthInfo* ai);
SalAuthInfo* authInfoFind(smartUACCore* core,const char* realm,const char*  username);
void authInfoRemove(smartUACCore* core,SalAuthInfo* ai);
void authInfoClear(smartUACCore* core);




#endif