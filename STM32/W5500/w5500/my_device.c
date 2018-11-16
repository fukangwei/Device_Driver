#include "config.h"
#include "device.h"
#include "string.h"
#include "w5500.h"

CONFIG_MSG ConfigMsg, RecvMsg;

uint8 txsize[MAX_SOCK_NUM] = {2, 2, 2, 2, 2, 2, 2, 2};
uint8 rxsize[MAX_SOCK_NUM] = {2, 2, 2, 2, 2, 2, 2, 2};

void set_default ( void ) {
    uint8 mac[6] = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11};
    uint8 lip[4] = {192, 168, 1, 230};
    uint8 sub[4] = {255, 255, 255, 0};
    uint8 gw[4] = {192, 168, 1, 1};
    uint8 dns[4] = {114, 114, 114, 114};
    memcpy ( ConfigMsg.lip, lip, 4 );
    memcpy ( ConfigMsg.sub, sub, 4 );
    memcpy ( ConfigMsg.gw,  gw, 4 );
    memcpy ( ConfigMsg.mac, mac, 6 );
    memcpy ( ConfigMsg.dns, dns, 4 );
    ConfigMsg.dhcp = 0;
    ConfigMsg.debug = 1;
    ConfigMsg.fw_len = 0;
    ConfigMsg.state = NORMAL_STATE;
    ConfigMsg.sw_ver[0] = FW_VER_HIGH;
    ConfigMsg.sw_ver[1] = FW_VER_LOW;
}

void set_network ( void ) {
    uint8 ip[4];
    setSHAR ( ConfigMsg.mac ); /* 配置Mac地址 */
    setSUBR ( ConfigMsg.sub ); /* 配置子网掩码 */
    setGAR ( ConfigMsg.gw ); /* 配置默认网关 */
    setSIPR ( ConfigMsg.lip ); /* 配置Ip地址 */
    sysinit ( txsize, rxsize ); /* 初始化8个socket */
    setRTR ( 2000 ); /* 设置溢出时间值 */
    setRCR ( 3 ); /* 设置最大重新发送次数 */
    getSIPR ( ip );
    printf ( "IP : %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3] );
    getSUBR ( ip );
    printf ( "SN : %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3] );
    getGAR ( ip );
    printf ( "GW : %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3] );
}