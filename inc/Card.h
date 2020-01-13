#ifndef AFX_CARD_H
#define AFX_CARD_H

 
enum
{
	MASK_INCARDNO_HANDIN=	0x01,	 
	MASK_INCARDNO_MAGCARD=	0x02,	 
	MASK_INCARDNO_ICC=		0x04,	 
	MASK_INCARDNO_PICC=		0x08,	                                  
	MASK_INCARDNO_KEYAGAIN=	0x10,	 
};                                   
 
enum
{
	CARD_EMVFULL=			0x00,	 
	CARD_EMVSIMPLE=			0x01,	 
	CARD_EMVFULLNOCASH=		0x02,	 
	CARD_EMVFULLCASH=		0x04,	 
	CARD_EMVFULLCASHSALE=	0x08,
	CARD_QPASSONLINE = 0x10
};                                   
                                     
 
int GetCard(u8 mode, u8 type);
 
int GetTrackData(char *Inbuf );
 
int GetCardFromTrack(u8 *CardNo,u8 *track2,u8 *track3);
 
int DispCardNumber(char *CardNo,int len);
 
int DetectCardEvent(u8 Mode, u8 *CardData, u8 timeoutS);
 
 
int PiccCheck(u8 picctype); 
int PiccInit(void);
int PiccStop(void);
#endif




