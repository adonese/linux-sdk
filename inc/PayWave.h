#ifndef __PAYWAVE_H__
#define	__PAYWAVE_H__

#define	PAYWAVE_PATH_QVSDC		1
#define	PAYWAVE_PATH_MSD		2

#define	PAYWAVE_CVM_NO_CVM		0
#define	PAYWAVE_CVM_SIGNATURE	1
#define	PAYWAVE_CVM_ONLINE_PIN	2

typedef struct {
	unsigned char AID[16];
	unsigned char AidLen;

	unsigned long TermFloorLimit;
	unsigned char SelFlag;

	unsigned char bStatusCheck;
	unsigned char bZeroAmtCheck;
	unsigned char ZeroAmtCheckOpt;
	unsigned char bTransLimitCheck;
	unsigned char bCVMLimitCheck;
	unsigned char bFloorLimitCheck;
	unsigned char bHasFloorLimit;

	unsigned long TransLimit;
	unsigned long CVMLimit;
	unsigned long FloorLimit;
	unsigned char reserved[128];

	// Runtime Variables
	unsigned char bCLAppNotAllowed;
	unsigned char TTQ[4];
} PAYWAVE_APPLIST;

typedef struct {
	unsigned char TTQ[4];

	unsigned char bCheckBlacklist;

	unsigned char bDRL;
	unsigned char bCashDRL;
	unsigned char bCashbackDRL;

	// Separate Reader Limit Set for PayWave Cash
	unsigned char CA_TTQ[4];
	unsigned char CA_bStatusCheck;
	unsigned char CA_bZeroAmtCheck;
	unsigned char CA_ZeroAmtCheckOpt;
	unsigned char CA_bTransLimitCheck;
	unsigned char CA_bCVMLimitCheck;
	unsigned char CA_bFloorLimitCheck;
	unsigned char CA_bHasFloorLimit;
	unsigned long CA_TransLimit;
	unsigned long CA_CVMLimit;
	unsigned long CA_FloorLimit;

	// Separate Reader Limit Set for PayWave Cashback
	unsigned char CB_TTQ[4];
	unsigned char CB_bStatusCheck;
	unsigned char CB_bZeroAmtCheck;
	unsigned char CB_ZeroAmtCheckOpt;
	unsigned char CB_bTransLimitCheck;
	unsigned char CB_bCVMLimitCheck;
	unsigned char CB_bFloorLimitCheck;
	unsigned char CB_bHasFloorLimit;
	unsigned long CB_TransLimit;
	unsigned long CB_CVMLimit;
	unsigned long CB_FloorLimit;

	unsigned char reserved[128];
} PAYWAVE_TERM_PARAM;

typedef struct {
	unsigned char Index[16];
	unsigned char IndexLen;

	unsigned char bStatusCheck;
	unsigned char bZeroAmtCheck;
	unsigned char ZeroAmtCheckOpt;
	unsigned char bTransLimitCheck;
	unsigned char bCVMLimitCheck;
	unsigned char bFloorLimitCheck;
	unsigned char bHasFloorLimit;
	unsigned long TransLimit;
	unsigned long CVMLimit;
	unsigned long FloorLimit;
} PAYWAVE_PID_PARAM;

char *PayWave_GetVersion_Api(void);
char *PayWave_GetCheckSum_Api(void);

int PayWave_Init_Api(void);

int PayWave_PreProcess_Api(unsigned char *AuthAmt,
						   unsigned char *CashBackAmt);
int PayWave_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus);
int PayWave_InitApp_Api(int *Path);
int PayWave_ReadAppData_Api(void);
int PayWave_ProcRestrictions_Api(void);
int PayWave_OfflineDataAuth_Api(void);
int PayWave_VerifyCardholder_Api(int *CVMType, int *NeedOnline);
int PayWave_Completion_Api(int OnlineStatus,
						   int IssuerAuthDataLen,
						   int IssuerScriptLen,
						   int *NeedIssuerUpdate);
int PayWave_ProcIssuerUpdate_Api(int IssuerAuthDataLen,
								 unsigned char *IssuerAuthData,
								 int IssuerScriptLen,
								 unsigned char *IssuerScript);
void PayWave_GetScriptResult_Api(int *ScriptResultLen,
								 unsigned char *ScriptResult);

void PayWave_GetParam_Api(PAYWAVE_TERM_PARAM *Param);
void PayWave_SetParam_Api(PAYWAVE_TERM_PARAM *Param);
void PayWave_SaveParam_Api(PAYWAVE_TERM_PARAM *Param);

int PayWave_AddApp_Api(PAYWAVE_APPLIST *App);
int PayWave_GetApp_Api(int Index, PAYWAVE_APPLIST *App);
int PayWave_DelApp_Api(unsigned char *AID, int AidLen);
void PayWave_ClearApp_Api(void);

int PayWave_AddPID_Api(PAYWAVE_PID_PARAM *Pid);
int PayWave_GetPID_Api(int Index, PAYWAVE_PID_PARAM *Pid);
int PayWave_DelPID_Api(unsigned char *Pid, int PidLen);
void PayWave_ClearPID_Api(void);


#endif /* __PAYWAVE_H__ */
