#include "../inc/global.h"



//In order to strengthen battery protection,we'd better not print receipt and charing/power it off while quantity of electricity < 5%
//Here is the code you can refer to 
void BatChargeTest()
{
	int Ret, IsElectric;
	int SleepWaitTime = 1; 

	Ret = BatChargeProc_Api(3, 287, 0, 1, 0, &IsElectric);
	if(IsElectric == 0 && Ret < 0)//not charging and quantity of electricity < 5%
	{
		unsigned int TimeId = TimerSet_Api();
		while(1)
		{
			Beep_Api(0);
			ScrCls_Api();
			ScrDisp_Api(LINE5, 0, "Low battery, power off/charge", CDISP);
			Delay_Api(500);
			GetKey_Api();// this can make it:  long press ESC to make battery management menu appear (power off/standby/ reboot)
			Ret = BatChargeProc_Api(3, 287, 0, 1, 0, &IsElectric);
			if(IsElectric == 1 || Ret >= 0) //IsElectric:charging            Ret >= 0 :   quantity of electricity >= 5%
			{  
				break;
			}
			if(TimerCheck_Api(TimeId, SleepWaitTime*60*1000) == 1)   //if battery still low in SleepWaitTime minutes, make it power off
			{
				SysPowerStand_Api();
				break;
			}
		}
	}
	else if(IsElectric == 0 && Ret <= 0)
	{
		Beep_Api(0);
		ScrDisp_Api(LINE5, 0, "Low battery, charge it", CDISP);
	}
}




