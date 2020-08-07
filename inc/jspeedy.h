#ifndef __JSPEEDYLIB_H__
#define	__JSPEEDYLIB_H__

//#include <EmvCommon.h>
//#include <CommonPub.h>
#define JSPEEDY_PATH_EMV                 0x01         
#define JSPEEDY_PATH_MAGSTRIPE           0x02         
#define JSPEEDY_PATH_LEGACY              0x03		 
/***************** CVM Method***********************/
#define JSPEEDY_CVM_ONLINE_PIN               0x01
#define JSPEEDY_CVM_SIGNATURE                0x02
#define JSPEEDY_CVM_ON_DEVICE                0x03

/************************* Structures ************************/
typedef struct {
    unsigned char AID[16];
    unsigned char AidLen;

    unsigned char CombinationOption[2];
	unsigned char STIP[3];

    unsigned char MaxTargetPer;
	unsigned char TargetPer;
	unsigned long Threshold;

	unsigned char bHasTransLimit;
	unsigned char bHasCVMLimit;
	unsigned char bHasFloorLimit;
    unsigned long FloorLimit;
    unsigned long TransLimit;
	unsigned long CVMLimit;
	
	unsigned char TACDenial[6];
    unsigned char TACOnline[6];
    unsigned char TACDefault[6];
	unsigned short RemovalTime;
	unsigned char reserved[128];
} JSPEEDY_APPLIST;

typedef struct {
	unsigned char OPS[8];				// Outcome Parameter Set
	unsigned char UIRD[22];				// User Interface Request Data
	unsigned char UIRRD[2];				// User Interface Request Restart Data
} JSPEEDY_OUTCOME;

/************************* APIs ****************************/
int JSpeedy_Init_Api(void);
void JSpeedy_PreProcess_Api(unsigned char *amount,unsigned char *cashback);
int JSpeedy_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus);
int JSpeedy_InitApp_Api(int *Path);
int JSpeedy_ReadAppData_Api(void);
int JSpeedy_RiskManagement_Api(void);
int JSpeedy_ProcRestrictions_Api(void);
int JSpeedy_TermActAnalyse_Api();
int JSpeedy_CardActAnalyse_Api(unsigned char *bIfMoveCard);
int JSpeedy_OfflineDataAuth_Api(void);
int JSpeedy_CardholderVerify_Api(unsigned char *Method);
int JSpeedy_CompletionOutcome_Api(unsigned char *bIfGoOnline,unsigned char *bIfPresentCard);
int JSpeedy_ProcIssuerUpdate_Api(int OnlineResult,unsigned char *RspCode,
			int IssuerAuthDataLen,unsigned char *IssuerAuthData,
			int IssuerScriptLen,unsigned char *IssuerScript);

void JSpeedy_GetOutcome_Api(JSPEEDY_OUTCOME *outcomeparam);
char *JSpeedy_GetVersion_Api(void);

int JSpeedy_AddApp_Api(JSPEEDY_APPLIST *App);
int JSpeedy_GetApp_Api(int Index, JSPEEDY_APPLIST *App);
int JSpeedy_DelApp_Api(unsigned char *AID, int AidLen);
void JSpeedy_ClearApp_Api(void);
void JSpeedy_UserCancel_Api(void);
#endif /* __JSPEEDYLIB_H__ */
