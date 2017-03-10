//
// Created by RitoYan on 2017/3/1.
//

#ifndef PINYIN_PHP5_PINYIN_H
#define PINYIN_PHP5_PINYIN_H

#define py_add_next_index_string(arr, val, dup) add_next_index_string(arr, val, dup)
#define py_add_index_stringl(arr, index, str, len, dup) add_index_stringl(arr, index, str, len, dup) 
#define py_add_index_bool(arg, index, b) add_index_bool(arg, index, b)

#define PY_RETURN_ARR(arr) \
    Z_ARRVAL_P(return_value) = arr; \
    Z_TYPE_P(return_value) = IS_ARRAY;
		
#endif //PINYIN_PHP5_PINYIN_H
