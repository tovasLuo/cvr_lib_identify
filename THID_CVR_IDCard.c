/*
 * Copyright (c) 2009,北京海鑫智圣技术有限公司
 * All rights reserved.
 **THID_CVR_IDCard.c
 **current version: 1.0
 **created time:2011-07-14
 **Author:Ly
 **amended time:2011-08-15
 **description: 
 **		1: add verify result of random cmd_init0 in CVR_Authenticate()
 **		2: using fix lic (2010~2013)
 **		3: amend CVR_ReadBaseMsg()
 **			-add 0x0D 0x0A in wz data, and Sex and Nation is decode
 **			-store idcard info to memory, not create file
 **		4: amend CVR_Read_Content()
 **			-add 0x0D 0x0A in wz data, and Sex and Nation is decode
 **			-add type6
 **		5: amend GetPeopleName(),GetPeopleSex(),GetPeopleNation(),...etc, get detail info(
 **			GBK), Sex and Nation is decode
 **		
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>               
#include <time.h>
#include <iconv.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "WLTLib.h"
#include "THID_RS232.h"
#include "THID_CVR_IDCard.h"



#define SAVEDATA 0
#define GET_LICEN_PRINT 0
#define CARD_INFO_PRINT 0

/*
** DEBUG: print debug info
*/
#define DEBUG 0
#if DEBUG
#define DPRINTF printf
#else
#define DPRINTF
#endif

/*
**WARN: print warning info
*/
#define WARN 1
#if WARN
#define WARNING printf
#else
#define WARNING
#endif

/*
**ERR: print err info
*/
#define ERR 1
#if ERR
#define ERROR printf
#else
#define ERROR
#endif

/*
**switch CVR_100D_JG used for cvr-100D-JG, 1 is valid , 0 is invalid
*/
#define CVR_100D_JG 1
//#define CVR_100AU_NJ 1
//#define CVR_100R 

/*
**define file name
*/
#define FILE_NAME_WZ "WZ.TXT"  //word data
#define FILE_NAME_XP "XP.WLT"  //photo data
#define FILE_NAME_ZP_D "ZP.BMP" //photo data(decode) 
#define FILE_NAME_NEWADDR "NEWADD.TXT" //new address
#define FILE_NAME_CHIPNUM "IINSNDN.bin" //Chip management number 
#define BASE_FILE "base.dat"
#define READ_BUFF_SIZE 1500
/*
**define iner global variable
*/
static int portNo = 0;   //used store opened port number
static char divTmpBuf[512]; //used to store info after divded by 0x0D and 0x0A and change sexCode&nationCode to wz
static int idCardInfoDetailOffset[10]; //used to store idCard detail info offset
static int manuID = 0;   //used store use device unique number(lic's last part)
static int nAllReadBufSize = READ_BUFF_SIZE;
static unsigned char* allReadBuf = (unsigned char*)malloc(READ_BUFF_SIZE);

#if CVR_100D_JG
static unsigned char fixedLic[] = "0501-****0101-0012345678"; //fixed license
#endif  //CVR_100D_JG

#if CVR_100D_JG                 
unsigned char cmd_init0[18] = {0x02, 0x00, 0x11, 0x03, 0xCC, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x03 };
const unsigned char cmd_init[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x11, 0xFF, 0xED };
const unsigned char cmd_readsamv[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x12, 0xFF, 0xEE  };
const unsigned char cmd_find[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x20, 0x01, 0x22  };
const unsigned char cmd_selt[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x20, 0x02, 0x21  };
const unsigned char cmd_read[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x30, 0x01, 0x32 };
const unsigned char cmd_get_key[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x12, 0xFF, 0xEE };
const unsigned char cmd_readAll[]={0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x30, 0x10, 0x23 };

#endif  //CVR_100D_JG

#ifdef CVR_100AU_NJ
const unsigned char cmd_init0[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x01, 0xFC };
const unsigned char cmd_init1[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x01, 0xFC };
const unsigned char cmd_init2[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x01, 0xFC };
const unsigned char cmd_find[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x81, 0x7C };
const unsigned char cmd_selt[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x82, 0x7B };
const unsigned char cmd_read[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x00, 0x83, 0x7A };
const unsigned char cmd_get_key[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x12, 0xFF, 0xEE };
#endif

#ifdef CVR_100R
const unsigned char cmd_init0[] = {0x02, 0x00, 0x11, 0x03, 0xAA, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xB9, 0x03 };
const unsigned char cmd_init1[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x11, 0xFF, 0xED };
const unsigned char cmd_init2[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x12, 0xFF, 0xEE  };
const unsigned char cmd_find[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x20, 0x01, 0x22  };
const unsigned char cmd_selt[]  = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x20, 0x02, 0x21  };
const unsigned char cmd_read[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x30, 0x01, 0x32 };
const unsigned char cmd_get_key[] = {0xAA, 0xAA, 0xAA, 0x96, 0x69, 0x00, 0x03, 0x12, 0xFF, 0xEE };
#endif

/*
**program running state
*/
enum 
{
	STATE_NOSTART = 0,
	STATE_IDLE,
	STATE_INIT0, 
	STATE_INIT1, 
	STATE_INIT2,    
	STATE_FIND,   
	STATE_SELT,   
	STATE_READ
} state;

/*
**the type of readed Idcard info
*/
enum
{
	IDCARD_INFO_TYPE1 = 1, //create words data WZ.TXT, photo data XP.WLT and photo ZP.BMP(decode)
	IDCARD_INFO_TYPE2, //create words data WZ.TXT, photo data XP.WLT
	IDCARD_INFO_TYPE3, //create new address NEWADD.TXT, create blank file if no new address
	IDCARD_INFO_TYPE4, //create WZ.TXT(decode), photo ZP.BMP(decode)
	IDCARD_INFO_TYPE5, //Chip management number IINSNDN.bin
	IDCARD_INFO_TYPE6,  //use device unique number, create WZ.TXT(decode), photo XP.BMP(decode)(used in terminating network)
} Active;

/*
**the Idcard info mode
*/
enum
{
	IDCARD_INFO_MODE1 = 1, //word default format is UCS-2, photo is not decompress to zp.bmp file
	IDCARD_INFO_MODE2, //word had changed to GBK format, photo is not decompress to zp.bmp file
	IDCARD_INFO_MODE3, //word default format is UCS-2, photo had decompressed to zp.bmp file
	IDCARD_INFO_MODE4, //word had changed to GBK format, photo had decompressed to zp.bmp file
	IDCARD_INFO_MODE5,  //not create file, store wz info(decode) and photo info(decode) in the memory
} InfoMode;

/*
**reffer addr:http://qq164587043.blog.51cto.com/261469/63349
**code_convert(): convert code form one to another
**
**return:
**	refer CVR_ERR_XXX
**note: inlen should == out len
*/
static int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
    char* pin = inbuf;
	char *pout = outbuf;
	cd = iconv_open(to_charset,from_charset);
	if (cd == 0)
    {
        return CVR_ERR_FAILED;
    }
	memset(outbuf,0,outlen);
    size_t s_inlen = inlen;
    size_t s_outlen = outlen;
	if (iconv(cd,&pin,&s_inlen,&pout,&s_outlen) == -1)
    {
        WARNING("iconv failed\n");
        iconv_close(cd);
        return CVR_ERR_FAILED;
    }
	iconv_close(cd);
	return CVR_ERR_NONE;
}


/*
**ComRdAll(): read all data from port 
**
**return:
**   the length of read data
*/
static int ComRdAll(int portNo,unsigned char buf[], int maxCnt,int Timeout)
{
	int read_ret;
	int left, tot;

	left = maxCnt;
	tot = 0;
	do
	{
		read_ret = ComRd(portNo, buf+tot, left, Timeout);
        if(read_ret==0)
            break;
        
		left -= read_ret;
		tot += read_ret;
	}while(left>0);

	return tot;
}

/*
**Decode2BmpFile(): decode wlt file  to bmp file
**wltFile[in]: wlt file name 
**bmpFile[in]: bmp file name
**return:
**	1: success, other:failed
*/
static int Decode2BmpFile(const char * wltFile, const char * bmpFile)
{
	struct timeval tv0, tv1;
	FILE *fpFile;
	int length = 1024;
	unsigned char * pData = NULL;
	unsigned char * pOutBuffer = NULL;
	int outlength;
	int ret;

	//1. open wlt file, store wlt file data to pData
	pData = (unsigned char*)malloc(length);
	fpFile = fopen(wltFile, "rb+");
	if(NULL==fpFile){
        ret = -1;
        goto end;
    }
	fread((char *)pData, 1, length, fpFile);
	fclose(fpFile);

	//2. call CV_Decode(), change wlt data to bmp data, store in pOutBuffer
	pOutBuffer = (unsigned char*)malloc(126 * 102 * 3 + 1024);
	gettimeofday(&tv0,NULL);
	ret = CV_Decode(pData, length, pOutBuffer, CV_FORMAT_BMP, &outlength);
	gettimeofday(&tv1,NULL);
#if CARD_INFO_PRINT
	DPRINTF("Decode return %d,len:%d t:%dms\n", ret, outlength,
		(tv1.tv_sec - tv0.tv_sec)*1000 + (tv1.tv_usec - tv0.tv_usec)/1000);
#endif
	//3. create bmpFile and store pOutBuffer data in it
	fpFile = fopen(bmpFile, "wb+");
	if(NULL==fpFile){
        ret = -1;
        goto end;
    }
	fwrite((char *)pOutBuffer, 1, outlength, fpFile);
	fclose(fpFile);
end:
    if(pData != NULL){
	    free(pData);
    }
    if(pOutBuffer != NULL){
	    free(pOutBuffer);
    }

	return ret;
}


/*
**Decode2BmpBuf(): decode wlt data and store bmp data to buffer
**
**return:
**	1: success, -1: failed
*/
static int Decode2BmpBuf(unsigned char *pData, unsigned char *pOutBuffer,
							unsigned int pDataLen, unsigned int *outlength,
							char *basefile, char* cLic)
{
    int ret;
    char version[128];
    char *pBaseData = (char*)malloc(2048);
    FILE *fp;

    fp = fopen(basefile, "rb+");
    if( fp==NULL)
    {
        WARNING("Can not open %s!\n", basefile);
        ret = -1;
        goto end;
    }
    fread(pBaseData, 1, 1920, fp);
    fclose(fp);

    CV_VersionInfo(version);

    //1.decode init
    ret = CV_InitializeEx("0501-20110511-0123456789", (const int *)pBaseData,"./license.lic");
    if (1 != ret)
    {
        WARNING("initialize ret: %d\n", ret);
        ret = -1;
        goto end;
    }

    //2.decode
    ret = CV_Decode(pData, pDataLen, pOutBuffer, CV_FORMAT_BMP,(int*) outlength);
    if (1 != ret)
    {
        WARNING("Decode ret: %d\n", ret);
        ret = -1;
        goto end;
    }

    //3.decode abort
end:
    CV_Finalize();
    free(pBaseData);
    return ret;
}

/*
**Decode2BmpCall(): decode wltfile to bmpfile 
**
**return:
**	1: success, -1: failed
*/
static int Decode2BmpCall(char *wlffile, char *bmpfile, char *basefile,unsigned char* cLic)
{
    int ret;
    char version[128];
    char *pBaseData = (char *)malloc(2048);

    FILE *fp;
    fp = fopen(basefile, "rb+");
    if( fp==NULL)
    {
        WARNING("Can not open %s!\n", basefile);
        ret = -1;
        goto end;
    }
    fread(pBaseData, 1, 1920, fp);
    fclose(fp);

    CV_VersionInfo(version);

    //1. decode init
    ret = CV_InitializeEx("0501-20110511-0123456789", (const int *)pBaseData,"./license.lic");
    if (1 != ret)
    {
        WARNING("initialize ret: %d\n", ret);
        char error[128] = {0};
        CV_ErrorInfo(ret,error);
        WARNING("cv_initialize error%s\n",error);
        ret = -1;
        goto end;
    }

    //2. decode - create bmpfile using wlffile
    ret = Decode2BmpFile(wlffile, bmpfile);
    if (1 != ret)
    {
        WARNING("Decode ret: %d\n", ret);
        ret = -1;
        goto end;
    }

    //3. decode abort
end:
    CV_Finalize();
    free(pBaseData);

    return ret;
}

/*
**randCreateNum(): rand fill cmd_init data
**startOffset: rand fill cmd_init data start from startOffset
**endOffset: rand fill cmd_init data end from endOffset
**return:
**     refer CVR_ERR_XXX
*/
static void randCreateNum(unsigned char*cmdInit0, int startOffset, int endOffset)
{
	int i;
	srand(time(NULL));

	for(i = startOffset; i <= endOffset; i ++)
	{	
		cmdInit0[i] = rand() % 256;
	}
}

/*
**authenticateCmdInit0():  verify the received data after send cmd_init0
**startOffset: start offset 
**endOffset:  end offset
**return:
**     refer CVR_ERR_XXX
*/
static int authenticateCmdInit0(unsigned char*receivedData, int startOffset, int endOffset)
{
	int i, j, ret = CVR_ERR_NONE;
	unsigned char cmdInit0Tmp[18];
	
	memcpy(cmdInit0Tmp, cmd_init0, sizeof(cmd_init0));
	for(i = startOffset, j = 0; i <= endOffset; i ++, j ++)
	{
		cmdInit0Tmp[i] = (j + 3) ^ cmd_init0[i];
	}
	ret = memcmp(cmdInit0Tmp, receivedData, sizeof(cmdInit0Tmp)-1);
	if (0 == ret)
	{
		ret = CVR_ERR_NONE;
	}
	else
	{
		ret = CVR_ERR_FAILED;
	}
	
	return ret;
}

/*
**Authenticate(): Authenticated between IDCard reader and IDCard once
**
**return: 
**      refer CVR_ERR_XXX
*/
static int Authenticate (void)
{
    int ret = CVR_ERR_FAILED;
    int read_ret;
    int wsize = 0;
    int retry;
    unsigned char buf_read[32];

    switch( state )
    {
        case STATE_NOSTART :
            {
                state = STATE_INIT0;
            }
            break;

        case STATE_INIT0:
            {
#if CARD_INFO_PRINT
                DPRINTF("###########STATE_INIT0#############\n");
                DPRINTF("\n");
#endif

#if CVR_100D_JG
                //send cmd_readsamv
                int wrt_ret = ComWrt(portNo, cmd_readsamv, sizeof(cmd_readsamv));
                read_ret = ComRdAll(portNo, buf_read, 7, 1000);
                if(read_ret == 7)
                {
                    int left = (buf_read[5] << 8) + buf_read[6];
#if CARD_INFO_PRINT
                    DPRINTF("cmd_readsamv left: %d\n",left);
#endif
                    read_ret = ComRdAll(portNo,buf_read,left,1000);
                    if(buf_read[0] == 0x00 && buf_read[1] == 0x00 && buf_read[2] == 0x90)
                    {
                        state = STATE_FIND;
#if CARD_INFO_PRINT
                        DPRINTF("cmd_readsamv success!\n");
                        DPRINTF("cmd_readsamv state:%d\n",state);
#endif
                        ret = CVR_ERR_PROCESS;
                    }
                    else
                    {
                        ret = CVR_ERR_FAILED;
                    }
                }
#endif  

#ifdef CVR_100AU_NJ
                ComWrt(portNo, cmd_init0, sizeof(cmd_init0));
                read_ret = ComRd(portNo, buf_read, 24, 1000);
                if(read_ret == 24)
                {
#if CARD_INFO_PRINT
                    int i;
                    for(i = 0; i < read_ret; i ++)
                        DPRINTF("%02X ", buf_read[i]);
                    DPRINTF("\n");
#endif //CARD_INFO_PRINT
                    state = STATE_FIND;
                }
                else
                {
                    ret = CVR_ERR_FAILED;
                }
#endif //CVR_100AU_NJ
            }
            break;

        case STATE_FIND:
            {
                int wrt_ret = ComWrt(portNo, cmd_find, sizeof(cmd_find));
#if CARD_INFO_PRINT
                DPRINTF("find card wrt_ret:%d\n",wrt_ret);
#endif
                read_ret = ComRdAll(portNo, buf_read, 7, 1000);
#if CARD_INFO_PRINT
                int i;
                for(i = 0; i < read_ret; i ++)
                    DPRINTF("%02X ", buf_read[i]);
                DPRINTF("\n");
#endif 
                if(read_ret == 7)
                {
                    short left = 0;
                    memcpy(&left,&buf_read[6],1);
                    memcpy((void*)(&left)+1,&buf_read[5],1);
#if CARD_INFO_PRINT
                    DPRINTF("find card left: %d\n",left);
#endif
                    read_ret = ComRdAll(portNo, buf_read, left, 1000);
#if CARD_INFO_PRINT
                    int i;
                    for(i = 0; i < read_ret; i ++)
                        DPRINTF("%02X ", buf_read[i]);
                    DPRINTF("\n");
#endif 
                    if(buf_read[0] == 0x00 && buf_read[1] == 0x00 && buf_read[2] == 0x9F)
                    {
                        state = STATE_SELT;
                        ret = CVR_ERR_NONE;
#if CARD_INFO_PRINT
                        int i;
                        for(i = 0; i < read_ret; i ++)
                            DPRINTF("%02X ", buf_read[i]);
                        DPRINTF("\n");
#endif 
                    }
                    else
                    {
                        state = STATE_FIND;
                    }
                }
                else
                {
                    state = STATE_INIT0;
                    ret = CVR_ERR_FAILED;
                }
            }
            break;
    }
    return ret;
}

/*
 *read card data include finger data
*/
static int Card_Reader_All(unsigned char* buf_read,int *pSize)
{
    memset(buf_read,0,*pSize);
    int ret = CVR_ERR_NONE;
    int wsize = 0;
    int wrt_ret = ComWrt(portNo, cmd_selt, sizeof(cmd_selt));
	int read_ret = ComRdAll(portNo, buf_read, 7, 1000);
	if(read_ret==7)
	{
		wsize = buf_read[6];
		read_ret = ComRdAll(portNo, buf_read, wsize, 1000);
		if(read_ret==wsize)
		{
			ret = CVR_ERR_NONE;
		}
		else
		{
			WARNING("##########1#Card Selected failed!#############\n");
			ret = CVR_ERR_FAILED;
		}
	}
	else
	{
		WARNING("##########2#Card Selected failed!#############\n");
		ret = CVR_ERR_FAILED;
	}

	//card read
	if (CVR_ERR_NONE == ret)
	{
        ComWrt(portNo,cmd_readAll,sizeof(cmd_readAll));
		read_ret = ComRdAll(portNo, buf_read, 7, 1000 * 2);
		if(read_ret==7)
		{
			wsize = buf_read[6] + (buf_read[5] << 8);
#if CARD_INFO_PRINT
            int i = 0;
            for(i = 0; i < read_ret; i ++)
				DPRINTF("%02X ", buf_read[i]);
			DPRINTF("\n");

            DPRINTF("cmd_readAll:%d\n",wsize);
#endif
            if(wsize > nAllReadBufSize){//实际读出的信息比预设的大
                free(buf_read);
                allReadBuf = (unsigned char*)malloc(wsize);
                buf_read = allReadBuf;
                nAllReadBufSize = wsize;
            }
            *pSize = wsize;
			read_ret = ComRdAll(portNo, buf_read, wsize, 1000);
			if(read_ret==wsize)
			{      
				if( wsize == 4 )  // no data
				{
					WARNING("##############no data!###############\n");
					ret = CVR_ERR_FAILED;
				}
			}
			else
			{
				WARNING("##########1#Card Read failed !#############\n");
				ret = CVR_ERR_FAILED;
			}
		}
		else
		{
#if CARD_INFO_PRINT
            DPRINTF("comrdall read_ret:%d\n",read_ret);
#endif
			WARNING("##########2#Card Read failed !#############\n");
			ret = CVR_ERR_FAILED;
		}
	}
	return ret;
}


/*
**create_file(): store dataBuf data to file 
**dataBuf: data buffer
**fileName: file name to create
**size: the size store to file
**bufOffset: the data buffer's offset
**return:
**	refer CVR_ERR_XXX
*/
static int create_file(unsigned char* dataBuf, char* fileName, 
							int size, int bufOffset)
{
	int ret = CVR_ERR_NONE;
	FILE *fpout;

	// open file , create if not exist
	fpout = fopen(fileName, "wb+");
	if(fpout!=NULL)
	{
		//write data to file
		ret = fwrite(dataBuf + bufOffset, size, 1, fpout);
		if (0 == ret)
		{
			WARNING("write data to %s failed!\n", fileName);
			ret = CVR_ERR_FAILED;
		}
		else
		{
			ret = CVR_ERR_NONE;
		}
		fclose(fpout);
	}
	else
	{
		WARNING("open %s failed!\n", fileName);
		ret = CVR_ERR_FAILED;
	}

	return ret;
}

/*
**getNation(): get nation(wz) by code
**code[in]: nation code
**pcNationStr[out]: used for store nation info(wz)
**nationStrLen[in/out]: pcNationStr max length if in, pcNationStr actual length if out 
**return:
**     void
*/
static void getNation(int code, char *pcNationStr, int *nationStrLen)
{
	int pcNationStrLen;

	pcNationStrLen = *nationStrLen;
	switch(code){
		case 01:  {snprintf(pcNationStr,pcNationStrLen,"汉"); *nationStrLen = 2;}break;  
		case 02:  {snprintf(pcNationStr,pcNationStrLen,"蒙古"); *nationStrLen = 4;}break;
		case 03:  {snprintf(pcNationStr,pcNationStrLen,"回"); *nationStrLen = 2;}break;
		case 04:  {snprintf(pcNationStr,pcNationStrLen,"藏"); *nationStrLen = 2;}break;
		case 05:  {snprintf(pcNationStr,pcNationStrLen,"维吾尔"); *nationStrLen = 6;}break;
		case 06:  {snprintf(pcNationStr,pcNationStrLen,"苗"); *nationStrLen = 2;}break;
		case 07:  {snprintf(pcNationStr,pcNationStrLen,"彝"); *nationStrLen = 2;}break;
		case 8:   {snprintf(pcNationStr,pcNationStrLen,"壮"); *nationStrLen = 2;}break;
		case 9:   {snprintf(pcNationStr,pcNationStrLen,"布依"); *nationStrLen = 4;}break;
		case 10:  {snprintf(pcNationStr,pcNationStrLen,"朝鲜"); *nationStrLen = 4;}break;
		case 11:  {snprintf(pcNationStr,pcNationStrLen,"满"); *nationStrLen = 2;}break;
		case 12:  {snprintf(pcNationStr,pcNationStrLen,"侗"); *nationStrLen = 2;}break;
		case 13:  {snprintf(pcNationStr,pcNationStrLen,"瑶"); *nationStrLen = 2;}break;
		case 14:  {snprintf(pcNationStr,pcNationStrLen,"白"); *nationStrLen = 2;}break;
		case 15:  {snprintf(pcNationStr,pcNationStrLen,"土家"); *nationStrLen = 4;}break;
		case 16:  {snprintf(pcNationStr,pcNationStrLen,"哈尼"); *nationStrLen = 4;}break;
		case 17:  {snprintf(pcNationStr,pcNationStrLen,"哈萨克"); *nationStrLen = 6;}break;
		case 18:  {snprintf(pcNationStr,pcNationStrLen,"傣"); *nationStrLen = 2;}break;
		case 19:  {snprintf(pcNationStr,pcNationStrLen,"黎"); *nationStrLen = 2;}break;
		case 20:  {snprintf(pcNationStr,pcNationStrLen,"傈僳"); *nationStrLen = 4;}break;
		case 21:  {snprintf(pcNationStr,pcNationStrLen,"佤"); *nationStrLen = 2;}break;
		case 22:  {snprintf(pcNationStr,pcNationStrLen,"畲"); *nationStrLen = 2;}break;
		case 23:  {snprintf(pcNationStr,pcNationStrLen,"高山"); *nationStrLen = 4;}break;
		case 24:  {snprintf(pcNationStr,pcNationStrLen,"拉祜"); *nationStrLen = 4;}break;
		case 25:  {snprintf(pcNationStr,pcNationStrLen,"水"); *nationStrLen = 2;}break;
		case 26:  {snprintf(pcNationStr,pcNationStrLen,"东乡"); *nationStrLen = 4;}break;
		case 27:  {snprintf(pcNationStr,pcNationStrLen,"纳西"); *nationStrLen = 4;}break;
		case 28:  {snprintf(pcNationStr,pcNationStrLen,"景颇"); *nationStrLen = 4;}break;
		case 29:  {snprintf(pcNationStr,pcNationStrLen,"柯尔克孜"); *nationStrLen = 8;}break;
		case 30:  {snprintf(pcNationStr,pcNationStrLen,"土"); *nationStrLen = 2;}break;
		case 31:  {snprintf(pcNationStr,pcNationStrLen,"达斡尔"); *nationStrLen = 6;}break;
		case 32:  {snprintf(pcNationStr,pcNationStrLen,"仫佬"); *nationStrLen = 4;}break;
		case 33:  {snprintf(pcNationStr,pcNationStrLen,"羌"); *nationStrLen = 2;}break;
		case 34:  {snprintf(pcNationStr,pcNationStrLen,"布朗"); *nationStrLen = 4;}break;
		case 35:  {snprintf(pcNationStr,pcNationStrLen,"撒拉"); *nationStrLen = 4;}break;
		case 36:  {snprintf(pcNationStr,pcNationStrLen,"毛南"); *nationStrLen = 4;}break;
		case 37:  {snprintf(pcNationStr,pcNationStrLen,"仡佬"); *nationStrLen = 4;}break;
		case 38:  {snprintf(pcNationStr,pcNationStrLen,"锡伯"); *nationStrLen = 4;}break;
		case 39:  {snprintf(pcNationStr,pcNationStrLen,"阿昌"); *nationStrLen = 4;}break;
		case 40:  {snprintf(pcNationStr,pcNationStrLen,"普米"); *nationStrLen = 4;}break;
		case 41:  {snprintf(pcNationStr,pcNationStrLen,"塔吉克"); *nationStrLen = 3;}break;
		case 42:  {snprintf(pcNationStr,pcNationStrLen,"怒"); *nationStrLen = 2;}break;
		case 43:  {snprintf(pcNationStr,pcNationStrLen,"乌孜别克"); *nationStrLen = 8;}break;
		case 44:  {snprintf(pcNationStr,pcNationStrLen,"俄罗斯"); *nationStrLen = 6;}break;
		case 45:  {snprintf(pcNationStr,pcNationStrLen,"鄂温克"); *nationStrLen = 6;}break;
		case 46:  {snprintf(pcNationStr,pcNationStrLen,"德昂"); *nationStrLen = 4;}break;
		case 47:  {snprintf(pcNationStr,pcNationStrLen,"保安"); *nationStrLen = 4;}break;
		case 48:  {snprintf(pcNationStr,pcNationStrLen,"裕固"); *nationStrLen = 4;}break;
		case 49:  {snprintf(pcNationStr,pcNationStrLen,"京"); *nationStrLen = 2;}break;
		case 50:  {snprintf(pcNationStr,pcNationStrLen,"塔塔尔"); *nationStrLen = 6;}break;
		case 51:  {snprintf(pcNationStr,pcNationStrLen,"独龙"); *nationStrLen = 4;}break;
		case 52:  {snprintf(pcNationStr,pcNationStrLen,"鄂伦春"); *nationStrLen = 6;}break;
		case 53:  {snprintf(pcNationStr,pcNationStrLen,"赫哲"); *nationStrLen = 4;}break;
		case 54:  {snprintf(pcNationStr,pcNationStrLen,"门巴"); *nationStrLen = 4;}break;
		case 55:  {snprintf(pcNationStr,pcNationStrLen,"珞巴"); *nationStrLen = 4;}break;
		case 56:  {snprintf(pcNationStr,pcNationStrLen,"基诺"); *nationStrLen = 4;}break;
		case 97:  {snprintf(pcNationStr,pcNationStrLen,"其他"); *nationStrLen = 4;}break;
		case 98:  {snprintf(pcNationStr,pcNationStrLen,"外国血统中国籍人士"); *nationStrLen = 18;}break;
		default : {snprintf(pcNationStr,pcNationStrLen,""); *nationStrLen = 0;}
	}     
}

/*
**fileDataExchange():segmentation IDCard info by 0x0D and 0x0A
**dataBuf[in/out]: in is before segmentation, out is after segmentation
**actualDataLen[in/out]: dataBuf max length if in, after segmentation databuf actual length
**return:
**      void
*/
static void fileDataExchange(unsigned char *dataBuf, int *actualDataLen)
{
	int i, j, tmp = 0; 
	char tmpBuf[512];
	char strTmp[2]; // nation code string or month code string or day code string
	char strNation[32];
	int strNationLen = sizeof(strNation);
	
	//1.name
	idCardInfoDetailOffset[0] = 0;
	for (i = 0; dataBuf[i] != ' '; i ++)
	{
		tmpBuf[i] = dataBuf[i];
	}
	for (j = i; dataBuf[j] == ' '; j ++)
	{
		tmpBuf[j] = dataBuf[j];
	}
	tmp = j;
	tmpBuf[tmp - 2] = 0x0D;
	tmpBuf[tmp - 1] = 0x0A;
	idCardInfoDetailOffset[1] = tmp;

	//2.sex
	if ('1' == dataBuf[j])
	{
		memcpy(tmpBuf + tmp, "男", sizeof(short));
	}
	else
	{
		memcpy(tmpBuf + tmp, "女", sizeof(short));
	}
	tmp += 2;
	tmpBuf[tmp++] = 0x0D;
	tmpBuf[tmp++] = 0x0A;
	idCardInfoDetailOffset[2] = tmp;

	//3.nation
	strTmp[0] = dataBuf[++j];
	strTmp[1] = dataBuf[++j];
	j++;
	getNation(atoi(strTmp), strNation, &strNationLen);
	memcpy(tmpBuf + tmp, strNation, strNationLen);
	tmp += strNationLen;
	tmpBuf[tmp++] = 0x0D;
	tmpBuf[tmp++] = 0x0A;
	idCardInfoDetailOffset[3] = tmp;

	//4.birth date: year, month, day
	//4.1 year
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(int));
	tmp += sizeof(int);
	j += sizeof(int);
	memcpy(tmpBuf + tmp, "年", sizeof(short));
	tmp += sizeof(short);
	//4.2 month
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	memcpy(tmpBuf + tmp, "月", sizeof(short));
	tmp += sizeof(short);
	//4.3 day
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	memcpy(tmpBuf + tmp, "日", sizeof(short));
	tmp += sizeof(short);
	tmpBuf[tmp++] = 0x0D;
	tmpBuf[tmp++] = 0x0A;
	idCardInfoDetailOffset[4] = tmp;

	//5.addr
	for (i = j; dataBuf[i] != ' '; i ++)
	{
		tmpBuf[tmp++] = dataBuf[i];
	}
	for (j = i; dataBuf[j] == ' '; j ++)
	{
		tmpBuf[tmp++] = dataBuf[j];
	}
	tmpBuf[tmp - 2] = 0x0D;
	tmpBuf[tmp - 1] = 0x0A;
	idCardInfoDetailOffset[5] = tmp;

	//6.IDCard number, 18 is IDCard number length
	memcpy(tmpBuf + tmp, dataBuf + j, 18);
	tmp += 18;
	j += 18;
	tmpBuf[tmp++] = 0x0D;
	tmpBuf[tmp++] = 0x0A;
	idCardInfoDetailOffset[6] = tmp;

	//7.department
	for (i = j; dataBuf[i] != ' '; i ++)
	{
		tmpBuf[tmp++] = dataBuf[i];
	}
	for (j = i; dataBuf[j] == ' '; j ++)
	{
		tmpBuf[tmp++] = dataBuf[j];
	}
	tmpBuf[tmp - 2] = 0x0D;
	tmpBuf[tmp - 1] = 0x0A;
	idCardInfoDetailOffset[7] = tmp;
	
	//8.valid date
	//8.1 start time
	//8.1.1 year
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(int));
	tmp += sizeof(int);
	j += sizeof(int);
	tmpBuf[tmp++] = '.';
	//8.1.2 month
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	tmpBuf[tmp++] = '.';
	//8.1.3 day
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	tmpBuf[tmp++] = '-';
	idCardInfoDetailOffset[8] = tmp;
	//8.2 end time
	//8.2.1 year
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(int));
	tmp += sizeof(int);
	j += sizeof(int);
	tmpBuf[tmp++] = '.';
	//8.2.2 month
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	tmpBuf[tmp++] = '.';
	//8.2.3 day
	memcpy(tmpBuf + tmp, dataBuf + j, sizeof(short));
	tmp += sizeof(short);
	j += sizeof(short);
	tmpBuf[tmp++] = 0x0D;
	tmpBuf[tmp++] = 0x0A;
	*actualDataLen = tmp;
	memcpy(dataBuf, tmpBuf, sizeof(tmpBuf));

	return ;
}

/*
**readFileInfo2Buf()
**
**return :
**     refer CVR_ERR_XXX
*/
static int readFileInfo2Buf(const char *fileName, char *outBuf, int size, int offset)
{
	int ret = CVR_ERR_NONE;
	FILE *fpout;
	fpout = fopen(fileName, "rb+");
	if(fpout!= NULL)
	{
		ret = fread(outBuf, size, 1, fpout + offset);
		if (0 == ret)
		{
			WARNING("read data from %s failed!\n", fileName);
			ret = CVR_ERR_FAILED;
		}
		else
		{
			ret = CVR_ERR_NONE;
		}
		fclose(fpout);
	}
	else
	{
		WARNING("open %s failed!\n", fileName);
		ret = CVR_ERR_FAILED;
	}

	return ret;
}

/*
**GetLicense(): get license
**
**
**return :
**	  refer CVR_ERR_XXX
*/
/**static int GetLicense(unsigned char *cLic, int bufSize)*/
/**{*/
	/**int ret = 0, offset = 0;*/
	/**int i;*/
	/**unsigned char tmpLic[32];*/
	/**int tmpLicense[4] = {0, 0, 0, 0};*/
	/**int *pTmp, nTmp;*/
	
	/**ComWrt(portNo, cmd_get_key, sizeof(cmd_get_key));*/
	/**usleep(10000);*/
	/**ret = ComRdAll(portNo, tmpLic, sizeof(tmpLic), 1000);*/
/**#if GET_LICEN_PRINT*/
	/**if (ret > 0)*/
	/**{*/
		/**for(i = 0; i < ret; i ++)*/
		/**{*/
			/**DPRINTF("%02X  ", tmpLic[i]);*/
		/**}*/
		/**DPRINTF("\n");*/
	/**}*/
/**#endif //GET_LICEN_PRINT*/
	/**if(27 == ret && 0x00 == tmpLic[7] && 0x00 == tmpLic[8] && 0x90 == tmpLic[9])*/
	/**{*/
		/**memcpy(&tmpLicense[0], &tmpLic[10], sizeof(short));*/
		/**memcpy(&tmpLicense[1], &tmpLic[12], sizeof(short));*/
		/**memcpy(&tmpLicense[2], &tmpLic[14], sizeof(int));*/
		/**memcpy(&tmpLicense[3], &tmpLic[18], sizeof(int));*/
		/**snprintf((char*)cLic, bufSize, "%02d%02d-%08d-%010d", */
				/**tmpLicense[0], tmpLicense[1], tmpLicense[2], tmpLicense[3]);*/
		/**manuID = tmpLicense[3];*/
		/**ret = CVR_ERR_NONE;*/
	/**}*/
	/**else*/
	/**{*/
		/**ret = CVR_ERR_FAILED;*/
	/**}*/

	/**return ret;*/
/**}*/

/*
**getFixedLic(): get fixed lic
**cLic[in/out]: store a fixed lic number
**
**return:
**   void
*/
/**static void getFixedLic(unsigned char *cLic)*/
/**{*/
	/**char *p, *q;*/
	
/**#if CVR_100D_JG*/
	/**p = strchr((char*)fixedLic, '-');*/
	/**q = strchr((char*)cLic, '-');*/
	/**p ++;*/
	/**q ++;*/
	/**memcpy(p, q, sizeof(int));*/
	/**memcpy(cLic, fixedLic, sizeof(fixedLic));*/
/**#endif*/
/**}*/

/*
**CVR_InitComm()
**port:
**      (COM1~COM16) or (1001~1016)
**deviceName: device name to open
**return:
** 	   refer CVR_ERR_XXX
*/
int CVR_InitComm (int Port, char *deviceName)
{
	int ret = CVR_ERR_NONE;
#if CVR_100D_JG
	ret = OpenCom(Port, deviceName, 115200);//115200
#else
	ret = OpenCom(Port, deviceName, 9600);//115200
#endif	//CVR_100D_JG	
	if (0 == ret)
	{
		portNo = Port;
		ret = CVR_ERR_NONE;
	}
	else
	{
		ret = CVR_ERR_FAILED;
	}

	return ret;
}

/*
**CVR_CloseComm()
**return:
** 	   refer CVR_ERR_XXX
*/
int CVR_CloseComm(void)
{
	int ret = CVR_ERR_NONE;

	//usleep(500000);
	sleep(1);
	ret = CloseCom(portNo);
	if (0 == ret)
	{
		ret = CVR_ERR_NONE;
	}
	else
	{
		ret = CVR_ERR_FAILED;
	}

    free(allReadBuf);
	return ret;
}

/*
**CVR_Authenticate(): Authenticated between IDCard reader and IDCard 
**
**return:
**      CVR_ERR_NONE: Correct placement of the card, 
**	   CVR_ERR_FAILED: Incorrect placement of the card or no placement    
*/
int CVR_Authenticate (void)
{
	int i = 0, ret = CVR_ERR_NONE;
	
	while (1)
	{
		ret = Authenticate();
#if CARD_INFO_PRINT
        printf("authenticate..\n");
        printf("authenticate return:%d\n", ret);
#endif
		if (CVR_ERR_NONE == ret)
		{
			ret = CVR_ERR_NONE;
			break;
		}
		else
		{
			i ++;
			//program exit after find card 25 times, return err
			if (i > 25)
			{
				ret = CVR_ERR_FAILED;
				break;
			}
		}
	}
#if CARD_INFO_PRINT
    DPRINTF("Authenticate finshed!\n");
#endif
	return ret;
}		
		
/*
**CVR_Read_Content():
**
**Active: the type of readed Idcard info
**return:
**      refer CVR_ERR_XXX 
*/
int CVR_Read_Content(int Active)
{
	int ret = 0;
	int pSize = 0, offset = 0;
	unsigned char cLic[32];
	unsigned char tmpWzMsg[256];
	unsigned char tmpXpMsg[1024];
	int actualTmpBufLen = sizeof(divTmpBuf);
	
	memset(allReadBuf, 0, sizeof(allReadBuf));
	ret = Card_Reader_All(allReadBuf, &pSize);
	if (CVR_ERR_FAILED == ret)
	{
		state = STATE_NOSTART;
		WARNING("card read failed!\n");
		return CVR_ERR_FAILED;
	}
	memset(tmpWzMsg, 0, sizeof(tmpWzMsg));
	memset(tmpXpMsg, 0, sizeof(tmpXpMsg));

	memcpy(tmpWzMsg, allReadBuf+ 9, 256);
	memcpy(tmpXpMsg, allReadBuf+ 9 + 256, 1024);
	
	//change UCS-2 to GBK
	ret = code_convert("UCS-2", "GBK",(char*)tmpWzMsg, 256,(char*)divTmpBuf, sizeof(divTmpBuf));
	//divide buf by 0x0D 0x0A
	fileDataExchange((unsigned char*)divTmpBuf, &actualTmpBufLen);
	switch(Active)
	{
	case IDCARD_INFO_TYPE1:
		{
			/*
			**create words data WZ.TXT, photo data XP.WLT and photo ZP.BMP(decode)
			*/
			//1.create words data WZ.TXT
			offset = 0;
			ret = create_file(tmpWzMsg, FILE_NAME_WZ, 256, offset);
			if (CVR_ERR_NONE == ret)
			{
				//2.create photo data XP.WLT
				ret = create_file(tmpXpMsg, FILE_NAME_XP, 1024, offset);
			}
			
			if (CVR_ERR_NONE == ret)
			{
				//3.create photo ZP.BMP(decode)
				memset(cLic, 0, sizeof(cLic));
				//ret = GetLicense(cLic, sizeof(cLic));
				#if CVR_100D_JG
				//getFixedLic(cLic);
				#endif
				if (CVR_ERR_NONE == ret)
				{
					ret = Decode2BmpCall(FILE_NAME_XP, FILE_NAME_ZP_D, BASE_FILE, cLic);
					if (CV_ERR_NONE == ret)
					{
						DPRINTF("Decode success!\n");
					}
					else
					{
						WARNING("Decode failed!\n");
					}
				}
			}
		}
	break;
	
	case IDCARD_INFO_TYPE2:
		{
			/*
			**create words data WZ.TXT, photo data XP.WLT
			*/
			offset = 0;
			ret = create_file(tmpWzMsg, FILE_NAME_WZ, 256, offset);
			if (CVR_ERR_NONE == ret)
			{
				ret = create_file(tmpXpMsg, FILE_NAME_XP, 1024, offset);
			}
		}
	break;
	
	case IDCARD_INFO_TYPE3:
		{
			ret = CVR_ERR_MOUDLENOAUTHORITY;
		}
	break;
	case IDCARD_INFO_TYPE4:
		{
			/*
			**create WZ.TXT(decode), photo ZP.BMP(decode)
			*/
			//1.create words data (decode)
			if (CVR_ERR_NONE == ret)
			{
				offset = 0;
				ret = create_file((unsigned char*)divTmpBuf, FILE_NAME_WZ, actualTmpBufLen, offset);
			}
			if (CVR_ERR_NONE == ret)
			{
				ret = create_file(tmpXpMsg, FILE_NAME_XP, 1024, offset);
			}
			if (CVR_ERR_NONE == ret)
			{
				//3.create photo ZP.BMP(decode)
				memset(cLic, 0, sizeof(cLic));
				//ret = GetLicense(cLic, sizeof(cLic));
				#if CVR_100D_JG
				//getFixedLic(cLic);
				#endif
				if (CVR_ERR_NONE == ret)
				{
					ret = Decode2BmpCall(FILE_NAME_XP, FILE_NAME_ZP_D, BASE_FILE, cLic);
					if (CV_ERR_NONE == ret)
					{
						DPRINTF("Decode success!\n");
					}
					else
					{
						WARNING("Decode failed!\n");
						ret = CVR_ERR_FAILED;
					}
					remove(FILE_NAME_XP);
				}
			}
		}
	break;
	
	case IDCARD_INFO_TYPE6:
		{
			/*
			**use device unique number, create WZ.TXT(decode), 
			**photo XP.BMP(decode)(used in terminating network)
			*/
#if CARD_INFO_PRINT
            DPRINTF("IDCARD_INFO_TYPE6\n");
#endif
			char tmpFileName[512];
			
			//1.create words data (decode)
			if (CVR_ERR_NONE == ret)
			{
				offset = 0;
				ret = create_file((unsigned char*)divTmpBuf, FILE_NAME_WZ, actualTmpBufLen, offset);
			}
			if (CVR_ERR_NONE == ret)
			{
				ret = create_file(tmpXpMsg, FILE_NAME_XP, 1024, offset);
			}
			if (CVR_ERR_NONE == ret)
			{
				//3.create photo ZP.BMP(decode)
				memset(cLic, 0, sizeof(cLic));
				//ret = GetLicense(cLic, sizeof(cLic));
#if CVR_100D_JG
				//getFixedLic(cLic);
#if CARD_INFO_PRINT
				DPRINTF("fixed lic:%s\n", cLic);
#endif
#endif
				if (CVR_ERR_NONE == ret)
				{
					ret = Decode2BmpCall(FILE_NAME_XP, FILE_NAME_ZP_D, BASE_FILE, cLic);
					if (CV_ERR_NONE == ret)
					{
						snprintf(tmpFileName, sizeof(tmpFileName), "%d.TXT", manuID);
						rename(FILE_NAME_WZ, tmpFileName);
						snprintf(tmpFileName, sizeof(tmpFileName), "%d.BMP", manuID);
						rename(FILE_NAME_ZP_D, tmpFileName);
						DPRINTF("Decode success!\n");
					}
					else
					{
						WARNING("Decode failed!\n");
						ret = CVR_ERR_FAILED;
					}
					remove(FILE_NAME_XP);
				}
                else
                {
#if CARD_INFO_PRINT
                    DPRINTF("GetLicense failed!\n");
#endif
                }
			}
		}
	break;
	default:
		{
			WARNING("Type:%d is input err!\n", Active);
			ret = CVR_ERR_FAILED;
		}
	break;
	}

	return ret;
}

/*
**CVR_ReadBaseMsg():can instead CVR_Read_Content(), read Idcard info to defined memory,
**CVR_ReadBaseMsg() can be called only if CVR_Authenticate() be called successful
**pucCHMsg[out]: used for store wz data
**puiCHMsgLen:  pucCHMsg's max length
**pucPHMsg[out]: used for store photo data
**puiPHMsgLen: pucPHMsg's max length
**nMode: Idcard info mode
**	
**return:
**	   refer CVR_ERR_XXX 
*/
int CVR_ReadBaseMsg (unsigned char *pucCHMsg, unsigned int *puiCHMsgLen, 
						unsigned char *pucPHMsg, unsigned int *puiPHMsgLen,
                        unsigned char *finger,int *finger_size,int nMode)
{
	int ret = 0, offset = 0;
	int pSize = 0;
	unsigned char cLic[32];
	unsigned char tmpWzMsg[256] = {0};
	int actualTmpBufLen = sizeof(divTmpBuf);

	if (pucCHMsg == NULL && pucPHMsg == NULL)
	{
		WARNING("##Buf is blank!\n");
		state = STATE_NOSTART;
		return CVR_ERR_FAILED;
	}
	if (*puiCHMsgLen < 256 && *puiPHMsgLen < 1024)
	{
		WARNING("##Buf not enough!\n");
		state = STATE_NOSTART;
		return CVR_ERR_FAILED;
	}
    ret = Card_Reader_All(allReadBuf,&nAllReadBufSize);
	if (CVR_ERR_FAILED == ret)
	{
		state = STATE_NOSTART;
		WARNING("card read failed!\n");
		return CVR_ERR_FAILED;
	}
	
	memset(tmpWzMsg, 0, sizeof(tmpWzMsg));
    int text_size = (allReadBuf[3]<<8) + allReadBuf[4];
    int photo_size = (allReadBuf[5]<<8) + allReadBuf[6];
    int finger_size_read = (allReadBuf[7]<<8) + allReadBuf[8];
    *finger_size = finger_size_read;
#if CARD_INFO_PRINT
    DPRINTF("text_size:%d,photo_size:%d,finger_size:%d\n",text_size,photo_size,finger_size_read);
#endif
	memcpy(tmpWzMsg, allReadBuf + 9, text_size);
    ret = code_convert("UCS-2", "UTF-8", (char*)tmpWzMsg, 256, divTmpBuf, 256);
	fileDataExchange((unsigned char*)divTmpBuf, &actualTmpBufLen);
    if(finger_size > 0){
        memcpy(finger, allReadBuf + 9 + text_size + photo_size,*finger_size);
    }
    state = STATE_NOSTART;
	switch(nMode)
	{
	case IDCARD_INFO_MODE1:
		{
			/*
			*word default format is UCS-2, photo is not decompress to bmp
			*/
			memcpy(pucCHMsg, allReadBuf + 9, text_size);
			memcpy(pucPHMsg, allReadBuf+ 9 + text_size, photo_size);
			*puiCHMsgLen = text_size;
			*puiPHMsgLen = photo_size;
		}
	break;
	case IDCARD_INFO_MODE2:
		{
			/*
			*word had changed to GBK format, photo is not decompress to bmp
			*/
			memcpy(pucPHMsg, allReadBuf+ 9 + text_size, photo_size);
			memcpy(pucCHMsg, divTmpBuf, 256);
			*puiCHMsgLen = actualTmpBufLen;
			*puiPHMsgLen = photo_size;
		}
	break;
	case IDCARD_INFO_MODE3:
		{
			/*
			*word default format is UCS-2, photo had decompressed to bmp
			*/
			//1.store wz info(UCS-2) to memory
			unsigned char tmpXpMsg[1024];
			
			memcpy(pucCHMsg, divTmpBuf, 256);
			memset(tmpXpMsg, 0, sizeof(tmpXpMsg));
			memcpy(tmpXpMsg, allReadBuf + 9 + text_size, photo_size);
			*puiCHMsgLen = actualTmpBufLen;
			if (CVR_ERR_NONE == ret)
			{
				//2.store photo data(decode) to memory
				memset(cLic, 0, sizeof(cLic));
				//ret = GetLicense(cLic, sizeof(cLic));
#if CVR_100D_JG
				//getFixedLic(cLic);
#if CARD_INFO_PRINT
				DPRINTF("clicense:%s\n", cLic);
#endif
#endif
				if (CVR_ERR_NONE == ret)
				{
					ret = Decode2BmpBuf(tmpXpMsg, pucPHMsg, sizeof(tmpXpMsg), 
						puiPHMsgLen, BASE_FILE, (char*)cLic);
					if (ret < 0)
					{
						ret = CVR_ERR_FAILED;
					}
					else
					{
						ret = CVR_ERR_NONE;
					}
				}
			}
		}
	break;
	case IDCARD_INFO_MODE4:
		{
			/*
			*word had changed to GBK format, photo had decompressed to bmp
			*/
			unsigned char tmpXpMsg[1024];

			//1. store wz info(decode) to memory
			memcpy(pucCHMsg, divTmpBuf, 256);
			memset(tmpXpMsg, 0, sizeof(tmpXpMsg));
			memcpy(tmpXpMsg, allReadBuf + 9 + 256, 1024);
			*puiCHMsgLen = actualTmpBufLen;
			if (CVR_ERR_NONE == ret)
			{
				//2. store photo data (decode) to memory
				memset(cLic, 0, sizeof(cLic));
				//ret = GetLicense(cLic, sizeof(cLic));
				#if CVR_100D_JG
				//getFixedLic(cLic);
				#endif
				if (CVR_ERR_NONE == ret)
				{
					ret = Decode2BmpBuf(tmpXpMsg, pucPHMsg, sizeof(tmpXpMsg),
						puiPHMsgLen, BASE_FILE, (char*)cLic);
					if (ret < 0)
					{
						ret = CVR_ERR_FAILED;
					}
					else
					{
						ret = CVR_ERR_NONE;
					}
				}
			}
		}
	break;
    case IDCARD_INFO_MODE5:
    {
	    DPRINTF("%s\n", divTmpBuf);

        unsigned char tmpXpMsg[1024];

		//1. store wz info(decode) to memory
		memcpy(pucCHMsg, divTmpBuf, 256);
		memset(tmpXpMsg, 0, sizeof(tmpXpMsg));
		memcpy(tmpXpMsg, allReadBuf + 9 + 256, 1024);
		*puiCHMsgLen = actualTmpBufLen;
		if (CVR_ERR_NONE == ret)
		{
			//2. store photo data (decode) to memory
			memset(cLic, 0, sizeof(cLic));
			//ret = GetLicense(cLic, sizeof(cLic));
			#if CVR_100D_JG
			//getFixedLic(cLic);
			#endif
			if (CVR_ERR_NONE == ret)
			{
				ret = Decode2BmpBuf(tmpXpMsg, pucPHMsg, sizeof(tmpXpMsg),
						puiPHMsgLen, BASE_FILE, (char*)cLic);
				if (ret < 0)
				{
                    WARNING("Decode2BmpCall failed!\n");
					ret = CVR_ERR_FAILED;
				}
				else
				{
					ret = CVR_ERR_NONE;
				}
			}
		}
    }
    break;
	default:
		{
			WARNING("mode:%d is input err!\n", nMode);
			ret = CVR_ERR_FAILED;
		}
	break;
	}
	return ret;
}

/*
**GetPeopleName(): get person name info
**strTmp:point to name info
**strLen:name info len
**return:
**     refer CVR_ERR_XXX
*/
int GetPeopleName(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[0], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}

/*
**GetPeopleSex(): get person sex info
**strTmp:point to sex info
**strLen:sex info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleSex(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[1], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}

/*
**GetPeopleNation(): get person Nation info
**strTmp:point to Nation info
**strLen:Nation info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleNation(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[2], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}

/*
**GetPeopleNation(): get person Birthday info
**strTmp:point to Birthday info
**strLen:Birthday info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleBirthday(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	
	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		memcpy(strTmp, divTmpBuf + idCardInfoDetailOffset[3], sizeof(int));
		strTmp[4] = '-';
		memcpy(strTmp + 5, divTmpBuf + idCardInfoDetailOffset[3] + 6, sizeof(short));
		strTmp[7] = '-';
		memcpy(strTmp + 8, divTmpBuf + idCardInfoDetailOffset[3] + 10, sizeof(short));
		strTmp[10] = '\0';
	}

	return ret;
}

/*
**GetPeopleNation(): get person Address info
**strTmp:point to Address info
**strLen:Address info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleAddress(char *strTmp, int *strLen)	
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[4], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}
/*
**GetPeopleNation(): get person IDCode info
**strTmp:point to IDCode info
**strLen:IDCode info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleIDCode(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[5], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;

}

/*
**GetPeopleNation(): get person Department info
**strTmp:point to Department info
**strLen:Department info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetDepartment(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
		int i, j;
	
		if (NULL == strTmp )
		{
			ret = CVR_ERR_FAILED;
		}
		else
		{
			for(i = idCardInfoDetailOffset[6], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
			{
				strTmp[j] = divTmpBuf[i];
			}
			strTmp[j] = '\0';
		}
	
		return ret;

}

/*
**GetPeopleNation(): get person StartDate info
**strTmp:point to StartDate info
**strLen:StartDate info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetStartDate(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[7], j = 0; divTmpBuf[i] != '-'; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}

/*
**GetPeopleNation(): get person EndDate info
**strTmp:point to EndDate info
**strLen:EndDate info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetEndDate(char *strTmp, int *strLen)
{
	int ret = CVR_ERR_NONE;
	int i, j;

	if (NULL == strTmp )
	{
		ret = CVR_ERR_FAILED;
	}
	else
	{
		for(i = idCardInfoDetailOffset[8], j = 0; divTmpBuf[i] != 0x0D; i ++, j ++)
		{
			strTmp[j] = divTmpBuf[i];
		}
		strTmp[j] = '\0';
	}

	return ret;
}


int AutoCheckDevice(char* serial_port)
{
    int ret = CVR_ERR_NONE;
    char* original = serial_port;
    while(*serial_port){
        if(*serial_port >= '0' && *serial_port <= '9'){
            break;
        }
        serial_port++;
    }
    int port = atoi(serial_port);
    unsigned char buf_read[32] = {0};
#if CVR_100D_JG
	ret = OpenCom(port, original, 115200);//115200
#else
	ret = OpenCom(port, original, 9600);//115200
#endif	//CVR_100D_JG	
	if (0 == ret){
		ret = CVR_ERR_NONE;
#if CARD_INFO_PRINT
        printf("OpenCom success!\n");
#endif
	}else{
		ret = CVR_ERR_FAILED;
#if CARD_INFO_PRINT
        printf("OpenCom failed!\n");
#endif
	}
    if(ret == CVR_ERR_NONE){
        int wrt_ret = ComWrt(port, cmd_readsamv, sizeof(cmd_readsamv));
        int read_ret = ComRdAll(port, buf_read, 7, 1000);
#if CARD_INFO_PRINT
            int i;
            for(i = 0; i < read_ret; i ++)
                DPRINTF("%02X ", buf_read[i]);
            DPRINTF("\n");
#endif 
        if(read_ret == 7){
            int left = (buf_read[5] << 8) + buf_read[6];
            read_ret = ComRdAll(port,buf_read,left,1000);
#if CARD_INFO_PRINT
            int i;
            for(i = 0; i < read_ret; i ++)
                DPRINTF("%02X ", buf_read[i]);
            DPRINTF("\n");
#endif 

            if(buf_read[0] == 0x00 && buf_read[1] == 0x00 && buf_read[2] == 0x90)
            {
#if CARD_INFO_PRINT
                DPRINTF("cmd_readsamv success!\n");
                DPRINTF("cmd_readsamv state:%d\n",state);
#endif
                ret = CVR_ERR_NONE;
            }
            else
            {
                ret = CVR_ERR_FAILED;
            }
        }else{
            ret = CVR_ERR_FAILED;
        }
    }

    CVR_CloseComm();
    return ret;
}

