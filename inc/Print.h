#ifndef AFX_PRINTP_H
#define AFX_PRINTP_H
#include "../inc/global.h"

 
void prnShieldPanCardNum(u8 flag, char *prtCard, const char *cardNum);
 
void PrnFormatCardNum(char *pTemp, char *pCardNO, LOG_STRC *pLog, u8 inputmode, u8 ifSecondCard);
 
int PrtTicket(LOG_STRC *pLog, u8 bErrPrint, u8 bRePrint);

void PrnStr_AE(u8 *strE, u8 *strA, u8 *content, int flag);

void sprintf_str(char* des, char* strE, char* strA, char* val);

void sprintf_float(char* des, char* strE, char* strA, fp32 val);

void LPrnStr_Api(char* strE, char* strA);

void PrnLines(int num);
 
int PrintData(void);
 
#endif

