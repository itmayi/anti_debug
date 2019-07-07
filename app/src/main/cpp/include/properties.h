//
// Created by yt on 19-7-2.
//

#ifndef ANTIDEBUG_PROPERTIES_H
#define ANTIDEBUG_PROPERTIES_H

#define PROP_NAME_MAX   32
#define PROP_VALUE_MAX  92

#define PROP_AREA_MAGIC   0x504f5250
#define PROP_AREA_VERSION 0xfc6ed0ab
#define PROP_AREA_VERSION_COMPAT 0x45434f76

/**
 * 头部区域数据结构
 *
 * /bionic/libc/include/sys/_system_properties.h
 */

struct prop_area {
    unsigned volatile count;
    unsigned volatile serial; //增加或者修改属性时用到的同步变量
    unsigned magic; //属性内存区魔数，用来识别属性内存区
    unsigned version;   //属性内存区版本号，用来识别属性内存区
    unsigned reserved[4];   //保留字段
    unsigned toc[1];    //属性内容索引表
};

/**
 * 属性值区域数据结构
 *
 * 属性值区域由一系列的熟悉值组成，每一个属性都通过结构体prop_info描述
 */
struct prop_info {
    char name[PROP_NAME_MAX];   //属性名称，长度最大为32个字节
    unsigned volatile serial;   //修改属性值时用到的同步变量，同时也用来描述属性值的长度
    char value[PROP_VALUE_MAX]; //属性值，长度最大为92个字节
};

#endif //ANTIDEBUG_PROPERTIES_H
