#ifndef _FILE_H_
#define _FILE_H_

#define COMMPARAMFILE   "CommParamFile"	
#define CtrlPARAMFILE   "CtrlParamFile"	
#define RECORDLOG		"record"
#define DUPFILE			"dup_file"
#define FILE_OPER_LOG	"oper.log"                                               
#define SIGNFILE		"sign_file"
                      
typedef struct                                   
{                                    
	u16 nSrcTradeLocate;			 
	char sHolderPhone[16];			 
	u8 reserved[4];                  
	u16 signlen;					 
	u8 signdata[1000];				 
}SIGNLOG;
#define SIGNLOGSIZE  sizeof(SIGNLOG)
 
void InitCommParam(void);
 
void SaveCommParam(void);
 
void ReadCommParam(void);
 
void IsDelfile( char *FileName );
 
void SaveCtrlParam(void);
 
void ReadCtrlParam(void);
 
void InitCtrlParam(void);
 
int ReadLog( LOG_STRC *pLog, int logNo );
 
void DelLog(int bClearTraceNo);
 
int SaveLogFile(void);
 
int UpdateLogFile(void);
 
int ReadReversalData(void);
 
int SaveReversalData(char *pReson);
 
int DelReversalData(void);
 
int ChgRecordFlag(int nRecNO, u8 ucFlag);
 
 
int ResetOper(unsigned char *TLVData, unsigned int TLVLen);
 
int ResetCommParam(unsigned char *TLVData, unsigned int TLVLen);

int ResetCtrlParam(unsigned char *TLVData, unsigned int TLVLen);
 
int SaveSignFile(SIGNLOG *ptSignLog);
 
int ReadSignFile(SIGNLOG *ptSingLog, u16 recnum);


#endif
