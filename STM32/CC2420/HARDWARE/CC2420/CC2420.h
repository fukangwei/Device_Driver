#ifndef _CC2420_H_
#define _CC2420_H_
#include "public.h"

#define CC2420_PSDU_Len 127
/* SPI口访问方式定义 */
#define REG_READ  0x40
#define REG_WRITE 0x00
#define RAM       0x80
#define RAM_READ  0x20
#define RAM_WRITE 0x00
/* CC2420 命令字定义 */
#define CMD_SNOP              (0x00      )
#define CMD_SXOSCON           (0x01      )
#define CMD_STXCAL            (0x02      )
#define CMD_SRXON             (0x03      )
#define CMD_STXON             (0x04      )
#define CMD_STXONCCA          (0x05      )
#define CMD_SRFOFF            (0x06      )
#define CMD_SXOSCOFF          (0x07      )
#define CMD_SFLUSHRX          (0x08      )
#define CMD_SFLUSHTX          (0x09      )
#define CMD_SACK              (0x0a      )
#define CMD_SACKPEND          (0x0b      )
#define CMD_SRXDEC            (0x0c      )
#define CMD_STXENC            (0x0d      )
#define CMD_SAES              (0x0e      )
/* CC2420配置寄存器 */
#define REG_MAIN              (0x10      )
#define RESETN                (0x01 << 15)
#define ENC_RESETN            (0x01 << 14)
#define DEMOD_RESETN          (0x01 << 13)
#define MOD_RESETN            (0x01 << 12)
#define FS_RESETN             (0x01 << 11)
#define XOSC16M_BYPASS        (0x01 << 0 )
#define REG_MDMCTRL0          (0x11      )
#define RESERVED_FRAME_MODE   (0x00 << 13)
#define PAN_COORDINATOR       (0x01 << 12)
#define ADR_DECODE            (0x01 << 11)
#define CCA_HYST              (0x02 << 8 )
#define CCA_MODE              (0x03 << 6 )
#define AUTOCRC               (0x01 << 5 )
#define AUTOACK               (     << 4 )
#define PREAMBLE_LENGTH       (0x02 << 0 )
#define REG_MDMCTRL1          (0x12      )
#define CORR_THR              (0x20 << 6 )
#define DEMOD_AVG_MODE        (     << 5 )
#define MODULATION_MODE       (     << 4 )
#define TX_MODE               (     << 2 )
#define RX_MODE               (     << 0 )
#define REG_RSSI              (0x13      )
#define REG_SYNCWORD          (0x14      )
#define SYNCWORD              (0xA70F    )
#define REG_TXCTRL            (0x15      )
#define TXMIXBUF_CUR          (     << 14)
#define TX_TURNAROUND         (     << 13)
#define TXMIX_CAP_ARRAY       (     << 11)
#define TXMIX_CURRENT         (     << 9 )
#define PA_CURRENT            (     << 6 )
#define PA_LEVEL              (     << 0 )
#define REG_RXCTRL0           (0x16      )
#define RXMIXBUF_CUR          (     << 12)
#define HIGH_LNA_GAIN         (     << 10)
#define MED_LNA_GAIN          (     << 8 )
#define LOW_LNA_GAIN          (     << 6 )
#define HIGH_LNA_CURRENT      (     << 4 )
#define MED_LNA_CURRENT       (     << 2 )
#define LOW_LNA_CURRENT       (     << 0 )
#define REG_RXCTRL1           (0x17      )
#define RXBPF_LOCUR           (     << 13)
#define RXBPF_MIDCUR          (     << 12)
#define LOW_LOWGAIN           (     << 11)
#define MED_LOWGAIN           (     << 10)
#define HIGH_HGM              (     << 9 )
#define MED_HGM               (     << 8 )
#define LNA_CAP_ARRAY         (     << 6 )
#define RXMIX_TAIL            (     << 4 )
#define RXMIX_VCM             (     << 2 )
#define RXMIX_CURRENT         (     << 0 )
#define REG_FSCTRL            (0x18      )
#define LOCK_THR              (     << 14)
#define CAL_DONE              (     << 13)
#define CAL_RUNNING           (     << 12)
#define LOCK_LENGTH           (     << 11)
#define LOCK_STATUS           (     << 10)
#define FREQ                  (     << 0 )
#define REG_SECCTRL0          (0x19      )
#define RXFIFO_PROTECTION     (     << 9 )
#define SEC_CBC_HEAD          (     << 8 )
#define SEC_SAKEYSEL          (     << 7 )
#define SEC_TXKEYSEL          (     << 6 )
#define SEC_RXKEYSEL          (     << 5 )
#define SEC_M                 (     << 2 )
#define SEC_MODE              (     << 0 )
#define REG_SECCTRL1          (0x1a      )
#define SEC_TXL               (     << 8 )
#define SEC_RXL               (     << 0 )
#define REG_BATTMON           (0x1b      )
#define BATTMON_EN            (     << 5 )
#define BATTMON_VOLTAGE       (     << 0 )
#define REG_IOCFG0            (0x1c      )
#define BCN_ACCEPT            (     << 11)
#define FIFO_POLARITY         (     << 10)
#define FIFOP_POLARITY        (     << 9 )
#define SFD_POLARITY          (     << 8 )
#define CCA_POLARITY          (     << 7 )
#define FIFOP_THR             (     << 0 )
#define REG_IOCFG1            (0x1d      )
#define HSSD_SRC              (     << 10)
#define SFDMUX                (     << 5 )
#define CCAMUX                (     << 0 )
#define REG_MANFIDL           (0x1e      )
#define REG_MANFIDH           (0x1f      )
#define REG_FSMTC             (0x20      )
#define TC_RXCHAIN2RX         (     << 13)
#define TC_SWITCH2TX          (     << 10)
#define TC_PAON2TX            (     << 6 )
#define TC_TXEND2SWITCH       (     << 3 )
#define TC_TXEND2PAOFF        (     << 0 )
#define REG_MANAND            (0x21      )
#define MANAND_VGA_RESET_N    (     << 15)
#define MANAND_BIAS_PD        (     << 14)
#define MANAND_BALUN_CTRL     (     << 13)
#define MANAND_RXTX           (     << 12)
#define MANAND_PRE_PD         (     << 11)
#define MANAND_PA_N_PD        (     << 10)
#define MANAND_PA_P_PD        (     << 9 )
#define MANAND_DAC_LPF_PD     (     << 8 )
#define MANAND_XOSC16M_PD     (     << 7 )
#define MANAND_RXBPF_CAL_PD   (     << 6 )
#define MANAND_CHP_PD         (     << 5 )
#define MANAND_FS_PD          (     << 4 )
#define MANAND_ADC_PD         (     << 3 )
#define MANAND_VGA_PD         (     << 2 )
#define MANAND_RXBPF_PD       (     << 1 )
#define MANAND_LNAMIX_PD      (     << 0 )
#define REG_MANOR             (0x22      )
#define MANOR_VGA_RESET_N     (     << 15)
#define MANOR_BIAS_PD         (     << 14)
#define MANOR_BALUN_CTRL      (     << 13)
#define MANOR_RXTX            (     << 12)
#define MANOR_PRE_PD          (     << 11)
#define MANOR_PA_N_PD         (     << 10)
#define MANOR_PA_P_PD         (     << 9 )
#define MANOR_DAC_LPF_PD      (     << 8 )
#define MANOR_XOSC16M_PD      (     << 7 )
#define MANOR_RXBPF_CAL_PD    (     << 6 )
#define MANOR_CHP_PD          (     << 5 )
#define MANOR_FS_PD           (     << 4 )
#define MANOR_ADC_PD          (     << 3 )
#define MANOR_VGA_PD          (     << 2 )
#define MANOR_RXBPF_PD        (     << 1 )
#define MANOR_LNAMIX_PD       (     << 0 )
#define REG_AGCCTRL           (0x23      )
#define VGA_GAIN_OE           (     << 11)
#define VGA_GAIN              (     << 4 )
#define LNAMIX_GAINMODE_O     (     << 2 )
#define REG_AGCTST0           (0x24      )
#define LNAMIX_HYST           (     << 12)
#define LNAMIX_THR_H          (     << 6 )
#define LNAMIX_THR_L          (     << 0 )
#define REG_AGCTST1           (0x25      )
#define AGC_BLANK_MODE        (     << 14)
#define PEAKDET_CUR_BOOST     (     << 13)
#define AGC_SETTLE_WAIT       (     << 11)
#define AGC_PEAK_DET_MODE     (     << 8 )
#define AGC_WIN_SIZE          (     << 6 )
#define AGC_REF               (     << 0 )
#define REG_AGCTST2           (0x26      )
#define MED2HIGHGAIN          (     << 5 )
#define LOW2MEDGAIN           (     << 0 )
#define REG_FSTST0            (0x27      )
#define VCO_ARRAY_SETTLE_LONG (     << 11)
#define VCO_ARRAY_OE          (     << 10)
#define VCO_ARRAY_O           (     << 5 )
#define VCO_ARRAY_RES         (     << 0 )
#define REG_FSTST1            (0x28      )
#define VCO_TX_NOCAL          (     << 15)
#define VCO_ARRAY_CAL_LONG    (     << 14)
#define VCO_CURRENT_REF       (     << 10)
#define VCO_CURRENT_K         (     << 4 )
#define VC_DAC_EN             (     << 3 )
#define VC_DAC_VAL            (     << 0 )
#define REG_FSTST2            (0x29      )
#define VCO_CURCAL_SPEED      (     << 13)
#define VCO_CURRENT_OE        (     << 12)
#define VCO_CURRENT_O         (     << 6 )
#define VCO_CURRENT_RES       (     << 0 )
#define REG_FSTST3            (0x2a      )
#define CHP_CAL_DISABLE       (     << 15)
#define CHP_CURRENT_OE        (     << 14)
#define CHP_TEST_UP           (     << 13)
#define CHP_TEST_DN           (     << 12)
#define CHP_DISABLE           (     << 11)
#define PD_DELAY              (     << 10)
#define CHP_STEP_PERIOD       (     << 8 )
#define STOP_CHP_CURRENT      (     << 4 )
#define START_CHP_CURRENT     (     << 0 )
#define REG_RXBPFTST          (0x2b      )
#define RXBPF_CAP_OE          (     << 14)
#define RXBPF_CAP_O           (     << 7 )
#define REG_FSMSTATE          (0x2c      )
#define REG_ADCTST            (0x2d      )
#define REG_DACTST            (0x2e      )
#define DAC_SRC               (     << 12)
#define DAC_I_O               (     << 6 )
#define DAC_Q_O               (     << 0 )
#define REG_TOPTST            (0x2f      )
#define RAM_BIST_RUN          (     << 7 )
#define TEST_BATTMON_EN       (     << 6 )
#define VC_IN_TEST_EN         (     << 5 )
#define ATESTMOD_PD           (     << 4 )
#define ATESTMOD_MODE         (     << 0 )
#define REG_TXFIFO            (0x3e      )
#define REG_RXFIFO            (0x3f      )
#define RAM_IEEEADR           (0x60      )
#define RAM_PANID             (0x68      )
#define RAM_SHORTADR          (0x6A      )
/* PSDU相关定义 */
#define FRAME_TYPE_BEACCON     0x00
#define FRAME_TYPE_DATA        0x01
#define FRAME_TYPE_ACK         0x02
#define FRAME_TYPE_MAC         0x03
#define SECURITY_ENABLE        0x00
#define FRAME_PENDING          0x00
#define ACKNOWLEDGMENT_REQUEST 0x00
#define PAN_ID_COMPRESSION     0x00
#define DEST_ADDRESSING_MODE   0x03
#define FRAME_VERSION          0x01
#define SOURCE_ADDRESSING_MODE 0x03
#define SEQUENCE_NUMBER        0x02
/* CC2420引脚定义 */
#define  RESET    PAout(0)
#define  SFD      PAin(1)
#define  CCA      PAin(2)
#define  FIFOP    PAin(3)
#define  FIFO     PAin(4)
#define  CSN      PAout(5)
#define  SCLK     PAout(6)
#define  MOSI     PAout(7)
#define  MISO     PAin(8)
#define  VREG_EN  PAout(9)

#define RESET_ON()  RESET = 1
#define RESET_OFF() RESET = 0
#define SCLK_ON()   SCLK = 1
#define SCLK_OFF()  SCLK = 0
#define MOSI_ON()   MOSI = 1
#define MOSI_OFF()  MOSI = 0
#define CSN_ON()    CSN = 1
#define CSN_OFF()   CSN = 0
#define FIFO_IN     FIFO
#define MISO_IN     MISO
#define CCA_IN      CCA
#define SFD_IN      SFD
#define FIFOP_IN    FIFOP

void CC2420_IOinit ( void );
void SPI_Init ( void );
uint8 SPI_Read ( void );
void SPI_Write ( uint8 txdata );
uint8 SPI_WriteRead ( uint8 txdata );
uint16 SPI_Word_Read ( void );
void SPI_Word_Write ( uint16 txdata );
void CC2420_Init ( void );
uint16 CC2420_ReadReg ( uint8  addr );
void  CC2420_WriteReg ( uint8  addr, uint16 value );
void  CC2420_Command ( uint8 cmd );
uint8 CC2420_RAM_Read ( uint8 addr, uint8 block );
void CC2420_RAM_Write ( uint8  addr, uint8 block, uint8 value );
void CC2420_SetRxMode ( void );
uint8 CC2420_RxPacket ( void ) ;
void CC2420_TxPacket ( void );
void CC2420_ReadRXFIFO ( void );
void CC2420_WriteTXFIFO ( void );

#define UART_READ_BACK   0x01
#define CC2420_REG_READ  0xE0
#define CC2420_REG_WRITE 0xE1
#define CC2420_RAM_READ  0xE2
#define CC2420_RAM_WRITE 0xE3
#define CC2420_TX        0xE4
#define CC2420_RX        0xE5
#define CC2420_SETRX     0xE6
#define CC2420_RXFIFO    0xE7
#define CC2420_TXFIFO    0xE8
#define CC2420_INIT      0xEE
#define CC2420_EXT       0xEF
#endif