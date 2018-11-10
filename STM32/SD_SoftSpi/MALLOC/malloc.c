#include "malloc.h"
#include "stdio.h"

static void mymemset ( void *s, INT8U c, INT32U count );
static void mymemcpy ( void *des, void *src, INT32U n );
static void mem_init ( void );
static INT8U mem_perused ( void );

__align ( 4 ) INT8U membase[MEM_MAX_SIZE]; /* SRAM内存池(4字节对齐) */
INT16U memmapbase[MEM_ALLOC_TABLE_SIZE]; /* 内存管理表(SRAM内存池MAP) */

/* 内存管理参数 */
static const INT32U memtblsize = MEM_ALLOC_TABLE_SIZE; /* 内存表大小 */
static const INT32U memblksize = MEM_BLOCK_SIZE; /* 内存分块大小 */
static const INT32U memsize = MEM_MAX_SIZE; /* 内存总大小 */

static _m_mallco_dev mallco_dev = { /* 内存管理控制器 */
    mem_init, /* 内存初始化 */
    mem_perused, /* 内存使用率 */
    membase, /* 内存池 */
    memmapbase, /* 内存管理状态表 */
    0, /* 内存管理未就绪 */
};

/* 复制内存，参数des是目的地址，src是源地址，n是需要复制的内存长度(以字节为单位) */
static void mymemcpy ( void *des, void *src, INT32U n ) {
    INT8U *xdes = des;
    INT8U *xsrc = src;

    while ( n-- ) {
        *xdes++ = *xsrc++;
    }
}

/* 设置内存。参数s是内存首地址，c是要设置的值，count是需要设置的内存大小(以字节为单位) */
static void mymemset ( void *s, INT8U c, INT32U count ) {
    INT8U *xs = s;

    while ( count-- ) {
        *xs++ = c;
    }
}

static void mem_init ( void ) { /* 内存管理初始化 */
    mymemset ( mallco_dev.memmap, 0, memtblsize * 2 ); /* 内存状态表数据清零 */
    mymemset ( mallco_dev.membase, 0, memsize ); /* 内存池所有数据清零 */
    mallco_dev.memrdy = 1; /* 内存管理初始化OK */
}

static INT8U mem_perused ( void ) { /* 获取内存使用率，返回0至100 */
    INT32U used = 0;
    INT32U i;

    for ( i = 0; i < memtblsize; i++ ) {
        if ( mallco_dev.memmap[i] ) {
            used++;
        }
    }

    return ( used * 100 ) / ( memtblsize );
}

/* 内存分配(内部调用)。参数memx是所属内存块，size是要分配的内存大小(字节)。返回0XFFFFFFFF代表错误，其他则表示内存偏移地址 */
static INT32U mem_malloc ( INT32U size ) {
    INT32S offset = 0;
    INT16U nmemb; /* 需要的内存块数 */
    INT16U cmemb = 0; /* 连续空内存块数 */
    INT8U i;

    if ( !mallco_dev.memrdy ) {
        mallco_dev.init(); /* 未初始化，先执行初始化 */
    }

    if ( size == 0 ) {
        return 0XFFFFFFFF; /* 不需要分配 */
    }

    nmemb = size / memblksize; /* 获取需要分配的连续内存块数 */

    if ( size % memblksize ) {
        nmemb++;
    }

    for ( offset = memtblsize - 1; offset >= 0; offset-- ) { /* 搜索整个内存控制区 */
        if ( !mallco_dev.memmap[offset] ) {
            cmemb++; /* 连续空内存块数增加 */
        } else {
            cmemb = 0; /* 连续内存块清零 */
        }

        if ( cmemb == nmemb ) { /* 找到了连续nmemb个空内存块 */
            for ( i = 0; i < nmemb; i++ ) { /* 标注内存块非空 */
                mallco_dev.memmap[offset + i] = nmemb;
            }

            return ( offset * memblksize ); /* 返回偏移地址 */
        }
    }

    return 0XFFFFFFFF; /* 未找到符合分配条件的内存块 */
}

static INT8U mem_free ( INT32U offset ) { /* 释放内存(内部调用)，参数offset是内存地址偏移。返回0表示释放成功，1则是失败 */
    int i;

    if ( !mallco_dev.memrdy ) { /* 如果未初始化，则先执行初始化 */
        mallco_dev.init();
        return 1; /* 未初始化 */
    }

    if ( offset < memsize ) { /* 偏移在内存池内 */
        int index = offset / memblksize; /* 偏移所在内存块号码 */
        int nmemb = mallco_dev.memmap[index]; /* 内存块数量 */

        for ( i = 0; i < nmemb; i++ ) { /* 内存块清零 */
            mallco_dev.memmap[index + i] = 0;
        }

        return 0;
    } else {
        return 1;
    }
}

void myfree ( void *ptr ) { /* 释放内存(外部调用)，参数ptr是内存首地址 */
    INT32U offset;

    if ( ptr == NULL ) {
        return;
    }

    offset = ( INT32U ) ptr - ( INT32U ) mallco_dev.membase;
    mem_free ( offset ); /* 释放内存 */
}

void *mymalloc ( INT32U size ) { /* 分配内存(外部调用)，参数size表示内存大小(字节)。返回分配到的内存首地址 */
    INT32U offset;
    offset = mem_malloc ( size );

    if ( offset == 0XFFFFFFFF ) {
        return NULL;
    } else {
        return ( void * ) ( ( INT32U ) mallco_dev.membase + offset );
    }
}

/* 重新分配内存(外部调用)，参数ptr是旧内存首地址，size是要分配的内存大小(字节)。返回新分配到的内存首地址 */
void *myrealloc ( void *ptr, INT32U size ) {
    INT32U offset;
    offset = mem_malloc ( size );

    if ( offset == 0XFFFFFFFF ) {
        return NULL;
    } else {
        mymemcpy ( ( void * ) ( ( INT32U ) mallco_dev.membase + offset ), ptr, size ); /* 拷贝旧内存内容到新内存 */
        myfree ( ptr ); /* 释放旧内存 */
        return ( void * ) ( ( INT32U ) mallco_dev.membase + offset ); /* 返回新内存首地址 */
    }
}