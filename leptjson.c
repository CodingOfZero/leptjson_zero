#include"leptjson.h"
#include<assert.h>	//���� 
/*��õ����ں�����ʼ�ĵط���������в�����
��ʱ��Ҳ���ڵ��ú����󣬼���������Ƿ���ȷ
���������ɳ���Ա������루���紫�벻�Ϸ���������ʱ�ö���
�������ǳ���Ա�޷�����ģ�����������ʱ������ɣ������ļ�ʧ�ܣ���Ҫ�׳��쳣*/
#include<stdlib.h> 	//"NULL ,strtod() malloc() realloc() free()" 
#include<errno.h> /*errno, ERANGE*/
#include<math.h> /*HUGE_VAL*/
#include<string.h>/*memcpy()*/ 

#define EXPECT(c,ch)	do{assert(*c->json==(ch)); c->json++; }while(0)
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')
#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')

typedef struct{
	const char* json;
	char* stack;//��ջ��ַ 
	size_t size,top;//size�ǵ�ǰ�Ķ�ջ������top��ջ����λ�ã����ڻ���չstack����top������ָ����ʽ�洢�� 
}lept_context;
/* ws = *(%x20 / %x09 / %x0A / %x0D) �ո�����Ʊ�������з����س���*/  
static void lept_parse_whitespace(lept_context *c){
	const char *p=c->json;
	while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')
		p++;
	c->json=p;	
}
#if 0
static int lept_parse_null(lept_context *c,lept_value *v){	//����NULL 
	EXPECT(c,'n');
	if(c->json[0] != 'u'|| c->json[1] != 'l' || c->json[2] != 'l')
		return LEPT_PARSE_INVALID_VALUE;
	c->json+=3;
	v->type=LEPT_NULL;
	return LEPT_PARSE_OK;
}
static int lept_parse_false(lept_context *c,lept_value *v){
	EXPECT(c,'f');
	if(c->json[0] != 'a'|| c->json[1] != 'l' || c->json[2] != 's'||c->json[3] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json+=4;
	v->type=LEPT_FALSE;
	return LEPT_PARSE_OK;
}
static int lept_parse_true(lept_context *c,lept_value *v){
	EXPECT(c,'t');
	if(c->json[0] != 'r'|| c->json[1] != 'u' || c->json[2] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json+=3;
	v->type=LEPT_TRUE;
	return LEPT_PARSE_OK;
}
#endif
//�ع��ϲ� lept_parse_null������lept_parse_false������lept_parse_true����Ϊ lept_parse_literal���� 
static int lept_parse_literal(lept_context *c,lept_value *v,const char* literal,lept_type type){
	size_t i;
	EXPECT(c,literal[0]);
	for(i=0;literal[i+1];i++){
		if(c->json[i]!=literal[i+1])
			return LEPT_PARSE_INVALID_VALUE;
	}
	c->json+=i;
    v->type = type;
    return LEPT_PARSE_OK;	
}
/*******************************��������*********************************/ 
static int lept_parse_number(lept_context *c,lept_value *v){
	const char *p=c->json;
	if(*p=='-') p++;/* ���� ... */
    if(*p=='0'){
    	p++;
    	if(ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
	}
	else{
		if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* ���� ... */
	
    if(*p=='.'){
		p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* С��,���������Ӧ��һ�� digit������ digit �ͷ��ش��� */
    if(*p=='e'||*p=='E'){
		p++;
		if(*p=='+'||*p=='-') p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* ָ�� ... */
	
	errno=0;
	v->u.n=strtod(c->json,NULL);
	if((errno==ERANGE)&&(v->u.n==HUGE_VAL || v->u.n==-HUGE_VAL)) //v->==0 ����� Assertion failed!File: leptjson.c, Line 104
		return LEPT_PARSE_NUMBER_TOO_BIG;
	c->json=p;
	v->type=LEPT_NUMBER;
	return LEPT_PARSE_OK;
}
/*******************************�����ַ���*********************************/ 
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
//��ջ�����ֽڴ��棬ÿ�ο�Ҫ��ѹ�������С�����ݣ����᷵��������ʼ��ָ��
static void* lept_context_push(lept_context* c,size_t size){
	void *ret;
	assert(size>0);
	if(c->top+size>=c->size){
		if(c->size==0)
			c->size=LEPT_PARSE_STACK_INIT_SIZE;
		while(c->top+size>=c->size)
			c->size+=c->size>>1;	/*c->size * 1.5  */
		c->stack=(char*)realloc(c->stack,c->size);//c->stack�ڳ�ʼ��ʱΪ NULL��realloc(NULL, size)����Ϊ�ǵȼ���malloc(size)
	}
	ret=c->stack+c->top;
	c->top+=size;
	return ret;
}
static void* lept_context_pop(lept_context* c,size_t size){
	assert(c->top >= size);
	return c->stack+(c->top-=size);
}
#define PUTC(c,ch) do{ *(char*)lept_context_push(c,sizeof(char))=(ch);}while(0)
static int lept_parse_string(lept_context *c,lept_value *v){
	size_t head=c->top,len;
	const char* p;
	EXPECT(c,'\"');//string��˫���ſ�ͷ��˫���Ž�β���ʼ�⿪ʼ�Ƿ�Ϊ " 
	p=c->json;
	for(;;){
		char ch=*p++;
		switch(ch){
			case'\"':
				len=c->top-head; 
				lept_set_string(v,(const char*)lept_context_pop(c,len),len);
				c->json=p;
				return LEPT_PARSE_OK;
			case'\0':
				c->top=head;
				return LEPT_PARSE_MISS_QUOTATION_MARK;
			default:
				PUTC(c,ch);
		}
	}
	
}
/*******************************���������ж�*********************************/ 
static int lept_parse_value(lept_context *c,lept_value *v){
	switch(*c->json){
		case'n':return lept_parse_literal(c,v,"null",LEPT_NULL);
		case'f':return lept_parse_literal(c,v,"false",LEPT_FALSE);
		case't':return lept_parse_literal(c,v,"true",LEPT_TRUE);
		case'\0':return LEPT_PARSE_EXPECT_VALUE;
		case'"':return lept_parse_string(c,v);
		default: return lept_parse_number(c,v);
	}
}
/*******************************��������*********************************/  
int lept_parse(lept_value *v,const char*json){
	lept_context c;
	int ret; 
	assert(v != NULL);
	c.json=json;
	c.stack=NULL;
	c.size=c.top=0;
	lept_init(v);
	lept_parse_whitespace(&c);
	if((ret=lept_parse_value(&c,v))==LEPT_PARSE_OK){
			lept_parse_whitespace(&c);
			if(*c.json != '\0')
				ret=LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	assert(c.top==0);//ȷ���������ݶ�������
	free(c.stack); 
	return ret;
	
}
/*************************************����ʵ��*********************************/
lept_type lept_get_type(const lept_value *v){
	assert(v!=NULL);
	return v->type;
}
int lept_get_boolean(const lept_value *v){
	assert(v!=NULL);
	int i;
	i=(v->type==LEPT_FALSE)? 0 : 1;
	return i;	
}
void lept_set_boolean(lept_value *v,int b){
	if(b==0)
		v->type=LEPT_FALSE;
	else
		v->type=LEPT_TRUE;		
}
void lept_set_number(lept_value *v,double n){
	assert(v!=NULL);//�Ƿ���Ҫ�ж�n 
	v->type=LEPT_NUMBER;
	v->u.n=n;
}
double lept_get_number(const lept_value* v){
	assert(v!=NULL&&v->type==LEPT_NUMBER);
	return v->u.n;
}
const char* lept_get_string(const lept_value* v){
	assert(v!=NULL&&(v->type==LEPT_STRING));
	return v->u.s.s;
} 
size_t lept_get_string_length(const lept_value* v){
	assert(v!=NULL&&(v->type==LEPT_STRING));
	return v->u.s.len;
}
void lept_set_string(lept_value* v,const char* s,size_t len){
	assert(v!=NULL&&(s!=NULL||len == 0));
	lept_free(v);
	v->u.s.s=(char *)malloc(len+1);
	memcpy(v->u.s.s,s,len);
	v->u.s.s[len]='\0';
	v->u.s.len=len;
	v->type=LEPT_STRING;
}
void lept_free(lept_value *v){
	assert(v!=NULL);
	if(v->type==LEPT_STRING)
		free(v->u.s.s);
	v->type=LEPT_NULL;
}
