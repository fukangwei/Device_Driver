#ifndef _TYPE_H_
#define _TYPE_H_

#define MAX_SOCK_NUM 8 /* Maxmium number of socket */

/* __DEF_IINCHIP_MAP_xxx__: define memory map for iinchip */
#define __DEF_IINCHIP_MAP_BASE__ 0x0000
#define COMMON_BASE 0x0000
#define __DEF_IINCHIP_MAP_TXBUF__ (COMMON_BASE + 0x8000) /* Internal Tx buffer address of the iinchip */
#define __DEF_IINCHIP_MAP_RXBUF__ (COMMON_BASE + 0xC000) /* Internal Rx buffer address of the iinchip */

#define IINCHIP_ISR_DISABLE()
#define IINCHIP_ISR_ENABLE()

#ifndef NULL
    #define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
    #define _SIZE_T
    typedef unsigned int size_t;
#endif

typedef char int8;
typedef volatile char vint8;
typedef unsigned char uint8;
typedef volatile unsigned char vuint8;
typedef int int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

typedef uint8 u_char;
typedef uint8 SOCKET;
typedef uint16 u_short;
typedef uint16 u_int;
typedef uint32 u_long;

typedef union _un_l2cval {
    u_long lVal;
    u_char cVal[4];
} un_l2cval;

typedef union _un_i2cval {
    u_int iVal;
    u_char cVal[2];
} un_i2cval;
#endif