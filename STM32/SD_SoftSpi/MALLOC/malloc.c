#include "malloc.h"
#include "stdio.h"

static void mymemset ( void *s, INT8U c, INT32U count );
static void mymemcpy ( void *des, void *src, INT32U n );
static void mem_init ( void );
static INT8U mem_perused ( void );

__align ( 4 ) INT8U membase[MEM_MAX_SIZE]; /* SRAM�ڴ��(4�ֽڶ���) */
INT16U memmapbase[MEM_ALLOC_TABLE_SIZE]; /* �ڴ�����(SRAM�ڴ��MAP) */

/* �ڴ������� */
static const INT32U memtblsize = MEM_ALLOC_TABLE_SIZE; /* �ڴ���С */
static const INT32U memblksize = MEM_BLOCK_SIZE; /* �ڴ�ֿ��С */
static const INT32U memsize = MEM_MAX_SIZE; /* �ڴ��ܴ�С */

static _m_mallco_dev mallco_dev = { /* �ڴ��������� */
    mem_init, /* �ڴ��ʼ�� */
    mem_perused, /* �ڴ�ʹ���� */
    membase, /* �ڴ�� */
    memmapbase, /* �ڴ����״̬�� */
    0, /* �ڴ����δ���� */
};

/* �����ڴ棬����des��Ŀ�ĵ�ַ��src��Դ��ַ��n����Ҫ���Ƶ��ڴ泤��(���ֽ�Ϊ��λ) */
static void mymemcpy ( void *des, void *src, INT32U n ) {
    INT8U *xdes = des;
    INT8U *xsrc = src;

    while ( n-- ) {
        *xdes++ = *xsrc++;
    }
}

/* �����ڴ档����s���ڴ��׵�ַ��c��Ҫ���õ�ֵ��count����Ҫ���õ��ڴ��С(���ֽ�Ϊ��λ) */
static void mymemset ( void *s, INT8U c, INT32U count ) {
    INT8U *xs = s;

    while ( count-- ) {
        *xs++ = c;
    }
}

static void mem_init ( void ) { /* �ڴ�����ʼ�� */
    mymemset ( mallco_dev.memmap, 0, memtblsize * 2 ); /* �ڴ�״̬���������� */
    mymemset ( mallco_dev.membase, 0, memsize ); /* �ڴ�������������� */
    mallco_dev.memrdy = 1; /* �ڴ�����ʼ��OK */
}

static INT8U mem_perused ( void ) { /* ��ȡ�ڴ�ʹ���ʣ�����0��100 */
    INT32U used = 0;
    INT32U i;

    for ( i = 0; i < memtblsize; i++ ) {
        if ( mallco_dev.memmap[i] ) {
            used++;
        }
    }

    return ( used * 100 ) / ( memtblsize );
}

/* �ڴ����(�ڲ�����)������memx�������ڴ�飬size��Ҫ������ڴ��С(�ֽ�)������0XFFFFFFFF��������������ʾ�ڴ�ƫ�Ƶ�ַ */
static INT32U mem_malloc ( INT32U size ) {
    INT32S offset = 0;
    INT16U nmemb; /* ��Ҫ���ڴ���� */
    INT16U cmemb = 0; /* �������ڴ���� */
    INT8U i;

    if ( !mallco_dev.memrdy ) {
        mallco_dev.init(); /* δ��ʼ������ִ�г�ʼ�� */
    }

    if ( size == 0 ) {
        return 0XFFFFFFFF; /* ����Ҫ���� */
    }

    nmemb = size / memblksize; /* ��ȡ��Ҫ����������ڴ���� */

    if ( size % memblksize ) {
        nmemb++;
    }

    for ( offset = memtblsize - 1; offset >= 0; offset-- ) { /* ���������ڴ������ */
        if ( !mallco_dev.memmap[offset] ) {
            cmemb++; /* �������ڴ�������� */
        } else {
            cmemb = 0; /* �����ڴ������ */
        }

        if ( cmemb == nmemb ) { /* �ҵ�������nmemb�����ڴ�� */
            for ( i = 0; i < nmemb; i++ ) { /* ��ע�ڴ��ǿ� */
                mallco_dev.memmap[offset + i] = nmemb;
            }

            return ( offset * memblksize ); /* ����ƫ�Ƶ�ַ */
        }
    }

    return 0XFFFFFFFF; /* δ�ҵ����Ϸ����������ڴ�� */
}

static INT8U mem_free ( INT32U offset ) { /* �ͷ��ڴ�(�ڲ�����)������offset���ڴ��ַƫ�ơ�����0��ʾ�ͷųɹ���1����ʧ�� */
    int i;

    if ( !mallco_dev.memrdy ) { /* ���δ��ʼ��������ִ�г�ʼ�� */
        mallco_dev.init();
        return 1; /* δ��ʼ�� */
    }

    if ( offset < memsize ) { /* ƫ�����ڴ���� */
        int index = offset / memblksize; /* ƫ�������ڴ����� */
        int nmemb = mallco_dev.memmap[index]; /* �ڴ������ */

        for ( i = 0; i < nmemb; i++ ) { /* �ڴ������ */
            mallco_dev.memmap[index + i] = 0;
        }

        return 0;
    } else {
        return 1;
    }
}

void myfree ( void *ptr ) { /* �ͷ��ڴ�(�ⲿ����)������ptr���ڴ��׵�ַ */
    INT32U offset;

    if ( ptr == NULL ) {
        return;
    }

    offset = ( INT32U ) ptr - ( INT32U ) mallco_dev.membase;
    mem_free ( offset ); /* �ͷ��ڴ� */
}

void *mymalloc ( INT32U size ) { /* �����ڴ�(�ⲿ����)������size��ʾ�ڴ��С(�ֽ�)�����ط��䵽���ڴ��׵�ַ */
    INT32U offset;
    offset = mem_malloc ( size );

    if ( offset == 0XFFFFFFFF ) {
        return NULL;
    } else {
        return ( void * ) ( ( INT32U ) mallco_dev.membase + offset );
    }
}

/* ���·����ڴ�(�ⲿ����)������ptr�Ǿ��ڴ��׵�ַ��size��Ҫ������ڴ��С(�ֽ�)�������·��䵽���ڴ��׵�ַ */
void *myrealloc ( void *ptr, INT32U size ) {
    INT32U offset;
    offset = mem_malloc ( size );

    if ( offset == 0XFFFFFFFF ) {
        return NULL;
    } else {
        mymemcpy ( ( void * ) ( ( INT32U ) mallco_dev.membase + offset ), ptr, size ); /* �������ڴ����ݵ����ڴ� */
        myfree ( ptr ); /* �ͷž��ڴ� */
        return ( void * ) ( ( INT32U ) mallco_dev.membase + offset ); /* �������ڴ��׵�ַ */
    }
}