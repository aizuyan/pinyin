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

#ifdef PHP_WIN32
#	define PHP_PINYIN_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PINYIN_API __attribute__ ((visibility("default")))
#else
#	define PHP_PINYIN_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(pinyin)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(pinyin)
*/

PHP_MINIT_FUNCTION(pinyin);
PHP_MSHUTDOWN_FUNCTION(pinyin);
ZEND_FUNCTION(my_test);

typedef struct mylist {
    char *key;
    char *val;
    struct mylist *next;
} MyList;

typedef struct mytone {
    char *key;
    char *val;
    unsigned int tone;
} MyTone;

ZEND_BEGIN_MODULE_GLOBALS(pinyin)
    MyList *myList;
	MyList *myLast;
    MyList *mySurnameList;
    MyList *mySurnameLast;
    MyTone *myTones;
ZEND_END_MODULE_GLOBALS(pinyin)

//calculate hash by string and keyLen
MyList *pinyin_list_append(MyList *last, const char *key, const char *value);
void scan_words_from_dir(const char *dir);
const char *get_key_from_line(const char *line, char *ret);
const char *get_val_from_line(const char *line, char *ret);
void str_replace(const char *from, const char *to, char *str, char *ret, zend_bool is_name);

#define MAX_READ_WORD_NUM 10
#define MAX_FILE_PATH_SIZE 50
#define MAX_WORD_LINE_SIZE 100
#define MAX_WORD_WORD_SIZE 50
#define true 1
#define false 0
#define MY_TONES_NUM 28

//用到的几个常量
#define PINYIN_NONE (1<<0)
#define PINYIN_UNICODE (1<<1)
#define PINYIN_ISNAME (1<<2)
#define PINYIN_TRIM (1<<3)

/* In every utility function you add that needs to use variables 
   in php_pinyin_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PINYIN_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define PINYIN_G(v) TSRMG(pinyin_globals_id, zend_pinyin_globals *, v)
#else
#define PINYIN_G(v) (pinyin_globals.v)
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
