//
// Created by RitoYan on 2017/3/1.
//

#ifndef PINYIN_PHP5_PINYIN_H
#define PINYIN_PHP5_PINYIN_H

#define py_add_next_index_string(arr, val, dup) add_next_index_string(arr, val, dup)
#define py_add_index_stringl(arr, index, str, len, dup) add_index_stringl(arr, index, str, len, dup) 

#define PY_RETURN_ARR(arr) \
    array_init(return_value); \
    Z_ARRVAL_P(return_value) = arr;
		
#endif //PINYIN_PHP5_PINYIN_H
