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

#ifndef PHP_PINYIN_H
#define PHP_PINYIN_H

extern zend_module_entry pinyin_module_entry;

#define PHP_PINYIN_VERSION "0.2.0" /* Replace with version number for your extension */

PHP_MINIT_FUNCTION(pinyin);
PHP_MSHUTDOWN_FUNCTION(pinyin);

typedef struct _py_data_list
{
    char *key;
    char *val;
    struct _py_data_list *next;
} py_data_list;

typedef struct
{
    char *complete;
    char *simple;
    size_t tone;
} py_tone_info;

typedef struct
{
    char *from;
    char *to;
} py_punctuation_map;

/* 存储一次处理的数据，包括原始拼音，不带音调的拼音，首字母大消息，声调数字 */
typedef struct _py_row_data_list
{
    char *ori;
    char *none;
    char ucfirst;
    char lcfirst;
    size_t tone;
    struct _py_row_data_list *next;
} py_row_data_list;

#define CREATE_ROW_DATA_ITEM(ptr) \
    ptr = (py_row_data_list *)py_malloc(sizeof(py_row_data_list), 0); \
    ptr->ori = NULL; \
    ptr->none = NULL; \
    ptr->ucfirst = 0; \
    ptr->lcfirst = 0; \
    ptr->tone = 0; \
    ptr->next = NULL;

#define CHANGE_STR(ret, ori, beginPtr, from, to, j, k) do{ \
        for (j=0; j <(beginPtr-ori); j++) { \
            ret[j] = ori[j]; \
        } \
        for (k=0; k<py_strlen(to); k++){ \
            ret[j] = to[k]; \
            j++; \
        } \
        beginPtr += py_strlen(from); \
        while (*beginPtr) { \
            ret[j] = *beginPtr; \
            beginPtr++; \
            j++; \
        } \
        ret[j] = 0; \
    }while(0)

ZEND_BEGIN_MODULE_GLOBALS(pinyin)
    py_data_list *wordList;
    py_data_list *surnameList;
    zend_bool can_access;
ZEND_END_MODULE_GLOBALS(pinyin)

//calculate hash by string and keyLen
py_data_list *py_data_list_append(py_data_list *last, const char *key, const char *value);
void py_fill_data_list(const char *dir, unsigned int num);
void py_analysis_chinese_tones(const char *line, char *chinese, char *tones);
void str_replace(const char *from, const char *to, char *str, char *ret, zend_bool is_name);
static int php_array_key_compare(const void *a, const void *b);
py_row_data_list *py_split_sentence(const char *chinese, size_t flag);
void py_destory_row_list(py_row_data_list *list);

#define MAX_READ_WORD_NUM 10
#define true 1
#define false 0
#define PY_TONE_INFO_NUM 28
#define PY_CHAR_TRANS_MAP_NUM 10

/* 转化时候的优化项 */
#define PINYIN_NONE (1<<0)
#define PINYIN_UNICODE (1<<1)
#define PINYIN_ISNAME (1<<2)
#define PINYIN_ASCII (1<<3)
#define PINYIN_UCFIRST (1<<4)
#define PINYIN_LCFIRST (1<<5)

/* 保存数据的文件名 */
#define FORMAT_WORD_PATH "%swords_%d"
#define FORMAT_SURNAME_PATH "%ssurnames"

/* 检测文件是否存在的mode */
#define ACCESS_MODE_EXISTS 0

/* 替换时候原字符串替换符，这个ascii字符1很不常用 */
#define CHINESE_SUB_CHAR 1

/* 函数封装 */
#define py_strdup(s, persistent) pestrdup(s, persistent)
#define py_strstr strstr
#define py_malloc(size, persistent) pemalloc(size, persistent)
#define py_strlen strlen
#define py_memcpy memcpy

#define PY_GLOBAL(v) (pinyin_globals.v)

#if PHP_MAJOR_VERSION < 7
#include "php5_pinyin.h"
#else
#include "php7_pinyin.h"
#endif

#endif	/* PHP_PINYIN_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
