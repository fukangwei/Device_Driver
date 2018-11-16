#ifndef __MALLOC_H_
#define __MALLOC_H_
#include "os_def.h"

#define MALLOC(num,type) (type*)mymalloc((num) * sizeof(type)) /* 定义malloc新接口 */

/* 内存分配与释放接口 */
void myfree ( void *ptr );
void *mymalloc ( INT32U size );
void *myrealloc ( void *ptr, INT32U size );

/* 内存参数设定 */
#define MEM_BLOCK_SIZE 8 /* 内存块大小为8字节 */
#define MEM_MAX_SIZE 10 * 1024 /* 最大管理内存10K */
#define MEM_ALLOC_TABLE_SIZE MEM_MAX_SIZE/MEM_BLOCK_SIZE /* 内存表大小 */

typedef struct _m_mallco_dev { /* 内存管理控制器 */
    void ( *init ) ( void ); /* 初始化 */
    INT8U ( *perused ) ( void ); /* 内存使用率 */
    INT8U *membase; /* 内存池 */
    INT16U *memmap; /* 内存管理状态表 */
    INT8U memrdy; /* 内存管理是否就绪 */
} _m_mallco_dev;

#endif