#ifndef AFX_FUNC_H
#define AFX_FUNC_H

#define BEEPERROR			1		 
#define BEEPNORMAL      	0	

#define LAST_REC_LOG	0xffffffff

#define		PIN_PED			0X00	 
#define		PIN_PP			0X01	 

#define		TIMEOUT			-2	

#define		MAX_LCDWIDTH				21
#define		MCARDNO_MAX_LEN 			19	
#define		INPUT_TIMEOUT_VAL			30  


#define  MAX_OPER_NUM 			100   

#define ENGLISH_LANG		2
#define ARABIC_LANG		1

#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define BLACK  0x0000
#define BLUE  0x1A3C


enum{			 
	POS_PURCHASE = 1,
	POS_PURCHASE_REFUND,			 
	POS_WORKING_KEY,
	POS_PAPER_OUT					 			 
};  


typedef struct {
	u16 nOptNO[MAX_OPER_NUM];		 
	char sOptPwd[MAX_OPER_NUM][8];	 
	u16 nCurOptNO;					 				 
}OPER_STRC;


typedef struct {
	u8 ucRecFalg;					  					 
	int IccOnline;					 
	u8 IccFallBack;					 
	u16 nIccDataLen;				 
	u8 Trans_id;					 				 
	char MainAcc[22];				 
	u8 TradeAmount[6+1];
	double tranamount;	 
	u8 TradeDate[4];				 
	u8 TradeTime[3];				 
	u8 OperatorNo;					 
	u32 lTraceNo;				 	 
	u32 lNowBatchNum;				 
	char szRespCode[2+1];       	 	 
	u8 ExpDate[4];					 
	u8 EntryMode[4];				 			 
	char SysReferNo[13];			 
	char AuthCode[7];				 
	char TerminalNo[9];				 
	char MerchantNo[16];			 
	char MerchTermNO[5];			 		 
	u8 SecondAmount[6];				 			 
	char SecondAcc[21];				  
	char HoldCardName[20+1];		 			 				 
	char AddInfo[122+1];			 
	u32 OldTraceNo;					 
	u32 OldBatchNum;				 
	char OldTransDate[9];			 
	char OldSysRefNo[13];			 
	char OldTermNo[9];				 
	// EMV                   
	u8	IccData[1+255];				 
	char szCardUnit[4];  			 
	u8	bPanSeqNoOk;				 
	u8	ucPanSeqNo;					 
	u8	sAppCrypto[8];				 
	u8	sAuthRspCode[2];             
	u8	sTVR[5];                     
	u8	sAIP[2];                     
	u8  szUnknowNum[4];              
	char szAID[32+1];                
	char szAppLable[16+1];           
	u8	sTSI[2];                     
	u8	sATC[2];                     
	u8	szAppPreferName[16+1];     

	char ClientId[32];				 
	char ReferenceNo[13];				 
	fp64 TranFee;
	int ResponseCode;
	char ResponseStatus[31];
	char ResponseMessage[101];
	char ApprovalCode[20];
	char WKey[17];
	u32 RefundTraceNo;
	u32 ResTraceNo;

} LOG_STRC;                          
#define LOG_SIZE  sizeof(LOG_STRC)   

typedef struct{  
	u8 sPIN[9];						    	 
	u8 BalanceAmount[1+40];			 
	u8 Track1[88]; 					 
	u8 Track2[2+37]; 				 
	u8 Track3[2+107];				 
	u8 Track1Len;					 
	u8 Track2Len;					 
	u8 Track3Len;					 		  
	LOG_STRC stTrans;				                            
	u8 HaveInputAmt;				 
	u8 HaveInputPin;				 
	u16 nRespIccLen;				 
	u8 RespIccData[512];
}POS_COM;

struct _CtrlParam{    
	u8		pinpad_type;			 
	u8		AKeyIndes;				 
	u8		MainKeyIdx;				 
	u8		PinKeyIndes;			 
	u8		MacKeyIndes;			 			 
	u32		lTraceNo;				 
	u32		lNowBatchNum;			 		 
	u16		iTransNum;				                             
	u8		beepForInput;			 
	u8		oprTimeoutValue;		 
	u8		tradeTimeoutValue;		 	 		 		 		 
	char	TerminalNo[9];			 
	char	MerchantNo[16];			 
	char	MerchantName[41]; 	 	 			 				 		 		 		 
	u8		DesType;				 
	u8		PreDial;				 
	u8		ShieldPAN;	 			 			 			 			 	                                  
	u8		SupportICC;				 			 			 		 
	u8		SupportPICC;	
	u8		SupportSignPad;			 
	u8		SignTimeoutS;			 
	u16		SingRecNum;				 
	u8		SignMaxNum;				 
	u16		SignBagMaxLen;	

	char	ClientId[32];
	u8 		Lang;
};                                   
EXTERN struct _CtrlParam gCtrlParam; 



int GetAmount(u8 *pAmt); 
int EnterPIN(u8 flag);

void DispTradeTitle(u8 TranId); 
int ConverTranNameCh(u8 TranId, char *TransName);
int ConverTranNameEn(u8 TranId, char *TransName); 
void ConvErrCode(int errCode, char *szErrInfo); 
int WaitEvent(void); 
void TraceNoInc(void);
int GetCoreVerSion(char *CoreFlag,char *VerSion);
 
int  SysManagerManu(void);
void CommDebugInfo(char *title, unsigned char *Date, int DateLen, int flag);

void ACTSLogo(void);
int SelectMainMenu(void);


void InitOper(void);


int ReadOperFile(void);
int SaveOperFile(void);


/*****************ERROR MESSAGE****************************/
#define  E_TRANS_FAIL		2		 
#define  E_NO_TRANS			3		 
#define  E_MAKE_PACKET		4		 
#define  E_ERR_CONNECT		5		 
#define  E_SEND_PACKET		6		 
#define  E_RECV_PACKET		7		 
#define  E_RESOLVE_PACKET	8		 
#define  E_REVERSE_FAIL		9		 
#define  E_NO_OLD_TRANS		10		 
#define  E_TRANS_VOIDED		11		 
#define  E_ERR_SWIPE		12		 
#define  E_MEM_ERR			13		 
#define  E_PINPAD_KEY		14		 
#define  E_FILE_OPEN		15		 
#define  E_FILE_SEEK		16		 
#define  E_FILE_READ		17		 
#define  E_FILE_WRITE		18		 
#define  E_CHECK_MAC_VALUE	19		 
#define  E_TRANS_CANCEL		20		 
#define	 E_MAC				21		 
#define  E_SYS				22		 
#define  E_FAILURE			23		 
#define	 E_REVTIMEOUT		24		 
#define	 E_PPNORESP			25		 

#define  NO_DISP          26         

enum
{
	PAN_KEYIN =		0x01,	 
	PAN_MAGCARD =	0x02,	 
	PAN_ICCARD =	0x05,	 
	PAN_PAYWAVE =	0x07,	 
	PAN_PAYPASS =	0x91,	 
	PAN_MIR =	0x07,
	PAN_JSPEEDY = 0x08
	//PAN_PICCFULL =	0x98,	 
};                           

enum
{                                
	PIN_HAVE_INPUT =	0x10,	 
	PIN_NOT_INPUT =		0x20,	 
}; 

enum	 
{
	RECORDNORMAL	=	0x00,	 
	RECORDVOID		=	0x01,	 
	RECORDHAVEUP	=	0x02	 
};


int GetCardNoFromTrack2Data(char *cardNo, u8 *track2Data);

int ShowMenuItem(char *Title, const char *menu[], u8 ucLines, u8 ucStartKey, u8 ucEndKey, int IsShowX, u8 ucTimeOut);

char GetNameFromTrack1(char *Inbuf, char *Name);


void InitPosCom(void);

void DispTitle(char* Title);

void WaitRemoveICC(void);

void DisplayProcessing(void);

int GetScanf(u32 mode, int Min, int Max, char *outBuf, u32 TimeOut, u8 StartRow, u8 EndRow, int howstr);

int GetTraceAuditNo(u32 *num);

int TerminalParaMenu(void);

int SetTerminalId(void);

int SetClientId(void);

int SetSystemTraceNo(void);

void MakeTimeDispBuf(char *pDisp, char *pStrTime);

int SetMaskterKeyValue(int flag);

void SetPinkey(char *wKey, int flag);

int LanguageMenu(void);

int SetLanguage(int lang);

int SetHostAddress(void);

int SetGprsType(void);

int NetworkMenu(void);

void LDispTitle_AE(char* eng, char* ar);

void Lstrcpy(char* des, char* strE, char* strA);

void ScrDispTitle_AE(int row ,int col ,const char *str  , unsigned char atr);

void LScrDispRam_Api(int row ,int col ,const char *str  , unsigned char atr);

void LPrompt(char* msgE, char* msgA);

void LScrDisp(int row ,int col ,const char *str  , unsigned char atr);

void LScrDisp_AE(int row ,int col ,const char *strE,  const char *strA, unsigned char atr);

void GetPanNumber();

#endif


