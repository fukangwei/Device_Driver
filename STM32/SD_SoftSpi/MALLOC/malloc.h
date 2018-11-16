#ifndef __MALLOC_H_
#define __MALLOC_H_
#include "os_def.h"

#define MALLOC(num,type) (type*)mymalloc((num) * sizeof(type)) /* ����malloc�½ӿ� */

/* �ڴ�������ͷŽӿ� */
void myfree ( void *ptr );
void *mymalloc ( INT32U size );
void *myrealloc ( void *ptr, INT32U size );

/* �ڴ�����趨 */
#define MEM_BLOCK_SIZE 8 /* �ڴ���СΪ8�ֽ� */
#define MEM_MAX_SIZE 10 * 1024 /* �������ڴ�10K */
#define MEM_ALLOC_TABLE_SIZE MEM_MAX_SIZE/MEM_BLOCK_SIZE /* �ڴ���С */

typedef struct _m_mallco_dev { /* �ڴ��������� */
    void ( *init ) ( void ); /* ��ʼ�� */
    INT8U ( *perused ) ( void ); /* �ڴ�ʹ���� */
    INT8U *membase; /* �ڴ�� */
    INT16U *memmap; /* �ڴ����״̬�� */
    INT8U memrdy; /* �ڴ�����Ƿ���� */
} _m_mallco_dev;

#endif