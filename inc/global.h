#ifndef HX_GLOBAL_H
#define HX_GLOBAL_H

#ifdef HX_APP_VARIABLES
    #define EXTERN 
#else
    #define EXTERN extern 
#endif

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "struct.h"
#include "coredef.h"

#include "Card.h"
#include "communicate.h"
#include "func.h"
#include "Print.h"
#include "file.h"
#include "TranProc.h"
#include "EmvCommon.h"
#include "EMV.h"
#include "OtherDef.h"
#include "poslib.h"
#include "PayPass.h"
#include "PayWave.h"
#include "Mir.h"
#include "JSpeedy.h"
#include "cJSON.h"
#include "time.h"

#define APPVERSION	"191112"  

#define REPORT_PAPER_OUT 0

#define PAPER_OUT_REPORTED 1
// #define _RECVDATA_DEBUG_	  //virtual receive data for demo  	 				 
#define EMVDEBUG	   //add some emv/paypass/paywave parameters for testing 
//#define _MACHINE_V37   //Opened(not print) for V37/V36H, closed(print) for V71/V80SE
#define _OUTPUTLOG_    //output log by serial port  
//#define _SAVE_APDU_COMMAND_   //save APDU command into file "ComLog.dat" 

//#define LST_DISPLAY

#define V71_MACHINE

#define GPRS_IP 0
#define GPRS_Domain 1


EXTERN POS_COM PosCom;
EXTERN OPER_STRC gtOperator;
EXTERN u8 BitMap[17];
EXTERN char g_EmvFullOrSim;			 
EXTERN u8 g_SwipedFlag;				                			 
	 
                                          
EXTERN EMV_TERM_PARAM stEmvParam;
EXTERN COMMON_TERMINAL_PARAM termParam;		 		 	                  

EXTERN int g_ucKerType;
EXTERN int PWaveCLAllowed;

EXTERN int MirCLAllowed;

EXTERN int PaperOut;
                                         
#endif

