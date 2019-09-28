#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include<crtdbg.h>
#endif

#include"leptjson.h"
#include<assert.h>	//断言 
/*最常用的是在函数开始的地方，检测所有参数，
有时候也可在调用函数后，检查上下文是否正确
若错误是由程序员错误编码（例如传入不合法参数）此时用断言
若错误是程序员无法避免的，而是由运行时环境造成（开启文件失败），要抛出异常*/
#include<stdlib.h> 	//"NULL ,strtod() malloc() realloc() free()" 
#include<errno.h> /*errno, ERANGE*/
#include<math.h> /*HUGE_VAL*/
#include<string.h>/*memcpy()*/ 

#define EXPECT(c,ch)	do{assert(*c->json==(ch)); c->json++; }while(0)
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')
#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')

typedef struct{
	const char* json;
	char* stack;//堆栈基址 
	size_t size,top;//size是当前的堆栈容量，top是栈顶的位置（由于会扩展stack，故top不采用指针形式存储） 
}lept_context;
/* ws = *(%x20 / %x09 / %x0A / %x0D) 空格符，制表符，换行符，回车符*/  
static void lept_parse_whitespace(lept_context *c){
	const char *p=c->json;
	while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')
		p++;
	c->json=p;	
}
#if 0
static int lept_parse_null(lept_context *c,lept_value *v){	//解析NULL 
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
//重构合并 lept_parse_null函数、lept_parse_false函数、lept_parse_true函数为 lept_parse_literal函数 
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
/*******************************解析数字*********************************/ 
static int lept_parse_number(lept_context *c,lept_value *v){
	const char *p=c->json;
	if(*p=='-') p++;/* 负号 ... */
    if(*p=='0'){
    	p++;
    	if(ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
	}
	else{
		if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* 整数 ... */
	
    if(*p=='.'){
		p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* 小数,检查它至少应有一个 digit，不是 digit 就返回错误 */
    if(*p=='e'||*p=='E'){
		p++;
		if(*p=='+'||*p=='-') p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}/* 指数 ... */
	
	errno=0;
	v->u.n=strtod(c->json,NULL);
	if((errno==ERANGE)&&(v->u.n==HUGE_VAL || v->u.n==-HUGE_VAL)) //v->==0 会产生 Assertion failed!File: leptjson.c, Line 104
		return LEPT_PARSE_NUMBER_TOO_BIG;
	c->json=p;
	v->type=LEPT_NUMBER;
	return LEPT_PARSE_OK;
}
/*******************************解析字符串*********************************/ 
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
//下一语句使用sizeof(char)而非sizeof(ch)原因在于：C 中字符字面值的类型是 int，C++ 中则是 char
#define PUTC(c,ch) do{ *(char*)lept_context_push(c,sizeof(char))=(ch);}while(0) 
#define STRING_ERROR(ret) do{ c->top = head;return ret; }while(0)
//堆栈是以字节储存，每次可要求压入任意大小的数据，它会返回数据起始的指针
static void* lept_context_push(lept_context* c,size_t size){
	void *ret;
	assert(size>0);
	if(c->top+size>=c->size){
		if(c->size==0)
			c->size=LEPT_PARSE_STACK_INIT_SIZE;
		while(c->top+size>=c->size)
			c->size+=c->size>>1;	/*c->size * 1.5  */
		c->stack=(char*)realloc(c->stack,c->size);//c->stack在初始化时为 NULL，realloc(NULL, size)的行为是等价于malloc(size)
	}
	ret=c->stack+c->top;
	c->top+=size;
	return ret;
}
static void* lept_context_pop(lept_context* c,size_t size){
	assert(c->top >= size);
	return c->stack+(c->top-=size);
}
static void* lept_parse_hex4(const char *p,unsigned* u){
	int i;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0'&&ch <= '9') *u |= ch - '0';
		else if (ch >= 'A'&&ch <= 'F')*u |= ch - ('A' - 10);
		else if (ch >= 'a'&&ch <= 'f')*u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}
static void lept_encode_utf8(lept_context* c, unsigned u) {
	if ( u <= 0x7f) {
		PUTC(c, u & 0xFF);
	}else if (u <= 0x07ff) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | ( u       & 0x3F));
	}else if ( u <= 0xffff) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}else if (u <= 0x10ffff) {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}
}

static int lept_parse_string(lept_context *c,lept_value *v){
	size_t head=c->top,len;
	unsigned u,u2;
	const char* p;
	EXPECT(c,'\"');//string以双引号开头，双引号结尾，故检测开始是否为 " 
	p=c->json;
	for(;;){
		char ch=*p++;
		switch(ch){
		case'\\':
			switch (*p++)
			{
			case '\"':  PUTC(c, '\"'); break;
			case '\\':  PUTC(c, '\\'); break;
			case '/':  PUTC(c, '/'); break;
			case 'b':  PUTC(c, '\b'); break;
			case 'f':  PUTC(c, '\f'); break;
			case 'n':  PUTC(c, '\n'); break;
			case 'r':  PUTC(c, '\r'); break;
			case 't':  PUTC(c, '\t'); break;
			case 'u':
				if (!(p=lept_parse_hex4(p,&u)))
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				if (u >= 0xD800 && u <= 0xDBFF) {//high surrogate
					if (*p++ != '\\')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if(*p++!='u')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (!(p = lept_parse_hex4(p, &u2)))
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					if (u2 < 0xDC00 || u2 > 0xDFFF)
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
				}
				lept_encode_utf8(c,u);
				break;
			default:
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case'\"':
			len=c->top-head; 
			lept_set_string(v,(const char*)lept_context_pop(c,len),len);
			c->json=p;
			return LEPT_PARSE_OK;
		case'\0':
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		default:
			if ((unsigned char)ch < 0x20) { //char是否带符号，由实现定义，若不进行转换，ch>=80的字符，
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);//都会变为负数，并产生该错误。
			}
			PUTC(c,ch);
		}
	}
	
}
/*******************************解析类型判断*********************************/ 
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
/*******************************解析函数*********************************/  
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
	assert(c.top==0);//确保所有数据都被弹出
	free(c.stack); 
	return ret;
	
}
/*************************************函数实现*********************************/
lept_type lept_get_type(const lept_value *v){
	assert(v!=NULL);
	return v->type;
}
int lept_get_boolean(const lept_value *v){
	assert(v!=NULL&&(v->type==LEPT_FALSE||v->type==LEPT_TRUE));
	return v->type==LEPT_TRUE;	
}
void lept_set_boolean(lept_value *v,int b){
	lept_free(v);
	v->type=b?LEPT_TRUE:LEPT_FALSE;
	/*if(b==0)
		v->type=LEPT_FALSE;
	else
		v->type=LEPT_TRUE;*/		
}
void lept_set_number(lept_value *v,double n){
	assert(v!=NULL);
	lept_free(v);
	v->u.n=n;
	v->type=LEPT_NUMBER;
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
