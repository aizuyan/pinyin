# pinyin
php 汉字转pinyin 扩展形式的

###
fcgi模式常驻内存，速度很快

### 配置
这里面需要在/path/to/php.ini中配置`pinyin.dir=/path/to/pinyindir`，配置的路径为数据文件存放的目录。
目录下面的文件分为两种：

一种是姓氏文件，只有一个，名称为`surnames`

另一种为普通句子和汉字文件，名称为`words_0`，`words_1`, ..... , `words_9` ，最多10个，越靠前的是越常用的词组

### 使用
这里用了最简洁的方式，用了一个函数`chinese_to_pinyin(char *str, int flags)`，根据不同的参数，转换为不用的形式
> PINYIN_NONE    拼音不加音调
> PINYIN_UNICODE    拼音加音调
> PINYIN_ISNAME     要转化的内容为名字
> PINYIN_TRIM       简洁模式，去掉所有的标点符号
> PINYIN_FORMAT_EN  将标点符号全转化为英文格式
> PINYIN_FORMAT_CH     将标点符号全转化中文格式

#### PINYIN_NONE 不带音调
```php
print_r(chinese_to_pinyin("你因为穷用盗版的时候至少要知道自己是不对的，这说明你还有救。", PINYIN_NONE));
```
输出结果：

```
Array
(
    [0] => ni
    [1] => yin
    [2] => wei
    [3] => qiong
    [4] => yong
    [5] => dao
    [6] => ban
    [7] => de
    [8] => shi
    [9] => hou
    [10] => zhi
    [11] => shao
    [12] => yao
    [13] => zhi
    [14] => dao
    [15] => zi
    [16] => ji
    [17] => shi
    [18] => bu
    [19] => dui
    [20] => de，
    [21] => zhe
    [22] => shuo
    [23] => ming
    [24] => ni
    [25] => hai
    [26] => you
    [27] => jiu。
)
```

#### 不带音调和格式化标点符号
```php
print_r(chinese_to_pinyin("你因为穷用盗版的时候至少要知道自己是不对的，这说明你还有救。", PINYIN_NONE|PINYIN_TRIM));
```

结果如下，可以看出标点符号全过滤掉了
```
Array
(
    [0] => ni
    [1] => yin
    [2] => wei
    [3] => qiong
    [4] => yong
    [5] => dao
    [6] => ban
    [7] => de
    [8] => shi
    [9] => hou
    [10] => zhi
    [11] => shao
    [12] => yao
    [13] => zhi
    [14] => dao
    [15] => zi
    [16] => ji
    [17] => shi
    [18] => bu
    [19] => dui
    [20] => de
    [21] => zhe
    [22] => shuo
    [23] => ming
    [24] => ni
    [25] => hai
    [26] => you
    [27] => jiu
)
```

#### 带音调和格式化标点
```php
print_r(chinese_to_pinyin("你因为穷用盗版的时候至少要知道自己是不对的，这说明你还有救。", PINYIN_UNICODE|PINYIN_FORMAT_CH));
```

输出结果如下，标点符号也输出了
```
Array
(
    [0] => nǐ
    [1] => yīn
    [2] => wèi
    [3] => qióng
    [4] => yòng
    [5] => dào
    [6] => bǎn
    [7] => de
    [8] => shí
    [9] => hòu
    [10] => zhì
    [11] => shǎo
    [12] => yào
    [13] => zhī
    [14] => dào
    [15] => zì
    [16] => jǐ
    [17] => shì
    [18] => bú
    [19] => duì
    [20] => de
    [21] => ，
    [22] => zhè
    [23] => shuō
    [24] => míng
    [25] => nǐ
    [26] => hái
    [27] => yǒu
    [28] => jiù
    [29] => 。
)
```

#### 姓名
这里使用了几个朋友的名称（比较有特色的名字）
```php
print_r(chinese_to_pinyin("冼佩君", PINYIN_ISNAME));
print_r(chinese_to_pinyin("袁旭东", PINYIN_ISNAME));
print_r(chinese_to_pinyin("燕睿涛", PINYIN_ISNAME));
print_r(chinese_to_pinyin("单净净", PINYIN_ISNAME));
```

```
Array
(
    [0] => xiǎn
    [1] => pèi
    [2] => jūn
)
Array
(
    [0] => yuán
    [1] => xù
    [2] => dōng
)
Array
(
    [0] => yān
    [1] => ruì
    [2] => tāo
)
Array
(
    [0] => shàn
    [1] => jìng
    [2] => jìng
)
```
