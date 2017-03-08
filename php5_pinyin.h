//
// Created by RitoYan on 2017/3/1.
//

#ifndef PINYIN_PHP5_PINYIN_H
#define PINYIN_PHP5_PINYIN_H

#define py_add_next_index_string add_next_index_string

#define ZVAL_ARR(z, a) do {                     \
        zval *__z = (z);                        \
        Z_ARR_P(__z) = (a);                     \
        Z_TYPE_INFO_P(__z) = IS_ARRAY_EX;       \
    } while (0)
#define RETVAL_ARR(r) ZVAL_ARR(return_value, r)
		
#endif //PINYIN_PHP5_PINYIN_H
