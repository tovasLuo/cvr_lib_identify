#ifndef CV_WLTLIB
#define CV_WLTLIB

#define CV_ERR_NONE			 1
#define CV_ERR_LICENSE		 2
#define CV_ERR_HDADER		 3
#define CV_ERR_DATA			 4
#define CV_ERR_UNINITIALIZED 5

#define CV_FORMAT_BMP		 0
#define CV_FORMAT_RAW		 1

#ifdef __cplusplus
extern "C" {
#endif
	//************************************
	// Method:    CV_Initialize
	// FullName:  CV_Initialize
	// Access:    public 
	// Returns:   int					见CV_ERR_XXX定义
	// Parameter: const char * pLicense	安全模块编号
	// Parameter: const int * pBaseData	小波变换参数
	// Desc:      初始化函数，启动时调用一次
	//************************************
	int CV_Initialize(const char * pLicense, const int * pBaseData);

	//************************************
	// Method:    CV_Finalize
	// FullName:  CV_Finalize
	// Access:    public 
	// Returns:   int					见CV_ERR_XXX定义
	// Desc:      终止函数，退出时调用一次
	//************************************
	int CV_Finalize();

	//************************************
	// Method:    CV_Decode
	// FullName:  CV_Decode
	// Access:    public 
	// Returns:   int					见CV_ERR_XXX定义
	// Parameter: unsigned char * pData 待解码数据
	// Parameter: int length			待解码数据长度
	// Parameter: unsigned char * pOutBuffer	解码输出缓冲区，需由外部程序分配足够空间
	// Parameter: int format			解码输出格式，见CV_FORMAT_XXX定义	
	// Parameter: int outLength			实际输出数据长度
	// Desc:      解码函数，不支持多线程调用
	//************************************

	int CV_Decode(unsigned char *pData, 
		int length, 
		unsigned char * pOutBuffer, 
		int format, 
		int *outLength);

	//************************************
	// Method:    CV_ErrorInfo
	// FullName:  CV_ErrorInfo
	// Access:    public 
	// Returns:   void
	// Parameter: int errCode			错误码
	// Parameter: char * errMSg			文字说明，内存由外部分配，最少128字节
	// Desc:      错误码详细信息说明
	//************************************
	void CV_ErrorInfo(int errCode, char * errMSg);

	//************************************
	// Method:    CV_VersionInfo
	// FullName:  CV_VersionInfo
	// Access:    public 
	// Returns:   void
	// Parameter: char * version		版本说明，内存由外部分配，最少128字节
	// Desc:      版本信息
	//************************************
	void CV_VersionInfo(char * version);


#ifdef __cplusplus
}
#endif

#endif	// CV_WLTLIB
