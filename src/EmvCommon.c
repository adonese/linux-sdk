//#include "stdafx.h"
#include "../inc/global.h"

                                               
//u8 g_bSupportVipVsdtJcb=0;					 
int gnTermAidNum;                              
static int   iAuthDataLen, iScriptLen;         
static unsigned char sAuthData[16];				 
static unsigned char sIssuerScript[256];		      
                                               
static int Tag71or72 = 0;						 


int EmvInitDefParam(void)
{
	Common_GetParam_Api(&termParam); 
	EMV_GetParam_Api(&stEmvParam);

	sprintf((char *)termParam.MerchName, "%.40s", gCtrlParam.MerchantName);	 
	memcpy(termParam.MerchCateCode, "\x00\x01", 2);                             
	strcpy((char*)termParam.MerchName, gCtrlParam.MerchantName);                
	memcpy(termParam.MerchId, gCtrlParam.MerchantNo, 15);						 
	memcpy(termParam.TermId, gCtrlParam.TerminalNo, 8);		
	strcpy((char *)termParam.AcquierId, "12345678");                                                                                                                                                
	memcpy(termParam.CountryCode,   "\x08\x40", 2);							 
	memcpy(termParam.TransCurrCode, "\x08\x40", 2);							 
	memcpy(termParam.ReferCurrCode, "\x08\x40", 2);					   
	termParam.TransCurrExp = 0x02;
	termParam.ReferCurrExp = 0x02;
	termParam.ReferCurrCon = 1000;
	termParam.TerminalType = 0x22;
	//termParam.TransType = tradetype;
	memcpy(stEmvParam.Capability, "\xE0\xF1\xC8", 3);	
	//memcpy(stEmvParam.ExCapability, "\x60\x00\xF0\xA0\x01", 5);
	stEmvParam.ForceOnline = 1;
	stEmvParam.GetDataPIN    = 1;												 
	stEmvParam.SupportPSESel = 1;

	Common_SetParam_Api(&termParam);
	Common_SaveParam_Api(&termParam);
	EMV_SetParam_Api(&stEmvParam);	
	EMV_SaveParam_Api(&stEmvParam);
									 
	return 0;
}
 
int EmvIsFullTrans(void)
{
	return(g_EmvFullOrSim);
}
 
int EMVICCOnlineTransComplete( int IsOnLineSucc)
{
    int ret = 0, IAuthDataLen = 0, ScriptLen = 0, AuthCodeLen = 0, Curp = 0;
    unsigned char RspCode[2], AuthCode[6], IAuthData[16], Script[400];//300

	memset(RspCode, 0, sizeof(RspCode));
	memset(AuthCode, 0, sizeof(AuthCode));
	memset(AuthCode, 0, sizeof(AuthCode));
	memset(IAuthData, 0, sizeof(IAuthData));
	
	DisplayProcessing();
	//
	ret = EMV_Complete_Api((u8)ret,RspCode,AuthCode,AuthCodeLen,IAuthData,IAuthDataLen,Script,ScriptLen);
	CommDebugInfo("EMV_Complete_Api", (u8 *)&ret, sizeof(ret), 1);
#ifdef _RECVDATA_DEBUG_
	ret = 0 ; //for test
#endif
	if(ScriptLen != 0)
		SaveScriptResult();	 

	if(ret)
		return ret;
	EmvSaveDataOnline();

	if(IsOnLineSucc == ONLINE_APPROVE)
	{
		memset(Script, 0, sizeof(Script));
		ret = Common_GetTLV_Api(0x9f26, Script, &ScriptLen);
		if(ret == EMV_OK)
		{
			memcpy(IAuthData, "\x9F\x26\x08", 3);
			memcpy(IAuthData+3, Script, 8);
			memcpy(PosCom.stTrans.IccData, IAuthData, 11);
			Curp += 11;
		}
		memset(Script, 0, sizeof(Script));
		ret = Common_GetTLV_Api(0x9f27, Script, &ScriptLen);
		if(ret == EMV_OK)
		{
			memcpy(IAuthData, "\x9F\x27\x01", 3);
			memcpy(IAuthData+3, Script, 1);
			memcpy(PosCom.stTrans.IccData + Curp, IAuthData, 4);
			Curp+=4;
		}
		memset(Script, 0, sizeof(Script));
		ret = Common_GetTLV_Api(0x8a, Script, &ScriptLen);
		if(ret == EMV_OK)
		{
			memcpy(IAuthData, "\x8a\x02", 2);
			memcpy(IAuthData+2, Script, 2);
			memcpy(PosCom.stTrans.IccData+PosCom.stTrans.nIccDataLen, IAuthData, 4);
			PosCom.stTrans.nIccDataLen += 4;
		}
	}
	return ret;
}

void RemoveTailChars(char* pString, char cRemove)
{
	int nLen = 0;

	nLen = strlen(pString);
	while(nLen)
	{
		nLen--;
		if(pString[nLen] == cRemove)
			pString[nLen] = 0;
		else
			break;
	};
} 
int MatchTrack2AndPan(u8 *pTrack2, u8 *pPan)
{
	int  i = 0;
	char szTemp[19+1], sTrack[256], sPan[256];

	memset(szTemp, 0, sizeof(szTemp));
	memset(sTrack, 0, sizeof(sTrack));
	memset(sPan, 0, sizeof(sPan));
	
	//track2
	BcdToAsc_Api(sTrack, &pTrack2[1], (u16)(pTrack2[0]*2));
	RemoveTailChars(sTrack, 'F');		// erase padded 'F' chars
	for(i=0; sTrack[i]!='\0'; i++)		// convert 'D' to '='
	{
		if(sTrack[i]=='D' )
		{
			sTrack[i] = '=';
			break;
		}
	}
	for(i=0; i<19 && sTrack[i]!='\0'; i++)
	{
		if(sTrack[i] == '=') break;
		szTemp[i] = sTrack[i];
	}
	szTemp[i] = 0;
	//pan
	BcdToAsc_Api(sPan, &pPan[1], (int)(pPan[0]*2));
	RemoveTailChars(sPan, 'F');         // erase padded 'F' chars

	if(strcmp(szTemp, sPan)==0)
		return 0;
	else
		return 1;
}
 
int GetEmvTrackData(u8 * pTrackBuf)
{
     int iRet = 0, iLength = 0;
     u8 sTemp[30], cHaveTrack2 = 0, cReadPan = 0, cRet = 0;

     cHaveTrack2 = 0;
     cReadPan = 0;
     // Read Track 2 Equivalent Data
     memset(sTemp, 0, sizeof(sTemp));
     iRet = Common_GetTLV_Api(0x57, sTemp, &iLength);
     if(iRet == EMV_OK)
     {
          cHaveTrack2 = 1;
		  
		  pTrackBuf[0] = iLength;
		  memcpy(&pTrackBuf[1], sTemp, iLength);
		  pTrackBuf[iLength+1] = 0;	// 
     }
     // read PAN
     memset(sTemp, 0, sizeof(sTemp));
     iRet = Common_GetTLV_Api(0x5A, &sTemp[1], &iLength);
	 sTemp[0] = iLength;
     if(iRet == EMV_OK)
     {
	 	if(cHaveTrack2)
	 	{
		 	cRet = MatchTrack2AndPan(pTrackBuf, sTemp);
			if(cRet != 0) return -1;
	 	}
		cReadPan = 1;
     }
	else
	{
		if(cHaveTrack2 == 0) return -2; 
	}
	// read PAN sequence number 
	iRet = Common_GetTLV_Api(0x5F34, &PosCom.stTrans.ucPanSeqNo, &iLength);
	if(iRet == EMV_OK)
		PosCom.stTrans.bPanSeqNoOk = 1;
	else
		PosCom.stTrans.bPanSeqNoOk = 0;
	// read Application Expiration Date
	if(cReadPan)
	{
		memset(sTemp, 0, sizeof(sTemp));
		iRet = Common_GetTLV_Api(0x5F24, sTemp, &iLength);
		if(iRet == EMV_OK)
		{
			memcpy(PosCom.stTrans.ExpDate, sTemp, 3);
		}
	}
	// read other data for print slip
	Common_GetTLV_Api(0x50, (u8*)PosCom.stTrans.szAppLable, &iLength);
	// read application preferred name
	Common_GetTLV_Api(0x9F12, PosCom.stTrans.szAppPreferName, &iLength);
	iRet = Common_GetTLV_Api(0x4F, sTemp, &iLength);
	if(iRet == EMV_OK)
	{
		BcdToAsc_Api(PosCom.stTrans.szAID, sTemp, (iLength*2));
		RemoveTailChars(PosCom.stTrans.szAID, 'F');
	}
	Common_GetTLV_Api(0x82, PosCom.stTrans.sAIP, &iLength);
	iRet = Common_GetTLV_Api(0x9F37, PosCom.stTrans.szUnknowNum, &iLength);
	 
	if( (PosCom.stTrans.Trans_id != POS_PURCHASE) && (PosCom.stTrans.Trans_id != POS_WORKING_KEY) )	//R POS_QUE) )
	{
		iRet = SetIssuerName();
		if(iRet != 0)
		{
			return -4;
		}
	}
	return 0;
}
 
int EmvCardProc(u8 tradetype, u8 cardtype, u8 *pTrackBuf)
{
	int iRet = 0, nLen = 0;
	u8 sTemp[256],buf[10];
	unsigned short ulen;

	memset(sTemp, 0, sizeof(sTemp));
	memset(buf, 0, sizeof(buf));

	DisplayProcessing();
	Common_SetIcCardType_Api(PEDICCARD, 0);	
	iRet = IccInit_Api(0, 1, sTemp, &ulen);
	if(iRet != 0)
		return -20; //ERR_ICCRESET   

	EMV_Clear_Api();
	EMV_SetTradeAmt_Api(PosCom.stTrans.TradeAmount, PosCom.stTrans.SecondAmount);
	if(cardtype&CARD_EMVSIMPLE)                  
		g_EmvFullOrSim = 0;						 
	else                                         
		g_EmvFullOrSim = 1;						 
                                                                     
	EmvInitDefParam();
	iRet = App_EMVTrans(tradetype, cardtype, pTrackBuf);
	return iRet;
}


void EmvSaveDataOnline(void)
{
    int iLength = 0;
	u8 Buf[100];
	
	memset(Buf, 0, sizeof(Buf));

	Common_GetTLV_Api(0x5f24, Buf, &iLength);		// 
	memcpy(PosCom.stTrans.ExpDate, Buf, 3);	
	Common_GetTLV_Api(0x5f20, (u8*)PosCom.stTrans.HoldCardName, &iLength);
	PosCom.stTrans.HoldCardName[iLength] = 0;

//Common_GetTLV_Api(0x9f74, (u8*)PosCom.stTrans.AuthCode,	&iLength);	// 
	Common_GetTLV_Api(0x9F26, PosCom.stTrans.sAppCrypto,   &iLength);
	Common_GetTLV_Api(0x8A,   PosCom.stTrans.sAuthRspCode, &iLength);
	Common_GetTLV_Api(0x95,   PosCom.stTrans.sTVR,         &iLength);
	Common_GetTLV_Api(0x9B,   PosCom.stTrans.sTSI,         &iLength);
	Common_GetTLV_Api(0x9F36, PosCom.stTrans.sATC,         &iLength);
} 
int EmvSaveData(void)
{
	int iLength = 0 ;
	unsigned char sTemp[32], buf[20];

	memset(sTemp, 0, sizeof(sTemp	));
	memset(buf	, 0, sizeof(buf	));

	//  
	if(!PosCom.stTrans.IccOnline && (PosCom.stTrans.Trans_id == POS_PURCHASE))
	{
		if(Common_GetTLV_Api(0x9F74, sTemp, &iLength) != 0)
		{
			Common_SetTLV_Api(0x9F74, (unsigned char *)"ECC001", 6);// 
		}
		GetSysTime_Api(sTemp);
		memcpy(PosCom.stTrans.TradeDate, sTemp, 4);
		memcpy(PosCom.stTrans.TradeTime, &sTemp[4], 3);
	}

	Common_GetTLV_Api(0x5f24, buf, &iLength);		 
	memcpy(PosCom.stTrans.ExpDate, buf, 3);	
	Common_GetTLV_Api(0x5f20, (u8*)PosCom.stTrans.HoldCardName, &iLength);
	PosCom.stTrans.HoldCardName[iLength] = 0;
	
//Common_GetTLV_Api(0x9f74, (u8*)PosCom.stTrans.AuthCode,	&iLength);	// 
	Common_GetTLV_Api(0x9F26, PosCom.stTrans.sAppCrypto,   &iLength);
	Common_GetTLV_Api(0x8A,   PosCom.stTrans.sAuthRspCode, &iLength);
	Common_GetTLV_Api(0x95,   PosCom.stTrans.sTVR,         &iLength);
	Common_GetTLV_Api(0x9B,   PosCom.stTrans.sTSI,         &iLength);
	Common_GetTLV_Api(0x9F36, PosCom.stTrans.sATC,         &iLength);
	return EMV_OK;
}


#if ( defined(EMVDEBUG) )

void AddCapkExample()
{
	int i, ret;
	EMV_CAPK tempCAPK;

	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;
	memcpy(tempCAPK.ExpDate, "\x25\x12\x31", 3);	
	AscToBcd_Api(tempCAPK.RID, "A000000333", 10);
	tempCAPK.KeyID = 0x03;
	tempCAPK.ModulLen = 352/2;
	AscToBcd_Api(tempCAPK.Modul, "B0627DEE87864F9C18C13B9A1F025448BF13C58380C91F4CEBA9F9BCB214FF8414E9B59D6ABA10F941C7331768F47B2127907D857FA39AAF8CE02045DD01619D689EE731C551159BE7EB2D51A372FF56B556E5CB2FDE36E23073A44CA215D6C26CA68847B388E39520E0026E62294B557D6470440CA0AEFC9438C923AEC9B2098D6D3A1AF5E8B1DE36F4B53040109D89B77CAFAF70C26C601ABDF59EEC0FDC8A99089140CD2E817E335175B03B7AA33D", 352);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	ret = Common_AddCapk_Api(&tempCAPK);

	//paypass
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.ArithInd = 0x01;
	tempCAPK.HashInd = 0x01;
	AscToBcd_Api(tempCAPK.RID, "A000000004", 10);
	tempCAPK.KeyID = 0x05;
	AscToBcd_Api(tempCAPK.Modul, "B8048ABC30C90D976336543E3FD7091C8FE4800DF820ED55E7E94813ED00555B573FECA3D84AF6131A651D66CFF4284FB13B635EDD0EE40176D8BF04B7FD1C7BACF9AC7327DFAA8AA72D10DB3B8E70B2DDD811CB4196525EA386ACC33C0D9D4575916469C4E4F53E8E1C912CC618CB22DDE7C3568E90022E6BBA770202E4522A2DD623D180E215BD1D1507FE3DC90CA310D27B3EFCCD8F83DE3052CAD1E48938C68D095AAC91B5F37E28BB49EC7ED597", 352);
	tempCAPK.ModulLen = (352/2);
	AscToBcd_Api(tempCAPK.Exponent, "030000", 6);
	tempCAPK.ExponentLen = 0x01;
	AscToBcd_Api(tempCAPK.ExpDate, "351231", 6);  //211231
	AscToBcd_Api(tempCAPK.CheckSum, "EBFA0D5D06D8CE702DA3EAE890701D45E274C845", 40);
	tempCAPK.KeyID = 0x05;
	ret = Common_AddCapk_Api(&tempCAPK);

	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.ArithInd = 0x01;
	tempCAPK.HashInd = 0x01;
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x50;
	tempCAPK.ModulLen = 256/2;
	AscToBcd_Api(tempCAPK.Modul, "D11197590057B84196C2F4D11A8F3C05408F422A35D702F90106EA5B019BB28AE607AA9CDEBCD0D81A38D48C7EBB0062D287369EC0C42124246AC30D80CD602AB7238D51084DED4698162C59D25EAC1E66255B4DB2352526EF0982C3B8AD3D1CCE85B01DB5788E75E09F44BE7361366DEF9D1E1317B05E5D0FF5290F88A0DB47", 256);
	tempCAPK.ExponentLen = 3;		
	AscToBcd_Api(tempCAPK.Exponent, "010001", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "B769775668CACB5D22A647D1D993141EDAB7237B", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x51;
	tempCAPK.ModulLen = 288/2;
	AscToBcd_Api(tempCAPK.Modul, "DB5FA29D1FDA8C1634B04DCCFF148ABEE63C772035C79851D3512107586E02A917F7C7E885E7C4A7D529710A145334CE67DC412CB1597B77AA2543B98D19CF2CB80C522BDBEA0F1B113FA2C86216C8C610A2D58F29CF3355CEB1BD3EF410D1EDD1F7AE0F16897979DE28C6EF293E0A19282BD1D793F1331523FC71A228800468C01A3653D14C6B4851A5C029478E757F", 288);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "B9D248075A3F23B522FE45573E04374DC4995D71", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	//3
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x53;
	tempCAPK.ModulLen = 496/2;
	AscToBcd_Api(tempCAPK.Modul, "BCD83721BE52CCCC4B6457321F22A7DC769F54EB8025913BE804D9EABBFA19B3D7C5D3CA658D768CAF57067EEC83C7E6E9F81D0586703ED9DDDADD20675D63424980B10EB364E81EB37DB40ED100344C928886FF4CCC37203EE6106D5B59D1AC102E2CD2D7AC17F4D96C398E5FD993ECB4FFDF79B17547FF9FA2AA8EEFD6CBDA124CBB17A0F8528146387135E226B005A474B9062FF264D2FF8EFA36814AA2950065B1B04C0A1AE9B2F69D4A4AA979D6CE95FEE9485ED0A03AEE9BD953E81CFD1EF6E814DFD3C2CE37AEFA38C1F9877371E91D6A5EB59FDEDF75D3325FA3CA66CDFBA0E57146CC789818FF06BE5FCC50ABD362AE4B80996D", 496);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "AC213A2E0D2C0CA35AD0201323536D58097E4E57", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	//4
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x96;
	tempCAPK.ModulLen = 256/2;
	AscToBcd_Api(tempCAPK.Modul, "B74586D19A207BE6627C5B0AAFBC44A2ECF5A2942D3A26CE19C4FFAEEE920521868922E893E7838225A3947A2614796FB2C0628CE8C11E3825A56D3B1BBAEF783A5C6A81F36F8625395126FA983C5216D3166D48ACDE8A431212FF763A7F79D9EDB7FED76B485DE45BEB829A3D4730848A366D3324C3027032FF8D16A1E44D8D", 256);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "7616E9AC8BE014AF88CA11A8FB17967B7394030E", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	//5
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x58;
	tempCAPK.ModulLen = 400/2;
	AscToBcd_Api(tempCAPK.Modul, "99552C4A1ECD68A0260157FC4151B5992837445D3FC57365CA5692C87BE358CDCDF2C92FB6837522842A48EB11CDFFE2FD91770C7221E4AF6207C2DE4004C7DEE1B6276DC62D52A87D2CD01FBF2DC4065DB52824D2A2167A06D19E6A0F781071CDB2DD314CB94441D8DC0E936317B77BF06F5177F6C5ABA3A3BC6AA30209C97260B7A1AD3A192C9B8CD1D153570AFCC87C3CD681D13E997FE33B3963A0A1C79772ACF991033E1B8397AD0341500E48A24770BC4CBE19D2CCF419504FDBF0389BC2F2FDCD4D44E61F", 400);
	tempCAPK.ExponentLen = 3;		
	AscToBcd_Api(tempCAPK.Exponent, "010001", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "753ED0AA23E4CD5ABD69EAE7904B684A34A57C22", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	//6
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x52;
	tempCAPK.ModulLen = 352/2;
	AscToBcd_Api(tempCAPK.Modul, "AFF740F8DBE763F333A1013A43722055C8E22F41779E219B0E1C409D60AFD45C8789C57EECD71EA4A269A675916CC1C5E1A05A35BD745A79F94555CE29612AC9338769665B87C3CA8E1AC4957F9F61FA7BFFE4E17631E937837CABF43DD6183D6360A228A3EBC73A1D1CDC72BF09953C81203AB7E492148E4CB774CDDFAAC3544D0DD4F8C8A0E9C70B877EA79F2C22E4CE52C69F3EF376F61B0F43A540FE96C63F586310C3B6E39C78C4D647CADB5933", 352);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "42D96E6E1217E5B59CC2079CE50C3D9F55B6FC1D", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

	//7
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000003", 10);
	tempCAPK.KeyID = 0x57;
	tempCAPK.ModulLen = 192/2;
	AscToBcd_Api(tempCAPK.Modul, "942B7F2BA5EA307312B63DF77C5243618ACC2002BD7ECB74D821FE7BDC78BF28F49F74190AD9B23B9713B140FFEC1FB429D93F56BDC7ADE4AC075D75532C1E590B21874C7952F29B8C0F0C1CE3AEEDC8DA25343123E71DCF86C6998E15F756E3", 192);
	tempCAPK.ExponentLen = 3;		
	AscToBcd_Api(tempCAPK.Exponent, "010001", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "251A5F5DE61CF28B5C6E2B5807C0644A01D46FF5", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

//paypass 
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000004", 10);
	tempCAPK.KeyID = 0xF5;
	tempCAPK.ModulLen = 496/2;
	AscToBcd_Api(tempCAPK.Modul, "A6E6FB72179506F860CCCA8C27F99CECD94C7D4F3191D303BBEE37481C7AA15F233BA755E9E4376345A9A67E7994BDC1C680BB3522D8C93EB0CCC91AD31AD450DA30D337662D19AC03E2B4EF5F6EC18282D491E19767D7B24542DFDEFF6F62185503532069BBB369E3BB9FB19AC6F1C30B97D249EEE764E0BAC97F25C873D973953E5153A42064BBFABFD06A4BB486860BF6637406C9FC36813A4A75F75C31CCA9F69F8DE59ADECEF6BDE7E07800FCBE035D3176AF8473E23E9AA3DFEE221196D1148302677C720CFE2544A03DB553E7F1B8427BA1CC72B0F29B12DFEF4C081D076D353E71880AADFF386352AF0AB7B28ED49E1E672D11F9", 496);
	tempCAPK.ExponentLen = 3;		
	AscToBcd_Api(tempCAPK.Exponent, "010001", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "C2239804C8098170BE52D6D5D4159E81CE8466BF", 40);
	ret = Common_AddCapk_Api(&tempCAPK);
	

//Mir
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "991231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000658", 10);
	tempCAPK.KeyID = 0xD1;
	tempCAPK.ModulLen = 240;
	AscToBcd_Api(tempCAPK.Modul, "A3547415A7D237C09FC8AFF989FDA49E5B3275545026361C1A8DE477467F963D8F6F58A2F16E0885E4759CA58F72A5B5446CE3893155EFD978B2F0D8D1A7294AC7870D65B5CC78286F96237EFCBA02C6844A84DB79A01D225FF3BEAB3761AFC52AEDD57764483C980076D10E4C3485011DD93A970C57FC72A1CCA47C7D1B57E5D7798A180BF08455A4D602CFC3C881034B52D6DF2C3B1A8FEE7E6539EA35F6B5C123A822AA73FB6BDFD894AEB8381A62413EFB030F85DC45D71B66A322F1532A91C9AD8E4820AC18C544A623FC3E401D42498C1C9B88E5A6B7DA2D9E0BF7CB3F921242B5352302B95EE1344D79ECE49D", 240*2);
	tempCAPK.ExponentLen = 3;		
	AscToBcd_Api(tempCAPK.Exponent, "010001", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "05567628480B757FE633999C9AE1D9F420F84EE3", 40);
	ret = Common_AddCapk_Api(&tempCAPK);

//Jspeedy
	memset(&tempCAPK, 0, sizeof(EMV_CAPK));
	tempCAPK.HashInd = 1;
	tempCAPK.ArithInd = 1;	
	AscToBcd_Api(tempCAPK.ExpDate, "251231", 6);	
	AscToBcd_Api(tempCAPK.RID, "A000000065", 10);
	tempCAPK.KeyID = 0x09;
	tempCAPK.ModulLen = 256/2;
	AscToBcd_Api(tempCAPK.Modul, "A0BA1E941BA2B11DFB9AC5139041CC58B870A3B328F4712DD844439E6544469FD31106167FE926583CBCED6D573DECF9AF67D09875AF285C189681D4045883031E99A0A0F456DD31857DC58960EC24689F68FECEF88832B389D66D2A0481B14B0E05FD36CC00163FCAABAE73B5273D5F1206D4E246DC8AA1977A685FDD344B0D", 256);
	tempCAPK.ExponentLen = 1;		
	AscToBcd_Api(tempCAPK.Exponent, "03", tempCAPK.ExponentLen*2);
	AscToBcd_Api(tempCAPK.CheckSum, "0000000000000000000000000000000111111111", 40);
	ret = Common_AddCapk_Api(&tempCAPK);
	// TipAndWaitEx_Api("AddCapk Js:%d", ret);

}


void EmvAddAppsExp()
{
	int OnLinePinFlag = 1, i = 0;
	EMV_APPLIST emv_applist;

	//EMV.ClearApp_Api();
	memset(&emv_applist, 0, sizeof(emv_applist));
	AscToBcd_Api(emv_applist.Version, "008C", 4);
	AscToBcd_Api(emv_applist.AID, "A000000333010101", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.Version, "008C", 2*2);
	AscToBcd_Api(emv_applist.AID, "A0000003330101", 7*2);
	emv_applist.AidLen = 7;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A0000000031010", 7*2);
	AscToBcd_Api(emv_applist.Version, "0096", 2*2);
	emv_applist.AidLen = 7;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003101003", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003101004", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003101005", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003101006", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003101007", 8*2);
	emv_applist.AidLen = 8;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A0000000999090", 7*2);
	emv_applist.AidLen = 7;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A00000999901", 6*2);
	emv_applist.AidLen = 6;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A0000000041010", 7*2);
	emv_applist.AidLen = 7;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A0000000651010", 7*2);
	emv_applist.AidLen = 7;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A122334455", 5*2);
	AscToBcd_Api(emv_applist.Version, "0096", 2*2);
	emv_applist.AidLen = 5;
	emv_applist.SelFlag = PART_MATCH;
	EMV_AddApp_Api(&emv_applist);

	AscToBcd_Api(emv_applist.AID, "A000000003", 5);
	AscToBcd_Api(emv_applist.Version, "0096", 2);
	emv_applist.AidLen = 5;
	EMV_AddApp_Api(&emv_applist);
}


void PayPassAddAppExp(int transType)
{
	PAYPASS_APPLIST app;

	memset(&app, 0, sizeof(PAYPASS_APPLIST));
	AscToBcd_Api(app.AID, "A0000000041010", 14);
	app.AidLen = 7;
	AscToBcd_Api(app.Version, "0002", 4);
	app.CVMCapabilityCVM = 0x60;
	app.CVMCapabilityNoCVM = 0x08;
	AscToBcd_Api(app.uDOL, "039F6A04", 6);
	app.KernelID = 0x02;
	AscToBcd_Api(app.MagStripeAVN, "0001", 4);
	AscToBcd_Api(app.RiskManData, "086CFF000000000000", 18);
	if (transType == 0x17)
		app.MagCVMCapabilityCVM = 0x20;
	else
		app.MagCVMCapabilityCVM = 0x10;
	app.MagCVMCapabilityNoCVM = 0x00;
	app.FloorLimit = 10000;
	app.TransLimitNoODCVM = 30000;
	app.TransLimitODCVM = 50000;
	memset(app.TACDenial, 0, 6);
	memset(app.TACOnline, 0, 6);
	memset(app.TACDefault, 0, 6);
	app.KernelConfig = 0x20;
	if ((transType == 0x00) || (transType == 0x12) || (transType == 0x21))
		app.CVMLimit = 1000;
	else
		app.CVMLimit = 20000;
	PayPass_AddApp_Api(&app);


	AscToBcd_Api(app.AID, "A0000000043060", 14);
	app.AidLen = 7;
	app.KernelConfig = 0xA0;
	app.CVMLimit = 30000;
	AscToBcd_Api(app.RiskManData, "0844FF800000000000", 18);
	if (transType == 0x17) {
		app.MagCVMCapabilityCVM = 0xF0;
		app.MagCVMCapabilityNoCVM = 0xF0;
	}
	else {
		app.MagCVMCapabilityCVM = 0x10;
		app.MagCVMCapabilityNoCVM = 0x00;
	}
	PayPass_AddApp_Api(&app);

}

void PayWaveAddAppExp()
{
	int i =0 ;
	PAYWAVE_APPLIST item;

	memset(&item, 0, sizeof(item));
	item.SelFlag = 0;
	item.bStatusCheck = 1;
	item.bZeroAmtCheck = 1;
	item.ZeroAmtCheckOpt = 1;
	item.bTransLimitCheck = 1;
	item.bCVMLimitCheck = 1;
	item.bFloorLimitCheck = 1;
	item.bHasFloorLimit = 1;
	item.TransLimit = 1000;
	item.CVMLimit = 2;
	item.FloorLimit = 1000;
	item.TermFloorLimit = 1000;

	memset(item.AID, 0, sizeof(item.AID));
	AscToBcd_Api(item.AID, "A0000000031010", 7*2 ); 
	item.AidLen= 7;
	PayWave_AddApp_Api(&item);

	memset(item.AID, 0, sizeof(item.AID));
	memcpy(item.AID, "A000000003", 5*2); 
	item.AidLen= 5;
	PayWave_AddApp_Api(&item);

	memset(item.AID, 0, sizeof(item.AID));
	memcpy(item.AID, "A00000000310", 6*2); 
	item.AidLen= 6;
	PayWave_AddApp_Api(&item);

	memset(item.AID, 0, sizeof(item.AID));
	memcpy(item.AID, "A000000003101002", 8*2); 
	item.AidLen= 8;    	 	
	PayWave_AddApp_Api(&item);
}



void MirAddAppExp()
{
	int i =0 ;
	MIR_APPLIST item;

	memset(&item, 0, sizeof(item));

	memset(item.ucTACDenial, 0, 6);
	memset(item.ucTACOnline, 0, 6);
	memset(item.ucTACDefault, 0, 6);

	item.ucFloorLimitCheck = 1;
	item.ucNoCVMLimitCheck = 1;
	item.ucCLNOCDCVMTransLimitCheck = 1;
	item.ucCLCDCVMTransLimitCheck = 1;	

	item.ulFloorLimit = 1000;
	item.ulNoCVMLimit = 1000;
	item.ulCLNOCDCVM_TransLimit = 1000;
	item.ulCLCDCVM_TransLimit = 1000;

	memset(item.ucAID, 0, sizeof(item.ucAID));
	AscToBcd_Api(item.ucAID, "A000000658", 5*2); 
	item.ucAidLen= 5;
	Mir_AddApp_Api(&item);
}

void JSpeedyAddAppsExp()
{
	COMMON_TERMINAL_PARAM param;
	JSPEEDY_APPLIST app;
	JSpeedy_DelApp_Api((unsigned char *)"\xA0\x00\x00\x00\x65\x10\x10",7);
	JSpeedy_DelApp_Api((unsigned char *)"\xA0\x00\x00\x00\x65\x20\x20",7);
	JSpeedy_DelApp_Api((unsigned char *)"\xA0\x00\x00\x00\x65\x10\x10\x00\x00\x00\x00\x00\x00\x00\x10\x10",16);
	JSpeedy_GetApp_Api(0,&app);

	memcpy(&app.AID,"\xA0\x00\x00\x00\x65\x10\x10",7);
	app.AidLen = 7;
	memcpy(&app.CombinationOption,"\x7B\x00",2);
	app.bHasCVMLimit = 1;
	app.bHasFloorLimit = 1;
	app.bHasTransLimit = 1;
	app.FloorLimit = 4500;
	app.TransLimit = 20000;
	app.CVMLimit = 10000;
	app.MaxTargetPer = 0;
	app.RemovalTime = 10;
	app.TargetPer = 0;
	memcpy(&app.TACDenial,"\x04\x10\x00\x00\x00",5);
	memcpy(&app.TACOnline,"\x90\x60\x00\x90\x00",5);
	memcpy(&app.TACDefault,"\x90\x40\x00\x80\x00",5);
	memcpy(&app.STIP,"\x70\x80\x00",3);
	app.Threshold = 2000;	

	Common_ClearBlackList_Api();
	Common_AddBlackList_Api("\x33\x35\x34\x30\x38\x32\x30\x30\x30\x30\x34\x32\x31\x30\x31\x30",0x00);
	Common_AddIPKRevoke_Api("\xA0\x00\x00\x00\x65",0x09,"\x00\x10\x00");
	JSpeedy_AddApp_Api(&app);
}

#endif
 
int SaveScriptResult(void)
{
	int iRet = 0, iCnt = 0, iLength = 0;
	u8 *psTemp, sBuff[128];
	
	iRet = EMV_GetScriptResult_Api(sBuff, &iLength);
	if(iRet != EMV_OK)
		return 0;
	Common_SetTLV_Api(0xDF31, sBuff, iLength);
	return 0;
}
 
int SetIssuerName(void)
{
	int       iRet = 0, iLength = 0;
	unsigned char     sAID[16];

	// Application Identifier (AID) - ICC
	memset(sAID, 0, sizeof(sAID));

	iRet = Common_GetTLV_Api(0x4F, sAID, &iLength);
	if( iRet!=EMV_OK )
	{
		return E_TRANS_FAIL;
	}
	if( memcmp(sAID, "\xD1\x56", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "CUP");
	}
	else if( memcmp(PosCom.stTrans.MainAcc, "4", 1)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "VIS");
	}
	else if( memcmp(PosCom.stTrans.MainAcc, "5", 1)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "MCC");
	}
	else if( memcmp(PosCom.stTrans.MainAcc, "34", 2)==0 ||
	memcmp(PosCom.stTrans.MainAcc, "37", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "AEX");
	}
	else if( memcmp(PosCom.stTrans.MainAcc, "30", 2)==0 ||
	memcmp(PosCom.stTrans.MainAcc, "36", 2)==0 ||
	memcmp(PosCom.stTrans.MainAcc, "38", 2)==0 ||
	memcmp(PosCom.stTrans.MainAcc, "39", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "DCC");
	}
	else if( memcmp(PosCom.stTrans.MainAcc, "35", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "JCB");
	}
	return 0;
}
 
extern int G_IcCardMode;
unsigned char GetPosEntryMode(void)
{
	//fallback:0x92   magstrip_notfallback:0x02  chip:0x05
	if(PosCom.stTrans.EntryMode[0] == PAN_ICCARD ||  PosCom.stTrans.EntryMode[0] == PAN_PAYPASS ||  PosCom.stTrans.EntryMode[0] == PAN_PAYWAVE )
		return 0x05;
	else if(PosCom.stTrans.EntryMode[0] == PAN_MAGCARD)  
	{
		if(PosCom.stTrans.IccFallBack == 1)
			return 0x92;
		else
			return 0x02;
	}
	else
		return 0;
}
 
unsigned char GetPosBatchCaptureInfo(void)
{
     return 0;	//TermInfo.bBatchCapture;
}
 
unsigned char GetPosAdviceSupportInfo(void)
{
     return 1;	//TermInfo.bSurportAdvices;  
}
 
void FormatAmt_Str(char *pDest, char *pSrc)
{
	unsigned char i = 0;
	unsigned char cLen = 0;

	// 
	cLen = strlen(pSrc);
	for(i = 0; i < cLen; i++)
	{
		if((pSrc[i] != '0')||(pSrc[i] == 0)) break;
	}
	pSrc += i;
	cLen = strlen(pSrc);
	// 
	strcpy(pDest, "0.00");
	if(cLen > 2)
	{
		memcpy(pDest, pSrc, cLen-2);
		pDest[cLen-2] = '.';
		strcpy(&pDest[cLen-1], &pSrc[cLen-2]);
	}
	else
	{
		memcpy(&pDest[4-cLen], pSrc, cLen);
	}
}
 
void FormatAmtToDisp_Api(char *pOut, u8 *pIn, u8 flag)
{
	u8 len;
	char stemp1[64], stemp2[64];
	
	memset(stemp1, 0, sizeof(stemp1));
	memset(stemp2, 0, sizeof(stemp2));
	if(flag == 0)
		BcdToAsc_Api(stemp1, pIn, 12);
	else
	{
		len = strlen((char*)pIn);
		if(len > 12)
			len = 12;
		memcpy(stemp1, pIn, len);
	}
	FormatAmt_Str(stemp2, stemp1);
	strcpy(pOut, stemp2);
}


int  CEmvGetDateTime(unsigned char *DateTime)
{
	unsigned char DataTimeTemp[10];
	memset(DataTimeTemp, 0, sizeof(DataTimeTemp));

	GetSysTime_Api(DataTimeTemp);
	memcpy(DateTime, DataTimeTemp+1, 6);
	return 0;
}

int CEmvReadSN(unsigned char *Sn)
{
	unsigned char SnBuf[50];
	unsigned char Ret;
	int Len;

	if(Sn == NULL)
		return 1;

	memset(SnBuf, 0, sizeof(SnBuf));
	Ret = PEDReadPinPadSn_Api(SnBuf);
	if(Ret == 0)
	{
		Len = AscToLong_Api(SnBuf, 2);
		memcpy(Sn, SnBuf+2+Len-8, 8);
	}
	return Ret;
}

int CEmvGetUnknowTLV(unsigned short Tag, unsigned char *dat, int len)
{
	switch( Tag )
	{
	case 0x9F53:
		*dat=0x52;
		return 0;
	}

	return -1;
}

int App_EMVTrans(u8 tradetype, u8 cardtype, u8 *pTrackBuf)
{
	int ret = EMV_OK, nLen ;	
	u8 sTemp[128];

	PosCom.stTrans.IccOnline = 1;
	ret = EMV_SelectApp_Api(0, gCtrlParam.lTraceNo);
	CommDebugInfo("EMV_SelectApp_Api 0", (u8 *)&ret, sizeof(ret), 1);

RESELECT:
	if(ret != EMV_OK)
		goto EMVRESULT;
		
	ret = EMV_InitApp_Api();
	CommDebugInfo("EMV_InitApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;

	ret = EMV_ReadAppData_Api();
	CommDebugInfo("EMV_ReadAppData_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;
	
	ret = DispCardNo();
	if (ret != EMV_OK)
		goto EMVRESULT;

	memset(sTemp, 0, sizeof(sTemp));
	if( PEDReadPinPadSn_Api(sTemp) == 0x00 )
	{
		nLen = (sTemp[0] - 0x30)*10 + (sTemp[1] - 0x30) + 2;
		Common_SetTLV_Api(0x9f1e, sTemp + (nLen-8), 8);
	}

	if(GetEmvTrackData(pTrackBuf) < 0)
		return E_TRANS_FAIL;

	if(cardtype&CARD_EMVSIMPLE)		 //simple process	
		goto EMVRESULT;


	ret = EMV_OfflineDataAuth_Api();
	CommDebugInfo("EMV_OfflineDataAuth_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;
		
	ret = EMV_ProcRestrictions_Api();
	CommDebugInfo("EMV_ProcRestrictions_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;
		
	ret = EMV_VerifyCardholder_Api();
	CommDebugInfo("EMV_VerifyCardholder_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;

	ret = EMV_RiskManagement_Api();
	CommDebugInfo("EMV_RiskManagement_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		goto EMVRESULT;

	ret = EMV_TermActAnalyse_Api(&PosCom.stTrans.IccOnline);
	CommDebugInfo("EMV_TermActAnalyse_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK)
		goto EMVRESULT;

	if(PosCom.stTrans.IccOnline==TRUE)		 
		EmvSaveDataOnline();

	EmvSaveData();

EMVRESULT:
	if (ret == ERR_SELECTNEXT) 
	{
		ret = EMV_SelectApp_Api(1, gCtrlParam.lTraceNo);
		CommDebugInfo("EMV_SelectApp_Api 1", (u8 *)&ret, sizeof(ret), 1);
		goto RESELECT;
	}
	return ret;
}

//int CEmvDispCardNo(void)
int DispCardNo(void)
{
	int iRet;
	u8 pTrackBuf[256];

	memset (pTrackBuf, 0, sizeof(pTrackBuf));		
	if(g_EmvFullOrSim == 0)  
		return 0;

	iRet = GetEmvTrackData(pTrackBuf);	
	if(iRet == 0)
	{
		GetCardNoFromTrack2Data(PosCom.stTrans.MainAcc, pTrackBuf);
		//ScrClrLineRam_Api(LINE2, LINE5);
		//ScrCls_Api(line);
		ScrClrLine_Api(LINE2, LINE10);
		DispTradeTitle((u8)PosCom.stTrans.Trans_id);
		ScrDispRam_Api(LINE2, 0,"PLS confirm:", LDISP);
		ScrDispRam_Api(LINE3, 0,PosCom.stTrans.MainAcc, RDISP);
		ScrDispRam_Api(LINE5, 0,"ENTER to continue", RDISP);
		ScrBrush_Api();
	}
	KBFlush_Api ();
	iRet = WaitEnterAndEscKey_Api(30);
	if(iRet != ENTER)
		return ERR_NOTACCEPT;
	return 0;
}




int App_CommonSelKernel()
{
	int ret;
	int Path;
	COMMON_PPSE_STATUS ppse;

	ret = Common_SelectPPSE_Api(&ppse);
	CommDebugInfo("Common_SelectPPSE_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK)
		return ret;

	//paypass or not ?
	ret = PayPass_SelectApp_Api(&ppse);
	CommDebugInfo("PayPass_SelectApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret == ERR_SELECTNEXT)
	{
		while(1)
		{
			ret = PayPass_SelectApp_Api(NULL);
			CommDebugInfo("PayPass_SelectApp_Api2", (u8 *)&ret, sizeof(ret), 1);
			if (ret == ERR_SELECTNEXT)
				continue;
			else
				break;
		}
	}
	if (ret == EMV_OK)
		return TYPE_KER_PAYPASS;

	//paywave or not ?
	ret = PayWave_SelectApp_Api(&ppse);
	CommDebugInfo("PayWave_SelectApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret == ERR_SELECTNEXT)
	{
		while(1)
		{
			ret = PayWave_SelectApp_Api(NULL);
			CommDebugInfo("PayWave_SelectApp_Api2", (u8 *)&ret, sizeof(ret), 1);
			if (ret == ERR_SELECTNEXT)
				continue;
			else
				break;
		}
	}

	if (ret == EMV_OK) 
		return TYPE_KER_PAYWAVE;


//MIR
	if(MirCLAllowed==1) {
		ret = Mir_SelectApp_Api(&ppse, &Path);
		CommDebugInfo("Mir_SelectApp_Api", (u8 *)&ret, sizeof(ret), 1);
		if(ret == ERR_SELECTNEXT)
		{
			while(1)
			{
				ret = Mir_SelectApp_Api(NULL, &Path);
				CommDebugInfo("Mir_SelectApp_Api2", (u8 *)&ret, sizeof(ret), 1);
				if (ret == ERR_SELECTNEXT)
					continue;
				else
					break;
			}
		}

		if (ret == EMV_OK) 
			return TYPE_KER_MIR;
	}
	
	
	ret = JSpeedy_SelectApp_Api(&ppse);
	CommDebugInfo("JSpeedy_SelectApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret == EMV_OK) 
		return TYPE_KER_JSPEEDY;
	
	return TYPE_KER_ERR;	  //not paypass/paywave/mir/jspeedy
}



int App_PaypassTrans()
{
	int ret , onlineResult = 0;
	int path = 0, cvm = 0, canRemoveCard = 0, needOnline = 0;
	PAYPASS_OUTCOME  outcome;

	ret = EMV_OK;

SELECT_NEXT:
	if(ret != EMV_OK)
		goto TRANS_COMPLETED;

	ret = PayPass_InitApp_Api(&path);
	CommDebugInfo("PayPass_InitApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK)
		goto TRANS_COMPLETED;

	ret = PayPass_ReadAppData_Api();
	CommDebugInfo("PayPass_ReadAppData_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK)
		goto TRANS_COMPLETED;

	GetPanNumber();

	if (path == PAYPASS_PATH_MAGSTRIPE) {
		ret = PayPass_ProcMSTrans_Api();
		CommDebugInfo("PayPass_ProcMSTrans_Api", (u8 *)&ret, sizeof(ret), 1);
		if (ret != EMV_OK)
			goto TRANS_COMPLETED;
		canRemoveCard = 1;
	}
	else {
		ret = PayPass_ProcRestrictions_Api();
		CommDebugInfo("PayPass_ProcRestrictions_Api", (u8 *)&ret, sizeof(ret), 1);
		if (ret != EMV_OK)
			goto TRANS_COMPLETED;

		ret = PayPass_VerifyCardholder_Api(&cvm);
		CommDebugInfo("PayPass_VerifyCardholder_Api", (u8 *)&ret, sizeof(ret), 1);
		CommDebugInfo("Verify cvm", (u8 *)&cvm, sizeof(cvm), 1);
		if (ret != EMV_OK)
			goto TRANS_COMPLETED;

		ret = PayPass_TermActAnalyse_Api();
		CommDebugInfo("PayPass_TermActAnalyse_Api", (u8 *)&ret, sizeof(ret), 1);
		if (ret != EMV_OK)
			goto TRANS_COMPLETED;

		ret = PayPass_CardActAnalyse_Api(&canRemoveCard);
		CommDebugInfo("PayPass_CardActAnalyse_Api", (u8 *)&ret, sizeof(ret), 1);
		if (ret != EMV_OK)
			goto TRANS_COMPLETED;
	}

	if (canRemoveCard == 1) {
		Beep_Api(BEEPNORMAL);
		ScrDisp_Api(LINE3, 0, "Please remove card", FDISP|CDISP);
	}

	if (path == PAYPASS_PATH_MAGSTRIPE)
	{
		ret = PayPass_CompleteMSTrans_Api(&cvm);
		CommDebugInfo("PayPass_CompleteMSTrans_Api", (u8 *)&ret, sizeof(ret), 1);
		CommDebugInfo("MAG cvm", (u8 *)&cvm, sizeof(cvm), 1);
	}
	else {
		ret = PayPass_CompleteTrans_Api(&needOnline);  //online or not
		CommDebugInfo("PayPass_CompleteTrans_Api", (u8 *)&ret, sizeof(ret), 1);
		CommDebugInfo("needOnline", (u8 *)&needOnline, sizeof(needOnline), 1);
		PosCom.stTrans.IccOnline = needOnline; 
	}   
	if(ret != EMV_OK)
		goto TRANS_COMPLETED;

	if (cvm == PAYPASS_CVM_SIGNATURE) { //Add for transaction
		//Signature		
	}
	else if (cvm == PAYPASS_CVM_ONLINE_PIN) {
		ret = EnterPIN(0);
		if(ret != 0 )
			goto TRANS_COMPLETED;
	}

	onlineResult = ONLINE_APPROVE;
	if (PosCom.stTrans.IccOnline) { //online transaction
		//build the message from variable outcome and sent to server if necessary
		PayPass_GetOutcome_Api(&outcome); 
		//onlineResult = online process
		if (onlineResult == ONLINE_FAILED) {
			//reversal 
		}
	}

TRANS_COMPLETED:
	if (ret == ERR_SELECTNEXT) {
		ret = PayPass_SelectApp_Api(NULL);
		goto SELECT_NEXT;
	}
	return ret;
}

int App_PaywaveTrans() 
{ 
	int ret, onlineResult, len;
	u8 buf[1024];
	int path = 0, cvm = 0, needOnline = 0, needIssuer = 0;
	COMMON_TERMINAL_PARAM param;
	int authCodeLen = 0, scriptLen = 0; 

	ret = EMV_OK;
	DisplayProcessing();

WAVE_SELECT_NEXT:
	if(ret != EMV_OK)
		goto WAVE_COMPLETED;

	ret = PayWave_InitApp_Api(&path);
	CommDebugInfo("PayWave_InitApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK)
		goto WAVE_COMPLETED;

	if (path == PAYWAVE_PATH_MSD) {
		return MsgMsdNoSupport;
	}

	ret = PayWave_ReadAppData_Api();
	CommDebugInfo("PayWave_ReadAppData_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK) 
		goto WAVE_COMPLETED;

	ScrDisp_Api(LINE3, 0, "Please remove card", FDISP|CDISP);
	PiccClose_Api();

	GetPanNumber();

	ret = PayWave_ProcRestrictions_Api();
	CommDebugInfo("PayWave_ProcRestrictions_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK) 
		goto WAVE_COMPLETED;

	ret = PayWave_OfflineDataAuth_Api();
	CommDebugInfo("PayWave_OfflineDataAuth_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK) 
		goto WAVE_COMPLETED;

	ret = PayWave_VerifyCardholder_Api(&cvm, &needOnline);
	CommDebugInfo("PayWave_VerifyCardholder_Api", (u8 *)&ret, sizeof(ret), 1);
	CommDebugInfo("cvm", (u8 *)&cvm, sizeof(cvm), 1);
	CommDebugInfo("needOnline", (u8 *)&needOnline, sizeof(needOnline), 1);
	if (ret != EMV_OK)
		goto WAVE_COMPLETED;

	if (cvm == PAYWAVE_CVM_SIGNATURE) {
		//signature
	} else if (cvm == PAYWAVE_CVM_ONLINE_PIN) {
		ret = EnterPIN(0);
		if(ret != 0)
			goto WAVE_COMPLETED;
	}

	onlineResult = ONLINE_APPROVE;
	PosCom.stTrans.IccOnline =  needOnline;
	if (PosCom.stTrans.IccOnline != 0) { //---onlineProc---
		//send message to server and get message from server 
		//.....
		if (onlineResult == ONLINE_FAILED) {
		}
	}

WAVE_COMPLETED:	
	if (ret == ERR_SELECTNEXT) {
		ret = PayWave_SelectApp_Api(NULL);
		goto WAVE_SELECT_NEXT;
	}
	return ret;
}


//return  :0-continue       1-next loop        2-process flow from start: open picc->detect->commsetic->...commonselect  selectapp....
int CheckNextLoop(int ret)
{
	JSPEEDY_OUTCOME OutcomeParam;
	if ((ret != EMV_OK) && (ret != ERR_EMVDENIAL))
	{
		memset(&OutcomeParam, 0, sizeof(OutcomeParam));
		JSpeedy_GetOutcome_Api(&OutcomeParam);
		if ((OutcomeParam.OPS[1] & 0xF0) != 0xF0)
		{
			if ((OutcomeParam.OPS[1] & 0xF0) == 0x20) {
				return 1;
			} else {
				//PiccClose_Api();
				return 2;
				//process flow from start: open picc->detect->commsetic->...commonselect  selectapp....
			}
		}
	}
	return 0;
}

//JSpeedy_SelectApp_Api~JSpeedy_CardActAnalyse_Api   may return  NextLoop
int App_JSpeedyTrans() 
{
    int  ret, nlret;
    u8 aCVM= 0;
    u8 abTempBuf[256];
    u8 aIfMoveCard=0;
    u8 aIfPresentCard=0;
    COMMON_PPSE_STATUS ppse;
    JSPEEDY_OUTCOME OutcomeParam;
    int	 appPath = 0;
	int iNeedIssureScript = 0;
	u8 onlineFlag = 0;
	u8 nextsel = 0;

NextLoop:
	if(nextsel == 1)
	{
		ret = JSpeedy_SelectApp_Api(NULL);
		CommDebugInfo("JSpeedy_SelectApp_Api:", (u8 *)&ret, sizeof(ret), 1);
		if (ret != EMV_OK) {
			goto JSPEEDY_CHECKNEXTLOOP;
		}
	}
	
	LongToBcd_Api(abTempBuf, PosCom.stTrans.lTraceNo, 4);
    Common_SetTLV_Api(0x9F41, abTempBuf, 4);
	Common_SetTLV_Api(0x9F33, "\x80\xD8\xC8", 3); //"\xE0\xF8\xC8"

    ret = JSpeedy_InitApp_Api(&appPath);  //may return ERR_SELECTNEXT
	CommDebugInfo("JSpeedy_InitApp_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK) {
        goto JSPEEDY_CHECKNEXTLOOP;
    }
	CommDebugInfo("appPath:", (u8 *)&appPath, sizeof(appPath), 1);
    if (appPath == JSPEEDY_PATH_EMV) {
        //TransData.entryMode = 0x07;
        //TransData.transMode = 0x01;
    } else if (appPath == JSPEEDY_PATH_MAGSTRIPE) {
        //TransData.entryMode = (byte)0x91;
        //TransData.transMode = 0x02;
    } else {    // Legacy
        //TransData.entryMode = 0x07;
        //TransData.transMode = 0x03;
    }

    ret = JSpeedy_ReadAppData_Api();
	CommDebugInfo("JSpeedy_ReadAppData_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK) 
        goto JSPEEDY_CHECKNEXTLOOP;
    ret = JSpeedy_RiskManagement_Api();
	CommDebugInfo("JSpeedy_RiskManagement_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK)            
        goto JSPEEDY_CHECKNEXTLOOP;
    
    ret = JSpeedy_ProcRestrictions_Api();
	CommDebugInfo("JSpeedy_ProcRestrictions_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK)             
        goto JSPEEDY_CHECKNEXTLOOP;
	
    ret = JSpeedy_TermActAnalyse_Api();
	CommDebugInfo("JSpeedy_TermActAnalyse_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK)            
        goto JSPEEDY_CHECKNEXTLOOP;
    
    ret = JSpeedy_CardActAnalyse_Api(&aIfMoveCard);
	CommDebugInfo("JSpeedy_CardActAnalyse_Api:", (u8 *)&ret, sizeof(ret), 1);

JSPEEDY_CHECKNEXTLOOP:
	if (ret == EMV_OK) {
		if(aIfMoveCard == 1)
		{
			Beep_Api(0);
			PiccClose_Api();
			ScrDisp_Api(LINE2, 0,"Please remove card...", FDISP|CDISP); 
			WaitAnyKey_Api(1);
		}
	}
	else
	{
		nlret = CheckNextLoop(ret);
		if(nlret == 1)
		{
			nextsel = 1;
			goto NextLoop;
		}
		else if(nlret == 2)
		{
			PiccClose_Api();
			nextsel = 0;
			//process flow from start is necessary: open picc->detect->commsetic->...commonselect  selectapp...
			return ret;
		}
		return ret ;
	}

    ret = JSpeedy_OfflineDataAuth_Api();
	CommDebugInfo("JSpeedy_OfflineDataAuth_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK)   
        return ret;

    ret = JSpeedy_CardholderVerify_Api(&aCVM);
	CommDebugInfo("JSpeedy_CardholderVerify_Api:", (u8 *)&ret, sizeof(ret), 1);
    if (ret != EMV_OK)             
        return ret;
    
    if (aCVM == JSPEEDY_CVM_ONLINE_PIN) {
		ret = EnterPIN(0);
		if(ret != 0)
			return ret;
    } else if (aCVM == JSPEEDY_CVM_SIGNATURE) {
        //TransData.CVM = 0x10;
    } else if (aCVM == JSPEEDY_CVM_ON_DEVICE) {
        //TransData.CVM = 0x30;
    }

    JSpeedy_CompletionOutcome_Api(&onlineFlag, &aIfPresentCard);    //no matter what JSpeedy_CompletionOutcome_Api return , continue 
	CommDebugInfo("JSpeedy_CompletionOutcome_Api:", (u8 *)&ret, sizeof(ret), 1);
	CommDebugInfo("aIfPresentCard[0]:", (u8 *)&aIfPresentCard, sizeof(aIfPresentCard), 1);
	CommDebugInfo("aIfMoveCard[0]:", (u8 *)&aIfMoveCard, sizeof(aIfMoveCard), 1);
    if (onlineFlag == 1) {
        int iTime, iRet;
        int len = 0;
        long balance = 0xFFFFFFFF;

        iTime = 0;
        JSpeedy_GetOutcome_Api(&OutcomeParam);
        //Sendmsg(GetMsgCode(OutcomeParam.UIRD[0]), 0);

        ret = Common_GetTLV_Api(0x9F5F, abTempBuf, &len);
        if (len > 0) {
			balance = BcdToLong_Api(abTempBuf, len);
            //Sendmsg(DefConstants.DisplayBalance, (int) balance);
        }            

        if ((aIfPresentCard == 1) || (aIfMoveCard == 0)) {
        	iNeedIssureScript = 1;
            //TransData.ifOutcomeAgain = 1;
            iTime = ((OutcomeParam.OPS[7] & 0xFF) * 10000);
        }

        ScrDisp_Api(LINE2, 0,"Processing......", FDISP|CDISP);    
		//send data to server  and receive data from server
       // iRC = TransData.onlineProc(iTime);
        if (ret == ONLINE_FAILED) {
            if (aIfMoveCard == 0) {
				ScrDisp_Api(LINE2, 0,"Please remove card...", FDISP|CDISP);   
				WaitAnyKey_Api(1);
            } else if (iNeedIssureScript == 0) {
                ret = ERR_EMVDENIAL;
				return ret;
            }
        }
    }      
    
    // Issuer Update
    if (iNeedIssureScript == 1) {
        int iRet, iTemp;
        if (aIfMoveCard == 1) {
			ScrDisp_Api(LINE2, 0,"Please wave card again...", FDISP|CDISP);
            iTemp = PiccOpen_Api();
            if (iTemp == 0) {
            	u8 CardType[8];
	            u8 SerialNo[64];
                while (1) {
					ScrDisp_Api(LINE2, 0,"Please swing card...", FDISP|CDISP);
                    iTemp = PiccCheck_Api(0, CardType, SerialNo);
                    if (iTemp == 0) {
                        break;
                    }
                }
            } else {
				TipAndWaitEx_Api("picc open err !!!");
            }
        }
        //JSpeedy_ProcIssuerUpdate_Api should be used for real transaction 
        //iRet = JSpeedy_ProcIssuerUpdate_Api(ret, rspCode,authDataLen, authData,scriptLen, script);
		//CommDebugInfo("JSpeedy_ProcIssuerUpdate_Api:", (u8 *)&iRet, sizeof(iRet), 1);
        //if (iRet != EMV_OK)				
          //  return iRet;
    }
    if (ret == ONLINE_APPROVE || ret == ONLINE_REFER) {
        ret = EMV_OK;
    } else {
        ret = ERR_EMVDENIAL;
    }
	return ret;
}




int PaywaveTransComplete()
{
	int ret, authCodeLen, scriptLen, needIssuer, onlineResult ;
	COMMON_TERMINAL_PARAM param;

	// authCodeLen scriptLen should be real data length got by server response message 
	ret = PayWave_Completion_Api(onlineResult, authCodeLen, scriptLen, &needIssuer);
	CommDebugInfo("PayWave_Completion_Api", (u8 *)&ret, sizeof(ret), 1);
	CommDebugInfo("needIssuer", (u8 *)&needIssuer, sizeof(needIssuer), 1);
	Common_GetParam_Api(&param);
#ifdef _RECVDATA_DEBUG_
	ret = 0;
#endif
	// For qVSDC, refund should not be declined for AAC
	if ((ret == ERR_EMVDENIAL) && (param.TransType == 33)) {   //TRAN_TYPE_DEPOSITS ==33
		ret = EMV_OK;
	}
	// Offline Declined trans must be uploaded also
	if ((PosCom.stTrans.IccOnline == 0) && (ret == ERR_EMVDENIAL)) {
		//send message to server and get response
	}

	if (1 == needIssuer) {
		u8 CardType[8];
		u8 SerialNo[64];
		unsigned int timerid;

		ret = PiccOpen_Api();
		if(ret != 0){
			TipAndWaitEx_Api("picc open err !!!");
			return ret;
		}
			
		timerid = TimerSet_Api();
		ScrDisp_Api(LINE4, 0, "Please wave card again", FDISP|LDISP);			
		while(!TimerCheck_Api(timerid, 60 * 1000)){ 
			if(PiccCheck_Api(0, CardType, SerialNo) == 0){	
				int authDataLen=0, scriptLen1=0;  //should got in response message 
				u8 authData[128], script[1024];  //should got in response message 
				ScrDisp_Api(LINE4, 0, "Processing", FDISP|LDISP);						
				ret = PayWave_ProcIssuerUpdate_Api(authDataLen,authData,scriptLen1,script);
				if (ret == ERR_AGAIN) {
					ScrDisp_Api(LINE4, 0, "Please wave card again2", FDISP|LDISP);
					continue;
				}
				TipAndWaitEx_Api("Please remove card");
				break;
			} //PiccCheck_Api
		}
	} 
	PiccClose_Api();
	return ret;
}


int App_Mir_Trans() 
{ 
	int ret, onlineResult, len;
	u8 buf[1024];
	int path = 0, CVMType = 0, needOnline = 0, needIssuer = 0;
	COMMON_TERMINAL_PARAM param;
	int authCodeLen = 0, scriptLen = 0; 
	MIR_OUTCOME gMir_outCome;


	ret = EMV_OK;
	DisplayProcessing();

WAVE_SELECT_NEXT:
	if(ret != EMV_OK)
		goto WAVE_COMPLETED;

	ret = Mir_InitApp_Api();
	CommDebugInfo("Mir_InitApp_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK)
		goto WAVE_COMPLETED;


	ret = Mir_ReadAppData_Api();
	CommDebugInfo("Mir_ReadAppData_Api", (u8 *)&ret, sizeof(ret), 1);
	if (ret != EMV_OK) 
		goto WAVE_COMPLETED;

	ret = Mir_OfflineDataAuth_Api();
	CommDebugInfo("Mir_OfflineDataAuth_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK) {
		goto WAVE_COMPLETED;
	}

	ret = Mir_ProcRestrictions_Api();
	CommDebugInfo("Mir_ProcRestrictions_Api", (u8 *)&ret, sizeof(ret), 1);
	if(ret != EMV_OK) {
		goto WAVE_COMPLETED;
	}

	ret = ret = Mir_VerifyCardholder_Api(&CVMType);
	CommDebugInfo("Mir_VerifyCardholder_Api", (u8 *)&ret, sizeof(ret), 1);
	CommDebugInfo("cvm", (u8 *)&CVMType, sizeof(CVMType), 1);
	if(ret != EMV_OK) {
		goto WAVE_COMPLETED;
	}
	
	ret = Mir_RiskManagement_Api();
	if(ret != EMV_OK) {
		goto WAVE_COMPLETED;
	}
	
	ret = Mir_TermActAnalyse_Api(&needOnline);
	CommDebugInfo("Mir_TermActAnalyse_Api", (u8 *)&ret, sizeof(ret), 1);
	CommDebugInfo("needOnline", (u8 *)&needOnline, sizeof(needOnline), 1);
	if(ret != EMV_OK) {
		goto WAVE_COMPLETED;
	}


	ScrDisp_Api(LINE3, 0, "Please remove card", FDISP|CDISP);
	PiccClose_Api();

	GetPanNumber();

	

	if (CVMType == MIR_CVM_SIGNATURE) {
		//signature
	} else if (CVMType == MIR_CVM_ONLINE_PIN) {
		ret = EnterPIN(0);
		if(ret != 0)
			goto WAVE_COMPLETED;
	}

	onlineResult = ONLINE_APPROVE;
	PosCom.stTrans.IccOnline =  needOnline;
	if (PosCom.stTrans.IccOnline != 0) { //---onlineProc---
		//send message to server and get message from server 
		//.....
		if (onlineResult == ONLINE_FAILED) {
		}
	}

WAVE_COMPLETED:	
	memset(&gMir_outCome, 0, sizeof(gMir_outCome));
	Mir_GetOutcome_Api(&gMir_outCome);
	if (ret == ERR_SELECTNEXT) {
		ret = Mir_SelectApp_Api(NULL, &path);
		goto WAVE_SELECT_NEXT;
	}
	return ret;
}


void initPayPassWaveMirConfig(int transType) 
{

	COMMON_TERMINAL_PARAM termParam;
	PAYPASS_TERM_PARAM ppParam; 
	PAYWAVE_TERM_PARAM pwparm;
	MIR_TERM_PARAM mParam = {0};

	if (transType == 0xFF)
		transType = 0x00;

	// Common Terminal Param

	Common_GetParam_Api(&termParam); //mtermParam
	termParam.AcquierId[0] = 0;
	memcpy(termParam.MerchName, gCtrlParam.MerchantName, strlen(gCtrlParam.MerchantName));
	AscToBcd_Api(termParam.MerchCateCode, "0001", 4);
	memcpy(termParam.MerchId, gCtrlParam.MerchantNo, strlen(gCtrlParam.MerchantNo));
	memcpy(termParam.TermId,gCtrlParam.TerminalNo, 8);	
	AscToBcd_Api(termParam.CountryCode, "0840", 4);
	AscToBcd_Api(termParam.TransCurrCode, "0840", 4);
	AscToBcd_Api(termParam.ReferCurrCode, "0840", 4);
	memcpy(termParam.AcquierId, "12345678", 8);
	termParam.ReferCurrExp = 0x02;
	termParam.ReferCurrCon = 1000;//1000
	termParam.TerminalType = 0x22;  //(byte)tradetype;
	termParam.TransCurrExp = 0x02;
	termParam.TransType = transType;
	Common_SetParam_Api(&termParam); 

	// PayPass Terminal Param
	PayPass_GetParam_Api(&ppParam);
	ppParam.CardDataInputCapability = 0x00;
	ppParam.SecurityCapability = 0x08;
	memset(ppParam.ExCapability, 0x00, 5);
	ppParam.ReadBalanceBeforeGenAC = 0;
	ppParam.ReadBalanceAfterGenAC = 0;
	memset(ppParam.BalanceBG, 0xFF, 6);
	memset(ppParam.BalanceAG, 0xFF, 6);
	memset(ppParam.MaxTornTransTime, 0, 2);
	ppParam.MaxTornTransNum = 0;
	memcpy(ppParam.MessageHoldTime, "\x00\x00\x13", 3);
	memcpy(ppParam.MaxRRTGrace, "\x00\x32", 3);
	memcpy(ppParam.MinRRTGrace, "\x00\x14", 3);
	ppParam.RRTThresholdM = 0x32;
	memcpy(ppParam.RRTThresholdA, "\x01\x2c", 2);
	memcpy(ppParam.ExpRRTCAPDU, "\x00\x12", 2);
	memcpy(ppParam.ExpRRTRAPDU, "\x00\x18", 2);
	memcpy(ppParam.MerchantCustomData, "\x04\x11\x22\x33\x44", 5);
	ppParam.TransCategoryCode = 0x01;
	PayPass_SetParam_Api(&ppParam);

	/*
	// PayPass DE Param
	ppDEParam.clear();
	ppDEParam.hasDSVNTerm = 1;
	ppDEParam.hasDSACType = 1;
	ppDEParam.hasDSInputCard = 1;
	ppDEParam.hasDSInputTerm = 1;
	ppDEParam.hasDSODSInfo = 1;
	ppDEParam.hasDSODSInfoForReader = 1;
	ppDEParam.hasDSODSTerm = 1;
	PayPass_SetDEParam_Api(ppDEParam);
	//configType = PPS_MChip1;
	*/
	

	
	PayWave_GetParam_Api(&pwparm);
	AscToBcd_Api(pwparm.TTQ , "3600C000", 8);
	pwparm.bCheckBlacklist = 1;
	pwparm.bDRL = 1;
	pwparm.bCashDRL = 1;
	pwparm.bCashbackDRL = 1;
	AscToBcd_Api(pwparm.CA_TTQ , "3600C000", 8);
	pwparm.CA_bStatusCheck = 1;
	pwparm.CA_bZeroAmtCheck = 1;
	pwparm.CA_ZeroAmtCheckOpt = 0; 
	pwparm.CA_bTransLimitCheck = 1;
	pwparm.CA_bCVMLimitCheck = 1;
	pwparm.CA_bFloorLimitCheck = 1;
	pwparm.CA_bHasFloorLimit = 1; 
	pwparm.CA_TransLimit = 3000;
	pwparm.CA_CVMLimit = 1000;
	pwparm.CA_FloorLimit = 2000;
	AscToBcd_Api(pwparm.CB_TTQ , "3600C000", 8);
	pwparm.CB_bStatusCheck = 1;
	pwparm.CB_bZeroAmtCheck = 1;
	pwparm.CB_ZeroAmtCheckOpt = 0; 
	pwparm.CB_bTransLimitCheck = 1;     
	pwparm.CB_bCVMLimitCheck = 1;
	pwparm.CB_bFloorLimitCheck = 1;
	pwparm.CB_bHasFloorLimit = 1; 
	pwparm.CB_TransLimit = 5000;        
	pwparm.CB_CVMLimit = 1000;
	pwparm.CB_FloorLimit = 2000;        
	PayWave_SetParam_Api(&pwparm);
	PayWave_SaveParam_Api(&pwparm);


	 
	//Mir
	Mir_GetParam_Api(&mParam);
	mParam.ucTranRecoveryLimit = 3;
	memcpy(mParam.ucTPMCap, "\xF8\x00", 2);
	Mir_SetParam_Api(&mParam);
	Mir_SaveParam_Api(&mParam);
}


void initJSpeedyConfig() 
{
	COMMON_TERMINAL_PARAM param;
	Common_GetParam_Api(&param);
	param.TerminalType = 0x00;
	param.TerminalType = 0x22;
	memcpy(&param.AcquierId[0],"000000000010",12);
	memcpy(&param.MerchCateCode,"\x70\x32",2);
	strcpy(&param.MerchName[0],"\x58\x58\x20\x4D\x45\x52\x43\x48\x41\x4E\x54\x20\x59\x59\x20\x4C\x4F\x43\x41\x54\x49\x4F\x4E");
	memcpy(&param.CountryCode,"\x03\x92",2);
	memcpy(&param.TransCurrCode,"\x03\x92",2);
	param.TransCurrExp = 2;
	Common_SaveParam_Api(&param);
}

void CTLPreProcess()
{
	int nRet;

	PWaveCLAllowed = 1;
	PayPass_PreProcess_Api(PosCom.stTrans.TradeAmount, PosCom.stTrans.SecondAmount);
	nRet = PayWave_PreProcess_Api(PosCom.stTrans.TradeAmount, PosCom.stTrans.SecondAmount);
	CommDebugInfo("PayWave_PreProcess_Api:", (u8 *)&nRet, sizeof(nRet), 1);
	if (nRet != EMV_OK) 
		PWaveCLAllowed = 0;	



	MirCLAllowed = 1;
	nRet = Mir_PreProcess_Api(PosCom.stTrans.TradeAmount, PosCom.stTrans.SecondAmount);
	if (nRet != EMV_OK) 
		MirCLAllowed = 0;
		
	JSpeedy_PreProcess_Api(PosCom.stTrans.TradeAmount, PosCom.stTrans.SecondAmount);
}



int PayPassCB_DEKDET(int DTSLen, unsigned char *DTS,int DNLen, unsigned char *DN) {	return 0;}
void CEmvDebugIccCommand(APDU_SEND *send, APDU_RESP *recv, int retValue){}


#ifdef WIN32  // only used for compiling for simulator******start*******
//comlib init
int Common_Init_Api(){ return 0; }

//get Ver
char* Common_GetVersion_Api(){ return 0; }
int Common_SetIcCardType_Api(unsigned char ucType, unsigned char ucSlot){ return 0; }
int Common_SelectPPSE_Api(COMMON_PPSE_STATUS* pComPPSEStatus){ return 0; }

//TLV api
int Common_SetTLV_Api(unsigned int Tag, unsigned char *Data, int len){ return 0; }
int Common_GetTLV_Api(unsigned int Tag, unsigned char *DataOut, int *OutLen){ return 0; }
unsigned short Common_GetTagAttr_Api(unsigned int Tag){ return 0; }


//black list
void Common_ClearBlackList_Api(void){  }
int Common_AddBlackList_Api(const char *cardNo, unsigned char seq){ return 0; }
int Common_DelBlackList_Api(const char *cardNo, unsigned char seq){ return 0; }
int Common_GetBlackList_Api(int index, unsigned char *blackPan, unsigned char *seq){ return 0; }

//cert
int Common_AddCapk_Api(EMV_CAPK *capk){ return 0; }
int Common_GetCapk_Api(int Index, EMV_CAPK *capk){ return 0; }
int Common_SearchCapk_Api(EMV_CAPK *pCapk, const unsigned char *rid,
						  unsigned char keyID){ return 0; }

int Common_DelCapk_Api(unsigned char KeyID, unsigned char *RID){ return 0; }
int Common_CheckCapk_Api(unsigned char *KeyID, unsigned char *RID){ return 0; }
void Common_ClearCapk_Api(void){  }
void Common_ClearIPKRevoke_Api(void){  }
int Common_AddIPKRevoke_Api(unsigned char *rid, unsigned char capki,
							unsigned char *certserial){ return 0; }
int Common_GetIPKRevoke_Api(int slotNo, unsigned char *rid,
							unsigned char *capki, unsigned char *certserial){ return 0; }
int Common_DelIPKRevoke_Api(unsigned char *rid, unsigned char capki,
							unsigned char *certserial){ return 0; }

//param
void Common_GetParam_Api(COMMON_TERMINAL_PARAM *Param){  }
void Common_SetParam_Api(COMMON_TERMINAL_PARAM *Param){  }
void Common_SaveParam_Api(const COMMON_TERMINAL_PARAM *Param){ }

//Log dbg
void Common_DbgEN_Api(int nEnDbg){  }
int Common_DbgReadLog_Api(char* pcLog, int* pnLen){ return 0; }
void Common_DbgDelLog_Api(){ }
int Common_DbgGetFileLen_Api(){ return 0; }
int Common_DbgReadLogByPosLen_Api(int nPos, char* pcLog, int* pnLen){ return 0; }

char *EMV_GetVersion_Api(void){ return 0; }
unsigned char *EMV_GetKernelCheckSum_Api(void){ return 0; }
unsigned char *EMV_GetConfigCheckSum_Api(void){ return 0; }
void EMV_SetTradeAmt_Api(unsigned char *authAmt, unsigned char *CashbackAmt){  }
int EMV_Init_Api(void){ return 0; }
void EMV_Clear_Api(void){  }
int EMV_SelectApp_Api(int ReSelect, unsigned long TransNo){ return 0; }
int EMV_InitApp_Api(void){ return 0; }
int EMV_ReadAppData_Api(void){ return 0; }
int EMV_OfflineDataAuth_Api(void){ return 0; }
int EMV_ProcRestrictions_Api(void){ return 0; }
int EMV_VerifyCardholder_Api(void){ return 0; }
int EMV_RiskManagement_Api(void){ return 0; }
int EMV_TermActAnalyse_Api(int *NeedOnline){ return 0; }
int EMV_Complete_Api(unsigned char ucResult, unsigned char *RspCode,
					 unsigned char *AuthCode, int AuthCodeLen,
					 unsigned char *IAuthData, int IAuthDataLen,
					 unsigned char *script, int ScriptLen){ return 0; }
int EMV_GetScriptResult_Api(unsigned char *Result, int *RetLen){ return 0; }

int EMV_SelectAppForLog_Api(void){ return 0; }
int EMV_ReadLogRecord_Api(int RecordNo){ return 0; }
int EMV_GetLogItem_Api(unsigned short Tag, unsigned char *TagData, int *TagLen){ return 0; }

void EMV_GetParam_Api(EMV_TERM_PARAM *Param){  }
void EMV_SetParam_Api(EMV_TERM_PARAM *Param){  }
void EMV_SaveParam_Api(EMV_TERM_PARAM *Param){  }

int EMV_AddApp_Api(EMV_APPLIST *App){ return 0; }
int EMV_GetApp_Api(int Index, EMV_APPLIST *App){ return 0; }
int EMV_DelApp_Api(unsigned char *AID, int AidLen){ return 0; }
void EMV_ClearApp_Api(void){  }

//not sure if it's ok like this :
void VCCL_DEV_PiccUserCancel(int canCancel){}
//black list
int CommonPub_BlackListCheck(unsigned char *pan, int len, unsigned char seq){return 0;}






int PayWave_Init_Api(void){return 0;}

int PayWave_PreProcess_Api(unsigned char *AuthAmt,
						   unsigned char *CashBackAmt){return 0;}
int PayWave_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus){return 0;}
int PayWave_InitApp_Api(int *Path){return 0;}
int PayWave_ReadAppData_Api(void){return 0;}
int PayWave_ProcRestrictions_Api(void){return 0;}
int PayWave_OfflineDataAuth_Api(void){return 0;}
int PayWave_VerifyCardholder_Api(int *CVMType, int *NeedOnline){return 0;}
int PayWave_Completion_Api(int OnlineStatus,
						   int IssuerAuthDataLen,
						   int IssuerScriptLen,
						   int *NeedIssuerUpdate){return 0;}
int PayWave_ProcIssuerUpdate_Api(int IssuerAuthDataLen,
								 unsigned char *IssuerAuthData,
								 int IssuerScriptLen,
								 unsigned char *IssuerScript){return 0;}
void PayWave_GetScriptResult_Api(int *ScriptResultLen,
								 unsigned char *ScriptResult){}

void PayWave_GetParam_Api(PAYWAVE_TERM_PARAM *Param){}
void PayWave_SetParam_Api(PAYWAVE_TERM_PARAM *Param){}
void PayWave_SaveParam_Api(PAYWAVE_TERM_PARAM *Param){}

int PayWave_AddApp_Api(PAYWAVE_APPLIST *App){return 0;}
int PayWave_GetApp_Api(int Index, PAYWAVE_APPLIST *App){return 0;}
int PayWave_DelApp_Api(unsigned char *AID, int AidLen){return 0;}
void PayWave_ClearApp_Api(void){}

int PayWave_AddPID_Api(PAYWAVE_PID_PARAM *Pid){return 0;}
int PayWave_GetPID_Api(int Index, PAYWAVE_PID_PARAM *Pid){return 0;}
int PayWave_DelPID_Api(unsigned char *Pid, int PidLen){return 0;}
void PayWave_ClearPID_Api(void){}


char *PayPass_GetVersion_Api(void){return NULL;}

int PayPass_Init_Api(void){return 0;}
int PayPass_PreProcess_Api(unsigned char *AuthAmt, unsigned char *CashbackAmt){return 0;}
int PayPass_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus){return 0;}
int PayPass_InitApp_Api(int *Path){return 0;}
int PayPass_ReadAppData_Api(void){return 0;}

int PayPass_ProcRestrictions_Api(void){return 0;}
int PayPass_VerifyCardholder_Api(int *CVMType){return 0;}
int PayPass_TermActAnalyse_Api(void){return 0;}
int PayPass_CardActAnalyse_Api(int *CanRemoveCard){return 0;}
int PayPass_CompleteTrans_Api(int *NeedOnline){return 0;}

int PayPass_ProcMSTrans_Api(void){return 0;}
int PayPass_CompleteMSTrans_Api(int *CVMType){return 0;}

void PayPass_GetOutcome_Api(PAYPASS_OUTCOME *outcome){}
PAYPASS_TORN_RECORD *PayPass_CleanTornRecord_Api(void){return NULL;}
void PayPass_CleanAllTornRecord_Api(void){}

void PayPass_GetParam_Api(PAYPASS_TERM_PARAM *Param){}
void PayPass_SetParam_Api(PAYPASS_TERM_PARAM *Param){}
void PayPass_SaveParam_Api(PAYPASS_TERM_PARAM *Param){}

int PayPass_AddApp_Api(PAYPASS_APPLIST *App){return 0;}
int PayPass_GetApp_Api(int Index, PAYPASS_APPLIST *App){return 0;}
int PayPass_DelApp_Api(unsigned char *AID, int AidLen){return 0;}
void PayPass_ClearApp_Api(void){}



//MIR
char *Mir_GetVersion_Api(void){return "V1234";}
char *Mir_GetCheckSum_Api(void){return 0;}

int Mir_Init_Api(void){return 0;}
int Mir_PreProcess_Api(unsigned char *AuthAmt, unsigned char *CashbackAmt){return 0;}
int Mir_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus, int* pPath){return 0;}
int Mir_InitApp_Api(){return 0;}
int Mir_ReadAppData_Api(void){return 0;}

int Mir_ProcRestrictions_Api(void){return 0;}
int Mir_OfflineDataAuth_Api(void){return 0;}
int Mir_VerifyCardholder_Api(int *CVMType){return 0;}
int Mir_RiskManagement_Api(void){return 0;}
int Mir_TermActAnalyse_Api(int* pnNeedOnline){return 0;}
//int Mir_CardActAnalyse_Api(void);
//int Mir_CompleteTrans_Api(int *NeedOnline);

//Param
void Mir_GetParam_Api(MIR_TERM_PARAM *Param){}
void Mir_SetParam_Api(MIR_TERM_PARAM *Param){}
void Mir_SaveParam_Api(MIR_TERM_PARAM *Param){}



int Mir_AddApp_Api(MIR_APPLIST *App){return 0;}
int Mir_GetApp_Api(int Index, MIR_APPLIST *App){return 0;}
int Mir_DelApp_Api(unsigned char *AID, int AidLen){return 0;}
void Mir_ClearApp_Api(void){}


void Mir_GetOutcome_Api(MIR_OUTCOME *outcome){}



int JSpeedy_Init_Api(void){return 0;}
void JSpeedy_PreProcess_Api(unsigned char *amount,unsigned char *cashback){}
int JSpeedy_SelectApp_Api(COMMON_PPSE_STATUS *PPSEStatus){return 0;}
int JSpeedy_InitApp_Api(int *Path){return 0;}
int JSpeedy_ReadAppData_Api(void){return 0;}
int JSpeedy_RiskManagement_Api(void){return 0;}
int JSpeedy_ProcRestrictions_Api(void){return 0;}
int JSpeedy_TermActAnalyse_Api(){return 0;}
int JSpeedy_CardActAnalyse_Api(unsigned char *bIfMoveCard){return 0;}
int JSpeedy_OfflineDataAuth_Api(void){return 0;}
int JSpeedy_CardholderVerify_Api(unsigned char *Method){return 0;}
int JSpeedy_CompletionOutcome_Api(unsigned char *bIfGoOnline,unsigned char *bIfPresentCard){return 0;}
int JSpeedy_ProcIssuerUpdate_Api(int OnlineResult,unsigned char *RspCode,
								 int IssuerAuthDataLen,unsigned char *IssuerAuthData,
								 int IssuerScriptLen,unsigned char *IssuerScript){return 0;}

void JSpeedy_GetOutcome_Api(JSPEEDY_OUTCOME *outcomeparam){}
char *JSpeedy_GetVersion_Api(void){return NULL;}

int JSpeedy_AddApp_Api(JSPEEDY_APPLIST *App){return 0;}
int JSpeedy_GetApp_Api(int Index, JSPEEDY_APPLIST *App){return 0;}
int JSpeedy_DelApp_Api(unsigned char *AID, int AidLen){return 0;}
void JSpeedy_ClearApp_Api(void){}
void JSpeedy_UserCancel_Api(void){}

#endif  // only used for compiling for simulator******end*******


