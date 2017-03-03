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
#define phpext_pinyin_ptr &pinyin_module_entry

#define PHP_PINYIN_VERSION "0.1.0" /* Replace with version number for your extension */

PHP_MINIT_FUNCTION(pinyin);
PHP_MSHUTDOWN_FUNCTION(pinyin);
ZEND_FUNCTION(my_test);

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
    unsigned int tone;
} py_tone_info;

typedef struct
{
    char *from;
    char *to;
} py_punctuation_map;


ZEND_BEGIN_MODULE_GLOBALS(pinyin)
    py_data_list *wordList;
    py_data_list *surnameList;
    zend_bool can_access;
ZEND_END_MODULE_GLOBALS(pinyin)

//calculate hash by string and keyLen
py_data_list *py_data_list_append(py_data_list *last, const char *key, const char *value);
void py_fill_data_list(const char *dir, unsigned int num);
void py_analysis_chinese_tones(const char *line, char *chinese, char *tones);
//void str_replace(const char *from, const char *to, char *str, char *ret, zend_bool is_name);

#define MAX_READ_WORD_NUM 10
#define MAX_FILE_PATH_SIZE 50
#define MAX_WORD_LINE_SIZE 100
#define MAX_WORD_WORD_SIZE 50
#define true 1
#define false 0
#define PY_TONE_INFO_NUM 28
#define PY_CHAR_TRANS_MAP_NUM 10
#define MAX_PUNCTUATION_SIZE 10

//用到的几个常量
#define PINYIN_NONE (1<<0)
#define PINYIN_UNICODE (1<<1)
#define PINYIN_ISNAME (1<<2)
#define PINYIN_TRIM (1<<3)  //省略标点符号
#define PINYIN_FORMAT_EN (1<<4)    //将标点符号转为英文的
#define PINYIN_FORMAT_CH (1<<5)    //将表单符号分割为一个

/* 保存数据的文件名 */
#define FORMAT_WORD_PATH "%swords_%d\0"
#define FORMAT_SURNAME_PATH "%ssurnames\0"

/* 检测文件是否存在的mode */
#define ACCESS_MODE_EXISTS 0


/* 函数封装 */
#define py_strdup estrdup

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
