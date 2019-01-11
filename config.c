#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common_exp.h"
#include "cfile.h"
#include "func.h"

#define DEF_CFGLEN 4096	// 配置文件的最大长度
#define DEF_LINELEN 64	// 配置文件的最大行长度
#define CFG_INVALID_CHAR 0x00

int ReadLine(EV_cfs_fHandle pf, char* szLine, int iLineLen);
void parseLine(const char* cszLine, char* szKey, char* szValue);

void trim(char *strIn)
{
	// 定义去除空格后字符串的头尾指针和遍历指针
	char *start, *end, *temp, *tail;

	// 过滤空字符串
	if(strIn == NULL || *strIn == 0x0)
		return;

	temp = strIn;
	// 删除开头的空格
	while (*temp == ' ')
		++temp;
	
	// 求得头指针
	start = temp;

	// 头指针所在的字符为0x00，则整个字符串全为空格，直接变为空字符串
	if (*start == 0x00)
	{
		*strIn = 0x00;
		return;
	}

	// 删除尾部的空格
	temp = strIn + strlen(strIn) - 1;
	tail = temp;
	while (*temp == ' ')
		--temp;
	
	// 求得尾指针
	end = temp;

	// 没有空格，则直接返回
	if (strIn == start && tail == end)
	{
		return;
	}
	else if (strIn == start) // 保留开头，删除尾部空格
	{
		*(++end) = 0x00;
	} 
	else // 向前移动内存
	{
		// 计算需要移动的字符数
		int iMove = end - start + 1;
		// 逐个向前移动字符
		for (int i = 0; i < iMove; ++i)
		{
			strIn[i] = start[i];
		}
		// 字符串结尾
		strIn[iMove] = 0x00;
	}
}

int ReadLine(EV_cfs_fHandle pf, char* szLine, int iLineLen)
{
	char c[2] = { 0 };
	int n = 0;
	int l = 0;

	do
	{
		memset(c, 0x00, sizeof(c));
		n = EA_i64CfsRead(pf, c, 1);
		if (n < 0)
		{
			l = n;
			break;
		}
		else if (n == 0)
		{
			l = 0;
			break;
		} 
		else
		{
			l += n;
			// 存不下了
			if (l > iLineLen - 1)
			{
				n = -1;
				break;
			}
			else
			{
				strcat(szLine, c);
			}

			if (c[0] == '\n')
			{
				break;
			}
		}
	} while (1);

	return l;
}

void parseLine(const char* cszLine, char* szKey, char* szValue)
{
	const char eof = CFG_INVALID_CHAR;
	const char* istr = cszLine;
	
	// 删除开头的空格
	char c = *istr;
	while (c != eof && c == ' ')
		c = *(++istr);

	if (c != eof)
	{
		// 过滤注释
		if (c == '#' || c == '!')
		{
			//while (c != eof && c != '\n' && c != '\r')
			//	c = *(++istr);
			return;
		}
		else
		{
			// 获取KEY
			int index = 0;
			while (c != eof && c != '=' && c != ':' && c != '\r' && c != '\n')
			{
				szKey[index++] = c;
				c = *(++istr);
			}

			// 获取Value
			index = 0;
			if (c == '=' || c == ':')
			{
				c = *(++istr);
				while (c != eof && c != '\r' && c != '\n')
				{
					//*szValue++ = c;
					szValue[index++] = c;
					c = *(++istr);;
				}
			}

			// 剔除空格
			trim(szKey);
			trim(szValue);
		}
	}
}

int WriteConfig(const char* filename, const char* Key, const char* Value) 
{
	EV_cfs_fHandle pf = NULL;
	char ftemp[DEF_CFGLEN] = { 0 };
	char fline[DEF_LINELEN] = { 0 };    //文件缓存数组
	char fkey[DEF_LINELEN] = { 0 };
	char fvalue[DEF_LINELEN] = { 0 };
	int exist = 0;

	//EA_ucResetTimer(hdClockTest);

	if (filename == NULL || *filename == 0x00)
	{
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Configuration name is nullstr.");
		return CEC_INVAILD_PARAMS;
	}

	if (Key == NULL || *Key == 0x00)
	{
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Configuration key is nullstr.");
		return CEC_INVAILD_PARAMS;
	}

	// 重新拷贝一份，防止有空格
	char key[DEF_LINELEN] = { 0 };
	char value[DEF_LINELEN] = { 0 };
	strcpy(key, Key);
	strcpy(value, Value);
	trim(key);
	trim(value);

	pf = EA_iCfsOpen(filename, "r+");
	if (pf != NULL) // 文件存在，则一行一行的读，如果存在key则修改value存到缓存数组中
	{
		//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | EA_iCfsOpen(%s) ok", filename);
		int iRead = 0;
		// 一行一行的读，如果存在key则修改value存到缓存数组中
		do
		{
			// 读取一行
			memset(fline, 0x00, sizeof(fline));
			iRead = ReadLine(pf, fline, DEF_LINELEN);
			//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Line[%s] ", fline);
			if (iRead > 0)
			{
				// 从行中提取 KEY 和 Value
				memset(fkey, 0x00, sizeof(fkey));
				memset(fvalue, 0x00, sizeof(fvalue));
				parseLine(fline, fkey, fvalue);

				// 如果已经存在了，则直接跳过
				if (strcmp(fkey, key) == 0 && exist == 1)
					fline[0] = 0x00;
				//判断key是否存在
				if (strcmp(fkey, key) == 0 && exist == 0)
				{
					exist = 1;
					sprintf(fline, "%s=%s\n", key, value);
				}

				//printf("fline = %s\n", fline);
				ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Key[%s] : Val[%s]", key, value);
				strcat(ftemp, fline);
			}
			else
			{
				break;
			}
		} while (1);

		// 如果不存在则把key，则value写入到最后一行
		if (exist != 1)
		{
			sprintf(fline, "%s=%s\n", key, value);
			strcat(ftemp, fline);
		}
	}
	else // 文件不存在，则格式化保存
	{
		sprintf(fline, "%s=%s\n", key, value);
		strcat(ftemp, fline);
	}

	// 关闭文件
	if (pf != NULL)
	{
		EA_iCfsClose(pf);
		pf = NULL;
	}

	// 以可写方式打开
	pf = EA_iCfsOpen(filename, "w+");
	if (pf == NULL) 
	{
		//printf("CFG | WriteConfig() | Open Configuration %s w+ error=%d.", filename, errno);
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Open Configuration %s w+ error=%d.", filename, EA_iCfsGetError());
		return CEC_OPEN_FAIL;
	}
	
	int_64 iCount = strlen(ftemp);
	int_64 iWrite = EA_i64CfsWrite(pf, ftemp, iCount);
	if (iWrite != iCount)
	{
		EA_iCfsClose(pf);
		//printf("CFG | WriteConfig() | Write Configuration %s error=%d.", filename, errno);
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Write Configuration %s w+ error=%d.", filename, EA_iCfsGetError());
		return CEC_OPEN_FAIL;
	}

	if (pf != NULL) 
	{
		EA_iCfsClose(pf);
		pf = NULL;
	}

	//uint uPassedTime = 0;
	//EA_ucGetPassTimer(hdClockTest, &gbulPassedTimeRecv);
	//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Write Configuration Success. PassTime:%ums", uPassedTime);
	return CEC_SUCCES;
}

int ReadConfig(const char *filename, const char *Key, char *value)
{
	char fline[DEF_LINELEN] = { 0 };
	char fkey[DEF_LINELEN] = { 0 };
	char fvalue[DEF_LINELEN] = { 0 };
	int exist = 0;

	if (filename == NULL || *filename == 0x00)
	{
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | ReadConfig() | Configuration name is nullstr.");
		return CEC_INVAILD_PARAMS;
	}

	if (Key == NULL || *Key == 0x00)
	{
		ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | ReadConfig() | Configuration key is nullstr.");
		return CEC_INVAILD_PARAMS;
	}

	// 重新拷贝一份，防止有空格
	char key[DEF_LINELEN] = { 0 };
	strcpy(key, Key);
	trim(key);

	EV_cfs_fHandle pf = EA_iCfsOpen(filename, "r+");
	if (pf != NULL) 
	{
		int iRead = 0;
		// 一行一行的读，如果存在key则修改value存到缓存数组中
		do
		{
			memset(fline, 0x00, sizeof(fline));
			iRead = ReadLine(pf, fline, DEF_LINELEN);
			//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | ReadConfig() | Line[%s] ", fline);
			if (iRead > 0)
			{
				// 从行中提取 KEY 和 Value
				memset(fkey, 0x00, sizeof(fkey));
				memset(fvalue, 0x00, sizeof(fvalue));
				parseLine(fline, fkey, fvalue);

				// 判断key是否存在
				if (strcmp(fkey, key) == 0)
				{
					exist = 1;
					strcpy(value, fvalue);
					ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | ReadConfig() | Key[%s]=Value[%s] ", key, value);
					break;
				}
			}
			else
			{
				break;
			}
		} while (1);
	}
	else // 文件不存在，则格式化保存
	{
		return CEC_OPEN_FAIL;
	}

	// 关闭文件
	if (pf != NULL)
	{
		EA_iCfsClose(pf);
		pf = NULL;
	}

	if (exist == 1)
		return CEC_SUCCES;

	return CEC_FAIL;
}

int WriteConfigNumber(const char *filename, const char *key, int value)
{
	char szValue[DEF_LINELEN] = { 0 };
	sprintf(szValue, "%d", value);
	return WriteConfig(filename, key, szValue);
}

//读取配置文件Number
int ReadConfigNumber(const char *filename, const char *key, int *value)
{
	if (value == NULL)
		return CEC_INVAILD_PARAMS;

	char szValue[DEF_LINELEN] = { 0 };
	if (ReadConfig(filename, key, szValue) == CEC_SUCCES)
	{
		*value = atoi(szValue);
		return CEC_SUCCES;
	}

	return CEC_FAIL;
}
