/*宏的名字唯一，习惯以 _H__ 作为后缀,项目有多个文件或目录结构，可以用 项目名称_目录_文件名称_H__ 这种命名方式。*/
#ifndef LEPTJSON_H_	
#define LEPTJSON_H_
typedef enum { LEPT_NULL,LEPT_FALSE,LEPT_TRUE,LEPT_NUMBER,LEPT_STRING,LEPT_ARRAY,LEPT_OBJECT}lept_type; //json数据类型 
typedef struct{			//json数据结构 
	lept_type type;
	double n; 
}lept_value;
enum{					//返回值 
	LEPT_PARSE_OK=0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR
};
int lept_parse(lept_value *v,const char*json); 	//解析函数 
lept_type lept_get_type(const lept_value *v);	//获取结果的类型 

#endif /*LEPTJSON_H_*/
