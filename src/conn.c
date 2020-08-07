
void WifiConnect() //ok
{
	u8 ret, tmp[1024*8];
	unsigned short len;

	TipAndWaitEx_Api("getlang:%d", GetLang_Api());

	memset(&G_CommPara, 0, sizeof(G_CommPara));
	G_CommPara.UseSSL = 0x81;
	strcpy((char *)G_CommPara.WifiSet.SSID, "JW-2.4G");  //JW-2.4G
	strcpy((char *)G_CommPara.WifiSet.WpaPsk, "jiewen-2017"); //jiewen-2017
	//strcpy((char *)G_CommPara.WifiSet.SSID, "cccc");
	//strcpy((char *)G_CommPara.WifiSet.WpaPsk, "87654321");
	G_CommPara.WifiSet.Dhcp = 1;
	G_CommPara.WifiSet.EncryptType = 3;
	G_CommPara.WifiSet.SecurityType = 3; 
	strcpy((char *)G_CommPara.NetSet.NetServerIp, _HOSTIP_);    //212.0.129.118:443
	strcpy((char *)G_CommPara.NetSet.NetServerPort, _HOSTPORT_); 
	strcpy((char *)G_CommPara.NetSet.NetServer2Ip, _HOSTIP_); 
	strcpy((char *)G_CommPara.NetSet.NetServer2Port, _HOSTPORT_); 
	G_CommPara.CurCommMode = WIFI;
	SaveCommParam();
	ret = CommModuleInit_Api(&G_CommPara);  //this should be used when G_CommPara.CurCommMode is changed or device is started , and only related with G_CommPara.CurCommMode .
	if(ret != 0)
	{
		TipAndWaitEx_Api("CommModuleInit_Api:%d", ret);
		goto GPRSCONNECTEND ;
	}
	else
		TipAndWaitEx_Api("CommModuleInit_Api:%d", ret);
	CommParamSet_Api(&G_CommPara);  //update the communication paramters
	TipAndWaitEx_Api("CommParamSet_Api");

	ret = CommStart_Api();
	TipAndWaitEx_Api("CommStart_Api:%d", ret);
	if(ret != 0)
	{
		goto GPRSCONNECTEND ;
	}

	ret = CommCheck_Api(20);
	TipAndWaitEx_Api("CommCheck_Api:%d", ret);
	if(ret != 0)
	{
		goto GPRSCONNECTEND ;
	}
	//TipAndWaitEx_Api("CommCheck_Api:%d", ret);
	
	memset(tmp, 0, sizeof(tmp));
	//strcpy((char *)tmp, "Test123");
	strcpy(tmp, "merchantAPI/isAlive\r\n{\"applicationId\": \"Test\",\r\n\"terminalId\": \"19000019\"}");
	//strcpy(tmp, "POST /terminal_api/isAlive/ HTTP/1.1\r\nContent-Type: application/json\r\nHost: 212.0.129.118\r\nAPI-KEY: 5d6f54d4-3af4-4ffc-b78d-c2f1ca7827d9\r\nContent-Length: 94\r\n\r\n{\"systemTraceAuditNumber\":\"191\",\"terminalId\":\"22100007\",\"tranDateTime\":\"2018-06-21T12:15:38Z\"}");
	ret = CommTxd_Api(tmp, (unsigned short)strlen(tmp), 1);
	TipAndWaitEx_Api("CommTxd_Api:%d", ret);
	if(ret != 0)
	{		
		goto GPRSCONNECTEND;
	}


	/*len = 0;
	memset(tmp, 0, sizeof(tmp));
	ret = CommRxd_Api(tmp, &len, 1, 1, 60*1000);
	TipAndWaitEx_Api("Rxd=%d %d", ret, len);*/

	//CommRxd_Api
	len = 0;
	memset(tmp, 0, sizeof(tmp));
	ret = RecvPacket(tmp, &len, 60);
	if(ret != 0)
	{
		TipAndWaitEx_Api("Rxd error:%d", ret);
		goto GPRSCONNECTEND;	
	}
	if(ret == 0)
	{
		TipAndWaitEx_Api("recvlen:%d  %s", len, tmp);
	}
GPRSCONNECTEND:	
	CommClose_Api();
}

void WifiConnectWithoutInit() //ok
{
	u8 ret, tmp[1024*2];
	unsigned short len;

	/*memset(&G_CommPara, 0, sizeof(G_CommPara));
	strcpy((char *)G_CommPara.WifiSet.SSID, "cccc");
	strcpy((char *)G_CommPara.WifiSet.WpaPsk, "87654321");
	G_CommPara.WifiSet.Dhcp = 1;
	G_CommPara.WifiSet.EncryptType = 3;
	G_CommPara.WifiSet.SecurityType = 3;
	G_CommPara.CurCommMode = WIFI;
	SaveCommParam();
	ret = CommModuleInit_Api(&G_CommPara);  //this should be used when G_CommPara.CurCommMode is changed or device is started , and only related with G_CommPara.CurCommMode .
	if(ret != 0)
	{
		TipAndWaitEx_Api("CommModuleInit_Api:%d", ret);
		goto GPRSCONNECTEND ;
	}
	else
		TipAndWaitEx_Api("CommModuleInit_Api:%d", ret);
	CommParamSet_Api(&G_CommPara);  //update the communication paramters
	TipAndWaitEx_Api("CommParamSet_Api");*/

	ret = CommStart_Api();
	TipAndWaitEx_Api("CommStart_Api:%d", ret);
	if(ret != 0)
	{
		goto GPRSCONNECTEND ;
	}

	ret = CommCheck_Api(20);
	TipAndWaitEx_Api("CommCheck_Api:%d", ret);
	if(ret != 0)
	{
		goto GPRSCONNECTEND ;
	}
	//TipAndWaitEx_Api("CommCheck_Api:%d", ret);
	
	/*memset(tmp, 0, sizeof(tmp));
	//strcpy(tmp, "GET / HTTP/1.1\r\nHOST:\r\n\r\n");
	strcpy(tmp, "POST /terminal_api/isAlive/ HTTP/1.1\r\nContent-Type: application/json\r\nHost: 212.0.129.118\r\nAPI-KEY: 5d6f54d4-3af4-4ffc-b78d-c2f1ca7827d9\r\nContent-Length: 94\r\n\r\n{\"systemTraceAuditNumber\":\"191\",\"terminalId\":\"22100007\",\"tranDateTime\":\"2018-06-21T12:15:38Z\"}");
	ret = CommTxd_Api(tmp, (unsigned short)strlen(tmp), 1);
	TipAndWaitEx_Api("CommTxd_Api:%d", ret);
	if(ret != 0)
	{		
		goto GPRSCONNECTEND;
	}*/


	/*len = 0;
	memset(tmp, 0, sizeof(tmp));
	ret = CommRxd_Api(tmp, &len, 1, 1, 60*1000);
	TipAndWaitEx_Api("Rxd=%d %d", ret, len);*/

	//CommRxd_Api
	/*len = 0;
	memset(tmp, 0, sizeof(tmp));
	ret = RecvPacket(tmp, &len, 60);
	if(ret != 0)
	{
		TipAndWaitEx_Api("Rxd error:%d", ret);
		goto GPRSCONNECTEND;	
	}*/
GPRSCONNECTEND:	
	CommClose_Api();
}
