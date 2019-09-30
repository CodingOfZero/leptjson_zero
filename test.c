/*TDD测试驱动开发*/ 
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include<crtdbg.h>
#endif

#include<stdio.h>
#include<stdlib.h> 
#include<string.h>
#include"leptjson.h"
static int main_ret=0;
static int test_count=0;
static int test_pass=0;
/*宏定义，非单语句时，使用do{}while(0)包裹成单个语句，只用{}是不行的
以下宏测试实际是否与期望的相同*/
#define EXPECT_EQ_BASE(equality,expect,actual,format) \
	do{ \
		test_count++;\
		if(equality) \
			test_pass++;\
		else{\
			fprintf(stderr,"%s:%d: expect: " format " actual: " format "\n",__FILE__,__LINE__,expect,actual);\
			main_ret=1;\
		}\
	}while(0)
#define EXPECT_EQ_INT(expect,actual) EXPECT_EQ_BASE((expect)==(actual),expect,actual,"%d")
#define EXPECT_EQ_DOUBLE(expect,actual) EXPECT_EQ_BASE((expect)==(actual),expect,actual,"%.17g")
#define EXPECT_EQ_STRING(expect,actual,alength) \
			EXPECT_EQ_BASE(sizeof(expect)-1==alength && memcmp(expect,actual,alength)==0 ,expect,actual,"%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
#define TEST_NUMBER(expect,json)\
	do{\
		lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,json));\
		EXPECT_EQ_INT(LEPT_NUMBER,lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect,lept_get_number(&v));\
	}while(0)
#define TEST_ERROR(error,json)\
	do{\
		lept_value v;\
		v.type=LEPT_FALSE;\
		EXPECT_EQ_INT(error,lept_parse(&v,json));\
		EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));\
	}while(0)
	
static void test_parse_null(){
	lept_value v;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"null"));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
	lept_free(&v);
}
static void test_parse_false(){
	lept_value v;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"false"));
	EXPECT_EQ_INT(LEPT_FALSE,lept_get_type(&v));
	lept_free(&v);
}
static void test_parse_true(){
	lept_value v;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK,lept_parse(&v,"true"));
	EXPECT_EQ_INT(LEPT_TRUE,lept_get_type(&v));
	lept_free(&v);
}
static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /*must underflow*/
								  /* the smallest number > 1 */
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	/* minimum denormal */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	/* Max subnormal double */
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	/* Min normal positive double */
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	/* Max double */
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}
#define TEST_STRING(expect,json)\
	do{\
		lept_value v;\
		lept_init(&v);\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
		EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v));\
		lept_free(&v);\
	}while(0)
static void test_parse_string() {
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect,actual) EXPECT_EQ_BASE((expect)==(actual),(size_t)expect,(size_t)actual,"%Iu0")
#else
#define EXPECT_EQ_SIZE_T(expect,actual) EXPECT_EQ_BASE((expect)==(actual),(size_t)expect,(size_t)actual,"%Iu0")
#endif
static void test_parse_array() {
	lept_value v;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(0, lept_get_array_size(&v));
	lept_free(&v);
}
/***********************************测试错误返回*****************************************/
static void test_parse_expect_value(){
#if 0
	lept_value v;
	v.type=LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE,lept_parse(&v,""));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
	
	v.type=LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE,lept_parse(&v," "));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
#endif
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE,"");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE," ");
}
static void test_parse_invalid_value(){
#if 0
	lept_value v;
	v.type=LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE,lept_parse(&v,"nul"));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
	
	v.type=LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE,lept_parse(&v,"?"));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
#endif 
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"?");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"+1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"0123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,".123");/*at least one digit before '.'*/
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"1.");/*at least one digit after '.'*/	
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"INF");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"NAN");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"nan");
	
}
static void test_parse_root_not_singular(){	//非单值 
#if 0
	lept_value v;
	v.type=LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR,lept_parse(&v,"null x"));
	EXPECT_EQ_INT(LEPT_NULL,lept_get_type(&v));
#endif
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"nul x");
}
static void test_parse_number_too_big(){
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG,"1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG,"-1e309");
} 
static void test_parse_miss_quotation_mark() {
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"2019\\/9\/27");//?
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"WEB");
}	
static void test_parse_invalid_string_escape() {
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"test\\a\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"test\\a");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}
static void test_parse_invalid_string_char() {
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}
static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}
static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}
/*************************************测试访问函数***********************************/
static void test_access_null() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_null(&v);
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
    lept_free(&v);
}

static void test_access_boolean() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v,"a",1);
    lept_set_boolean(&v,1);
    EXPECT_TRUE(lept_get_boolean(&v));
    lept_set_boolean(&v,0);
    EXPECT_FALSE(lept_get_boolean(&v));
    lept_free(&v);
}
static void test_access_number() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v,"a",1);
    lept_set_number(&v,232.4);
    EXPECT_EQ_DOUBLE(232.4,lept_get_number(&v));
    lept_free(&v);
}
static void test_access_string(){
	lept_value v;
	lept_init(&v);
	lept_set_string(&v,"",0);
	EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
	lept_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
	lept_free(&v);	
}
static void test_parse(){
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_parse_number();
	test_parse_string();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big(); 
	test_parse_miss_quotation_mark();
	test_parse_invalid_string_escape();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}
int main(){
	
	test_parse();
	printf("%d/%d (%3.2f%%) passed \n",test_pass,test_count,test_pass*100.0/test_count);
	_CrtDumpMemoryLeaks();
	return main_ret;	
}
