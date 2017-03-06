/*
* Copyright (c) 2009,北京海鑫智圣技术有限公司
* All rights reserved.
**THID_CVR_IDCard.h
**current version: 1.0
**created time:2011-07-14
**Author:Ly
*/

#ifndef __THID_CVR_IDCARD_H__
#define __THID_CVR_IDCARD_H__

/*
**error code definition
*/
#define CVR_ERR_PROCESS      4  //in the middle of authenticate  
#define CVR_ERR_PICDECODE    2   //photo decode err
#define CVR_ERR_NONE		 1   //success
#define CVR_ERR_FAILED		 0   //failed
#define CVR_ERR_MOUDLENOAUTHORITY  -1  //this module is not authority.
#define CVR_ERR_PERMISSION   5   //no permission

/*
**CVR_InitComm(): create the link between PC and IDCard reader. 
**
**port:
**      (COM1~COM16) or (1001~1016)
**deviceName: device name to open
**return:
**      refer CVR_ERR_XXX   
*/
int CVR_InitComm (int Port, char *deviceName);

/*
**CVR_CloseComm(): close the link between PC and IDCard reader. 
**
**return:
**      refer CVR_ERR_XXX   
*/
int CVR_CloseComm(void);

/*
**CVR_Authenticate(): Authenticated between IDCard reader and IDCard 
**
**return:
**      refer CVR_ERR_XXX     
*/
int CVR_Authenticate (void);

/*
**CVR_Read_Content():
**
**Active: the type of readed Idcard info
** 1:create words data WZ.TXT, photo data XP.WLT and photo ZP.BMP(decode)
** 2:create words data WZ.TXT, photo data XP.WLT
** 4:create WZ.TXT(decode), photo ZP.BMP(decode)
** 6:create .TXT(decode), photo .BMP(decode)name by device unique number.
**      used in terminating network.
**return:
**      refer CVR_ERR_XXX 
*/
int CVR_Read_Content(int Active);

/*
**CVR_ReadBaseMsg():can instead CVR_Read_Content(), read Idcard info to defined memory,
**CVR_ReadBaseMsg() can be called only if CVR_Authenticate() be called successful
**pucCHMsg[out]:
**puiCHMsgLen:
**pucPHMsg[out]:
**puiPHMsgLen:
**finger[out]finger length1024
**finger_size[out]
**nMode:
**	1:word default format is UCS-2, photo is not decompress to bmp
**	2:word had changed to GBK format, photo is not decompress to bmp
**	3:word default format is UCS-2, photo had decompressed to bmp
**	4:word had changed to GBK format, photo had decompressed to bmp
**return:
**	   refer CVR_ERR_XXX 
*/
int CVR_ReadBaseMsg (unsigned char *pucCHMsg, unsigned int *puiCHMsgLen, 
						unsigned char *pucPHMsg, unsigned int *puiPHMsgLen,
                        unsigned char *finger,int *finger_size,int nMode);

/*
**GetPeopleName(): get person name info
**strTmp:point to name info
**strLen:name info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleName(char *strTmp, int *strLen);

/*
**GetPeopleSex(): get person sex info
**strTmp:point to sex info
**strLen:sex info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleSex(char *strTmp, int *strLen);	

/*
**GetPeopleNation(): get person Nation info
**strTmp:point to Nation info
**strLen:Nation info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleNation(char *strTmp, int *strLen);

/*
**GetPeopleNation(): get person Birthday info
**strTmp:point to Birthday info
**strLen:Birthday info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleBirthday(char *strTmp, int *strLen);

/*
**GetPeopleNation(): get person Address info
**strTmp:point to Address info
**strLen:Address info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleAddress(char *strTmp, int *strLen);	

/*
**GetPeopleNation(): get person IDCode info
**strTmp:point to IDCode info
**strLen:IDCode info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetPeopleIDCode(char *strTmp, int *strLen);

/*
**GetPeopleNation(): get person Department info
**strTmp:point to Department info
**strLen:Department info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetDepartment(char *strTmp, int *strLen);	

/*
**GetPeopleNation(): get person StartDate info
**strTmp:point to StartDate info
**strLen:StartDate info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetStartDate(char *strTmp, int *strLen);

/*
**GetPeopleNation(): get person EndDate info
**strTmp:point to EndDate info
**strLen:EndDate info len
**return:
**     refer CVR_ERR_XXX 
*/
int GetEndDate(char *strTmp, int *strLen);	        


/*
 *Automatically detects whether the serial port is connected to the device
 *return CVR_ERR_NONE all fine
 *return CVR_ERR_PERMISSION no permission
 *other failed
*/

int AutoCheckDevice(char* serial_port);
#endif  //__THID_CVR_IDCARD_H__
