//#include "stdafx.h"
#include "../inc/global.h"
 
extern void BatChargeTest();
int GetAmount(u8 *pAmt)
{
	int ret;
	char buf[32], temp[32];
	
	memset( buf, 0, sizeof( buf));
	memset( temp, 0, sizeof( temp));
	
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		ret = GetScanf(MMI_POINT, 1, 12, buf, 60, LINE3, LINE3, MAX_LCDWIDTH);
		if(ret == ENTER)
		{
			memset(temp, 0x30, 12-buf[0]);
			strcpy(&temp[12-buf[0]], &buf[1]);
			AscToBcd_Api(pAmt, temp, 12);
			return 0;
		}
		else
		{
			return E_TRANS_CANCEL;
		}
	}
}
 
int GetSysRefNo(char *pSysRefNO)
{
	int ret;
	char Tempbuf[32];
	
	memset(Tempbuf, 0, sizeof(Tempbuf));
	
	ScrClrLine_Api(LINE2, LINE7);
    	ScrDisp_Api(LINE2, 0, "input REF code:", CDISP);  //REF:reference
	memset(Tempbuf, 0, sizeof(Tempbuf));
	ret = GetScanf(MMI_NUMBER|MMI_LETTER, 12, 12, Tempbuf, 60, LINE4, LINE4,16);
	if(ret == ENTER)			// 
	{
		sprintf(pSysRefNO,"%012s", &Tempbuf[1]);
	}
	else
		return ret;
	
	return 0;
}

int GetTraceAuditNo(u32 *num){
	char Tempbuf[16];
	int nRet, audit;

	memset(Tempbuf, 0, sizeof(Tempbuf));

	nRet = GetScanf(MMI_NUMBER, 1, 6, Tempbuf, 30, LINE4, LINE4, 16);
	if(nRet != ENTER)
			return -1;
	audit = atoi(&Tempbuf[1]);
	*num = audit;
	return 0;
} 

int EnterPIN(u8 flag)
{
	int ret;
	u8 DesFlag;
	char dispbuf[32];
	
	memset(dispbuf,0,sizeof(dispbuf));
	ScrClrLine_Api(LINE2, LINE7);
	if(flag != 0)
		LScrDisp_AE(LINE2, 0, "PIN Is Wrong, Please Input Again:", "الرقم السري خطأ،اعد الادخال:", LDISP);
	else 
		LScrDisp_AE(LINE2, 0, "Enter Card PIN Code:", "ادخل الرقم السري:", LDISP);

	if(gCtrlParam.DesType == 1) //Des
		DesFlag = 0x01;
	else
		DesFlag = 0x03;

	if(gCtrlParam.pinpad_type == PIN_PP)
	{
		u8 temp[128];
		ret = PPReadPinPadSn_Api(temp);
		if(ret == 0)
        	ret = PPGetPwd_Api(gCtrlParam.PinKeyIndes, 4, 8, PosCom.stTrans.MainAcc, PosCom.sPIN, DesFlag);
		#ifdef LST_DISPLAY
				ScrCls_Api();
				{
					u8 aa[32];
					memset(aa, 0, sizeof(aa));
					ScrDisp_Api(LINE1, 0, "pp offline pin", FDISP|LDISP);
					BcdToAsc_Api(aa, PosCom.sPIN, 16);
					ScrDisp_Api(LINE2, 0, aa, FDISP|LDISP);
					ScrDisp_Api(LINE3, 0, PosCom.stTrans.MainAcc, FDISP|LDISP);
					WaitAnyKey_Api(60);
				}
		#endif
	}
	else
	{
		ret = PEDGetPwd_Api(gCtrlParam.PinKeyIndes, 4, 4, PosCom.stTrans.MainAcc, PosCom.sPIN, DesFlag);
	}
	
	if(ret != 0)
		return E_TRANS_CANCEL;
	
	if(memcmp(PosCom.sPIN, "\0\0\0\0\0\0\0\0", 8) == 0)
		PosCom.stTrans.EntryMode[1] = PIN_NOT_INPUT;
	else
		PosCom.stTrans.EntryMode[1] = PIN_HAVE_INPUT;
	
	return 0;
}
 
void DispTradeTitle(u8 TranId)
{
	char tradename[32];

	memset(tradename, 0, sizeof(tradename));
	ConverTranNameCh(PosCom.stTrans.Trans_id, tradename);
	if(strlen(tradename) > 2)	
	{
		ScrClrLine_Api(LINE1, LINE1);
		DispTitle(tradename);
	}
}

int ConverTranNameCh(u8 TranId, char *TransName)
{
	switch(TranId) 
	{									
	case POS_PURCHASE:	
		strcpy(TransName,"PURCHASE"); 	
		break;
	case POS_PURCHASE:	
		strcpy(TransName,"POS_CARD_TO_CARD"); 	
		break;
	case POS_TOP_UP:	
		strcpy(TransName,"POS_TOP_UP"); 	
		break;
	case POS_WORKING_KEY:				
		strcpy(TransName,"Working Key"); 
		break;
	default	:					
		strcpy(TransName,"");				
		break;

	}
	return 0;
}

int ConverTranNameEn(u8 TranId, char *TransName)
{
	switch(TranId) 
	{
		case POS_PURCHASE:					Lstrcpy(TransName,"PURCHASE", "الشراء"); 				break;
		case POS_PURCHASE_REFUND:					Lstrcpy(TransName," REFUND", "ارجاع المشتريات"); 				break;
		case POS_TOP_UP:					Lstrcpy(TransName," POS_TOP_UP", " تحويل رصيد "); 				break;
		case POS_CARD_TO_CARD:					Lstrcpy(TransName," POS_CARD_TO_CARD", "تحويل الى بطاقة"); 				break;

		default:						strcpy(TransName,"");						break;
	}
	return 0;
} 
void ConvErrCode(int errCode, char *szErrInfo)
{	
	switch(errCode) 
	{
		case	E_MAC:				strcpy(szErrInfo,"Mac error"); 					break;
		case	E_SYS:				strcpy(szErrInfo,"system error"); 						break;

		case	E_FAILURE:	
			Lstrcpy(szErrInfo,"Transaction Failed ", "المعاملة فشلت");					 			break;				
		case	E_TRANS_CANCEL:	
				Lstrcpy(szErrInfo,"Transaction Cancelled ", "إلغاء المعاملة"); 						break;			
		case	E_NO_TRANS:			strcpy(szErrInfo,"no trade"); 						break;
		case	E_SEND_PACKET:		Lstrcpy(szErrInfo,"send failed", "فشل الارسال"); 						break;
		case	E_RECV_PACKET:		Lstrcpy(szErrInfo,"receive failed", "فشل الاستقبال"); 						break;
		case	E_RESOLVE_PACKET:	Lstrcpy(szErrInfo,"unpake failed", "خطأ قراءة الحزمة"); 						break;
		case	E_REVERSE_FAIL:		strcpy(szErrInfo,"reversal failed"); 						break;
		case	E_NO_OLD_TRANS:		strcpy(szErrInfo,"original trade not exist"); 					break;
		case	E_TRANS_VOIDED:		strcpy(szErrInfo,"original trade canceled"); 					break;
		case	E_ERR_SWIPE:		strcpy(szErrInfo,"swip failed"); 						break;
		case	E_MEM_ERR:			strcpy(szErrInfo,"file operate failed"); 					break;
		case	E_PINPAD_KEY:		strcpy(szErrInfo,"key error"); 						break;
	//	case	E_TIP_AMOUNT:		strcpy(szErrInfo,"С�ѽ���"); 					break;
	//	case	E_HAVE_ADJ:			strcpy(szErrInfo,"�����Ѿ�������");					break;
		case	E_FILE_OPEN:		strcpy(szErrInfo,"open file failed"); 					break;
		case	E_FILE_SEEK:		strcpy(szErrInfo,"seek file failed"); 					break;
		case	E_FILE_READ:		strcpy(szErrInfo,"read file failed"); 						break;
		case	E_FILE_WRITE:		strcpy(szErrInfo,"write file failed"); 						break;
		case	E_MAKE_PACKET:		strcpy(szErrInfo,"pack eror"); 						break;
		case	E_ERR_CONNECT:		Lstrcpy(szErrInfo,"connect failed", "فشل الاتصال"); 						break;
		case	E_TRANS_FAIL:
				Lstrcpy(szErrInfo,"Transaction Failed ", "المعاملة فشلت"); 						break;				
		case	E_REVTIMEOUT:
				Lstrcpy(szErrInfo,"Time Out! ", "انتهاء مهلة الاتصال"); 						break;				
		case	E_PPNORESP:			strcpy(szErrInfo,"no response!");				break;
	//
		case	ERR_EMVRSP:			strcpy(szErrInfo,"response code error!");					break;
		case	ERR_APPBLOCK:		strcpy(szErrInfo,"APP was locked!");						break;
		case	ERR_NOAPP:			strcpy(szErrInfo,"EMV APP not exist!");				break;
		case	ERR_USERCANCEL:		strcpy(szErrInfo,"trade canceled!");						break;
		case	ERR_TIMEOUT:		strcpy(szErrInfo,"operate timeout!");					break;
		case	ERR_EMVDATA:		strcpy(szErrInfo,"card data error!");					break;
		case	ERR_NOTACCEPT:		strcpy(szErrInfo,"trade not accept!");					break;
		case	ERR_EMVDENIAL:		strcpy(szErrInfo,"emv denial!");					break;
		case	ERR_KEYEXP:			strcpy(szErrInfo,"key expired!");						break;
		case	ERR_NOPINPAD:		strcpy(szErrInfo,"pinpad not exist!");		break;
		case	ERR_NOPIN:			strcpy(szErrInfo,"no pin!");	break;
		case	ERR_CAPKCHECKSUM:	strcpy(szErrInfo,"capk check failed!");		break;
		case	ERR_NOTFOUND:		strcpy(szErrInfo,"data not found!");		break;
		case	ERR_NODATA:			strcpy(szErrInfo,"data not found!");		break;
		case	ERR_OVERFLOW:		strcpy(szErrInfo,"memory overflow!");						break;
		//case	ERR_NOTRANSLOG:		strcpy(szErrInfo,"no record!");					break;
		//case	ERR_NORECORD:		strcpy(szErrInfo,"no record!");						break;
		//case	ERR_NOLOGITEM:		strcpy(szErrInfo,"record error!");					break;
		case	ERR_ICCRESET:		strcpy(szErrInfo,"IC reset failed!");					break;
		case	ERR_ICCCMD:			strcpy(szErrInfo,"IC command failed!");					break;
		case	ERR_ICCBLOCK:		strcpy(szErrInfo,"IC card locked!");						break;
		//case	ERR_ICCNORECORD:	strcpy(szErrInfo,"IC card no record!");					break;
		//case	ERR_GENAC1_6985:	strcpy(szErrInfo,"GEN AC return 6985!");			break;

		default	:					
				Lstrcpy(szErrInfo,"Transaction Failed ", "المعاملة فشلت"); 						break;				
	}	
	return;
}

void Lstrcpy(char* des, char* strE, char* strA){
	if(gCtrlParam.Lang == ARABIC_LANG){
		strcpy(des, strA);
	}
	else
		strcpy(des, strE);
}

int WaitEvent(void)
{
	u8 Key, Temp[32];
	char Time[32], TimeTemp[40], Dispbuf[32];
	u32 TimerId;
	int IsElectric;

	memset(Temp		, 0, sizeof(Temp));
	memset(Time		, 0, sizeof(Time));
	memset(Dispbuf	, 0, sizeof(Dispbuf));
	memset(TimeTemp	, 0, sizeof(TimeTemp));

	MagReset_Api();
	TimerId = TimerSet_Api();
	while(1)
	{
		#if ((defined V71_MACHINE) || (defined V36H_MACHINE) || (defined V80B_MACHINE))
				if(TimerCheck_Api(TimerId , 30*1000) == 1)
				{
					SysPowerStand_Api();
					return 0xfe;
				}
		#endif 
		BatChargeTest();
		GetSysTime_Api(Temp);					 
		if( memcmp(TimeTemp, Temp, 7) != 0 )
		{ 
			BcdToAsc_Api(Time, Temp, 14);
			MakeTimeDispBuf(Dispbuf, Time);
			ScrClrLineRam_Api(LINE7, LINE7);
			ScrDispRam_Api(LINE7, 0, Dispbuf, CDISP);
			ScrBrush_Api();
			memcpy(TimeTemp, Temp, 8);
		}
		
		Key = GetKey_Api();
		Key = TelephoneEvent_Api((int)Key);
		if(Key != 0)
			return Key;
	}
	return 0;
}

 void TraceNoInc(void)
{
	gCtrlParam.lTraceNo += 1;
	if(gCtrlParam.lTraceNo<=0 || gCtrlParam.lTraceNo > 999999)
	{
		gCtrlParam.lTraceNo = 1;
	}
	SaveCtrlParam();
} 


void DispAllVersion(void)
{
	u8 i=0, StrTemp[50], VersionLeng=0, OutBufer[1024*4];
	int VersionNum, Point;
	
	memset(StrTemp	, 0, sizeof(StrTemp));
	memset(OutBufer	, 0, sizeof(OutBufer));
	
	ScrCls_Api();
	VersionNum = 0;
	Point = 0;
	DispTitle("LIB VERSION");
	if( GetVersion_Api(OutBufer, &VersionNum) != 0x00 )
	{
		ScrDisp_Api(LINE4, 0, "read failed",FDISP);
		WaitAnyKey_Api(2);
		return;
	}
	
	ScrBackLight_Api(VersionNum*30);	
	for (i = 0; i < VersionNum; i++)
	{
		memset(StrTemp, 0, sizeof(StrTemp));
		memcpy(StrTemp, &OutBufer[Point], 20);
		Point += 20;
		ScrDisp_Api(LINE2, 0, (char*)StrTemp, FDISP);
		memset(StrTemp, 0, sizeof(StrTemp));
		VersionLeng = OutBufer[Point];
		Point += 1;
		memcpy(StrTemp, &OutBufer[Point], VersionLeng);
		ScrDisp_Api(LINE3, 0, (char*)StrTemp, FDISP);
		Point += VersionLeng;
		VersionLeng = 0;
		WaitAnyKey_Api(60);
		ScrCls_Api();
	}
}

void ACTSLogo(void)
{
//----------------------------------------
// width X hight (pixel): 100 x 100
//----------------------------------------
const char Logo[1300] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 
	0xC0, 0xC0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF0, 0xF8, 0xFC, 0xFC, 
	0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F, 0x3F, 0x3F, 
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7F, 0x7F, 
	0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 
	0xF0, 0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x1F, 0x0F, 0x07, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 
	0x0F, 0x1F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xF8, 0xE0, 0xC0, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0x1F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 
	0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 
	0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xF8, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xE0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF0, 0xC0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 
	0x03, 0x07, 0x07, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 
	0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x3F, 0x7F, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x07, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x1F, 
	0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 
	0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x1F, 0x1F, 0x0F, 
	0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 
	0xE0, 0xF8, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x07, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 
	0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC, 
	0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 
	0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x3F, 
	0x3F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00
};

	ScrDrLogoxyRam_Api(100, 100, 17, 32, Logo);
}

int NetworkMenu(void)
{
	int key;

	char maintitle[] = "Network Settings";
	char maintitle_ar[] = "اعدادات الشبكة";
	const char *main_menu[] = {
		"1.Domain or Ip",
		"2.Set Address"
	};
	const char *main_menu_ar[] = {
		"1.عنوان ام اي بي",
		"2.ضبط العنوان"
	};

	while(1)
	{
		if(gCtrlParam.Lang == ENGLISH_LANG)
			key = ShowMenuItem(maintitle, main_menu, 2, DIGITAL1, DIGITAL2, 0, 60);
		else	
			key = ShowMenuItem(maintitle_ar, main_menu_ar, 2, DIGITAL1, DIGITAL2, 0, 60);
		switch(key)
		{
		case DIGITAL1:	
			SetGprsType();		 
			break;
		case DIGITAL2:	
			SetHostAddress();		 
			break;
		case ESC:
			return ESC;
		default:
			break;
		}
	}
	return(ESC);
}

int TerminalParaMenu(void)
{
	int key;

	char maintitle[] = "Terminal Parameters";
	char maintitle_ar[] = "بيانات الجهاز";
	const char *main_menu[] = {
		"1.Client Id",
		"2.Terminal Id",
		"3.System Trace No",
		// "4.Network Settings"
	};
	const char *main_menu_ar[] = {
		"1.هوية العميل",
		"2.رقم الجهاز",
		"3.رقم تتبع النظام",
		// "4.اعدادات الشبكة"
	};

	while(1)
	{
		if(gCtrlParam.Lang == ENGLISH_LANG)
			key = ShowMenuItem(maintitle, main_menu, 3, DIGITAL1, DIGITAL3, 0, 60);
		else	
			key = ShowMenuItem(maintitle_ar, main_menu_ar, 3, DIGITAL1, DIGITAL3, 0, 60);
		switch(key)
		{
		case DIGITAL1:	
			SetClientId();		 
			break;
		case DIGITAL2:	
			SetTerminalId();		 
			break;	
		case DIGITAL3:
			SetSystemTraceNo();
			break;
		// case DIGITAL4:
		// 	NetworkMenu();
		// 	break;
		case ESC:
			return ESC;
		default:
			break;
		}
	}
	return(ESC);
}

int SettingMenu(void)
{
	int key;

	char maintitle[] = "Settings Menu";
	char maintitle_ar[] = "قائمة الاعدادات";
	const char *main_menu[] = {
		"1.Terminal Parameters",
		"2.Set Working Key",
		"3.Set Master Key",
		"4.Purchase Refund",
		"5.Language"
	};
	const char *main_menu_ar[] = {
		"1.بيانات الجهاز",
		"2.ضبط مفتاح التشفير",
		"3.ضبط المفتاح الرئيسي",
		"4.ارجاع المشتريات",
		"5.اللغة"
	};

	while(1)
	{
		if(gCtrlParam.Lang == ENGLISH_LANG)
			key = ShowMenuItem(maintitle, main_menu, 5, DIGITAL1, DIGITAL5, 0, 60);
		else
			key = ShowMenuItem(maintitle_ar, main_menu_ar, 5, DIGITAL1, DIGITAL5, 0, 60);
		switch(key)
		{
		case DIGITAL1:	
			TerminalParaMenu();		 
			InitPosCom();                                                                     
			break;
		case DIGITAL2:	
			PosCom.stTrans.Trans_id = POS_WORKING_KEY;		 
			return 0;		
		case DIGITAL3:
			SetMaskterKeyValue(1);
			break;
		case DIGITAL4:
			PosCom.stTrans.Trans_id = POS_PURCHASE_REFUND;
			return 0;
		case DIGITAL5:
			LanguageMenu();
			break;
			
		case DIGITAL6:
			PosCom.stTrans.Trans_id = POS_TOP_UP;
			return 0;
		case DIGITAL7:
			PosCom.stTrans.Trans_id = POS_CARD_TO_CARD;
			return 0;
		case ESC:
			return ESC;
		default:
			break;
		}
	}
	return(ESC);
}

// void GetFile()
// {
//  int ret;
//  ret=GetFileSize_Api("/mtd0/res/Persian32P.fnt");
//  TipAndWaitEx_Api("FileSize:%d",ret);
// }
#ifdef WIN32
int ScrSetColor_Api(unsigned int TextColor, unsigned int BackColor){return 0;}
#endif

int SelectMainMenu(void)
{
	int key;
	int ret;

	char maintitle[] = "Main Menu";
	char maintitle_ar[] = "القائمة الرئيسية";
	const char *main_menu[] = {
		"1.Purchase "
	};
	const char *main_menu_ar[] = {
		"1.الشراء"
	};

	while(1)
	{
		if(gCtrlParam.Lang == ENGLISH_LANG)
			key = ShowMenuItem(maintitle, main_menu, 1, DIGITAL0, DIGITAL1, 0, 60);
		else
			key = ShowMenuItem(maintitle_ar, main_menu_ar, 1, DIGITAL0, DIGITAL1, 0, 60);
		switch(key)
		{
		case DIGITAL0:
			ret = SettingMenu();
			if(ret == 0)
				return ret;
			break;
		case DIGITAL1:
			PosCom.stTrans.Trans_id = POS_PURCHASE;
			return 0;
		case ESC:
			return ESC;
		default:
			break;
		}
	}
	return(ESC);
}

void LPrompt(char* msgE, char* msgA)
{
	ScrClrLine_Api(LINE2, LINE7);
	if(gCtrlParam.Lang == ARABIC_LANG)
	{
		SetLang_Api(ARABIC_LANG, 3);
		TipAndWaitEx_Api("%s", msgA);
		SetLang_Api(ENGLISH_LANG, 3);
	}
	else
		TipAndWaitEx_Api("%s", msgE);

}

//0-string 1-bcd
void CommDebugInfo(char *title, unsigned char *Date, int DateLen, int flag)
{
#ifdef _OUTPUTLOG_
	char TempBuf[2048+1024];
	
	PortSends_Api(0, (char*)title, strlen(title));
	PortSends_Api(0, "\r\n", 2);
	if(flag == 0)
	{
		PortSends_Api(0, (char*)Date, DateLen);
		PortSends_Api(0, "\r\n", 2);
		return ;
	}
	BcdToAsc_Api(TempBuf, Date, DateLen*2);
	PortSends_Api(0, (char*)TempBuf, DateLen*2);
	PortSends_Api(0, "\r\n", 2);
#endif
}

int SetTerminalId(void)
{
	char Tempbuf[100];
	int nRet;

	memset(Tempbuf, 0, sizeof(Tempbuf));

	ScrClrLine_Api(LINE2, LINE7);
	// if(gCtrlParam.Lang == ENGLISH_LANG)
	sprintf_str(Tempbuf, "Terminal Id: %s", "رقم الجهاز: %s", gCtrlParam.TerminalNo);
	// else	
	// sprintf(Tempbuf, "رقم الجهاز: %s", gCtrlParam.TerminalNo);
	
	LScrDisp_AE(LINE2, 0, Tempbuf, Tempbuf, LDISP);
	memset(Tempbuf, 0, sizeof(Tempbuf));
	nRet = GetScanf(MMI_NUMBER, 8, 8, Tempbuf, 30, LINE4, LINE4, 16);
	if(nRet != ENTER)
		return -1;
	memcpy(gCtrlParam.TerminalNo, &Tempbuf[1], 8);
	SaveCtrlParam();
	LPrompt("Success", "نجاح");	
	return 0;
}

int SetClientId(void)
{
	char Tempbuf[100];
	int nRet;

	memset(Tempbuf, 0, sizeof(Tempbuf));

	ScrClrLine_Api(LINE2, LINE7);
	// if(gCtrlParam.Lang == ENGLISH_LANG)
	sprintf_str(Tempbuf, "Client Id: %s", "هوية العميل: %s", gCtrlParam.ClientId);
	// else	
		// sprintf(Tempbuf, "هوية العميل: %s", gCtrlParam.ClientId);
	
	LScrDisp_AE(LINE2, 0, Tempbuf, Tempbuf, LDISP);
	memset(Tempbuf, 0, sizeof(Tempbuf));
	nRet = GetScanf(MMI_NUMBER|MMI_LETTER|MMI_SYMBOL, 1, 31, Tempbuf, 30, LINE4, LINE4, 16);
	if(nRet != ENTER)
		return -1;
	memset(gCtrlParam.ClientId, 0, sizeof(gCtrlParam.ClientId));
	strcpy(gCtrlParam.ClientId, &Tempbuf[1]);
	SaveCtrlParam();
	LPrompt("Success", "نجاح");
	return 0;
}

int SetHostAddress(void)
{
	// char Tempbuf[100];
	// int nRet;

	// memset(Tempbuf, 0, sizeof(Tempbuf));

	// ScrClrLine_Api(LINE2, LINE7);
	
	// LScrDisp_AE(LINE2, 0, "Host Address:", "عنوان الخادم:", LDISP);
	// // if(G_CommPara.GprsSet.GprsDomainOrIp == GPRS_IP)
	// 	ScrDisp_Api(LINE3, 0, G_CommPara.GprsSet.GprsIp1, LDISP);
	// // else
	// 	// ScrDisp_Api(LINE4, 0, G_CommPara.GprsSet.GprsDomain, LDISP);

	// memset(Tempbuf, 0, sizeof(Tempbuf));
	// nRet = GetScanf(MMI_NUMBER|MMI_LETTER|MMI_SYMBOL, 1, 29, Tempbuf, 30, LINE5, LINE6, 16);
	// if(nRet != ENTER)
	// 	return -1;
	
	// // if(G_CommPara.GprsSet.GprsDomainOrIp == GPRS_IP)
	// // {
	// 	memset(G_CommPara.GprsSet.GprsIp1, 0, sizeof(G_CommPara.GprsSet.GprsIp1));
	// 	strcpy(G_CommPara.GprsSet.GprsIp1, &Tempbuf[1]);
	// // }
	// // else{
	// // 	memset(G_CommPara.GprsSet.GprsDomain, 0, sizeof(G_CommPara.GprsSet.GprsDomain));
	// // 	strcpy(G_CommPara.GprsSet.GprsDomain, &Tempbuf[1]);
	// // }
	// SaveCommParam();
	// LPrompt("Success", "نجاح");
	// CommModuleInit_Api(&G_CommPara);
	// CommParamSet_Api(&G_CommPara);
	return 0;
}

int SetGprsType(void)
{
// 	char Tempbuf[100];
// 	int nRet, num;

// 	memset(Tempbuf, 0, sizeof(Tempbuf));

// 	ScrClrLine_Api(LINE2, LINE7);
	
// 	if(G_CommPara.GprsSet.GprsDomainOrIp == GPRS_IP){
// 		ScrDisp_Api(LINE2, 0, "Current: Gprs Ip", LDISP);
// 	}
// 	else
// 		ScrDisp_Api(LINE2, 0, "Current: Gprs Domain", LDISP);

// 	ScrDisp_Api(LINE3, 0, "1.Gprs Ip", LDISP);
// 	ScrDisp_Api(LINE4, 0, "2.Gprs Domain", LDISP);

// 	memset(Tempbuf, 0, sizeof(Tempbuf));
// 	nRet = GetScanf(MMI_NUMBER, 1, 2, Tempbuf, 30, LINE6, LINE6, 16);
// 	if(nRet != ENTER)
// 		return -1;
	
// 	num = atoi(&Tempbuf[1]);
// 	if(num == 1)
// 		G_CommPara.GprsSet.GprsDomainOrIp = GPRS_IP;
	
// 	else
// 		G_CommPara.GprsSet.GprsDomainOrIp = GPRS_Domain;
	
// 	SaveCommParam();
// 	LPrompt("Success", "نجاح");
	return 0;
}

int SetSystemTraceNo(void)
{
	char Tempbuf[100];
	int nRet, num;

	memset(Tempbuf, 0, sizeof(Tempbuf));

	ScrClrLine_Api(LINE2, LINE7);
	// if(gCtrlParam.Lang == ENGLISH_LANG)
	sprintf_str(Tempbuf, "System Trace Audit Num: %d", "رقم تتبع النظام: %d", gCtrlParam.lTraceNo);
	// else		
		// sprintf(Tempbuf, "رقم تتبع النظام: %d", gCtrlParam.lTraceNo);
	
	LScrDisp_AE(LINE2, 0, Tempbuf, Tempbuf, LDISP);
	memset(Tempbuf, 0, sizeof(Tempbuf));
	nRet = GetScanf(MMI_NUMBER, 1, 9, Tempbuf, 30, LINE4, LINE4, 16);
	if(nRet != ENTER)
		return -1;
	num = atoi(&Tempbuf[1]);
	gCtrlParam.lTraceNo = num;
	SaveCtrlParam();
	LPrompt("Success", "نجاح");	
	return 0;
}

int LanguageMenu(void)
{
	int key;

	char maintitle[] = "Language";
	char maintitle_ar[] = "اللغة";
	const char *main_menu[] = {
		"1.Arabic",
		"2.English"
	};
	const char *main_menu_ar[] = {
		"1.العربية",
		"2.الانجليزية"
	};

	while(1)
	{
		if(gCtrlParam.Lang == ENGLISH_LANG)
			key = ShowMenuItem(maintitle, main_menu, 2, DIGITAL1, DIGITAL2, 0, 60);
		else	
			key = ShowMenuItem(maintitle_ar, main_menu_ar, 2, DIGITAL1, DIGITAL2, 0, 60);
		switch(key)
		{
		case DIGITAL1:	
			SetLanguage(1);		 
			break;
		case DIGITAL2:	
			SetLanguage(2);		 
			break;
		case ESC:
			return ESC;
		default:
			break;
		}
	}
	return (ESC);
}

int SetLanguage(int lang)
{	
	gCtrlParam.Lang = lang;
	SaveCtrlParam();
	LPrompt("Success", "نجاح");	
	return 0;
}

int SetMaskterKeyValue(int flag)
{
	char Tempbuf[64];
	u8 sKey[16];
	u8 mainkeyidx;
	int nRet;

	memset(Tempbuf, 0,sizeof(Tempbuf));
	memset(sKey, 0, sizeof(sKey));

	if(gCtrlParam.MainKeyIdx <0 ||gCtrlParam.MainKeyIdx > 9 )
	{
		gCtrlParam.MainKeyIdx = 0;
		SaveCtrlParam();
	}
	mainkeyidx = gCtrlParam.MainKeyIdx;  //set 

	memset(Tempbuf, 0, sizeof(Tempbuf));
	memset(sKey, 0, sizeof(sKey));

	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE3, 0, "Enter Master Key", "ادخل المفتاح الرئيسي", FDISP|CDISP);
	if(GetScanf(MMI_NUMBER|MMI_LETTER, 16, 16, Tempbuf, 60, LINE4, LINE5,16) != ENTER)
		return ESC;

	CommDebugInfo("Enter Master Key", Tempbuf, strlen(Tempbuf), 0);
	AscToBcd_Api(sKey, &Tempbuf[1], Tempbuf[0]);
	// AscToBcd_Api(sKey, Tempbuf, 32);
	if(gCtrlParam.DesType == 0)//3des
	{
		if(gCtrlParam.pinpad_type == PIN_PP)
			nRet = PPWriteMKey_Api(mainkeyidx, 0x03, sKey);
		else
			nRet = PEDWriteMKey_Api(mainkeyidx, 0x03, sKey);	
	}
	else
	{
		if(gCtrlParam.pinpad_type == PIN_PP)
			nRet = PPWriteMKey_Api(mainkeyidx, 0x01, sKey);
		else
			nRet = PEDWriteMKey_Api(mainkeyidx, 0x01, sKey);		
	}

	if(flag != 0)
	{
		if(nRet != 0) 
			LPrompt("MasterKey Write Failed", "خطأ كتابة المفتاح");
		else
			LPrompt("MasterKey Write Ok", "نجاح");
	}
	
}
int SetWorkKeyIdx(void)
{
	char Tempbuf[100];
	int nRet;

	memset(Tempbuf, 0, sizeof(Tempbuf));

	ScrCls_Api();
	DispTitle("trade key setting");
	sprintf(Tempbuf, "key index(0-9):%d", gCtrlParam.MainKeyIdx);
	ScrDisp_Api(LINE2, 0, Tempbuf, LDISP);
	memset(Tempbuf, 0, sizeof(Tempbuf));
	nRet = GetScanf(MMI_NUMBER, 1, 1, Tempbuf, 30, LINE4, LINE4, 16);
	if(nRet != ENTER)
		return -1;
	gCtrlParam.MainKeyIdx = Tempbuf[1]-0x30;
	SaveCtrlParam();
	return 0;
}

void SetPinkey(char *wKey, int flag)
{
	u8 PinKey[32], buf[64], ret;

	memset(buf, 0, sizeof(buf));
	memset(PinKey, 0, sizeof(PinKey));

	if ((gCtrlParam.DesType & 0x0f) == 0)
	{	
		strcpy(buf, "222222222222222222222222222222227B0205FF");
		AscToBcd_Api(PinKey, buf, 40);
		if(gCtrlParam.pinpad_type == PIN_PP )
			ret = PPWriteWKey_Api(gCtrlParam.MainKeyIdx, gCtrlParam.PinKeyIndes, 0x83, PinKey);
		else
			ret = PEDWriteWKey_Api(gCtrlParam.MainKeyIdx, gCtrlParam.PinKeyIndes, 0x83, PinKey);
	}
	else
	{
		strcpy(buf, wKey);
		AscToBcd_Api(PinKey, buf, strlen(buf));
		if(gCtrlParam.pinpad_type == PIN_PP )
			ret = PPWriteWKey_Api(gCtrlParam.MainKeyIdx, gCtrlParam.PinKeyIndes, 0x81, PinKey);
		else
			ret = PEDWriteWKey_Api(gCtrlParam.MainKeyIdx, gCtrlParam.PinKeyIndes, 0x81, PinKey);
	}

	if(flag != 0)
	{
		if(ret != 0)
			LPrompt("Pinkey Write Error!", "خطأ كتابة مفتاح التشفير");
		else
			LPrompt("Pinkey Write Ok!", "نجاح");
	}	
}

int ZeroOneSelectTwoLine(char *szTitle,char *choice1,char *choice0, BOOL IsDisOldSele, int OldSel, int iTimeOut)
{
	char buf[40];	
	int key;

	memset(buf, 0, sizeof(buf));

	ScrCls_Api();
	KBFlush_Api();

	DispTitle(szTitle);  
	sprintf(buf, "1-%s", choice1);
	ScrDisp_Api(LINE2, 0, buf, LDISP); 

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0-%s", choice0);
	ScrDisp_Api(LINE3, 0, buf, LDISP); 
	if(IsDisOldSele ==TRUE )
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",OldSel);
		ScrDisp_Api(LINE4, 0, buf, RDISP); 
	}

	do
	{
		key = WaitAnyKey_Api(iTimeOut);
		if(key == -2)
			return -1;
	} while(key != ENTER && key != ESC && key!= DIGITAL1 && key!= DIGITAL0);
	return key;
}



void InitOper(void)
{
	if(GetFileSize_Api(FILE_OPER_LOG) == sizeof(gtOperator))
		return;

	// 
	memset(&gtOperator, 0, sizeof(OPER_STRC));
	gtOperator.nOptNO[0] = 0;
	strcpy(gtOperator.sOptPwd[0], "123456");
	gtOperator.nOptNO[1] = 99;
	strcpy(gtOperator.sOptPwd[1], "000000");
	gtOperator.nOptNO[2] = 1;
	strcpy(gtOperator.sOptPwd[2], "0000");
	gtOperator.nOptNO[3] = 2;
	strcpy(gtOperator.sOptPwd[3], "0000");
	gtOperator.nOptNO[4] = 3;
	strcpy(gtOperator.sOptPwd[4], "0000");
	gtOperator.nOptNO[5] = 4;
	strcpy(gtOperator.sOptPwd[5], "0000");
	gtOperator.nOptNO[6] = 5;
	strcpy(gtOperator.sOptPwd[6], "0000");

	SaveOperFile();
}

int ReadOperFile(void)
{
	u32 FileLen;

	InitOper();
	FileLen = sizeof(gtOperator);
	ReadFile_Api(FILE_OPER_LOG, (u8*)&gtOperator, 0, &FileLen);
	return 0;
}

int SaveOperFile(void)
{
	char result ;
	u32 len;

	len = sizeof(OPER_STRC);  
	result = WriteFile_Api(FILE_OPER_LOG,(u8*)&gtOperator, 0, len);
	if(result == 0)						 
	{                                    
		return 0;                        
	}                                    
	else								 
	{                                    
		return (E_MEM_ERR);
	}
}

int GetCardNoFromTrack2Data(char *cardNo, u8 *track2Data)
{
	char tmp[MCARDNO_MAX_LEN + 2], *p = NULL ;
	u32 len;

	memset(tmp, 0, sizeof(tmp));

	len = MCARDNO_MAX_LEN+1 < track2Data[0]*2 ? MCARDNO_MAX_LEN+1 : track2Data[0]*2;

	FormBcdToAsc( tmp, track2Data+1, len);
	tmp[len] = '\0';
	p = strchr(tmp, '=');//
	if(p != NULL)
	{
		*p = 0;
		strcpy(cardNo, tmp);
		return TRUE;
	}

	return FALSE;
}


int ShowMenuItem(char *Title, const char *menu[], u8 ucLines, u8 ucStartKey, u8 ucEndKey, int IsShowX, u8 ucTimeOut)
{
	u8 IsShowTitle, cur_screen, OneScreenLines, Cur_Line, i, t;
	int nkey;
	char dispbuf[50];

	memset(dispbuf, 0, sizeof(dispbuf));

	if(Title != NULL)
	{
		IsShowTitle = 1;
		OneScreenLines = 9;
	}
	else
	{
		IsShowTitle = 0;
		OneScreenLines = 10;
	}
	IsShowX -= 1;	 
	cur_screen = 0;  
	while(1)
	{
		ScrClsRam_Api();
		if(IsShowTitle)
			DispTitle(Title);
		Cur_Line = LINE1+IsShowTitle;
		for(i = 0; i < OneScreenLines; i++) 
		{
			t = i+cur_screen*OneScreenLines;
			if (t >= ucLines || menu[t] == NULL)// 
			{
				break;
			}
			memset(dispbuf, 0, sizeof(dispbuf));
			strcpy(dispbuf, menu[t]);
			if(IsShowX == t)//if((IsShowX >> t) & 0x01)
				strcat(dispbuf, "��");
			LScrDispRam_Api(Cur_Line++, 0, dispbuf, FDISP);
		}
		ScrBrush_Api();
		
		nkey = WaitAnyKey_Api(ucTimeOut);
		switch(nkey)
		{
		case UP:
			#if ( defined(V70B_MACHINE) || defined(V80B_MACHINE) )
					case F2:
			#endif
			if(cur_screen)
				cur_screen--;
			break;
		case DOWN:
			#if ( defined(V70B_MACHINE) || defined(V80B_MACHINE) )
					case F3:
			#endif
			if ((t < (ucLines-1) ) ) 
				cur_screen++;
			break;
		case ESC:
		case TIMEOUT:
			return nkey;
			break;
		default:
			if( (nkey >= ucStartKey)&&(nkey <= ucEndKey) )
				return nkey;
			break;
		}
	};
}

char GetNameFromTrack1(char *Inbuf, char *Name)
{
	int i, j, Nl;
	char *p = NULL ;

	i = 0;
	j = 0;
	while( Inbuf[i] != 0 )  // '~' 
	{
		if( Inbuf[i] == 0x5e || Inbuf[i] ==0x7e)//'^' '~'
		{
			p = &Inbuf[i+1];
			break;  	
		}
		i++;
	}

	if(Inbuf[i] == 0 ) // '~', 
		return -1;

	while( *p != 0 )   
	{
		if( *p == 0x5e || *p == 0x7e )//'~'			   
			break;
		Name[j++] = *p++;
		if(j >= 19) 
			break;
	}

	if( *p == 0 )  
		return -2;

	Nl = strlen(Name);
	for(i = 0, j = 0; i < Nl ; )
	{
		if( Name[j] != '@' )
		{
			Name[i] = Name[j];
			i++;
		}
		else
			Name[j] = 0;
		j++;
	}  
	return 0;
}

void InitPosCom(void)
{
	u8 TimeBuf[28];

	memset(&PosCom, 0, sizeof(POS_COM));
	memset(TimeBuf, 0, sizeof(TimeBuf));

	memcpy(PosCom.stTrans.MerchantNo, gCtrlParam.MerchantNo, 15);
	memcpy(PosCom.stTrans.TerminalNo, gCtrlParam.TerminalNo, 8);
	memcpy(PosCom.stTrans.ClientId, gCtrlParam.ClientId, strlen(gCtrlParam.ClientId));

	if(gCtrlParam.lTraceNo<=0 || gCtrlParam.lTraceNo > 999999)
	{
		gCtrlParam.lTraceNo = 1;
		SaveCtrlParam();
	}
	if(gCtrlParam.lNowBatchNum <= 0 || gCtrlParam.lNowBatchNum > 999999)
	{
		gCtrlParam.lNowBatchNum = 1;
		SaveCtrlParam();
	}

	memset(PosCom.stTrans.AuthCode, '0', 6);	
	PosCom.stTrans.lTraceNo = gCtrlParam.lTraceNo;
	PosCom.stTrans.lNowBatchNum = gCtrlParam.lNowBatchNum;	

	GetSysTime_Api(TimeBuf);                                 
	memcpy(PosCom.stTrans.TradeDate, TimeBuf, 4);			 
	memcpy(PosCom.stTrans.TradeTime, &TimeBuf[4], 3);        
	memset(TimeBuf,0,sizeof(TimeBuf));                       
	//                                                       
	PosCom.stTrans.EntryMode[1] = PIN_NOT_INPUT;			 
	PosCom.stTrans.OperatorNo = (u8)gtOperator.nCurOptNO;	 
}                                                            

void DispTitle(char* Title)
{
	char DisBuf[200];
	int Slen, charNum = 26;		// Char Num varies according to selected font size
	unsigned char atr = CDISP|NOFDISP;

	memset(DisBuf, 0, sizeof(DisBuf));

	if(gCtrlParam.Lang == ARABIC_LANG)
	{		
		ScrDispTitle_AE(LINE1, 0, Title, atr);
	}
	else
	{
		if(atr & NOFDISP)
		{
			//memset(DisBuf, 0x20, sizeof(DisBuf));
			memset(DisBuf, 0x20, charNum+1);
			if(atr & CDISP) //����
			{
				Slen = (int)strlen(Title);  
				if(Slen > charNum) //21
					memcpy(DisBuf,Title, charNum );
				else 
					memcpy(&DisBuf[(charNum - Slen)/2],Title, Slen );

				ScrDisp_Api(LINE1, 0, DisBuf, atr);
			}
			else
				ScrDisp_Api(LINE1, 0, Title, atr);
		}
	}
}

void LScrDisp_AE(int row ,int col ,const char *strE,  const char *strA, unsigned char atr)
{
	if(gCtrlParam.Lang == ARABIC_LANG)
	{
		char atr_ar = (atr==LDISP)? ((atr&0)|RDISP) : atr;
		SetLang_Api(ARABIC_LANG, 3);
		ScrDisp_Api(row , col , strA,  atr_ar);
		SetLang_Api(ENGLISH_LANG, 3);
	}
	else
	{
		ScrDisp_Api(row , col , strE,  atr );
	}
}

void ScrDispTitle_AE(int row ,int col ,const char *str  , unsigned char atr){
	if(gCtrlParam.Lang == ARABIC_LANG)
	{
		ScrSetColor_Api(BLUE,BLUE);
		SetLang_Api(ARABIC_LANG, 3);
		ScrDisp_Api(row , col , str,  atr);
		SetLang_Api(ENGLISH_LANG, 3);
		ScrSetColor_Api(BLUE,WHITE);
	}
	else
		ScrDisp_Api(row , col , str,  atr );
	
}

void LScrDisp(int row ,int col ,const char *str  , unsigned char atr)
{
	if(gCtrlParam.Lang == ARABIC_LANG)
	{
		char atr_ar = (atr&FDISP) | RDISP;
		SetLang_Api(ARABIC_LANG, 3);
		ScrDisp_Api(row , col , str,  atr_ar);
		SetLang_Api(ENGLISH_LANG, 3);
	}
	else
	{
		ScrDisp_Api(row , col , str,  atr );
	}
}

void LScrDispRam_Api(int row ,int col ,const char *str  , unsigned char atr)
{
	if(gCtrlParam.Lang == ARABIC_LANG)
	{
		char atr_ar = (atr&FDISP) | RDISP;
		SetLang_Api(ARABIC_LANG, 3);
		ScrDispRam_Api(row , col , str,  atr_ar);
		SetLang_Api(ENGLISH_LANG, 3);
	}
	else
	{
		ScrDispRam_Api(row , col , str,  atr );
	}
}

void LDispTitle_AE(char* eng, char* ar){
	if(gCtrlParam.Lang == ENGLISH_LANG)
		DispTitle(eng);
	else	
		DispTitle(ar);
}

void WaitRemoveICC(void)
{
	while(IccDetectOut_Api(ICC_EMV) != 0)
	{
		ScrClrLine_Api(LINE3, LINE3);
		ScrDisp_Api(LINE3, 0, "Please Pull Card", CDISP);
		Beep_Api(BEEPERROR);
	};
}

void DisplayProcessing(void)
{
	ScrClrLine_Api(LINE2, LINE7);
	ScrDisp_Api(LINE3, 0, "Processing", CDISP);
	ScrDisp_Api(LINE4, 0, "Please Wait", CDISP);
}

int GetScanf(u32 mode, int Min, int Max, char *outBuf, u32 TimeOut, u8 StartRow, u8 EndRow, int howstr) 
{
	return GetScanfEx_Api(mode, Min, Max, outBuf, TimeOut, StartRow, EndRow, howstr, MMI_NUMBER);
}

void MakeTimeDispBuf(char *pDisp, char *pStrTime)
{
	memcpy(&pDisp[0], &pStrTime[0], 4);
	memcpy(&pDisp[4], "/", 1);
	memcpy(&pDisp[5], &pStrTime[4], 2);
	memcpy(&pDisp[7], "/", 1);
	memcpy(&pDisp[8], &pStrTime[6], 2);
	memcpy(&pDisp[10], " ", 1);
	memcpy(&pDisp[11], &pStrTime[8], 2);
	memcpy(&pDisp[13], ":", 1);
	memcpy(&pDisp[14], &pStrTime[10], 2);
	memcpy(&pDisp[16], ":", 1);
	memcpy(&pDisp[17], &pStrTime[12], 2);
	pDisp[19] = 0;
}

void GetPanNumber()
{
	int tlvLen = 0, i;
	u8 buf[64];

	memset(buf, 0, sizeof(buf));
	if(Common_GetTLV_Api(0x5A, buf, &tlvLen) == 0) // 
	{
		BcdToAsc_Api(PosCom.stTrans.MainAcc, buf, tlvLen * 2);
		for(i = tlvLen * 2 - 1; i >= 0; --i)
		{
			if(PosCom.stTrans.MainAcc[i] == 'F' || PosCom.stTrans.MainAcc[i] == 'f')
				PosCom.stTrans.MainAcc[i] = 0;
			else
				break;
		}
	}
}



