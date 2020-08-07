#ifndef __PAYPASS_H__
#define	__PAYPASS_H__

#define	PAYPASS_PATH_MCHIP		1
#define	PAYPASS_PATH_MAGSTRIPE	2

#define	PAYPASS_CVM_NO_CVM			0x00
#define	PAYPASS_CVM_SIGNATURE		0x01
#define	PAYPASS_CVM_ONLINE_PIN		0x02
#define	PAYPASS_CVM_CCV				0x03
#define	PAYPASS_CVM_NA				0x0F


typedef struct {
	unsigned char AID[16];
	unsigned char AidLen;

	unsigned char Version[2];
	unsigned char MagStripeAVN[2];

	unsigned char RiskManData[9];		// LEN + DATA

	unsigned char KernelID;
	unsigned char KernelConfig;

	unsigned char CVMCapabilityCVM;
	unsigned char CVMCapabilityNoCVM;
	unsigned char MagCVMCapabilityCVM;
	unsigned char MagCVMCapabilityNoCVM;

	unsigned long TransLimitODCVM;
	unsigned long TransLimitNoODCVM;
	unsigned long CVMLimit;
	unsigned long FloorLimit;

	unsigned char TACDenial[6];
	unsigned char TACOnline[6];
	unsigned char TACDefault[6];

	unsigned char uDOL[128];

	unsigned char reserved[128];
} PAYPASS_APPLIST;

typedef struct {
	unsigned char SecurityCapability;
	unsigned char CardDataInputCapability;
	unsigned char ExCapability[5];
	unsigned char TransCategoryCode;
	unsigned char MaxTornTransNum;
	unsigned char MaxTornTransTime[2];
	unsigned char MessageHoldTime[3];
	unsigned char ReadBalanceBeforeGenAC;
	unsigned char ReadBalanceAfterGenAC;
	unsigned char MerchantCustomData[21];		// LEN + DATA

	unsigned char ExpRRTCAPDU[2];
	unsigned char ExpRRTRAPDU[2];
	unsigned char MinRRTGrace[2];
	unsigned char MaxRRTGrace[2];
	unsigned char RRTThresholdM;
	unsigned char RRTThresholdA[2];
	unsigned char AccountType;
	unsigned char BalanceBG[6];
	unsigned char BalanceAG[6];
	unsigned char reserved[104];
} PAYPASS_TERM_PARAM;

typedef struct _tag_PAYPASS_TORN_RECORD {
	int valid;

	unsigned char amount1Len;
	unsigned char amount1[6];

	unsigned char amount2Len;
	unsigned char amount2[6];

	unsigned char panLen;
	unsigned char pan[10];

	unsigned char panSequenceLen;
	unsigned char panSequence;

	unsigned char readBalanceBeforeGAC;
	unsigned char balanceBeforeGAC[6];

	unsigned char CVMResultLen;
	unsigned char CVMResult[3];

	unsigned char IDSNLen;
	unsigned char IDSN[8];

	unsigned char RCP;

	unsigned char capabilitiesLen;
	unsigned char capabilities[3];

	unsigned char countryCodeLen;
	unsigned char countryCode[2];

	unsigned char termTypeLen;
	unsigned char termType;

	unsigned char TVResultLen;
	unsigned char TVResult[5];

	unsigned char transCategoryLen;
	unsigned char transCategory;

	unsigned char transCurrencyLen;
	unsigned char transCurrency[2];

	unsigned char transDateLen;
	unsigned char transDate[3];

	unsigned char transTimeLen;
	unsigned char transTime[3];

	unsigned char transTypeLen;
	unsigned char transType;

	unsigned char unpreNumberLen;
	unsigned char unpreNumber[4];

	unsigned char PDOLDataLen;
	unsigned char PDOLData[256];

	unsigned char CDOL1DataLen;
	unsigned char CDOL1Data[256];

	unsigned char DRDOLDataLen;
	unsigned char DRDOLData[256];

	struct _tag_PAYPASS_TORN_RECORD *prev;
	struct _tag_PAYPASS_TORN_RECORD *next;
} PAYPASS_TORN_RECORD;

typedef struct {
	unsigned char OPS[8];
	unsigned char UIRD[22];
	unsigned char EI[6];

	unsigned char balance_pg[6];
	unsigned char balance_ag[6];

	int DDCardLen_Track1;
	unsigned char DDCard_Track1[56];

	int DDCardLen_Track2;
	unsigned char DDCard_Track2[16];

	PAYPASS_TORN_RECORD *tornRecord;
} PAYPASS_OUTCOME;

char *PayPass_GetVersion_Api(void);

int PayPass_Init_Api(void);
int PayPass_PreProcess_Api(unsigned char *AuthAmt, unsigned char *CashbackAmt);
int PayPass_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus);
int PayPass_InitApp_Api(int *Path);
int PayPass_ReadAppData_Api(void);

int PayPass_ProcRestrictions_Api(void);
int PayPass_VerifyCardholder_Api(int *CVMType);
int PayPass_TermActAnalyse_Api(void);
int PayPass_CardActAnalyse_Api(int *CanRemoveCard);
int PayPass_CompleteTrans_Api(int *NeedOnline);

int PayPass_ProcMSTrans_Api(void);
int PayPass_CompleteMSTrans_Api(int *CVMType);

void PayPass_GetOutcome_Api(PAYPASS_OUTCOME *outcome);
PAYPASS_TORN_RECORD *PayPass_CleanTornRecord_Api(void);
void PayPass_CleanAllTornRecord_Api(void);

void PayPass_GetParam_Api(PAYPASS_TERM_PARAM *Param);
void PayPass_SetParam_Api(PAYPASS_TERM_PARAM *Param);
void PayPass_SaveParam_Api(PAYPASS_TERM_PARAM *Param);

int PayPass_AddApp_Api(PAYPASS_APPLIST *App);
int PayPass_GetApp_Api(int Index, PAYPASS_APPLIST *App);
int PayPass_DelApp_Api(unsigned char *AID, int AidLen);
void PayPass_ClearApp_Api(void);


#endif /* __PAYPASS_H__ */
