#include"leptjson.h"
#include<assert.h>	//断言 
/*最常用的是在函数开始的地方，检测所有参数，
有时候也可在调用函数后，检查上下文是否正确
若错误是由程序员错误编码（例如传入不合法参数）此时用断言
若错误是程序员无法避免的，而是由运行时环境造成（开启文件失败），要抛出异常*/ 
#include<stdlib.h> 	//"NULL" 
#define EXPECT(c,ch)	do{assert(*c->json==(ch)); c->json++; }while(0)
typedef struct{
	const char* json;
}lept_context;
/* ws = *(%x20 / %x09 / %x0A / %x0D) 空格符，制表符，换行符，回车符*/  
static void lept_parse_whitespace(lept_context *c){
	const char *p=c->json;
	while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')
		p++;
	c->json=p;	
}
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
static int lept_parse_value(lept_context *c,lept_value *v){
	switch(*c->json){
		case'n':return lept_parse_null(c,v);
		case'f':return lept_parse_false(c,v);
		case't':return lept_parse_true(c,v);
		case'\0':return LEPT_PARSE_EXPECT_VALUE;
		default: return LEPT_PARSE_INVALID_VALUE;
	}
}
int lept_parse(lept_value *v,const char*json){
	lept_context c;
	int ret; 
	assert(v != NULL);
	c.json=json;
	v->type=LEPT_NULL;
	lept_parse_whitespace(&c);
	if((ret=lept_parse_value(&c,v))==LEPT_PARSE_OK){
			lept_parse_whitespace(&c);
			if(*c.json != '\0')
				ret=LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	return ret;
	
}
lept_type lept_get_type(const lept_value *v){
	assert(v!=NULL);
	return v->type;
}
