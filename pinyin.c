/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pinyin.h"

/* If you declare any globals in php_pinyin.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(pinyin);

/* 要转换的 字符列表*/
static py_punctuation_map charTransMap[PY_CHAR_TRANS_MAP_NUM] = {
    {"，", ","},
    {"。", "."},
    {"？", "?"},
    {"！", "!"},
    {"：", ":"},
    {"；", ";"},
    {"“", "\""},
    {"”", "\""},
    {"‘", "'"},
    {"’", "'"}
};

/*韵母信息表，完整的韵母,不带音标的韵母,音标*/
static py_tone_info toneInfos[PY_TONE_INFO_NUM] = {
    {"üē", "ue", 1},
    {"üé", "ue", 2},
    {"üě", "ue", 3},
    {"üè", "ue", 4},
    {"ā", "a", 1},    
    {"á", "a", 2},    
    {"ǎ", "a", 3},    
    {"à", "a", 4},    
    {"ē", "e", 1},    
    {"é", "e", 2},    
    {"ě", "e", 3},    
    {"è", "e", 4},    
    {"ī", "i", 1},    
    {"í", "i", 2},    
    {"ǐ", "i", 3},    
    {"ì", "i", 4},    
    {"ō", "o", 1},    
    {"ó", "o", 2},    
    {"ǒ", "o", 3},    
    {"ò", "o", 4},    
    {"ū", "u", 1},    
    {"ú", "u", 2},    
    {"ǔ", "u", 3},    
    {"ù", "u", 4},    
    {"ǖ", "v", 1},    
    {"ǘ", "v", 2},    
    {"ǚ", "v", 3},    
    {"ǜ", "v", 4}
};

/* True global resources - no need for thread safety here */

py_data_list *py_data_list_append(py_data_list *last, const char *key, const char *value)
{
    py_data_list *element = (py_data_list *)malloc(sizeof(py_data_list));
    element->key = py_strdup(key);
    element->val = py_strdup(value);
    element->next = NULL;
    last->next = element;

    return element;
}


/**
 * 从一行汉字、拼音中将汉字拼音分开
 *
 * @param line
 * @param ret
 * @return
 */
void py_analysis_chinese_tones(const char *line, char *chinese, char *tones)
{
    int i = 0,
        j = 0,
        flag = 0;
    while(*line)
    {
        if(*line == '\n')
        {
            break;
        }
        if(0 == flag && *line != ',')
        {
            chinese[i] = *line;
            i++;
        }else if (',' == *line){
            flag = 1;
        }else if (1 == flag){
            tones[j] = *line;
            j++;
        }
        line++;
    }
    chinese[i] = 0;
    tones[j] = 0;
}

/*
 * 扫描指定文件夹下指定数量的文件，最多num个
 *
 * surname最多一个
 */
void py_fill_data_list(const char *dir, unsigned int num)
{
    int i = 0;
    char filePath[100] = {0},
        fileLine[100] = {0},
        chinese[100] = {0},
        tones[100] = {0};

    FILE *fp;
    sprintf(filePath, FORMAT_SURNAME_PATH, dir);
    py_data_list *last = PY_GLOBAL(surnameList);
    if(0 == access(filePath, ACCESS_MODE_EXISTS))
    {
        fp = fopen(filePath, "r");
        while(fgets(fileLine, 100, fp))
        {
            py_analysis_chinese_tones(fileLine, chinese, tones);
            last = py_data_list_append(last, chinese, tones);
        }
        fclose(fp);
        fp = NULL;
    }

    last = PY_GLOBAL(wordList);
    for(; i<MAX_READ_WORD_NUM; i++)
    {
        sprintf(filePath, FORMAT_WORD_PATH, dir, i);
        if(0 != access(filePath, ACCESS_MODE_EXISTS))
            continue;
        fp  = fopen(filePath, "r");
        while(fgets(fileLine, 100, fp))
        {
            py_analysis_chinese_tones(fileLine, chinese, tones);
            last = py_data_list_append(last, chinese, tones);
        }
        fclose(fp);
        fp = NULL;
    }
}

// 替换字符串中的所有子字符串
/*
void str_replace(const char *from, const char *to, char *str, char *ret, zend_bool is_name)
{
    int pos = 0,
        fromLen = strlen(from),
		flag = 0;
    char *tmp = NULL,
		 *strTmp = str;
    while(tmp = strstr(str, from))
    {   
        pos = tmp - str;
        strncat(ret, str, pos);
        strcat(ret, to);
        str = tmp + fromLen;
		flag = 1;
        if(is_name)
            break;
    }   

    strcat(ret, str);
	if(1 == flag)
	{
		memcpy(strTmp, ret, strlen(ret));
		strTmp[strlen(ret)] = '\0';
	}
}*/

PHP_INI_BEGIN()
    PHP_INI_ENTRY("pinyin.dir", "", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string chinese_to_pinyin(string arg)
   Return a string to confirm that the module is compiled in */
/*PHP_FUNCTION(chinese_to_pinyin)
{
    char *arg = NULL;
    int arg_len;
    long l = PINYIN_UNICODE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &arg, &arg_len, &l) == FAILURE) {
        return;
    }

    if(pinyin_globals.can_access == false)
    {
        php_error(E_WARNING, "拼音转汉字初始化加载配置文件失败，转化失败！");
        RETURN_FALSE;
    }

	int alloc_len = arg_len * 4 + 1;
	char *ret = (char *)emalloc(alloc_len),
		 *oldMessage = (char *)emalloc(alloc_len),
         char_str[MAX_PUNCTUATION_SIZE];
	memset(oldMessage, '\0', alloc_len);
	strcat(oldMessage, arg);
	strcat(ret, arg);
    MyList *p_surname = pinyin_globals.mySurnameList->next;

    //去掉标点符号
    if(l & (PINYIN_TRIM|PINYIN_FORMAT_EN|PINYIN_FORMAT_CH))
    {
        int j = 0;
        for(; j<MY_TRIM_NUM; j++)
        {
            if(l & PINYIN_FORMAT_CH)   //仅仅格式化
            {
                memset(char_str, '\0', MAX_PUNCTUATION_SIZE);
                strcat(char_str, "\t");
                strcat(char_str, punctuations[j].key);
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].key, char_str, oldMessage, ret, false);   
                memset(char_str, '\0', MAX_PUNCTUATION_SIZE);
                strcat(char_str, "\t");
                strcat(char_str, punctuations[j].key);
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].val, char_str, oldMessage, ret, false);   
            }
            else if(l & PINYIN_FORMAT_EN)   //汉字符号转为英文符号
            {
                memset(char_str, '\0', MAX_PUNCTUATION_SIZE);
                strcat(char_str, punctuations[j].val);
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].key, char_str, oldMessage, ret, false);   
                memset(char_str, '\0', MAX_PUNCTUATION_SIZE);
                strcat(char_str, "\t");
                strcat(char_str, punctuations[j].val);
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].val, char_str, oldMessage, ret, false);   
            }else if(l & PINYIN_TRIM)   //去除标点符号
            {
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].key, "", oldMessage, ret, false);   
                memset(ret, '\0', alloc_len);
                str_replace(punctuations[j].val, "", oldMessage, ret, false);   
            }
        }
    }

    //如果是名字，先用名字解析
    if(l & PINYIN_ISNAME)
    {
        while(p_surname != NULL)
        {
            memset(ret, '\0', alloc_len);
            str_replace(p_surname->key, p_surname->val, oldMessage, ret, true);
            p_surname = p_surname->next;
        }
    }

    //正常的拼音化
	MyList *p = pinyin_globals.myList->next;
	while(p != NULL)
	{
		memset(ret, '\0', alloc_len);
		str_replace(p->key, p->val, oldMessage, ret, false);
		p = p->next;
	}

    //如果不需要声调，替换元音字母为正常字母
    if(l & PINYIN_NONE)
    {
        int i = 0;
        for(; i<MY_TONES_NUM; i++)
        {
		    memset(ret, '\0', alloc_len);
		    str_replace(_myTones[i].key, _myTones[i].val, oldMessage, ret, false);
        }
    }
	array_init(return_value);
	char *item;
	item = strtok(ret, "\t");
    py_add_next_index_string(return_value, item, 1);

	while((item = strtok(NULL, "\t")))
	{
        py_add_next_index_string(return_value, item, 1);
	}
}
 */
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_pinyin_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pinyin_init_globals(zend_pinyin_globals *pinyin_globals)
{
	pinyin_globals->global_value = 0;
	pinyin_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pinyin)
{
    REGISTER_INI_ENTRIES();

    /* 数据配置地址 */
    const char *pinyinDir = NULL;

    /* 初始化全局变量 */
    PY_GLOBAL(can_access) = false;
    PY_GLOBAL(wordList) = (py_data_list *)malloc(sizeof(py_data_list));
    PY_GLOBAL(wordList)->next = NULL;
    PY_GLOBAL(surnameList) = (py_data_list *)malloc(sizeof(py_data_list));
    PY_GLOBAL(surnameList)->next = NULL;
    pinyinDir = INI_STR("pinyin.dir");

    if(strlen(pinyinDir) == 0 || access(pinyinDir, ACCESS_MODE_EXISTS))
    {
        php_error(E_WARNING, "汉字转拼音配置文件夹【%s】访问不了，或者未配置文件夹", pinyinDir);
    }else {
        py_fill_data_list(pinyinDir);
        PY_GLOBAL(can_access) = true;
    }

    //注册常量
    REGISTER_LONG_CONSTANT("PINYIN_NONE", PINYIN_NONE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_UNICODE", PINYIN_UNICODE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_ISNAME", PINYIN_ISNAME, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_TRIM", PINYIN_TRIM, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_FORMAT_EN", PINYIN_FORMAT_EN, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_FORMAT_CH", PINYIN_FORMAT_CH, CONST_PERSISTENT | CONST_CS);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pinyin)
{
	/* uncomment this line if you have INI entries
	*/
	UNREGISTER_INI_ENTRIES();


    py_data_list *tmp,
        *ptr = PY_GLOBAL(wordList);
	
	tmp = ptr;
	while(ptr != NULL)
	{
		tmp = ptr->next;
		free(ptr->key);
		free(ptr->val);
		free(ptr);
        ptr = tmp;
	}

    ptr = PY_GLOBAL(surnameList);
    tmp = ptr;
    while(ptr != NULL)
    {
        tmp = ptr->next;
		free(ptr->key);
		free(ptr->val);
		free(ptr);
        ptr = tmp;
    }
    free(PY_GLOBAL(wordList));
    free(PY_GLOBAL(surnameList));
	
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pinyin)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pinyin support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ pinyin_functions[]
 *
 * Every user visible function must have an entry in pinyin_functions[].
 */
const zend_function_entry pinyin_functions[] = {
	//PHP_FE(chinese_to_pinyin,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in pinyin_functions[] */
};
/* }}} */

/* {{{ pinyin_module_entry
 */
zend_module_entry pinyin_module_entry = {
	STANDARD_MODULE_HEADER,
	"pinyin",
	pinyin_functions,
	PHP_MINIT(pinyin),
	PHP_MSHUTDOWN(pinyin),
	PHP_RINIT(pinyin),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pinyin),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pinyin),
	PHP_PINYIN_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PINYIN
ZEND_GET_MODULE(pinyin)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
