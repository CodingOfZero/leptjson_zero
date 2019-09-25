/*宏的名字唯一，习惯以 _H__ 作为后缀,项目有多个文件或目录结构，可以用 项目名称_目录_文件名称_H__ 这种命名方式。*/
#ifndef LEPTJSON_H_	
#define LEPTJSON_H_
#include<stdio.h> // "size_t"
typedef enum { LEPT_NULL,LEPT_FALSE,LEPT_TRUE,LEPT_NUMBER,LEPT_STRING,LEPT_ARRAY,LEPT_OBJECT}lept_type; //json数据类型 
typedef struct{			//json数据结构 
	lept_type type;
	union{
		struct{
			char *s;
			size_t len;
		}s;
		double n; 	
	}u; 
}lept_value;
enum{					//返回值 
	LEPT_PARSE_OK=0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG
};
#define lept_init(v) do{ (v)->type = LEPT_NULL; }while(0) //暂时未懂 ,用->与.之间区别 
#define lept_set_null(v) lept_free(v)
int lept_parse(lept_value *v,const char*json); 	//解析函数 

lept_type lept_get_type(const lept_value *v);	//获取结果的类型 

int lept_get_boolean(const lept_value *v);//布尔型 
void lept_set_boolean(lept_value *v,int b);

double lept_get_number(const lept_value* v); //数字
void lept_set_number(lept_value *v,double n);

const char* lept_get_string(const lept_value* v);//字符串 
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v,const char* s,size_t len);

void lept_free(lept_value *v);
 
#endif /*LEPTJSON_H_*/
