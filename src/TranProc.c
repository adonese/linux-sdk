//#include "stdafx.h"
#include "../inc/global.h"
 
int TransProcess(void)
{
	int ret;
	
	ret = OnlineTransProcess();	
	if(ret)
		ret = NO_DISP;
	CommHangUp(); 
	return ret;
}
 
int OnlineTransProcess(void)
{
	int ret = 0, reprnflag=0, tret = 0;
	int kret=0,pret=0;
	
	ScrCls_Api();
	switch(PosCom.stTrans.Trans_id)
	{                                                    
		case POS_PURCHASE:					 
			ret = PurchaseInput(POS_PURCHASE); 
			if(ret != 0)
				return ret;     
			break;         
		case POS_PURCHASE_REFUND:					 
			ret = PurchaseInput(POS_PURCHASE_REFUND); 
			if(ret != 0)
				return ret;     
			break;

		case POS_WORKING_KEY:       
			LDispTitle_AE("Working Key", "مفتاح التشفير");
			break;                       
		case POS_PAPER_OUT:       
			LDispTitle_AE("Paper Out", "نفذ الورق");
			break;  
			case POS_CARD_TO_CARD:					 
			ret = Card_To_CardInput(POS_CARD_TO_CARD); 
			if(ret != 0)
				return ret;     
			break;
			case POS_TOP_UP:					 
			ret = Mobile_Top_UpInput(POS_TOP_UP); 
			if(ret != 0)
				return ret;     
			break;                     
		default:
			return ESC;
	}
	ret = CommProcess();
	
	CommDebugInfo("EntryMode aa", PosCom.stTrans.EntryMode, sizeof(PosCom.stTrans.EntryMode), 1);
	if(PosCom.stTrans.EntryMode[0] == PAN_PAYWAVE)
		tret = PaywaveTransComplete();
	else if(PosCom.stTrans.EntryMode[0] == PAN_ICCARD && EmvIsFullTrans() == 0x01)
	{
		tret = EMVICCOnlineTransComplete(ONLINE_FAILED);
		//script process	 
	}

	if(ret == E_RESOLVE_PACKET)
		return ret;
		
	if(tret != EMV_OK)												 
	{     
		//emv save reversal data									 
		return tret;
	}	

	CommDebugInfo("DispResult", " ", 1, 0);
	DispResult(ret);		 

	if( (PosCom.stTrans.Trans_id == POS_PAPER_OUT) && (ret == 0) ){
		PaperOut = PAPER_OUT_REPORTED;
	}

	if(PosCom.stTrans.Trans_id == POS_WORKING_KEY){
		if(ret == 0)
			SetPinkey(PosCom.stTrans.WKey, 0);
		return ret;
	}

	else if( ret != E_ERR_CONNECT &&\
			(PosCom.stTrans.Trans_id == POS_PURCHASE) || (PosCom.stTrans.Trans_id == POS_PURCHASE_REFUND)\
			) 
	{
#ifdef _MACHINE_V37
// same code to be shared among v36 (non printer), and v71 (printer) devices.
// great use of macros here!
#else
		
		if (ret == 0)
			pret = PrtTicket(&PosCom.stTrans, 0, (u8)reprnflag);
		else
			pret = PrtTicket(&PosCom.stTrans, 1, (u8)reprnflag);
		
		if(pret == ESC){
			LPrompt("Print Cancelled", "إلغاء الطباعة");
		}
		else if(pret != 0)
			LPrompt("Print Failed", "فشل الطباعة");
			
#endif
	}		 
	
out:
	return 0;
}
 
int PurchaseInput(int type)
{
	int ret;	

	if(type == POS_PURCHASE_REFUND){
		LDispTitle_AE("Purchase Refund", "ارجاع المشتريات");
		LScrDisp_AE(LINE2, 0, "Enter Trace Audit No:", "ادخل رقم تتبع النظام:", LDISP);	
		ret = GetTraceAuditNo(&PosCom.stTrans.RefundTraceNo);
		if(ret != 0)
			return ret;
		ScrClrLine_Api(LINE2, LINE7);
	}
	else
		LDispTitle_AE("Purchase", "الشراء");

	LScrDisp_AE(LINE2, 0, "Enter Amount:", "ادخل المبلغ:", LDISP);
	if(GetAmount(PosCom.stTrans.TradeAmount) != 0)
		return 1;

	PosCom.HaveInputAmt = 1;
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE2, 0, "Please Swipe Your Card:", "الرجاء تمرير البطاقة:", LDISP);
	ret = GetCard(MASK_INCARDNO_MAGCARD|MASK_INCARDNO_ICC|MASK_INCARDNO_PICC, CARD_EMVFULLNOCASH|CARD_QPASSONLINE);
	CommDebugInfo("GetCard", (u8 *)&ret, sizeof(ret), 1);
	if(ret != 0)
		return (ret);
 
	if(PosCom.stTrans.EntryMode[0] == PAN_KEYIN || PosCom.stTrans.EntryMode[0] == PAN_MAGCARD ) 	
	{
		ret = EnterPIN(0);
		if(ret)
			return ret;
	}
	return 0;
}
/////////////ali 2020 changes add card to card and mobile topup
int Card_To_CardInput(int type)
{
	
		LDispTitle_AE("Card To Card transaction", "تحويل من بطاقة الى بطاقة");

	LScrDisp_AE(LINE2, 0, "Enter Amount:", "ادخل المبلغ:", LDISP);
	if(GetAmount(PosCom.stTrans.TradeAmount) != 0)
		return 1;

	PosCom.HaveInputAmt = 1;
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE2, 0, "Please Swipe Your Card:", "الرجاء تمرير البطاقة:", LDISP);
	ret = GetCard(MASK_INCARDNO_MAGCARD|MASK_INCARDNO_ICC|MASK_INCARDNO_PICC, CARD_EMVFULLNOCASH|CARD_QPASSONLINE);
	CommDebugInfo("GetCard", (u8 *)&ret, sizeof(ret), 1);
	if(ret != 0)
		return (ret);
 
	if(PosCom.stTrans.EntryMode[0] == PAN_KEYIN || PosCom.stTrans.EntryMode[0] == PAN_MAGCARD ) 	
	{
		ret = EnterPIN(0);
		if(ret)
			return ret;
	}
	///toCard
	LScrDisp_AE(LINE2, 0, "Enter toCard:", " ادخل رقم بطاقة المستلم", LDISP);
	if(GetTraceAuditNo(PosCom.stTrans.toCard) != 0)
		return 1;
	return 0;
}
////mobile top up
int Mobile_Top_UpInput(int type)
{
	
		LDispTitle_AE("Card To Card transaction", "تحويل من بطاقة الى بطاقة");

	LScrDisp_AE(LINE2, 0, "Enter Amount:", "ادخل المبلغ:", LDISP);
	if(GetAmount(PosCom.stTrans.TradeAmount) != 0)
		return 1;

	PosCom.HaveInputAmt = 1;
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE2, 0, "Please Swipe Your Card:", "الرجاء تمرير البطاقة:", LDISP);
	ret = GetCard(MASK_INCARDNO_MAGCARD|MASK_INCARDNO_ICC|MASK_INCARDNO_PICC, CARD_EMVFULLNOCASH|CARD_QPASSONLINE);
	CommDebugInfo("GetCard", (u8 *)&ret, sizeof(ret), 1);
	if(ret != 0)
		return (ret);
 
	if(PosCom.stTrans.EntryMode[0] == PAN_KEYIN || PosCom.stTrans.EntryMode[0] == PAN_MAGCARD ) 	
	{
		ret = EnterPIN(0);
		if(ret)
			return ret;
	}
	///chose operator 
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE2, 0, "Chose operator :", "اختر الشبكة", LDISP);
	LScrDisp_AE(LINE3, 0, "1=zain 2=mtn 3=sudani :", "1=زين 2=ام تي ان  3=سوداني", LDISP);
	if(GetTraceAuditNo(PosCom.stTrans.code) != 0)
		return 1;
	if(PosCom.stTrans.code=1)PosCom.stTrans.code=0010010001;
	if(PosCom.stTrans.code=2)PosCom.stTrans.code=0010010003;
	if(PosCom.stTrans.code=3)PosCom.stTrans.code=0010010005;
	ScrClrLine_Api(LINE2, LINE3);
	///phone GetTraceAuditNo
	ScrClrLine_Api(LINE2, LINE3);
	LScrDisp_AE(LINE2, 0, "write phone number :", " رقم الهاتف", LDISP);
	if(GetTraceAuditNo(PosCom.stTrans.phone)!= 0)
		return 1;
	return 0;
}




///////////////////// 
void DispResult(int rej)
{
	char DispBuf[32], sTemp[32];

	memset(DispBuf, 0, sizeof(DispBuf));
	memset(sTemp, 0, sizeof(sTemp));

	ScrClrLine_Api(LINE2, LINE7);
	if(rej == NO_DISP)
		return;
	if( ((rej == 0) && (PosCom.stTrans.ResponseCode == 0)) || ((rej == 0) && (PosCom.stTrans.Trans_id == POS_PAPER_OUT)) )
	{
		Beep_Api(BEEPNORMAL);
		LPrompt("Successful Transaction", "المعاملة ناجحة");

		// ScrClrLine_Api(LINE2, LINE7);
		// sprintf(DispBuf, "ResCode: %d", PosCom.stTrans.ResponseCode);
		// ScrDisp_Api(LINE3, 0, DispBuf, FDISP|LDISP);

		// memset(DispBuf, 0, sizeof(DispBuf));
		// sprintf(DispBuf, "ResMsg: %s", PosCom.stTrans.ResponseMessage);
		// ScrDisp_Api(LINE5, 0, DispBuf, FDISP|LDISP);	

		// WaitEnterAndEscKey_Api(5);
	}
	else
	{
		Beep_Api(BEEPERROR);
		if(rej == E_TRANS_FAIL)
		{
			LPrompt("Transaction Failed", "المعاملة فشلت");
			// WaitAnyKey_Api(5);

			ScrClrLine_Api(LINE2, LINE7);
			if(PosCom.stTrans.ResponseCode != -1){
				// if(gCtrlParam.Lang == ENGLISH_LANG)
				sprintf_str(DispBuf, "Response Code: %d", "رقم الاستجابة: %d", PosCom.stTrans.ResponseCode);
				// else	
					// sprintf(DispBuf, "رقم الاستجابة: %d", PosCom.stTrans.ResponseCode);
				
				LScrDisp_AE(LINE3, 0, DispBuf, DispBuf, LDISP);
			}

			LScrDisp_AE(LINE4, 0, "Response Message:", "الاستجابة:", LDISP);
			// if(gCtrlParam.Lang == ENGLISH_LANG)
			sprintf(DispBuf, "%s", PosCom.stTrans.ResponseMessage);
			// else		
				// sprintf(DispBuf, "الاستجابة: %s", PosCom.stTrans.ResponseMessage);
			
			LScrDisp_AE(LINE5, 0, DispBuf, DispBuf, LDISP);
		}
		else
		{
			ConvErrCode(rej, DispBuf);
			if(gCtrlParam.Lang == ENGLISH_LANG)
				LPrompt(DispBuf, "");
			else
				LPrompt("", DispBuf);

			if(strlen(PosCom.stTrans.ResponseMessage) != 0){
				ScrClrLine_Api(LINE2, LINE7);
				LScrDisp_AE(LINE4, 0, "Response Message:", "الاستجابة:", LDISP);
				// if(gCtrlParam.Lang == ENGLISH_LANG)
				sprintf(DispBuf, "%s", PosCom.stTrans.ResponseMessage);
				// else		
					// sprintf(DispBuf, "الاستجابة: %s", PosCom.stTrans.ResponseMessage);
				LScrDisp_AE(LINE5, 0, DispBuf, DispBuf, LDISP);
			}
		}

		WaitAnyKey_Api(10);
	}	
	return;
}
 

void ShowLog(LOG_STRC *pLog)
{
	char buf[32], tdName[40];
	
	memset(buf, 0, sizeof(buf));
	memset(tdName, 0, sizeof(tdName));
	
	ConverTranNameCh((u8)pLog->Trans_id, tdName);
	if(pLog->ucRecFalg == RECORDVOID)
		strcat(tdName, "(Revoked)");


	ScrCls_Api();
	ScrDisp_Api(LINE1, 0, tdName, LDISP);

	sprintf(buf, "CAED NO.:(%12s)", pLog->SysReferNo);
	ScrDisp_Api(LINE2, 0, buf, LDISP);
	ScrDisp_Api(LINE3, 0, pLog->MainAcc, FDISP);

	strcpy(buf, "Amount: ");
	FormatAmtToDisp_Api(buf+strlen(buf), pLog->TradeAmount, 0);
	ScrDisp_Api(LINE4, 0, buf, LDISP);
	
	sprintf(buf, "Time: %02x/%02x %02x:%02x:%02x", pLog->TradeDate[2], pLog->TradeDate[3],
	pLog->TradeTime[0], pLog->TradeTime[1], pLog->TradeTime[2]);

	ScrDisp_Api(LINE5, 0, buf, LDISP);	
}
 
 
BOOL SearchLogByTraceNo(LOG_STRC *pLog, u32 traceNo)
{
	int i;
	
	for(i = 0; i < (int)gCtrlParam.iTransNum; ++i)
	{
		if(ReadLog(pLog, i) == 0)
		{
			if(pLog->lTraceNo == traceNo)
				return TRUE;
		}
	}
	return FALSE;
}
 
