/*
* Copyright (c) 2009,北京海鑫智圣技术有限公司
* All rights reserved.
*
* 文件名称：THID_RS232.h
* 
* 摘 要：串口底层接口、设置模块
*
* 当前版本：1.0.0.0
* 作 者：张天帅、zwcai
* 完成日期：2009年8月10日

*/
#ifndef _HS_ATMDVS_RS232_H_
#define _HS_ATMDVS_RS232_H_

/* the maximum number of ports we are willing to open */
#define MAX_PORTS 4
#define DEBUGX 0

/*this array hold information about each port we have opened */
struct PortInfo{
	int busy;
	char name[32];
	int handle;
};

int OpenCom(int portNo,const char deviceName[],long baudRate);
int CloseCom(int portNo);
int ComRd(int portNo, unsigned char buf[],int maxCnt,int Timeout);
int ComWrt(int portNo,const unsigned char * buf,int maxCnt);

//long GetBaudRate(long baudRate);
int OpenComConfig(int port,
                  const char deviceName[],
                  long baudRate,
                  char parity,
                  int dataBits,
                  int stopBits,
                  int iqSize,
                  int oqSize);

#endif
