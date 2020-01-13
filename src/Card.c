//#include "stdafx.h"
#include "../inc/global.h"

 
int PiccInit(void)
{
	if(gCtrlParam.SupportPICC == PEDPICCCARD)
	{
		if( PiccOpen_Api() != 0x00 )
			return 1;
	}
	else if(gCtrlParam.SupportPICC == EXPICCCARD)
	{
		if( PiccOpenEx_Api() != 0x00 )
			return 1;
	}
	
	return 0;
}
int PiccStop(void)
{
	if(gCtrlParam.SupportPICC == PEDPICCCARD)
	{
		if( PiccClose_Api() != 0x00 )
			return 1;
	}
	else if(gCtrlParam.SupportPICC == EXPICCCARD)
	{
		if( PiccCloseEx_Api() != 0x00 )
			return 1;
	}
	
	return 0;
}

int PiccCheck(u8 picctype)
{
	u8 CardType[8], SerialNo[32];
	u8 ret;
	
	memset(CardType, 0, sizeof(CardType));
	memset(SerialNo, 0, sizeof(SerialNo));
	
	if(picctype == PEDPICCCARD)
	{
		ret = PiccCheck_Api(0, CardType, SerialNo);
		if(ret != 0x00)
			return -1;
		else
			return 0;
	}
	else if(picctype == EXPICCCARD)
	{
		if(PiccCheckEx_Api(0, CardType, SerialNo) != 0x00)
			return -1;
		else
			return 0;
	}
	else
		return -1;
}
 
int GetCardInputCardNO(u8 min, u8 max, char *pInputbuf, u8 displine, u8 timeoutS)
{
	u8 value, len;
	int key;

	while(1)
	{
		len = strlen(pInputbuf);
		if( (min == 0)&&(len == 0) )		 
			break;
		ScrClrLine_Api(displine, displine);
		ScrDisp_Api(displine, 0, pInputbuf, RDISP); 
		
		value = 0x30;
		key = WaitAnyKey_Api(timeoutS);
		switch(key)
		{
			case DIGITAL9:
				value++;
			case DIGITAL8:
				value++;
			case DIGITAL7:
				value++;
			case DIGITAL6:
				value++;
			case DIGITAL5:
				value++;
			case DIGITAL4:
				value++;
			case DIGITAL3:
				value++;
			case DIGITAL2:
				value++;
			case DIGITAL1:
				value++;
			case DIGITAL0:
				if((len+1) <= max)
				{
					pInputbuf[len] = value;
					pInputbuf[len+1] = 0;		 
				}
				break;
			case CLEAR:
				pInputbuf[0] = 0;
				break;
			case DEL:
				if(len)	pInputbuf[len-1] = 0;
				break;
			case ENTER:
				if(len <= 12)
				{
					ScrClrLine_Api(displine, displine);
					ScrDisp_Api(displine, 0, "PAN too short", CDISP);
					WaitAnyKey_Api(5);
					break;
				}
				if(len < min)
					break;
			case ESC:
			case TIMEOUT:
				return key;
			default:
				break;
		}
	}
	return 0;	// 
} 
int DetectCardEvent(u8 Mode, u8 *CardData, u8 timeoutS)
{
	u8 key;
	unsigned int timerid;
		 
	timerid = TimerSet_Api();
	while(!TimerCheck_Api(timerid, timeoutS * 1000))
	{
		if(Mode & MASK_INCARDNO_MAGCARD)
		{
			if( MagSwiped_Api() == 0x00)
			{
				return MASK_INCARDNO_MAGCARD;
			}
		}
	 
		key = GetKey_Api();
		if ( key == ESC )
		{
			return ESC;
		}
		if( (key >= '0') && (key <= '9'))
		{
			if(Mode&MASK_INCARDNO_HANDIN)
			{
				CardData[0] = key;
				return MASK_INCARDNO_HANDIN;
			}
		}
 
		if(Mode & MASK_INCARDNO_ICC)
		{
			if(IccDetect_Api(DEV_IC_NO) == 0x00)		 
			{
				return MASK_INCARDNO_ICC;
			}
		}
 
		if(Mode&MASK_INCARDNO_PICC)
		{
			if(PiccCheck(gCtrlParam.SupportPICC) == 0x00)
			{

				return MASK_INCARDNO_PICC;
			}
			else
				continue;
		}
	}
    return TIMEOUT;
}
 
int CheckIsEmvCard(char *Track2Data)
{
	unsigned char *Seperator = NULL;
	
	if(strlen(Track2Data) == 0)		 
		return 0;

	Seperator = (u8*)strchr(Track2Data, '=');
	AscToBcd_Api(PosCom.stTrans.ExpDate, (char*)Seperator+1, 4);
	
	if( (*(Seperator+5) == '2') || (*(Seperator+5) == '6'))
	{
		return 1;					 
	}
	return 0;
}
 
void GetCard_DispPrompt(u8 mode)
{
//	u8 i, len;
	char sDispOne[64], sDispTwo[64] ;//,*phead
	
	memset(sDispOne, 0, sizeof(sDispOne));
	memset(sDispTwo, 0, sizeof(sDispTwo));

	strcpy(sDispOne, "PLS");
	if(mode & MASK_INCARDNO_ICC)
		strcat(sDispOne, "/Insert");
	if(mode & MASK_INCARDNO_MAGCARD)
		strcat(sDispOne, "/SWIP");
	if(mode & MASK_INCARDNO_HANDIN)  
		strcat(sDispOne, "/HANDIN");
	if(mode & MASK_INCARDNO_PICC)
		strcpy(sDispTwo, "Tap card");

	memcpy(sDispOne, "PLS ", 4);
	ScrClrLine_Api(LINE2, LINE7);
	ScrDisp_Api(LINE2, 0, sDispOne, LDISP);
	ScrDisp_Api(LINE3, 0, sDispTwo, LDISP);

}
 
int GetCard(u8 mode, u8 type)
{
	u8 ReInput, modebak;
	u16 usCardLen;
	char sDispBuf[64];
	u8 CardData[256];
	int ret, event, redisp;

	memset( sDispBuf, 0, sizeof( sDispBuf));
	memset( CardData, 0, sizeof( CardData));

	if(mode&MASK_INCARDNO_PICC)
	{
		if( gCtrlParam.SupportPICC == 0 )
		{
			mode &= ~MASK_INCARDNO_PICC;
			if(mode == 0)
			{
				ErrorPrompt("not support CTLS card", 5);
				return E_TRANS_CANCEL;
			}
		}
		else
		{
			if(gCtrlParam.SupportPICC == EXPICCCARD)	 
			{
				ret = PPReadPinPadSn_Api(CardData);
				if(ret != 0)
				{
					mode &= ~MASK_INCARDNO_PICC;
					if(mode == 0)
						return E_PPNORESP;
				}
			}
			if(PiccInit() == 0)
			{
				initPayPassWaveMirConfig(0x00);
				initJSpeedyConfig();
				CTLPreProcess();
			}
		}		
	}
	if(mode & MASK_INCARDNO_KEYAGAIN)	 
	{
		ReInput = 1;
		mode |= MASK_INCARDNO_HANDIN;
	}
	else
	{
		ReInput = 0;
	}

	if(mode & MASK_INCARDNO_ICC) 
	{
		if( gCtrlParam.SupportICC == 0 )	 
		{                                                                
			mode &= ~MASK_INCARDNO_ICC;									 
			if(mode == 0)
			{
				ErrorPrompt("not support IC card", 5);
				return E_TRANS_CANCEL;
			}
		}
		else
		{
			if(type & CARD_EMVSIMPLE)								 
			{
				type = CARD_EMVSIMPLE;
			}
		}
	}

	modebak = mode;
	redisp = 0;		//Locked to Customize display redisp = 1;
	while(1)
	{
		if(mode == 0)	 
		{
			return NO_DISP;
		}
		if(g_SwipedFlag != 0)
		{
			event = g_SwipedFlag;
			g_SwipedFlag = 0;
		}
		else
		{
			if(redisp != 0)
				GetCard_DispPrompt(mode);
			redisp = 1;
			event = DetectCardEvent(mode, CardData, 60);
		}

		switch(event)
		{
		case 0:
			break;
		case MASK_INCARDNO_PICC:	
			Common_SetIcCardType_Api(PEDPICCCARD, 0);	
			g_ucKerType = App_CommonSelKernel();
			if(g_ucKerType==TYPE_KER_PAYWAVE)
			{
				PosCom.stTrans.EntryMode[0] = PAN_PAYWAVE;
				ret = App_PaywaveTrans();				
			}
			else if(g_ucKerType==TYPE_KER_PAYPASS)
			{
				PosCom.stTrans.EntryMode[0] = PAN_PAYPASS;
				ret = App_PaypassTrans();
				CommDebugInfo("App_PaypassTrans", (u8 *)&ret, sizeof(ret), 1);
			}
			else if(g_ucKerType==TYPE_KER_MIR)
			{
				PosCom.stTrans.EntryMode[0] = PAN_MIR;
				ret = App_Mir_Trans();
				CommDebugInfo("App_Mir_Trans", (u8 *)&ret, sizeof(ret), 1);
			}
			else if(g_ucKerType==TYPE_KER_JSPEEDY)
			{
				PosCom.stTrans.EntryMode[0] = PAN_JSPEEDY;
				ret = App_JSpeedyTrans();				
				CommDebugInfo("App_JSpeedyTrans", (u8 *)&ret, sizeof(ret), 1);
			}
			else
			{
				ErrorPrompt("no app match", 1);
				ret = NO_DISP;
			}
			return ret;
		case MASK_INCARDNO_ICC:				
			PosCom.stTrans.EntryMode[0] = PAN_ICCARD;
			memset(CardData, 0, sizeof(CardData));
			ret = EmvCardProc(PosCom.stTrans.Trans_id, type, CardData);
			if(ret != 0)
			{
				if( (ret == ERR_ICCRESET)||(ret == ERR_NOAPP)||(ret == E_NEED_FALLBACK) )
				{
					if(ret == ERR_NOAPP)
						ErrorPrompt("no app match", 1);
					else
					{
						if(modebak&MASK_INCARDNO_MAGCARD)			 
						{
							ErrorPrompt("Insert Card Error,Tap Card Please", 1);
							PosCom.stTrans.IccFallBack = 1;
							mode &= ~MASK_INCARDNO_ICC;			 
							mode &= ~MASK_INCARDNO_PICC;		 
							mode |= MASK_INCARDNO_MAGCARD;		 
						}                                        
						else
						{
							ConvErrCode(ret, sDispBuf);
							ErrorPrompt(sDispBuf, 1);
							mode = 0;
						}
					}
					break;
				}
				else
				{
					ConvErrCode(ret, sDispBuf);
					ErrorPrompt(sDispBuf, 5);
					return NO_DISP;
				}
			}
			return ret;
		case MASK_INCARDNO_MAGCARD:							 
			event = MagRead_Api(CardData, &usCardLen);
			ret = GetTrackData((char*)CardData);
			if( (event == 0x31) && (ret == 0) )
			{
				PosCom.stTrans.EntryMode[0] = PAN_MAGCARD;			// 
				ret = GetCardFromTrack((u8*)PosCom.stTrans.MainAcc,PosCom.Track2,PosCom.Track3);
			}
			else
			{
				ScrDisp_Api(LINE2, 0, "Card Swipe Error", LDISP);
				ScrDisp_Api(LINE3, 0, "Plz Swipe Again", LDISP);
				redisp = 0;
				break;
			}

			ret = CheckIsEmvCard((char*)PosCom.Track2);
			if( (ret == 1)&&(gCtrlParam.SupportICC||gCtrlParam.SupportPICC) ) 
			{				 
				if( (PosCom.stTrans.IccFallBack == 0) && !(type&CARD_EMVSIMPLE) )
				{
					ScrClrLine_Api(LINE2, LINE7);		
					ScrDisp_Api(LINE2, 0, "This Card Is IC Card", LDISP); 			
					ScrDisp_Api(LINE3, 0, "Insert Card Please-->", LDISP);
					Beep_Api(BEEPERROR);
					WaitAnyKey_Api(1);
					//
					if(gCtrlParam.SupportICC)
						mode |= MASK_INCARDNO_ICC;
					if(gCtrlParam.SupportPICC)
						mode |= MASK_INCARDNO_PICC;
					mode &= ~MASK_INCARDNO_MAGCARD;	 
					//redisp = 0;
					break;
				}
			}
			return ret;
		case MASK_INCARDNO_HANDIN:						 
			PosCom.stTrans.EntryMode[0] = PAN_KEYIN;	 
			memset(sDispBuf, 0, sizeof(sDispBuf));
			sDispBuf[0] = CardData[0];
			ret = GetCardInputCardNO(0, 19, sDispBuf, LINE4, 30);
			if(ret == 0)
				break;
			else if(ret == ENTER)
			{
				strcpy(PosCom.stTrans.MainAcc, sDispBuf);
				if(ReInput)
				{
					//ScrCls_Api();
					ScrClrLineRam_Api(LINE2, LINE5);
					ScrDisp_Api(LINE2, 0, "input PAN again:", LDISP);  //PAN: card nummber
					memset(sDispBuf, 0, sizeof(sDispBuf));
					ret = GetCardInputCardNO(1, 19, sDispBuf, LINE4, 30);
					if(ret == ENTER)
					{
						if(strcmp(PosCom.stTrans.MainAcc, sDispBuf) != 0)
						{
							ScrCls_Api();
							ErrorPrompt("PAN not match", 5); //PAN:card nummber
							return E_TRANS_CANCEL;
						}
					}
					else
					{
						return E_TRANS_CANCEL;
					}
				}
			}
			else
			{
				return E_TRANS_CANCEL;
			}
			return 0;
			break;
		case ESC:
		case TIMEOUT:
		default:
			return E_TRANS_CANCEL;
			break;
		}
	};
	return E_TRANS_CANCEL;
}


int GetTrackData(char *Inbuf )
{
	int iPos, track1Len, track2Len, track3Len;
	char track1[256],track2[256],track3[256];
	
	track1Len = track2Len =track3Len = 0;
	
	memset(track1, 0, sizeof(track1));
	memset(track2, 0, sizeof(track2));
	memset(track3, 0, sizeof(track3));

	iPos = 0;
	if(Inbuf[iPos] != 0)					 
	{ 
		memcpy(track2,&Inbuf[iPos+1], Inbuf[iPos]);
		track2Len = Inbuf[iPos];
		iPos += Inbuf[iPos];
	}
	else
	{
		return 1;
	}
	
	iPos += 1 ;							 
	                                     
	if(Inbuf[iPos] != 0)				 
	{
		memcpy(track3, &Inbuf[iPos+1], Inbuf[iPos]);
		track3Len = Inbuf[iPos];
		iPos += Inbuf[iPos];
	}
	                                         
	iPos += 1 ;								 
	                                         
	if(Inbuf[iPos] != 0)					 
	{                                        
		memcpy(track1, &Inbuf[iPos+1], Inbuf[iPos]);
		track1Len = Inbuf[iPos];
		iPos += Inbuf[iPos];
	}
	   
	if( track1Len != 0 )
	{
		memset(PosCom.stTrans.HoldCardName, 0, sizeof(PosCom.stTrans.HoldCardName));
		GetNameFromTrack1(track1, PosCom.stTrans.HoldCardName);
		memcpy(PosCom.Track1, track1,  track1Len);
		PosCom.Track1Len = track1Len;
	}
	
	if(track2Len!= 0)
	{
		FormBcdToAsc( (char *)PosCom.Track2, (u8*)track2, track2Len * 2);
		PosCom.Track2Len = track2Len;
		
		if(PosCom.Track2[strlen((char *)PosCom.Track2) -1] == 0x3f)
		{
			PosCom.Track2[strlen((char *)PosCom.Track2) -1] = 0;
			PosCom.Track2Len--;
		}
	}
	
	if(track3Len != 0)
	{
		BcdToAsc_Api((char *)PosCom.Track3, (u8*)track3, track3Len*2);
		PosCom.Track3Len = track3Len;
		if(PosCom.Track3[strlen((char *)PosCom.Track3) -1] == 0x3f)
		{
			PosCom.Track3[strlen((char *)PosCom.Track3) -1] = 0;
			PosCom.Track3Len--;
		}
	}
	
	return 0;
}
 
int GetCardFromTrack(u8 *CardNo,u8 *track2,u8 *track3)
{
	int i;
	
	track2[37] = 0;
	track3[104] = 0;
	
	if(strlen((char *)track2) != 0)			 
	{
		i = 0;
		while (track2[i] != '=')
		{
			if(i > 19)
			{
				return E_ERR_SWIPE;
			}
			i++;
		}
		if( i < 13 || i > 19)
		{
			return E_ERR_SWIPE;
		}
		memcpy(CardNo, track2, i);		
		CardNo[i] = 0;
	}
	else if(strlen((char *)track3 )!= 0) 
	{
		i = 0;
		while(track3[i] != '=') {
			if(i > 21)
			{
				return E_ERR_SWIPE;
			}
			i++;
		}			    
		if( i < 15 || i > 21)
		{
			return E_ERR_SWIPE;	
		}
		memcpy(CardNo,track3+2,i-2);		
		CardNo[i-2]=0;
	}
	
	return 0;
}
 
int DispCardNumber(char *CardNo,int len)
{
	ScrDisp_Api(LINE2, 0, "Confirm The Number:", LDISP);
	ScrDisp_Api(LINE3, 0, CardNo, RDISP);
	ScrDisp_Api(LINE4, 0, "According Continue", RDISP);
	if(WaitEnterAndEscKey_Api(30) != ENTER)
	{
		return(E_TRANS_CANCEL);
	}
	
	return 0;
}
 