/*�������Ψһ��ϰ���� _H__ ��Ϊ��׺,��Ŀ�ж���ļ���Ŀ¼�ṹ�������� ��Ŀ����_Ŀ¼_�ļ�����_H__ ����������ʽ��*/
#ifndef LEPTJSON_H_	
#define LEPTJSON_H_
typedef enum { LEPT_NULL,LEPT_FALSE,LEPT_TRUE,LEPT_NUMBER,LEPT_STRING,LEPT_ARRAY,LEPT_OBJECT}lept_type; //json�������� 
typedef struct{			//json���ݽṹ 
	lept_type type;
	double n; 
}lept_value;
enum{					//����ֵ 
	LEPT_PARSE_OK=0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR
};
int lept_parse(lept_value *v,const char*json); 	//�������� 
lept_type lept_get_type(const lept_value *v);	//��ȡ��������� 

#endif /*LEPTJSON_H_*/
