//#include "stdafx.h"
#include "../inc/global.h"

typedef struct {
	int Timer;
	int Sec;
}JumpSecStru;

JumpSecStru   g_MyJumpSec = {0};

#ifdef WIN32
unsigned char CommDomainStart_Api(char* pDomainIn, char* pPortIn){return 0;}
#endif

int CommProcess(void)
{
	u8 PinEntryTimes, tmp[32], i;
	u8 recvBuf[RECVPACKLEN], sendBuf[SENDPACKLEN];
	u16 PackLen;
	int ret;

	memset(recvBuf, 0, sizeof(recvBuf));
	memset(sendBuf, 0, sizeof(sendBuf));

#ifdef _RECVDATA_DEBUG_	//make this transaction as offline transaction 
	// PosCom.stTrans.IccOnline = 0;
	// memset(PosCom.stTrans.SysReferNo, 0, sizeof(PosCom.stTrans.SysReferNo));
	// BcdToAsc_Api(PosCom.stTrans.SysReferNo, PosCom.stTrans.TradeDate, 8);
	// BcdToAsc_Api(PosCom.stTrans.SysReferNo+8, PosCom.stTrans.TradeTime, 4);
	// BcdToAsc_Api(PosCom.stTrans.AuthCode, PosCom.stTrans.TradeTime, 6);;
#endif

	//online process
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE3, 0, "Processing", "جاري المعالجة", CDISP);
	LScrDisp_AE(LINE4, 0, "Please Wait", "الرجاء الانتظار", CDISP);		
	memset(sendBuf, 0, sizeof(sendBuf));
	ret = CommMakeSendbuf(sendBuf, &PackLen);
	if(ret != 0)
		return ret;

	memset(recvBuf, 0, sizeof(recvBuf));
	ret = SendRecvPacket(sendBuf, PackLen, recvBuf, &PackLen);
	if(ret != 0)
		return ret;
 
	ret = ResolveResponse(recvBuf, PackLen);
	if(ret != 0)
		return ret;

	return ret;
}

int ResolveResponse(char *data, int len){
	int i, res_status=0, slen, msglen=31;
	char *line = NULL;
	char *status_code = NULL;
    cJSON *body = NULL;
	cJSON *res_body = NULL;
    cJSON *para = NULL;

	status_code = strstr(data, "HTTP/1.1 ");
    if(status_code == NULL)
        goto end;
    
	status_code += 9;
    if(memcmp(status_code, "200", 3) != 0)
        res_status = E_TRANS_FAIL;

	if(PosCom.stTrans.Trans_id == POS_PAPER_OUT){
		cJSON_Delete(body);
		return res_status;   
	}

	line = strstr(data, "\n");
	slen = line - status_code - 1;
	memcpy(PosCom.stTrans.ResponseMessage, status_code, slen > msglen ? msglen:slen);
	PosCom.stTrans.ResponseCode = -1;

    data = strstr(data, "{");
    if(data == NULL)
		goto end;	

	// CommDebugInfo("bodyFound", "log", 3, 0);            
    body = cJSON_Parse(data);
    if(body == NULL){
        const char *err = cJSON_GetErrorPtr();
        if(err != NULL)
            CommDebugInfo("Error Resolve Packet Before:", err, strlen(err), 0);
        goto end;
    }

	CommDebugInfo("Response Unpacked", " ", 1, 0);
	if(res_status == E_TRANS_FAIL){
        res_body = cJSON_GetObjectItem(body, "message");
    
        if( (res_body == NULL) || cJSON_IsNull(res_body) )
            goto end;
        
        if( memcmp(res_body->valuestring, "EBSError", 8) == 0 ){
            res_body = cJSON_GetObjectItem(body, "details");
            if( (res_body == NULL) || cJSON_IsNull(res_body) )
                goto end;
            
			CommDebugInfo("EBSError", " ", 1, 0);
            para = cJSON_GetObjectItem(res_body, "responseCode");
            if( (para != NULL) && cJSON_IsNumber(para) ){
                PosCom.stTrans.ResponseCode = para->valueint;
            }else goto end;
            
            para = cJSON_GetObjectItem(res_body, "responseMessage");
            if( (para != NULL) && cJSON_IsString(para) ){
                memset(PosCom.stTrans.ResponseMessage, 0, sizeof(PosCom.stTrans.ResponseMessage));
				memcpy(PosCom.stTrans.ResponseMessage, para->valuestring, strlen(para->valuestring) > msglen ? msglen:strlen(para->valuestring));
            }else goto end;

			para = cJSON_GetObjectItem(res_body, "systemTraceAuditNumber");
			if( (para != NULL) && cJSON_IsNumber(para) ){
				PosCom.stTrans.ResTraceNo = para->valueint;
			}

			switch (PosCom.stTrans.Trans_id)
			{
			case POS_PURCHASE:
			case POS_PURCHASE_REFUND:
			case POS_CARD_TO_CARD:
			case POS_TOP_UP:

				para = cJSON_GetObjectItem(res_body, "tranAmount");
				if( (para != NULL) && cJSON_IsNumber(para) ){
					PosCom.stTrans.tranamount = para->valuedouble;
				}

				para = cJSON_GetObjectItem(res_body, "tranFee");
				if( (para != NULL) && cJSON_IsNumber(para) ){
					PosCom.stTrans.TranFee = para->valuedouble;
				}
				break;
			default:
				break;
			}
		}

        else{
            memset(PosCom.stTrans.ResponseMessage, 0, sizeof(PosCom.stTrans.ResponseMessage));
			memcpy(PosCom.stTrans.ResponseMessage, res_body->valuestring, strlen(res_body->valuestring) > msglen ? msglen:strlen(res_body->valuestring));
        }

        cJSON_Delete(body);
		return E_TRANS_FAIL;       
    }

	
	res_body = cJSON_GetObjectItem(body, "ebs_response");

	if( (res_body == NULL) && cJSON_IsNull(res_body) )
		goto end;
			
	para = cJSON_GetObjectItem(res_body, "responseCode");
	if( (para != NULL) && cJSON_IsNumber(para) ){
		PosCom.stTrans.ResponseCode = para->valueint;
	}else goto end;
	
	para = cJSON_GetObjectItem(res_body, "responseMessage");
	if( (para != NULL) && cJSON_IsString(para) ){
		memset(PosCom.stTrans.ResponseMessage, 0, sizeof(PosCom.stTrans.ResponseMessage));
		memcpy(PosCom.stTrans.ResponseMessage, para->valuestring, strlen(para->valuestring) > msglen ? msglen:strlen(para->valuestring));
	}else goto end;

	para = cJSON_GetObjectItem(res_body, "responseStatus");
	if( (para != NULL) && cJSON_IsString(para) ){
		memcpy(PosCom.stTrans.ResponseStatus, para->valuestring, strlen(para->valuestring));
	}else goto end;

	switch (PosCom.stTrans.Trans_id)
	{
	case POS_PURCHASE:
	case POS_PURCHASE_REFUND:
	case POS_CARD_TO_CARD:
	case POS_TOP_UP:
		para = cJSON_GetObjectItem(res_body, "referenceNumber");
		if( (para != NULL) && cJSON_IsString(para) ){
			memcpy(PosCom.stTrans.ReferenceNo, para->valuestring, strlen(para->valuestring));
		}// else goto end;
		
		para = cJSON_GetObjectItem(res_body, "tranFee");
		if( (para != NULL) && cJSON_IsNumber(para) ){
			PosCom.stTrans.TranFee = para->valuedouble;
		}

		para = cJSON_GetObjectItem(res_body, "tranAmount");
		if( (para != NULL) && cJSON_IsNumber(para) ){
			PosCom.stTrans.tranamount = para->valuedouble;
		}
		
		para = cJSON_GetObjectItem(res_body, "approvalCode");
		if( (para != NULL) && cJSON_IsString(para) ){
			memcpy(PosCom.stTrans.ApprovalCode, para->valuestring, strlen(para->valuestring));
		}

		para = cJSON_GetObjectItem(res_body, "systemTraceAuditNumber");
		if( (para != NULL) && cJSON_IsNumber(para) ){
			PosCom.stTrans.ResTraceNo = para->valueint;
		}
		break;
	
	case POS_WORKING_KEY:
		para = cJSON_GetObjectItem(res_body, "workingKey");
		if( (para != NULL) && cJSON_IsString(para) ){
			memcpy(PosCom.stTrans.WKey, para->valuestring, strlen(para->valuestring));
		}else goto end;
		break;

	default:
		break;
	}
			
    cJSON_Delete(body);
    return 0;
    
end:
	cJSON_Delete(body);	
	return E_RESOLVE_PACKET;
}
 
void getTranDateTime(char *tranDateTime){
	time_t tranTime;
	struct tm *timeInfo;

	tranTime = time(NULL);
	timeInfo = localtime(&tranTime);
	strftime(tranDateTime, strlen("YYMMDDHHMMSS")+1, "%y%m%d%H%M%S", timeInfo);
}

int CommMakeSendbuf(u8 *sendBuf, u16 *pLen)
{
	char *buf = NULL;
	u8 tmp[128], url[128], asc[64];
	fp64 amt;
	fp64 rounded_down;
	int len;
	cJSON *body = NULL;

	memset(tmp, 0, sizeof(tmp));
	memset(url, 0, sizeof(url));
	memset(asc, 0, sizeof(asc));

	body = cJSON_CreateObject();
	cJSON_AddStringToObject(body, "clientId", PosCom.stTrans.ClientId);
	cJSON_AddStringToObject(body, "terminalId", PosCom.stTrans.TerminalNo);
	cJSON_AddNumberToObject(body, "systemTraceAuditNumber", PosCom.stTrans.lTraceNo);
	getTranDateTime(tmp);
	cJSON_AddStringToObject(body, "tranDateTime", tmp);
	
	switch (PosCom.stTrans.Trans_id)
	{
	case POS_WORKING_KEY:
		// strcpy(url, "POST /api/blabla HTTP/1.1\r\n");
		strcpy(url, "POST /api/workingKey HTTP/1.1\r\n");
		break;
	
	case POS_PAPER_OUT:
		strcpy(url, "POST /api/dashboard/issues HTTP/1.1\r\n");
		
		break;
	
	case POS_PURCHASE:
		strcpy(url, "POST /api/purchase HTTP/1.1\r\n");
		cJSON_AddStringToObject(body, "tranCurrencyCode", "SDG");
		cJSON_AddStringToObject(body, "PAN", PosCom.stTrans.MainAcc);	//PosCom.stTrans.MainAcc); "9222081700176714465"
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.ExpDate, 4);
		cJSON_AddStringToObject(body, "expDate", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.sPIN, 16);
		cJSON_AddStringToObject(body, "PIN", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(asc, PosCom.stTrans.TradeAmount, 12);
		FormatAmt_Str(tmp, asc);
		amt = atof(tmp);
		rounded_down = floorf(amt * 100) / 100;
		cJSON_AddNumberToObject(body, "tranAmount", rounded_down);
		break;

	case POS_PURCHASE_REFUND:
		strcpy(url, "POST /play/refund HTTP/1.1\r\n");
		cJSON_AddNumberToObject(body, "originalSystemTraceAuditNumber", PosCom.stTrans.RefundTraceNo);
		cJSON_AddStringToObject(body, "tranCurrencyCode", "SDG");
		cJSON_AddStringToObject(body, "PAN", PosCom.stTrans.MainAcc);	//PosCom.stTrans.MainAcc); "9222081700176714465"
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.ExpDate, 4);
		cJSON_AddStringToObject(body, "expDate", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.sPIN, 16);
		cJSON_AddStringToObject(body, "PIN", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(asc, PosCom.stTrans.TradeAmount, 12);
		FormatAmt_Str(tmp, asc);
		amt = atof(tmp);
		rounded_down = floorf(amt * 100) / 100;
		cJSON_AddNumberToObject(body, "tranAmount", rounded_down);
		break;
		case POS_CARD_TO_CARD:
		strcpy(url, "POST /api/cardTransfer HTTP/1.1\r\n");
		cJSON_AddStringToObject(body, "tranCurrencyCode", "SDG");
		//cJSON_AddStringToObject(body, "clientId", PosCom.stTrans.ClientId);
		cJSON_AddStringToObject(body, "PAN", PosCom.stTrans.MainAcc);	//PosCom.stTrans.MainAcc); "9222081700176714465"
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.ExpDate, 4);
		cJSON_AddStringToObject(body, "expDate", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.sPIN, 16);
		cJSON_AddStringToObject(body, "PIN", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(asc, PosCom.stTrans.TradeAmount, 12);
		FormatAmt_Str(tmp, asc);
		amt = atof(tmp);
		rounded_down = floorf(amt * 100) / 100;
		cJSON_AddNumberToObject(body, "tranAmount", rounded_down);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.toCard, 19);
		cJSON_AddStringToObject(body, "toCard", tmp);	//PosCom.stTrans.MainAcc); "9222081700176714465"
		break;
		case POS_TOP_UP:
		strcpy(url, "POST /api/cardTransfer HTTP/1.1\r\n");
		cJSON_AddStringToObject(body, "tranCurrencyCode", "SDG");
		
		cJSON_AddStringToObject(body, "PAN", PosCom.stTrans.MainAcc);	//PosCom.stTrans.MainAcc); "9222081700176714465"
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.ExpDate, 4);
		cJSON_AddStringToObject(body, "expDate", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.sPIN, 16);
		cJSON_AddStringToObject(body, "PIN", tmp);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(asc, PosCom.stTrans.TradeAmount, 12);
		FormatAmt_Str(tmp, asc);
		amt = atof(tmp);
		rounded_down = floorf(amt * 100) / 100;
		cJSON_AddNumberToObject(body, "tranAmount", rounded_down);
		memset(tmp, 0, sizeof(tmp));
		BcdToAsc_Api(tmp, PosCom.stTrans.phone, 19);
		cJSON_AddStringToObject(body, "phone", tmp);
		BcdToAsc_Api(tmp, PosCom.stTrans.code, 10);
		cJSON_AddStringToObject(body, "code", tmp);	
		break;
	
	default:
		break;
	}

	buf = cJSON_PrintUnformatted(body);
	memcpy(sendBuf, url, strlen(url));
	sprintf(sendBuf+strlen(sendBuf), "Host: %s\r\n", G_CommPara.GprsSet.GprsIp1);
	strcat(sendBuf, "Content-Type: application/json\r\n");
	len = strlen(buf);
	sprintf(sendBuf+strlen(sendBuf), "Content-Length: %d\r\n\r\n", len);
	memcpy(sendBuf+strlen(sendBuf), buf, len);
	*pLen = strlen(sendBuf);
	// CommDebugInfo("Request", sendBuf, *pLen, 0);
	cJSON_Delete(body);

	return 0;
}

int ModemConnect(unsigned char Timeout)
{
	int ret;
	char dispbuf[32];
	
	ScrBackLight_Api(60);
	while(1)
	{
		ScrClrLine_Api(LINE2, LINE7);
		LScrDisp_AE(LINE4, 0, "Connecting...", "جاري الاتصال...", CDISP);
		
		LedLightOff_Api(0x0f);   
		LedLightOn_Api(0x01);    
		
		ScrBackLight_Api(Timeout+10);
		StartJumpSec();

		// if(G_CommPara.GprsSet.GprsDomainOrIp == GPRS_IP)
		CommStart_Api();		// IP Address
		// else
		// 	CommDomainStart_Api(G_CommPara.GprsSet.GprsIp1, G_CommPara.GprsSet.GprsPort1);	// Domain Name Address

		ret = CommCheck_Api(60);
		
		if(ret == 9)
		{
			ScrClrLine_Api(LINE2, LINE7);
			LScrDisp_AE(LINE2, 0, "put phone back to base", "put phone back to base", CDISP);
			LScrDisp_AE(LINE3, 0, "any key to redail", "any key to redail", CDISP);
			StopJumpSec();
			LedLightOff_Api(0x01);   
			LedLightOn_Api(0x04);    
			WaitAnyKey_Api(60);
		}
		else		 
		{
			break;
		}
	}

	switch(ret)
	{
		case 0:
			break;
		case 1: 
			ScrDisp_Api(LINE2, 0, "busy", CDISP);
			ret = ERRCODE_ENGAGED;
			break;
		case 2: 
			ScrDisp_Api(LINE2, 0, "attach tephone line", CDISP);
			ret = ERRCODE_NOLINE;
			break;
		case 3: 
			ScrDisp_Api(LINE2, 0, "busy", CDISP);
			ret = ERRCODE_ENGAGED;
			break;
		case 4: 
			ScrDisp_Api(LINE2, 0, "wave lost", CDISP);
			ret = ERRCODE_WAVELOST;
			break;
		case ESC:
			ScrDisp_Api(LINE2, 0, "user cancel", CDISP);
			ret = DIAL_CANCEL;
			break;
		case 8:		 
		case 0xFF:
		default:
			if(gCtrlParam.Lang == ENGLISH_LANG)
				sprintf(dispbuf, "connect failed %02x", ret);
			else
				sprintf(dispbuf, "فشل الاتصال %02x", ret);
				
			ScrDisp_Api(LINE2, 0, dispbuf, CDISP);
			ret = ERRCODE_CONNECTFAIL;
			break;
	}
	
	if(ret != 0)
	{
		CommHangUp();
		WaitAnyKey_Api(1);
	}
	StopJumpSec();
	LedLightOff_Api(0x01);   
	LedLightOn_Api(0x04);    
	
	return ret;
}
 
int SendPacket(unsigned char *SendData, unsigned short PacketLen)
{
	return( CommTxd_Api(SendData, PacketLen, 1) );
}
 
int RecvPacket(u8 *Packet, unsigned short *PacketLen, int WaitTime)
{
	u8 Ret;
	
	LedLightOff_Api(0x0f);   
	LedLightOn_Api(0x08);    
	
	StartJumpSec();
	
	Ret = CommRxd_Api(Packet, PacketLen, 1, 0, gCtrlParam.tradeTimeoutValue*1000);
	
	StopJumpSec();
	LedLightOff_Api(0x08);   
	LedLightOn_Api(0x04);    

	if( Ret== 0x00)
	{
		return 0;
	}
	else
	{
		CommHangUp();
		return E_REVTIMEOUT;
	}
}
 
int SendRecvData(u8 *SendBuf, long Senlen, u8 *RecvBuf, u16 *RecvLen, int psWaitTime)
{
	int ret;
	
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE4, 0, "Sending..", "جاري الارسال...", CDISP);
	if(SendPacket(SendBuf, (u16)Senlen) != 0)
		return E_SEND_PACKET;						 
	
	ScrClrLine_Api(LINE2, LINE7);
	LScrDisp_AE(LINE4, 0, "Receiving..", "جاري الاستقبال...", CDISP);
	*RecvLen = 0;
	ret = RecvPacket(RecvBuf, (u16*)RecvLen, psWaitTime);

	return ret;
}
 
int SendRecvPacket(u8 *SendBuf, u16 Senlen, u8 *RecvBuf, u16 *pRecvLen)
{
	int ret;
	
	ret = Connect(60);
	if(ret != 0)
	{
		if(ret < 0)
			return NO_DISP;
		
		ScrClrLine_Api(LINE2, LINE7);
		LScrDisp_AE(LINE2, 0, "dail failed", "فشل الاتصال", CDISP);
		LScrDisp_AE(LINE3, 0, "redail or not", "هل تود اعادة الاتصال؟", CDISP);
		LScrDisp_AE(LINE5, 0, "YES      NO", "نعم      لا", CDISP);
		ret = WaitEnterAndEscKey_Api(20);
		if(ret != ENTER)
			return E_ERR_CONNECT;							 
		else                                                 
		{                                                    
			ScrClrLine_Api(LINE2, LINE7);                    
			if(Connect(60) != 0)                             	
				return E_ERR_CONNECT;                           
		}                                                    
	}       
	                                                                                                              
	TraceNoInc();                                                                                             

	CommDebugInfo("request:", SendBuf, Senlen, 0);

	//if(SaveReversalData("06") != 0)   //for transactions that have reversal						 
	ret = SendRecvData(SendBuf, Senlen, RecvBuf, pRecvLen, gCtrlParam.tradeTimeoutValue);//60
	
	CommDebugInfo("response:", RecvBuf, *pRecvLen, 0);
	
	if(ret != 0)
	{
		if(ret == E_REVTIMEOUT)
		{
			ReadReversalData();
			SaveReversalData("98");	 
		}                            
		else                         
			ret = E_RECV_PACKET;	 
	}                                
	                                 
	return ret;
}
 
void CommHangUp()
{	
	CommClose_Api();
	return;
}
 
int Connect(unsigned char TimeOut)
{
	switch(ModemConnect(TimeOut))
	{
		case 0:
			return 0;
		case DIAL_CANCEL:
			return DIAL_CANCEL;
		default:
			return ERRCODE_DIALFAIL;				 
	}
}


void DispJumpSec(void)
{
	char DispBuf[20];
	
	memset(DispBuf, 0, sizeof(DispBuf));
	g_MyJumpSec.Sec++;
	sprintf(DispBuf, "%d", g_MyJumpSec.Sec);
	ScrDisp_Api(LINE6, 0, DispBuf, CDISP);
}
 
int StartJumpSec(void)
{
	g_MyJumpSec.Timer = CommStartJumpSec_Api(DispJumpSec);
	return 0;
} 
void StopJumpSec(void)
{
	g_MyJumpSec.Sec = 0;
	CommStopJumpSec_Api(g_MyJumpSec.Timer);
}
