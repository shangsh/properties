#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common_exp.h"
#include "cfile.h"
#include "func.h"

#define DEF_CFGLEN 4096	// �����ļ�����󳤶�
#define DEF_LINELEN 64	// �����ļ�������г���
#define CFG_INVALID_CHAR 0x00

int ReadLine(EV_cfs_fHandle pf, char* szLine, int iLineLen);
void parseLine(const char* cszLine, char* szKey, char* szValue);

void trim(char *strIn)
{
	// ����ȥ���ո���ַ�����ͷβָ��ͱ���ָ��
	char *start, *end, *temp, *tail;

	// ���˿��ַ���
	if(strIn == NULL || *strIn == 0x0)
		return;

	temp = strIn;
	// ɾ����ͷ�Ŀո�
	while (*temp == ' ')
		++temp;
	
	// ���ͷָ��
	start = temp;

	// ͷָ�����ڵ��ַ�Ϊ0x00���������ַ���ȫΪ�ո�ֱ�ӱ�Ϊ���ַ���
	if (*start == 0x00)
	{
		*strIn = 0x00;
		return;
	}

	// ɾ��β���Ŀո�
	temp = strIn + strlen(strIn) - 1;
	tail = temp;
	while (*temp == ' ')
		--temp;
	
	// ���βָ��
	end = temp;

	// û�пո���ֱ�ӷ���
	if (strIn == start && tail == end)
	{
		return;
	}
	else if (strIn == start) // ������ͷ��ɾ��β���ո�
	{
		*(++end) = 0x00;
	} 
	else // ��ǰ�ƶ��ڴ�
	{
		// ������Ҫ�ƶ����ַ���
		int iMove = end - start + 1;
		// �����ǰ�ƶ��ַ�
		for (int i = 0; i < iMove; ++i)
		{
			strIn[i] = start[i];
		}
		// �ַ�����β
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
			// �治����
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
	
	// ɾ����ͷ�Ŀո�
	char c = *istr;
	while (c != eof && c == ' ')
		c = *(++istr);

	if (c != eof)
	{
		// ����ע��
		if (c == '#' || c == '!')
		{
			//while (c != eof && c != '\n' && c != '\r')
			//	c = *(++istr);
			return;
		}
		else
		{
			// ��ȡKEY
			int index = 0;
			while (c != eof && c != '=' && c != ':' && c != '\r' && c != '\n')
			{
				szKey[index++] = c;
				c = *(++istr);
			}

			// ��ȡValue
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

			// �޳��ո�
			trim(szKey);
			trim(szValue);
		}
	}
}

int WriteConfig(const char* filename, const char* Key, const char* Value) 
{
	EV_cfs_fHandle pf = NULL;
	char ftemp[DEF_CFGLEN] = { 0 };
	char fline[DEF_LINELEN] = { 0 };    //�ļ���������
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

	// ���¿���һ�ݣ���ֹ�пո�
	char key[DEF_LINELEN] = { 0 };
	char value[DEF_LINELEN] = { 0 };
	strcpy(key, Key);
	strcpy(value, Value);
	trim(key);
	trim(value);

	pf = EA_iCfsOpen(filename, "r+");
	if (pf != NULL) // �ļ����ڣ���һ��һ�еĶ����������key���޸�value�浽����������
	{
		//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | EA_iCfsOpen(%s) ok", filename);
		int iRead = 0;
		// һ��һ�еĶ����������key���޸�value�浽����������
		do
		{
			// ��ȡһ��
			memset(fline, 0x00, sizeof(fline));
			iRead = ReadLine(pf, fline, DEF_LINELEN);
			//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | WriteConfig() | Line[%s] ", fline);
			if (iRead > 0)
			{
				// ��������ȡ KEY �� Value
				memset(fkey, 0x00, sizeof(fkey));
				memset(fvalue, 0x00, sizeof(fvalue));
				parseLine(fline, fkey, fvalue);

				// ����Ѿ������ˣ���ֱ������
				if (strcmp(fkey, key) == 0 && exist == 1)
					fline[0] = 0x00;
				//�ж�key�Ƿ����
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

		// ������������key����valueд�뵽���һ��
		if (exist != 1)
		{
			sprintf(fline, "%s=%s\n", key, value);
			strcat(ftemp, fline);
		}
	}
	else // �ļ������ڣ����ʽ������
	{
		sprintf(fline, "%s=%s\n", key, value);
		strcat(ftemp, fline);
	}

	// �ر��ļ�
	if (pf != NULL)
	{
		EA_iCfsClose(pf);
		pf = NULL;
	}

	// �Կ�д��ʽ��
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

	// ���¿���һ�ݣ���ֹ�пո�
	char key[DEF_LINELEN] = { 0 };
	strcpy(key, Key);
	trim(key);

	EV_cfs_fHandle pf = EA_iCfsOpen(filename, "r+");
	if (pf != NULL) 
	{
		int iRead = 0;
		// һ��һ�еĶ����������key���޸�value�浽����������
		do
		{
			memset(fline, 0x00, sizeof(fline));
			iRead = ReadLine(pf, fline, DEF_LINELEN);
			//ProSendLog("",__FILE__, __LINE__,"", 0 ,"CFG | ReadConfig() | Line[%s] ", fline);
			if (iRead > 0)
			{
				// ��������ȡ KEY �� Value
				memset(fkey, 0x00, sizeof(fkey));
				memset(fvalue, 0x00, sizeof(fvalue));
				parseLine(fline, fkey, fvalue);

				// �ж�key�Ƿ����
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
	else // �ļ������ڣ����ʽ������
	{
		return CEC_OPEN_FAIL;
	}

	// �ر��ļ�
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

//��ȡ�����ļ�Number
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
