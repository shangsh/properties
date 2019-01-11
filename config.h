// �������ļ�֧��64+��������
#ifndef __CONFIG_H__ 
#define __CONFIG_H__

enum CfgErrorCode
{
	CEC_SUCCES = 0,				// �ɹ�
	CEC_FAIL,					// ʧ��
	CEC_INVAILD_PARAMS,			// ��Ч�Ĳ���
	CEC_OPEN_FAIL,				// ���ļ�ʧ��
	CEC_READ_FAIL,				// ��ȡ�ļ�ʧ��
	CEC_WRITE_FAIL,				// д���ļ�ʧ��
	CEC_MAX
};

//ȥ���ַ�����β�ո�
void trim(char *strIn);

//д�������ļ�
int WriteConfig(const char *filename/*in*/, const char *key/*in*/, const char *value/*in*/);

//��ȡ�����ļ�
int ReadConfig(const char *filename/*in*/, const char *Key/*in*/, char *value/*out*/);

//д�������ļ�Number
int WriteConfigNumber(const char *filename/*in*/, const char *key/*in*/, int value/*in*/);

//��ȡ�����ļ�Number
int ReadConfigNumber(const char *filename/*in*/, const char *Key/*in*/, int *value/*out*/);

#endif //__CONFIG_H__
