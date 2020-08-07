//#include "stdafx.h"
#define HX_APP_VARIABLES
#include "../inc/global.h"
#undef HX_APP_VARIABLES

const APP_MSG App_Msg={
		"ACTS Demo",				      
		"Solus_App",				 
		"1.2.0.3",				 
		"SolusPay",				 
		__DATE__ " " __TIME__,	 
		"",
		0,
		0,
		0,
		"00001001140616"
};

int event_main(SET_EVENT_MSG *msg)
{
	return 0;
}
 

void CheckFile(void)
{
	int Len;
	
	FormatFileSystem_Api(0);					 
	ReadCommParam();					 
	ReadCtrlParam();
	ReadOperFile();

}
 
void InitSys(void)
{
	int Key;	
	char tmp[32];

	memset(tmp, 0, 32);

	SetLang_Api(2,3); //API tip can be English
	ScrFontSet_Api(5); 
    CheckFile();
	
	ScrCls_Api();
	ScrDisp_Api(LINE3, 0, (char*)App_Msg.Name, FDISP|CDISP);
	strcpy(tmp, "Ver: ");
	strcat(tmp, (char*)App_Msg.Version);
	ScrDisp_Api(LINE4, 0, tmp, FDISP|CDISP);
	ScrDisp_Api(LINE5, 0, (char*)App_Msg.Descript, FDISP|CDISP);
	Key = ProFastKey_Api(2);
	if(Key == 159)								//  
	{
		ScrCls_Api();
		ScrDisp_Api(LINE3, 0,"*", CDISP);
		while(1)
			Simulation_Api();
	}
	if(Key == 357)	
		RcvFileSysOrder();
	
	KBMute_Api(gCtrlParam.beepForInput);
	
	CommModuleInit_Api(&G_CommPara);
	CommParamSet_Api(&G_CommPara);

#ifdef _SIMULATOR   
		G_CommPara.CurCommMode = LAN;
		strcpy( G_CommPara.NetSet.NetServerIp , "197.251.5.77"); //197.251.5.77
		strcpy( G_CommPara.NetSet.NetServerPort , "443"); //
#endif
	

	Common_Init_Api();
	EMV_Init_Api();	
	PayPass_Init_Api();
	PayWave_Init_Api();
	Mir_Init_Api();
	JSpeedy_Init_Api();

	Common_GetParam_Api(&termParam);
	EMV_GetParam_Api(&stEmvParam);

	MagOpen_Api();
	SysConfig_Api("\x02\x01\x01", 3); // when long press [Cancel] key, go to shutdown-menu

#if ( defined(EMVDEBUG))
	EmvAddAppsExp();
	PayPassAddAppExp(0);
	PayWaveAddAppExp();
	MirAddAppExp();
	JSpeedyAddAppsExp();
	AddCapkExample();

#endif

#ifdef _SAVE_APDU_COMMAND_
	Common_DbgEN_Api(1);  //will write APDU command into file "ComLog.dat" 
#endif	

#ifdef _OUTPUTLOG_
	PortOpen_Api(0);
	PortSetBaud_Api(0, 115200, 8, 0, 1);
	CommDebugInfo("Enable Debugging..", "DEB Test..", 10, 0);
#endif	
}
 
void DispMainFace(void)
{
	char sDisp[32];
	
	ACTSLogo();
	ScrDispRam_Api(LINE3, 130, "ACTS", FDISP);
	sprintf(sDisp, "V%s%s%s", "12", "02.", APPVERSION);
	sDisp[12] = 0;
	ScrDispRam_Api(LINE4, 130, sDisp, FDISP);//58
	ScrDispRam_Api(LINE5, 130, "soluspay.net", FDISP);
	ScrBrush_Api();
}

int main(int argc, char *argv[])
{
	int Result = 1;
	PaperOut = REPORT_PAPER_OUT;
	SystemInit_Api(argc, argv);
	InitSys();	
	
	while(1)
	{
		if(Result)
			ScrClsRam_Api();
		DispMainFace();				 
		Result = WaitEvent();
		if(Result == 0xfe)										
			continue;						 
		if(Result != 0)                              
		{                                            
			PhoneHookDisable_Api();                  
			InitPosCom();                                                                     
			switch(Result)							 
			{
				case FUNCTION:						 
					return 0;
				case ENTER:                              
					if(SelectMainMenu() != 0)	 
						continue;                                                
					break;  
				default:
					continue;
			}
			if(PosCom.stTrans.Trans_id)
			{
				TransProcess();
				WaitRemoveICC();
				PiccStop();
			}
		}
	} 
}