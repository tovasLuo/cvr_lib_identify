/*
 * Copyright (c) 2009,北京海鑫智圣技术有限公司
 * All rights reserved.
 **THID_IDCardLibTest.c
 **current version: 1.0
 **created time:2011-07-14
 **Author:Ly
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../THID_CVR_IDCard.h"

/*
**test switch 
*/
#define Authenticate_tst 0
#define ReadContent_tst1 0
#define ReadContent_tst2 0
#define ReadMsg_tst1 1
#define ReadMsg_tst2 0
#define GetPersonDetailInfo_tst1 0
#define GetPersonDetailInfo_tst2 0
#define AutoCheck 0

#define deviceName "/dev/ttyS0" //CVR-100D, CVR-100D-JG
//#define deviceName "/dev/ttyUSB0" //farm machine(CVIDR-COMM-V100)

static void printInfo(char *infoName, char *strTmp, int strLen);
static void getDetailInfoTst(void);

int main(int argc, char *argv[])
{
	int success;
	int i;
	
	printf ("###########O(∩_∩)O~Test start!############\n");
/*
**Authenticate_tst
*/
#if Authenticate_tst
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}

	success = CVR_Authenticate();
	if (1 == success)
	{
		printf("Authenticate success!\n");
	}
    printf("start to CVR_CloseComm!\n");	
	CVR_CloseComm();
    printf("CVR_CloseComm success !\n");
#endif  //Authenticate_tst

/*
**ReadContent_tst1 
*/
#if ReadContent_tst1
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}
	
	success = CVR_Authenticate();
	if (1 != success)
	{
		printf("call CVR_Authenticate failed!\n");
		return -1;
	}
	else
	{
		printf("CVR_Authenticate() success!\n");
	}
	
	for (i = 1; i < 6; i ++)
	{
		success = CVR_Read_Content(i);
		if (1 != success)
		{
			printf("[%d]call CVR_Read_Content() failed!\n",i);
		}
		else
		{
			printf("[%d]call CVR_Read_Content(): success!\n", i);
		}
		sleep(2);
	}
	
	CVR_CloseComm();
#endif  //ReadContent_tst1

/*
**ReadContent_tst2
*/
#if ReadContent_tst2
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}
	success = CVR_Authenticate();
	if (1 != success)
	{
		printf("call CVR_Authenticate failed!\n");
		return -1;
	}
	else
	{
		printf("CVR_Authenticate() success!\n");
	}
	success = CVR_Read_Content(7);
	if (1 != success)
	{
		printf("call CVR_Read_Content failed!\n");
        return -1;
	}
	else
	{
		printf("CVR_Read_Content() success!\n");
	}
	
	CVR_CloseComm();
#endif  //ReadContent_tst2

/*
**ReadMsg_tst1: 
*/
#if ReadMsg_tst1
	unsigned char *pucCHMsg, *pucPHMsg;
    unsigned char finger[1024] = {0};
	unsigned int pucCHMsgLen = 256, pucPHMsgLen = 126 * 102 * 3 + 1024;
	pucCHMsg = (unsigned char*)malloc(pucCHMsgLen);
	pucPHMsg = (unsigned char*)malloc(pucPHMsgLen);
	if (NULL == pucCHMsg)
	{
		printf("malloc failed!\n");
		return -1;
	}
	if (NULL == pucPHMsg)
	{
		return -1;
		printf("malloc failed!\n");
	}
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}
	
	success = CVR_Authenticate();
	if (1 != success)
	{
		printf("call CVR_Authenticate failed!\n");
        free(pucCHMsg);
        free(pucPHMsg);
		return -1;
	}
	else
	{
		printf("CVR_Authenticate() success!\n");
	}
	int finger_size = 0;
	for (i = 1; i < 6; i ++)
	{	
		success = CVR_ReadBaseMsg(pucCHMsg, &pucCHMsgLen, pucPHMsg, &pucPHMsgLen,finger, &finger_size,i);
		if (1 != success)
		{
			printf("call CVR_ReadBaseMsg() failed!\n");
		}
		else
		{
			printf("[%d]call CVR_ReadBaseMsg() success!\n", i);
            printf("finger_size:%d\n",finger_size);
		}
		sleep(2);
	}
    char strTmp[256] = {0};
    int nlen = sizeof(strTmp);
    GetPeopleBirthday(strTmp,&nlen);
    printf("%s\n",strTmp);
	free(pucCHMsg);
	free(pucPHMsg);
	CVR_CloseComm();
#endif  //ReadMsg_tst1

/*
**ReadMsg_tst2: 
*/
#if ReadMsg_tst2
	unsigned char *pucCHMsg, *pucPHMsg;
    unsigned char finger[1024] = {0};
	unsigned int pucCHMsgLen = 512, pucPHMsgLen;
	pucCHMsg = (unsigned char*)malloc(pucCHMsgLen);
	if (NULL == pucCHMsg)
	{
		printf("malloc failed!\n");
		return -1;
	}
	//pucPHMsg = malloc(1024);
	pucPHMsg = (unsigned char*)malloc(126 * 102 * 3 + 1024);
	if (NULL == pucPHMsg)
	{
		return -1;
		printf("malloc failed!\n");
	}
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm success!\n");
	}
    int nIndex = 0;
//    while(1)
    {
        success = CVR_Authenticate();
        if (1 != success)
        {
            printf("call CVR_Authenticate failed!\n");
            usleep(40 * 1000);
//            continue;
        }
        else
        {
            printf("CVR_Authenticate() success!\n");
        }

        success = CVR_ReadBaseMsg(pucCHMsg, &pucCHMsgLen, pucPHMsg, &pucPHMsgLen,finger, 5);
        if (1 != success)
        {
            printf("call CVR_ReadBaseMsg failed!\n");
        }
        else
        {
            printf("call CVR_ReadBaseMsg success!\n");
            char strTmp[256] = {0};
            int nlen = sizeof(strTmp);
            GetPeopleName(strTmp,&nlen);
            printf("name:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetPeopleSex(strTmp,&nlen);
            printf("sex:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetPeopleNation(strTmp,&nlen);
            printf("nation:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetPeopleBirthday(strTmp,&nlen);
            printf("birthday:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetPeopleAddress(strTmp,&nlen);
            printf("address:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetPeopleIDCode(strTmp,&nlen);
            printf("idcode:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetDepartment(strTmp,&nlen);
            printf("department:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetStartDate(strTmp,&nlen);
            printf("startdate:%s\n",strTmp);

            memset(strTmp,0,sizeof(strTmp));
            GetEndDate(strTmp,&nlen);
            printf("enddate:%s\n",strTmp);
        }
    }
	if (NULL != pucCHMsg)
	{
		free(pucCHMsg);
		pucCHMsg = NULL;
	}
	if (NULL != pucPHMsg)
	{
		free(pucPHMsg);
		pucPHMsg = NULL;
	}
    
	CVR_CloseComm();
#endif  //ReadMsg_tst2

/*
**GetPersonDetailInfo_tst1: 
*/
#if GetPersonDetailInfo_tst1
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}
	
	success = CVR_Authenticate();
	if (1 != success)
	{
		printf("call CVR_Authenticate failed!\n");
		return -1;
	}
	else
	{
		printf("CVR_Authenticate() success!\n");
	}
	
	success = CVR_Read_Content(4);

	if (1 == success)
	{
		printf("CVR_Read_Content() success!\n");
		getDetailInfoTst();
	}

	CVR_CloseComm();
#endif  //GetPersonDetailInfo_tst1

/*
**GetPersonDetailInfo_tst2:
*/
#if GetPersonDetailInfo_tst2
	unsigned char *pucCHMsg, *pucPHMsg;
	unsigned int pucCHMsgLen = 512, pucPHMsgLen;
	pucCHMsg = (unsigned char*)malloc(256);
	if (NULL == pucCHMsg)
	{
		printf("malloc failed!\n");
		return -1;
	}
	//pucPHMsg = malloc(1024);
	pucPHMsg = (unsigned char*)malloc(126 * 102 * 3 + 1024);
	if (NULL == pucPHMsg)
	{
		return -1;
		printf("malloc failed!\n");
	}
	success = CVR_InitComm(0, deviceName);
	if (1 != success)
	{
		return -1;
	}
	else
	{
		printf("CVR_InitComm() success!\n");
	}
	
	success = CVR_Authenticate();
	if (1 != success)
	{
		printf("call CVR_Authenticate failed!\n");
		return -1;
	}
	else
	{
		printf("CVR_Authenticate() success!\n");
	}
	
	success = CVR_ReadBaseMsg(pucCHMsg, &pucCHMsgLen, pucPHMsg, &pucPHMsgLen, 4);
	if (1 == success)
	{	
		printf("CVR_ReadBaseMsg() success!\n");
		printf("start test get person detail Info\n");
		getDetailInfoTst();
	}
	CVR_CloseComm();
	free(pucCHMsg);
	free(pucPHMsg);
#endif  //GetPersonDetailInfo_tst2

#ifdef AutoCheck
    int n_ret = AutoCheckDevice((char*)"/dev/ttyUSB0");
    //int n_ret = AutoCheckDevice((char*)"/dev/ttyS0");
    if(n_ret == CVR_ERR_NONE){
        printf("idcard connected!\n");
    }else{
        printf("no connected!\n");
    }
#endif
	printf("\n############\(^o^)/~Test End!#############\n");

	return 0;
}

static void printInfo(char *infoName, char *strTmp, int strLen)
{
	int i;
	
	printf("\n%s,len :%d\n", infoName, strLen);
	printf("%s ", strTmp);

	return ;
}

static void getDetailInfoTst(void)
{
	char strTmp[80];
	int strLen = sizeof(strTmp);
	
	memset(strTmp, 0, 80);
	GetPeopleName(strTmp, &strLen);
	printInfo("get name", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetPeopleSex(strTmp, &strLen);
	printInfo("get sex", strTmp, strLen);
	
	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetPeopleNation(strTmp, &strLen);
	printInfo("get Nation", strTmp, strLen);
	
	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetPeopleBirthday(strTmp, &strLen);
	printInfo("get Birthday", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetPeopleAddress(strTmp, &strLen);
	printInfo("get Address", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetPeopleIDCode(strTmp, &strLen);
	printInfo("get IDCode", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetDepartment(strTmp, &strLen);
	printInfo("get Department", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetStartDate(strTmp, &strLen);
	printInfo("get StartDate", strTmp, strLen);

	memset(strTmp, 0, 80);
	strLen = sizeof(strTmp);
	GetEndDate(strTmp, &strLen);
	printInfo("get EndDate", strTmp, strLen);

	
	return;
}

