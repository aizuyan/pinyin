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
unsigned long hashLen = 50000;

/* True global resources - no need for thread safety here */
static int le_pinyin;


MyList *pinyin_list_append(const char *key, const char *value)
{
    //判断槽位是否为空
    MyList *element = (MyList *)malloc(sizeof(MyList));
    char *newKey = strdup(key);
    char *newVal = strdup(value);
    element->key = newKey;
    element->val = newVal;
    element->next = NULL;
    pinyin_globals.myLast->next = element;
	pinyin_globals.myLast = element;

    return element;
}

const char *get_key_from_line(const char *line, char *ret)
{   
    int i = 0;
    while(*line)
    {   
        if(*line != ',')
        {   
            ret[i] = *line;
        }else {
            break;
        }
        i++;
        line++;
    }
    ret[i] = '\0';
    return ret;
}

const char *get_val_from_line(const char *line, char *ret)
{   
    int i = 0; 
    int flag = 0;
    while(*line)
    {   
        if(*line == '\n')
		{
			break;
		}
        if(*line == ',')
        {   
            flag = 1;
            line++;
            continue;
        }else if(!flag) {
            line++;
            continue;
        }
        ret[i] = *line;
        i++;
        line++;
    }
    ret[i] = '\0';
    return ret;
}

//最多扫描10个words文件，按顺序从0开始
void scan_words_from_dir(const char *dir)
{
    const char *path = "%swords_%d";
    int i = 0;
    char file[MAX_FILE_PATH_SIZE], line[MAX_WORD_LINE_SIZE], key[MAX_WORD_WORD_SIZE], val[MAX_WORD_WORD_SIZE];
    FILE *fp;
    for(; i<MAX_READ_WORD_NUM; i++)
    {
        sprintf(file, path, dir, i);
        if(access(file, 0) == -1)
            continue;
        fp  = fopen(file, "r");
        while(fgets(line, 100, fp))
        {
            get_key_from_line(line, key);
            get_val_from_line(line, val);
            pinyin_list_append(key, val);
        }
        fclose(fp);
        fp = NULL;
    }
}

void str_replace(const char *from, const char *to, char *str, char *ret)
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
    }   

    strcat(ret, str);
	if(1 == flag)
	{
		memcpy(strTmp, ret, strlen(ret));
		strTmp[strlen(ret)] = '\0';
	}
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pinyin.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pinyin_globals, pinyin_globals)
    STD_PHP_INI_ENTRY("pinyin.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pinyin_globals, pinyin_globals)
PHP_INI_END()
*/
/* }}} */

PHP_INI_BEGIN()
    PHP_INI_ENTRY("pinyin.dir", "/tmp/mypinyin/", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string my_test(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(my_test)
{
    char *arg = NULL;
    int arg_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
        return;
    } 

	int alloc_len = arg_len * 4 + 1;
	char *ret = (char *)emalloc(alloc_len),
		 *oldMessage = (char *)emalloc(alloc_len);
	memset(oldMessage, '\0', alloc_len);
	strcat(oldMessage, arg);
	strcat(ret, arg);
	MyList *p = pinyin_globals.myList->next;
	while(p != NULL)
	{
		memset(ret, '\0', alloc_len);
		str_replace(p->key, p->val, oldMessage, ret);
		p = p->next;
	}

	array_init(return_value);
	char *item;
	item = strtok(ret, "\t");
	add_next_index_string(return_value, item, 1);

	while((item = strtok(NULL, "\t")))
	{
		add_next_index_string(return_value, item, 1);
	}

//	RETURN_STRING(ret, 1);
}
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
    pinyin_globals.myList = pinyin_globals.myLast = (MyList *)malloc(sizeof(MyList));
    const char *pinyindir = INI_STR("pinyin.dir");
    scan_words_from_dir(pinyindir);
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
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
	
	MyList *p = pinyin_globals.myList->next;
	MyList *tmp = p;
	while(p != NULL)
	{
		tmp = p->next;
		free(p->key);
		free(p->val);
		free(p);
		p = tmp;
	}
	

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
	PHP_FE(my_test,	NULL)		/* For testing, remove later. */
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
