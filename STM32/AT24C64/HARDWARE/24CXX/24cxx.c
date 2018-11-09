#include "24cxx.h"
#include "delay.h"

/* 24CXX��������(�ʺ�24C01֮24C16��24C32��256δ�������ԣ��д���֤) */
void AT24CXX_Init ( void ) { /* ��ʼ��IIC�ӿ� */
    IIC_Init();
}

u8 AT24CXX_ReadOneByte ( u16 ReadAddr ) { /* ��AT24CXXָ����ַ����һ�����ݣ�����ReadAddr�ǵ�ַ */
    u8 temp = 0;
    IIC_Start();

    if ( EE_TYPE > AT24C16 ) {
        IIC_Send_Byte ( 0XA0 ); /* ����д���� */
        IIC_Wait_Ack();
        IIC_Send_Byte ( ReadAddr >> 8 ); /* ���͸ߵ�ַ */
    } else {
        IIC_Send_Byte ( 0XA0 + ( ( ReadAddr / 256 ) << 1 ) ); /* ����������ַ0XA0��д���� */
    }

    IIC_Wait_Ack();
    IIC_Send_Byte ( ReadAddr % 256 ); /* ���͵͵�ַ */
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte ( 0XA1 ); /* �������ģʽ */
    IIC_Wait_Ack();
    temp = IIC_Read_Byte ( 0 );
    IIC_Stop(); /* ����һ��ֹͣ���� */
    return temp;
}

/* ��AT24CXXָ����ַд��һ�����ݣ�����WriteAddr�ǵ�ַ��DataToWrite������ */
void AT24CXX_WriteOneByte ( u16 WriteAddr, u8 DataToWrite ) {
    IIC_Start();

    if ( EE_TYPE > AT24C16 ) {
        IIC_Send_Byte ( 0XA0 ); /* ����д���� */
        IIC_Wait_Ack();
        IIC_Send_Byte ( WriteAddr >> 8 ); /* ���͸ߵ�ַ */
    } else {
        IIC_Send_Byte ( 0XA0 + ( ( WriteAddr / 256 ) << 1 ) ); /* ����������ַ0XA0��д���� */
    }

    IIC_Wait_Ack();
    IIC_Send_Byte ( WriteAddr % 256 ); /* ���͵͵�ַ */
    IIC_Wait_Ack();
    IIC_Send_Byte ( DataToWrite ); /* �����ֽ� */
    IIC_Wait_Ack();
    IIC_Stop(); /* ����һ��ֹͣ���� */
    delay_ms ( 10 );
}

/* ��AT24CXXָ����ַд�볤��ΪLen�����ݣ�����д��16bit����32bit�����ݡ�����WriteAddr�ǵ�ַ��DataToWrite�����������׵�ַ��Len��Ҫд�����ݵĳ���(2��4) */
void AT24CXX_WriteLenByte ( u16 WriteAddr, u32 DataToWrite, u8 Len ) {
    u8 t;

    for ( t = 0; t < Len; t++ ) {
        AT24CXX_WriteOneByte ( WriteAddr + t, ( DataToWrite >> ( 8 * t ) ) & 0xff );
    }
}

/* ��AT24CXXָ����ַ��������ΪLen�����ݣ����ڶ���16bit����32bit�����ݡ�����ReadAddr�ǵ�ַ��Len�Ƕ������ݵĳ���(2��4) */
u32 AT24CXX_ReadLenByte ( u16 ReadAddr, u8 Len ) {
    u8 t;
    u32 temp = 0;

    for ( t = 0; t < Len; t++ ) {
        temp <<= 8;
        temp += AT24CXX_ReadOneByte ( ReadAddr + Len - t - 1 );
    }

    return temp;
}

/* ���AT24CXX�Ƿ���������������24XX�����һ����ַ(255)���洢��־��(���������24Cϵ�У������ַҪ�޸�)������1��ʾ���ʧ�ܣ�����0��ʾ���ɹ� */
u8 AT24CXX_Check ( void ) {
    u8 temp;
    temp = AT24CXX_ReadOneByte ( EE_TYPE );

    if ( temp == 0X55 ) {
        return 0;
    } else { /* �ų���һ�γ�ʼ������� */
        AT24CXX_WriteOneByte ( EE_TYPE, 0X55 );
        temp = AT24CXX_ReadOneByte ( EE_TYPE );

        if ( temp == 0X55 ) {
            return 0;
        }
    }

    return 1;
}

/* ��AT24CXXָ����ַ��ʼ����ָ�����������ݡ�����ReadAddr�ǵ�ַ����24c02Ϊ0��255��pBuffer�����������׵�ַ��NumToRead��Ҫ�������ݵĸ��� */
void AT24CXX_Read ( u16 ReadAddr, u8 *pBuffer, u16 NumToRead ) {
    while ( NumToRead ) {
        *pBuffer++ = AT24CXX_ReadOneByte ( ReadAddr++ );
        NumToRead--;
    }
}

/* ��AT24CXXָ����ַ��ʼд��ָ�����������ݡ�����WriteAddr�ǵ�ַ����24c02Ϊ0��255��pBuffer�����������׵�ַ��NumToWrite��Ҫд�����ݵĸ��� */
void AT24CXX_Write ( u16 WriteAddr, u8 *pBuffer, u16 NumToWrite ) {
    while ( NumToWrite-- ) {
        AT24CXX_WriteOneByte ( WriteAddr, *pBuffer );
        WriteAddr++;
        pBuffer++;
    }
}