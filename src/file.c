//#include "stdafx.h"
#include "../inc/global.h"

void InitCommParam(void)
{
	memset(&G_CommPara, 0, sizeof(struct _COMMPARASTRUC_));
	
   	// G_CommPara.NetCdmaGprsSet.CommAuto = 0;
	// G_CommPara.HdlcSet.LinkType = 0;												 
	// G_CommPara.DialSet.ReDialTimes = 3;												 	                                                                                 
	// G_CommPara.HdlcSet.IfCheckPhone = 1;											 
   	// G_CommPara.HdlcSet.IfCheckDialTone = 1;											 
   	// G_CommPara.HdlcSet.CheckToneTime = 0x50;										 
   	// G_CommPara.HdlcSet.DelayBeforeDial = 1;											 
   	// G_CommPara.HdlcSet.DtmfOnTime = 120;											 
	// G_CommPara.HdlcSet.DtmfOffTime = 120;                                            
   	// G_CommPara.HdlcSet.WaveLostTime = 30;											 
   	// G_CommPara.HdlcSet.SendLevel = 10;												 
    // memset(G_CommPara.DialSet.PredialNum,0,sizeof(G_CommPara.DialSet.PredialNum));	 
	// memset(G_CommPara.DialSet.InputCenterNum,0,20);									 
    // memcpy(G_CommPara.DialSet.Tpdu,"\x60\x00\x08\x00\x00", 5);						 
	// strcpy(G_CommPara.DialSet.InputCenterNum,"2226");			//02083012240
	// strcpy((char *)G_CommPara.DialSet.InputCenterNum1,"2226");
	// strcpy((char *)G_CommPara.DialSet.InputCenterNum2,"2226");
	G_CommPara.DialSet.PredialNum[0] = 0;
	
	strcpy((char *)G_CommPara.GprsSet.GprsApn1, "internet");
	G_CommPara.NetCdmaGprsSet.ConnectMode = 0;
	strcpy((char *)G_CommPara.GprsSet.GprsIp1, "197.251.5.77");	// beta.soluspay.net 197.251.5.77

	strcpy((char *)G_CommPara.GprsSet.GprsPort1, "443");
	// strcpy(G_CommPara.GprsSet.GprsIp1, "82.165.115.228");     
 	// strcpy(G_CommPara.GprsSet.GprsPort1, "9092");
	// strcpy((char *)G_CommPara.GprsSet.GprsApn2, "CMWAP");
	// strcpy((char *)G_CommPara.GprsSet.GprsIp2, "124.207.195.116");
	// strcpy((char *)G_CommPara.GprsSet.GprsPort2, "8000");

	G_CommPara.UseSSL = 0x81;
#ifdef V71_MACHINE
	G_CommPara.CurCommMode = GPRS;
#elif  (defined(_MACHINE_V37))
	G_CommPara.CurCommMode = COM;
#elif  V80B_MACHINE
	G_CommPara.CurCommMode = LAN;
#endif
}
 
void SaveCommParam(void)
{
	u8 result;

	do
	{
	    result = WriteFile_Api(COMMPARAMFILE,(u8*)&G_CommPara, 0, sizeof(struct _COMMPARASTRUC_));
	}while(result != 0);
}
void SaveTermParam(void)
{
	SaveCommParam();
} 
void ReadCommParam(void)
{
    u32 ReadLen;

    ReadLen = sizeof(struct _COMMPARASTRUC_);
    memset(&G_CommPara,0,sizeof(struct _COMMPARASTRUC_));

    if(ReadFile_Api(COMMPARAMFILE, (u8*)&G_CommPara, 0, &ReadLen) == 0)
         return;							 
                                             
    InitCommParam();						 
    SaveCommParam();
}
 
void SaveCtrlParam(void)
{
	u8 result;

	do
	{
		result = WriteFile_Api(CtrlPARAMFILE,(u8*)&gCtrlParam, 0, sizeof(struct _CtrlParam));
	}while(result != 0);
} 
void ReadCtrlParam(void)
{
    char ret;
    unsigned int ReadLen;

    ReadLen = sizeof(struct _CtrlParam);
	ret = ReadFile_Api(CtrlPARAMFILE, (u8*)&gCtrlParam, 0, &ReadLen);
	if( (ret == 0)&&(ReadLen == sizeof(struct _CtrlParam)) )
		return;
	
	Beep_Api(BEEPERROR);
    InitCtrlParam();											 
    SaveCtrlParam();											 
}
 
void InitCtrlParam(void)
{
	memset(&gCtrlParam, 0, sizeof(gCtrlParam));				 	     						 
	gCtrlParam.lTraceNo    = 1;								 
	gCtrlParam.lNowBatchNum= 1;		 				 
                                                             
	strcpy(gCtrlParam.MerchantName, "ACTS Merchant");
	strcpy(gCtrlParam.MerchantNo, "105290045110223");
	strcpy(gCtrlParam.TerminalNo, "18000377");	//00119952
                                                         
	gCtrlParam.beepForInput			= 0;
	gCtrlParam.oprTimeoutValue		= 60;
	gCtrlParam.tradeTimeoutValue	= 60;
	gCtrlParam.ShieldPAN   = 1;
	
	gCtrlParam.pinpad_type = PIN_PED;		 
	gCtrlParam.AKeyIndes = 0;				 
	gCtrlParam.MainKeyIdx =	1;				 
	gCtrlParam.MacKeyIndes = 0;				 
	gCtrlParam.PinKeyIndes = 1;				 		 
	                                   
	gCtrlParam.SupportICC = 1;				 			 
	gCtrlParam.SupportPICC = PEDPICCCARD;	 
         
	gCtrlParam.SupportSignPad = 0;	 
	gCtrlParam.SignTimeoutS = 150;	 
	gCtrlParam.SignMaxNum = 8;		 
	gCtrlParam.SignBagMaxLen = 900;	 	

	gCtrlParam.DesType = 1;						 // gCtrlParam.DesType = 0;
	strcpy(gCtrlParam.ClientId, "ACTS");      
	gCtrlParam.Lang = 2;
}
 
int ReadLog( LOG_STRC *pLog, int logNo )
{
	u32 off, Len;

	if (gCtrlParam.iTransNum == 0)							 
		return 1;
	
	if( logNo == LAST_REC_LOG )
		off = (gCtrlParam.iTransNum - 1) * LOG_SIZE;
	else
		off = logNo * LOG_SIZE;
	
	Len = LOG_SIZE;
	if( ReadFile_Api(RECORDLOG, (unsigned char *)pLog, off, &Len) == 0)
	{
		if(Len == LOG_SIZE)
			return 0;
	}else 
	{
		return E_MEM_ERR;
	}

	return 0;
}
 
void DelLog(int bClearTraceNo)
{
	DelFile_Api(RECORDLOG);
	gCtrlParam.iTransNum = 0;
	
	if(bClearTraceNo)
	{
		gCtrlParam.lTraceNo = 1;
		SaveCtrlParam();
	}
}
 
int SaveLogFile(void)
{
	int ret;
	LOG_STRC  stLog;
	u8 buf[2];
	u8 buff1[2];
	u8 buf2[2];

	memset(buf,0,sizeof(buf));
	memset(buff1,0,sizeof(buff1));
	memset(buf2,0,sizeof(buf2));
	
	memcpy(&stLog, &PosCom.stTrans, sizeof(stLog));
	ret = WriteFile_Api(RECORDLOG, (u8*)&stLog, gCtrlParam.iTransNum*LOG_SIZE, LOG_SIZE) ;
	buf2[0] = ret;
	buf[0] = (u8)gCtrlParam.iTransNum;
	buff1[0] = (u8)LOG_SIZE;



	if(ret != 0 )
	{
		return (E_MEM_ERR);
	}
	
	gCtrlParam.iTransNum++;								 
	SaveCtrlParam();
	
	return(0);
}
 
int UpdateLogFile(void)
{
	u16 i;
	LOG_STRC Log;

	memset(&Log, 0, sizeof(Log));
	
	for(i = 0; i < gCtrlParam.iTransNum; i++)
	{
		if(ReadLog(&Log, i) == 0)
		{
			if(Log.lTraceNo == PosCom.stTrans.OldTraceNo)			// 
			{
				Log.ucRecFalg = RECORDVOID;
				if( WriteFile_Api(RECORDLOG,(u8*)&Log, i*sizeof(Log), LOG_SIZE) != 0)
					return E_MEM_ERR;
				else 
					return 0;
			}
		}
	}
	return 1;
}
 
int SaveSignFile(SIGNLOG *ptSignLog)
{
	int ret;
	u32 filelen;

	filelen = GetFileSize_Api(SIGNFILE);
	ret = WriteFile_Api(SIGNFILE, (u8*)ptSignLog, filelen, SIGNLOGSIZE);
	if(ret != 0)
		return E_MEM_ERR;
	
	gCtrlParam.SingRecNum++;								// 
	SaveCtrlParam();
	return 0;
}
 
int ReadSignFile(SIGNLOG *ptSignLog, u16 recnum)
{
	int ret;
	u32 current, filelen;

	filelen = SIGNLOGSIZE;
	current = recnum*SIGNLOGSIZE;
	ret = ReadFile_Api(SIGNFILE, (u8*)ptSignLog, current, &filelen);
	if(ret != 0)
		return E_MEM_ERR;
	return(0);
}
 
int ReadReversalData(void)
{
	u32 RLen;

	RLen = GetFileSize_Api(DUPFILE);
	ReadFile_Api(DUPFILE, (u8*)&PosCom, 0, &RLen);
	return 0;
}
 
int SaveReversalData(char *pReson)
{
	char stemp[4];
	
	DelFile_Api(DUPFILE);
	memcpy(stemp, PosCom.stTrans.szRespCode, 2);	// 
	strcpy(PosCom.stTrans.szRespCode, pReson);
	if(WriteFile_Api(DUPFILE, (u8*)&PosCom, 0, sizeof(PosCom)) != 0x00 )
		return -1;
	memcpy(PosCom.stTrans.szRespCode, stemp, 2);
	return 0;
}
 
int DelReversalData(void)
{
	if(DelFile_Api(DUPFILE) == 0)
	{
		return 0;
	}
	else 
	{
		return (E_MEM_ERR);
	}
}
 