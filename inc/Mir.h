#ifndef __MIR_H__
#define	__MIR_H__

#include "EmvCommon.h"


#define	MIR_PATH_PROCTOCOL1		1
#define	MIR_PATH_PROCTOCOL2		2

#define	MIR_CVM_NO_CVM			0x00
#define	MIR_CVM_SIGNATURE			0x01
#define	MIR_CVM_ONLINE_PIN		0x02
#define	MIR_CVM_MOBILE			0x03
#define	MIR_CVM_NA				0x0F



typedef struct {
	unsigned char ucAID[16];
	unsigned char ucAidLen;

	unsigned char ucVersion[2];
	//unsigned long ulTermFloorLimit;


	unsigned char ucKernelID;
	unsigned char KernelConfig;

	//unsigned long TransLimitODCVM;
	//unsigned long TransLimitNoODCVM;


	
	unsigned char ucCLCDCVMTransLimitCheck;
	unsigned char ucCLNOCDCVMTransLimitCheck;	
	unsigned char ucNoCVMLimitCheck;	
	unsigned char ucFloorLimitCheck;

	unsigned long ulNoCVMLimit;	
	unsigned long ulFloorLimit;
	unsigned long ulCLCDCVM_TransLimit;
	unsigned long ulCLNOCDCVM_TransLimit;	

	unsigned char ucTACDenial[5];
	unsigned char ucTACOnline[5];
	unsigned char ucTACDefault[5];

	//add priority for Term AID
	unsigned char priority;
	unsigned char ucReserved[127];
} MIR_APPLIST;




typedef struct {
	
	unsigned char ucCheckBlacklist;
	unsigned char ucForceOnline;

	unsigned char ucDelayedAuthFlag;
	//unsigned char ucMagStripeOnlyFlag;
	
	unsigned char TermCapability[3];//Term capability


	unsigned char ucTranRecoveryLimit;
	unsigned char ucTPMCap[2];

	unsigned char ucDETagList[256];

	//add Auto debit selection(Priority to select some AID)
	unsigned char EnAutoSel;
	unsigned char reserved[127];
} MIR_TERM_PARAM;


typedef struct {
	unsigned char OPS[2];			//  byte 1: status(approve, decline...), 	byte 2: CVM(online pin, no cvm...).
	unsigned char UI[2];			//  byte 1: message id('03' ("Approved") ...)	byte 2: status(ready to read, card read successfully)
	unsigned char UIRS[2];			//  byte 1: message id('03' ("Approved") ...)	byte 2: status(ready to read, card read successfully)
	unsigned char EI;				//  Error indicitor(09, data error  |  06, gac no answer...)

} MIR_OUTCOME;


char *Mir_GetVersion_Api(void);
char *Mir_GetCheckSum_Api(void);

int Mir_Init_Api(void);
int Mir_PreProcess_Api(unsigned char *AuthAmt, unsigned char *CashbackAmt);
int Mir_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus, int* pPath);
int Mir_InitApp_Api();
int Mir_ReadAppData_Api(void);

int Mir_ProcRestrictions_Api(void);
int Mir_OfflineDataAuth_Api(void);
int Mir_VerifyCardholder_Api(int *CVMType);
int Mir_RiskManagement_Api(void);
int Mir_TermActAnalyse_Api(int* pnNeedOnline);
//int Mir_CardActAnalyse_Api(void);
//int Mir_CompleteTrans_Api(int *NeedOnline);

//Param
void Mir_GetParam_Api(MIR_TERM_PARAM *Param);
void Mir_SetParam_Api(MIR_TERM_PARAM *Param);
void Mir_SaveParam_Api(MIR_TERM_PARAM *Param);



int Mir_AddApp_Api(MIR_APPLIST *App);
int Mir_GetApp_Api(int Index, MIR_APPLIST *App);
int Mir_DelApp_Api(unsigned char *AID, int AidLen);
void Mir_ClearApp_Api(void);


void Mir_GetOutcome_Api(MIR_OUTCOME *outcome);

#endif /* __MIR_H__ */
