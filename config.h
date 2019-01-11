// 本配置文件支持64+个配置项
#ifndef __CONFIG_H__ 
#define __CONFIG_H__

enum CfgErrorCode
{
	CEC_SUCCES = 0,				// 成功
	CEC_FAIL,					// 失败
	CEC_INVAILD_PARAMS,			// 无效的参数
	CEC_OPEN_FAIL,				// 打开文件失败
	CEC_READ_FAIL,				// 读取文件失败
	CEC_WRITE_FAIL,				// 写入文件失败
	CEC_MAX
};

//去除字符串首尾空格
void trim(char *strIn);

//写入配置文件
int WriteConfig(const char *filename/*in*/, const char *key/*in*/, const char *value/*in*/);

//读取配置文件
int ReadConfig(const char *filename/*in*/, const char *Key/*in*/, char *value/*out*/);

//写入配置文件Number
int WriteConfigNumber(const char *filename/*in*/, const char *key/*in*/, int value/*in*/);

//读取配置文件Number
int ReadConfigNumber(const char *filename/*in*/, const char *Key/*in*/, int *value/*out*/);

#endif //__CONFIG_H__
