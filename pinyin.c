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
static char *charTransMap[PY_CHAR_TRANS_MAP_NUM][2] = {
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
    if (strlen(key) <= 0 || strlen(value) <= 0)
        return last;
    py_data_list *element = (py_data_list *)py_malloc(sizeof(py_data_list), 1);
    element->key = py_strdup(key, 1);
    element->val = py_strdup(value, 1);
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

/**
 *
 * @param chinese
 * @param flag
 * @return
 */
py_row_data_list *py_split_sentence(const char *sentence, size_t flag)
{
    if(PY_GLOBAL(can_access) == false)
    {
        php_error(E_WARNING, "拼音转汉字初始化加载配置文件失败，转化失败！");
        return NULL;
    }
	
	char *chinese = estrdup(sentence);

    //正常的拼音化
    py_data_list *wordListPtr;
    char *wordPtr = NULL,
        *splitItem = NULL,
        *splitItemPtr = NULL,
        tmpStr[100] = {0};
    size_t splitLen = 0,
            i = 0,
            j = 0,
            k = 0,
            m = 0;
    zend_ulong numKey;
#if PHP_MAJOR_VERSION < 7
    zval **entry;
#else
    zval *entry;
#endif
    zval *pinyinPieces = (zval *)py_malloc(sizeof(zval), 0);
    py_row_data_list *rowDataList = (py_row_data_list *)py_malloc(sizeof(py_row_data_list), 0),
        *rowDataListPtr = rowDataList,
        *rowDataListTmpPtr = NULL;

    array_init(pinyinPieces);

    /* 替换姓名优先 */
    if (flag & PINYIN_ISNAME) {
        wordListPtr = PY_GLOBAL(surnameList)->next;
        while(wordListPtr != NULL)
        {
            while (NULL != (wordPtr = py_strstr(chinese, wordListPtr->key))) {
                py_add_index_stringl(pinyinPieces, wordPtr-chinese, wordListPtr->val, py_strlen(wordListPtr->val), 1);
                memset(wordPtr, CHINESE_SUB_CHAR, py_strlen(wordListPtr->key));
            }
            wordListPtr = wordListPtr->next;
        }
    }
    
    wordListPtr = PY_GLOBAL(wordList)->next;
    while(wordListPtr != NULL)
    {
        while (NULL != (wordPtr = py_strstr(chinese, wordListPtr->key))) {
            py_add_index_stringl(pinyinPieces, wordPtr-chinese, wordListPtr->val, py_strlen(wordListPtr->val), 1);
            memset(wordPtr, CHINESE_SUB_CHAR, py_strlen(wordListPtr->key));
        }
        wordListPtr = wordListPtr->next;
    }

    /* 切分标点符号 */
    wordPtr = chinese;
    for (; i<PY_CHAR_TRANS_MAP_NUM; i++) {
        while (NULL != (wordPtr = py_strstr(chinese, charTransMap[i][0]))) {
            py_add_index_stringl(pinyinPieces, wordPtr-chinese, charTransMap[i][0], py_strlen(charTransMap[i][0]), 1);
            memset(wordPtr, CHINESE_SUB_CHAR, py_strlen(charTransMap[i][0]));
        }
        while (NULL != (wordPtr = py_strstr(chinese, charTransMap[i][1]))) {
            py_add_index_stringl(pinyinPieces, wordPtr-chinese, charTransMap[i][1], py_strlen(charTransMap[i][1]), 1);
            memset(wordPtr, CHINESE_SUB_CHAR, py_strlen(charTransMap[i][1]));
        }
    }

    /* 切分非标点符号和汉字 */
    wordPtr = chinese;
    while (*wordPtr) {
        if (CHINESE_SUB_CHAR == *wordPtr) {
            if (splitLen > 0) {
                *wordPtr = 0;
                py_add_index_stringl(pinyinPieces, wordPtr-chinese-splitLen,wordPtr - splitLen, py_strlen(wordPtr - splitLen), 1);
            }
            splitLen = 0;
        } else {
            splitLen++;
        }
        ++wordPtr;
    }
	
	/* 特殊情况：最后一个为非汉字的时候 */
	if (splitLen > 0) {
		py_add_index_stringl(pinyinPieces, wordPtr-chinese-splitLen,wordPtr - splitLen, py_strlen(wordPtr - splitLen), 1);
	}

    /* 格式化数组，将汉字切分为单个的一个，去掉制表符 */
    for (i=0; i<=strlen(sentence); i++) {
        #if PHP_MAJOR_VERSION < 7
            if (zend_hash_index_find(Z_ARRVAL_P(pinyinPieces), i, (void**)&entry) == FAILURE || py_strlen(Z_STRVAL_PP(entry)) <= 0)
                continue;
            splitItem = strtok(Z_STRVAL_PP(entry), "\t");
        #else
            entry = zend_hash_index_find(Z_ARRVAL_P(pinyinPieces), i);
            if (NULL == entry)
                continue;
            splitItem = strtok(Z_STRVAL_P(entry), "\t");
        #endif
        /* 不需要拼音声调 */
        CREATE_ROW_DATA_ITEM(rowDataListTmpPtr);
        rowDataListTmpPtr->ori = py_strdup(splitItem, 0);
        rowDataListPtr->next = rowDataListTmpPtr;
        rowDataListPtr = rowDataListTmpPtr;
        if (flag & (PINYIN_NONE|PINYIN_ASCII|PINYIN_LCFIRST|PINYIN_UCFIRST)) {
            for(m=0 ; m<PY_TONE_INFO_NUM; m++) {
                if (NULL != (wordPtr=py_strstr(splitItem, toneInfos[m].complete))){
                    CHANGE_STR(tmpStr, splitItem, wordPtr, toneInfos[m].complete, toneInfos[m].simple, j, k);
                    rowDataListTmpPtr->none = py_strdup(tmpStr, 0);
                    rowDataListTmpPtr->tone = toneInfos[m].tone;
                    break;
                }
            }
        }
        if (flag & (PINYIN_LCFIRST|PINYIN_UCFIRST)){
            if (NULL != rowDataListTmpPtr->none) {
                rowDataListTmpPtr->lcfirst = *rowDataListTmpPtr->none;
                rowDataListTmpPtr->ucfirst = rowDataListTmpPtr->lcfirst - 32;
                if (!(rowDataListTmpPtr->lcfirst >= 65 && rowDataListTmpPtr->lcfirst <= 90)
                && !(rowDataListTmpPtr->lcfirst >= 97 && rowDataListTmpPtr->lcfirst <= 122)){
                    rowDataListTmpPtr->lcfirst = 0;
                    rowDataListTmpPtr->ucfirst = 0;
                }
            }
        }    

        while((splitItem = strtok(NULL, "\t")))
        {
            CREATE_ROW_DATA_ITEM(rowDataListTmpPtr);
            rowDataListTmpPtr->ori = py_strdup(splitItem, 0);
            rowDataListPtr->next = rowDataListTmpPtr;
            rowDataListPtr = rowDataListTmpPtr;      
            if (flag & (PINYIN_NONE|PINYIN_ASCII|PINYIN_LCFIRST|PINYIN_UCFIRST)) {
                for(m=0 ; m<PY_TONE_INFO_NUM; m++) {
                    if (NULL != (wordPtr=py_strstr(splitItem, toneInfos[m].complete))){
                        CHANGE_STR(tmpStr, splitItem, wordPtr, toneInfos[m].complete, toneInfos[m].simple, j, k);
                        rowDataListTmpPtr->none = py_strdup(tmpStr, 0);
                        rowDataListTmpPtr->tone = toneInfos[m].tone;
                        break;
                    }
                }
            }         
            if (flag & (PINYIN_LCFIRST|PINYIN_UCFIRST)){
                if (NULL != rowDataListTmpPtr->none) {
                    rowDataListTmpPtr->lcfirst = *rowDataListTmpPtr->none;
                    rowDataListTmpPtr->ucfirst = rowDataListTmpPtr->lcfirst - 32;
                    if (!(rowDataListTmpPtr->lcfirst >= 65 && rowDataListTmpPtr->lcfirst <= 90)
                    && !(rowDataListTmpPtr->lcfirst >= 97 && rowDataListTmpPtr->lcfirst <= 122)){
                        rowDataListTmpPtr->lcfirst = 0;
                        rowDataListTmpPtr->ucfirst = 0;
                    }
                }
            }   
        }
    }

	efree(chinese);
    zend_hash_destroy(Z_ARRVAL_P(pinyinPieces));
    efree(Z_ARRVAL_P(pinyinPieces));
    efree(pinyinPieces);

    return rowDataList;
}

void py_destory_row_list(py_row_data_list *list)
{
    py_row_data_list *ptr = list->next,
        *tmp = NULL;
    while (ptr != NULL) {
        if (NULL != ptr->ori)
            efree(ptr->ori);
        if (NULL != ptr->none)
            efree(ptr->none);
        tmp = ptr->next;
        efree(ptr);
        ptr = tmp;
    }
    efree(list);
}

PHP_INI_BEGIN()
    PHP_INI_ENTRY("pinyin.dir", "", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

PHP_FUNCTION(pinyin)
{
    char *chinese = NULL,
        tmp[100] = {0},
        str[2] = {0};
    size_t len;
    size_t l = PINYIN_UNICODE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &chinese, &len, &l) == FAILURE) {
        return;
    }
	
    py_row_data_list *list = py_split_sentence(chinese, l),
        *rowDataListPtr;

    array_init(return_value);
    rowDataListPtr = list->next;
    while(rowDataListPtr != NULL) {
        if (l & PINYIN_UNICODE) {
            py_add_next_index_string(return_value, rowDataListPtr->ori, 1);
        } else if (l & PINYIN_ASCII) {
            tmp[0] = 0;
            if (NULL != rowDataListPtr->none){
                strcat(tmp, rowDataListPtr->none);
                str[0] = rowDataListPtr->tone + 48;
                str[1] = 0;
                strcat(tmp, str);
                py_add_next_index_string(return_value, tmp, 1);
            } else {
                py_add_next_index_string(return_value, rowDataListPtr->ori, 1);
            }
        } else if (l & PINYIN_NONE) {
            if (NULL != rowDataListPtr->none){
                py_add_next_index_string(return_value, rowDataListPtr->none, 1);
            } else {
                py_add_next_index_string(return_value, rowDataListPtr->ori, 1);
            }
        } else if (l & PINYIN_UCFIRST) {
            if (rowDataListPtr->ucfirst > 0){
                str[0] = rowDataListPtr->ucfirst;
                str[1] = 0;
                py_add_next_index_string(return_value, str, 1);
            } else {
                py_add_next_index_string(return_value, rowDataListPtr->ori, 1);
            }
        } else if (l & PINYIN_LCFIRST) {
            if (rowDataListPtr->lcfirst > 0){
                str[0] = rowDataListPtr->lcfirst;
                str[1] = 0;
                py_add_next_index_string(return_value, str, 1);
            } else {
                py_add_next_index_string(return_value, rowDataListPtr->ori, 1);
            }
        }
        rowDataListPtr = rowDataListPtr->next;
    }
    py_destory_row_list(list);
}

PHP_MINIT_FUNCTION(pinyin)
{
    REGISTER_INI_ENTRIES();

    /* 数据配置地址 */
    const char *pinyinDir = NULL;

    /* 初始化全局变量 */
    PY_GLOBAL(can_access) = false;
    PY_GLOBAL(wordList) = (py_data_list *)py_malloc(sizeof(py_data_list), 1);
    PY_GLOBAL(wordList)->next = NULL;
    PY_GLOBAL(surnameList) = (py_data_list *)py_malloc(sizeof(py_data_list), 1);
    PY_GLOBAL(surnameList)->next = NULL;
    pinyinDir = INI_STR("pinyin.dir");

    if(strlen(pinyinDir) == 0 || access(pinyinDir, ACCESS_MODE_EXISTS))
    {
        php_error(E_WARNING, "汉字转拼音配置文件夹【%s】访问不了，或者未配置文件夹", pinyinDir);
    }else {
        py_fill_data_list(pinyinDir, 10);
        PY_GLOBAL(can_access) = true;
    }

    //注册常量
    REGISTER_LONG_CONSTANT("PINYIN_NONE", PINYIN_NONE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_UNICODE", PINYIN_UNICODE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_ISNAME", PINYIN_ISNAME, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_ASCII", PINYIN_ASCII, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_UCFIRST", PINYIN_UCFIRST, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("PINYIN_LCFIRST", PINYIN_LCFIRST, CONST_PERSISTENT | CONST_CS);

	return SUCCESS;
}
/* }}} */

/*
 * 清理内存工作
 */
PHP_MSHUTDOWN_FUNCTION(pinyin)
{
	UNREGISTER_INI_ENTRIES();

    py_data_list *tmp,
        *ptr;

    ptr = PY_GLOBAL(surnameList)->next;
    tmp = ptr;
    while(ptr != NULL)
    {
        tmp = ptr->next;
        free(ptr->key);
        free(ptr->val);
        free(ptr);
        ptr = tmp;
    }

    ptr = PY_GLOBAL(wordList)->next;
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

PHP_RINIT_FUNCTION(pinyin)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pinyin)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(pinyin)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pinyin support", "enabled");
	php_info_print_table_end();
}

const zend_function_entry pinyin_functions[] = {
	PHP_FE(pinyin,	NULL)		/* For testing, remove later. */
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
