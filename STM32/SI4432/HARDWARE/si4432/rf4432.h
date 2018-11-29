#ifndef _RF4432_H
#define _RF4432_H
#include "sys.h"

#define RF4432_SDN PCout(0)
#define RF4432_IRQ PCin(1)
#define RF4432_SEL PCout(2)

#define RF4432_TxBuf_Len 32
#define RF4432_RxBuf_Len 32

extern unsigned char RF4432_RxBuf[RF4432_RxBuf_Len];
extern char RF4432_TxBuf[RF4432_TxBuf_Len];

#define DEVICE_TYPE                                0x00
#define DEVICE_VERSION                             0x01
#define DEVICE_STATUS                              0x02
#define INTERRUPT_STATUS_1                         0x03
#define INTERRUPT_STATUS_2                         0x04
#define INTERRUPT_ENABLE_1                         0x05
#define INTERRUPT_ENABLE_2                         0x06
#define OPERATING_FUNCTION_CONTROL_1               0x07
#define OPERATING_FUNCTION_CONTROL_2               0x08
#define CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE        0x09
#define MICROCONTROLLER_OUTPUT_CLOCK               0x0A
#define GPIO0_CONFIGURATION                        0x0B
#define GPIO1_CONFIGURATION                        0x0C
#define GPIO2_CONFIGURATION                        0x0D
#define IO_PORT_CONFIGURATION                      0x0E
#define ADC_CONFIGURATION                          0x0F
#define ADC_SENSOR_AMPLIFIER_OFFSET                0x10
#define ADC_VALUE                                  0x11
#define TEMPERATURE_SENSOR_CONTROL                 0x12
#define TEMPERATURE_VALUE_OFFSET                   0x13
#define WAKE_UP_TIMER_PERIOD_1                     0x14
#define WAKE_UP_TIMER_PERIOD_2                     0x15
#define WAKE_UP_TIMER_PERIOD_3                     0x16
#define WAKE_UP_TIMER_VALUE_1                      0x17
#define WAKE_UP_TIMER_VALUE_2                      0x18
#define LOW_DUTY_CYCLE_MODE_DURATION               0x19
#define LOW_BATTERY_DETECTOR_THRESHOLD             0x1A
#define BATTERY_VOLTAGE_LEVEL                      0x1B
#define IF_FILTER_BANDWIDTH                        0x1C
#define AFC_LOOP_GEARSHIFT_OVERRIDE                0x1D
#define AFC_TIMING_CONTROL                         0x1E
#define CLOCK_RECOVERY_GEARSHIFT_OVERRIDE          0x1F
#define CLOCK_RECOVERY_OVERSAMPLING_RATIO          0x20
#define CLOCK_RECOVERY_OFFSET_2                    0x21
#define CLOCK_RECOVERY_OFFSET_1                    0x22
#define CLOCK_RECOVERY_OFFSET_0                    0x23
#define CLOCK_RECOVERY_TIMING_LOOP_GAIN_1          0x24
#define CLOCK_RECOVERY_TIMING_LOOP_GAIN_0          0x25
#define RECEIVED_SIGNAL_STRENGTH_INDICATOR         0x26
#define RSSI_THRESHOLD_FOR_CLEAR_CHANNEL_INDICATOR 0x27
#define ANTENNA_DIVERSITY_REGISTER_1               0x28
#define ANTENNA_DIVERSITY_REGISTER_2               0x29
#define DATA_ACCESS_CONTROL                        0x30
#define EZMAC_STATUS                               0x31
#define HEADER_CONTROL_1                           0x32
#define HEADER_CONTROL_2                           0x33
#define PREAMBLE_LENGTH                            0x34
#define PREAMBLE_DETECTION_CONTROL                 0x35
#define SYNC_WORD_3                                0x36
#define SYNC_WORD_2                                0x37
#define SYNC_WORD_1                                0x38
#define SYNC_WORD_0                                0x39
#define TRANSMIT_HEADER_3                          0x3A
#define TRANSMIT_HEADER_2                          0x3B
#define TRANSMIT_HEADER_1                          0x3C
#define TRANSMIT_HEADER_0                          0x3D
#define TRANSMIT_PACKET_LENGTH                     0x3E
#define CHECK_HEADER_3                             0x3F
#define CHECK_HEADER_2                             0x40
#define CHECK_HEADER_1                             0x41
#define CHECK_HEADER_0                             0x42
#define HEADER_ENABLE_3                            0x43
#define HEADER_ENABLE_2                            0x44
#define HEADER_ENABLE_1                            0x45
#define HEADER_ENABLE_0                            0x46
#define RECEIVED_HEADER_3                          0x47
#define RECEIVED_HEADER_2                          0x48
#define RECEIVED_HEADER_1                          0x49
#define RECEIVED_HEADER_0                          0x4A
#define RECEIVED_PACKET_LENGTH                     0x4B
#define ANALOG_TEST_BUS                            0x50
#define DIGITAL_TEST_BUS                           0x51
#define TX_RAMP_CONTROL                            0x52
#define PLL_TUNE_TIME                              0x53
#define CALIBRATION_CONTROL                        0x55
#define MODEM_TEST                                 0x56
#define CHARGEPUMP_TEST                            0x57
#define CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE       0x58
#define DIVIDER_CURRENT_TRIMMING                   0x59
#define VCO_CURRENT_TRIMMING                       0x5A
#define VCO_CALIBRATION_OVERRIDE                   0x5B
#define SYNTHESIZER_TEST                           0x5C
#define BLOCK_ENABLE_OVERRIDE_1                    0x5D
#define BLOCK_ENABLE_OVERRIDE_2                    0x5E
#define BLOCK_ENABLE_OVERRIDE_3                    0x5F
#define CHANNEL_FILTER_COEFFICIENT_ADDRESS         0x60
#define CHANNEL_FILTER_COEFFICIENT_VALUE           0x61
#define CRYSTAL_OSCILLATOR_CONTROL_TEST            0x62
#define RC_OSCILLATOR_COARSE_CALIBRATION_OVERRIDE  0x63
#define RC_OSCILLATOR_FINE_CALIBRATION_OVERRIDE    0x64
#define LDO_CONTROL_OVERRIDE_ENSPOR                0x65
#define LDO_LEVEL_SETTING                          0x66
#define DELTASIGMA_ADC_TUNING_1                    0x67
#define DELTASIGMA_ADC_TUNING_2                    0x68
#define AGC_OVERRIDE_1                             0x69
#define AGC_OVERRIDE_2                             0x6A
#define GFSK_FIR_FILTER_COEFFICIENT_ADDRESS        0x6B
#define GFSK_FIR_FILTER_COEFFICIENT_VALUE          0x6C
#define TX_POWER                                   0x6D
#define TX_DATA_RATE_1                             0x6E
#define TX_DATA_RATE_0                             0x6F
#define MODULATION_MODE_CONTROL_1                  0x70
#define MODULATION_MODE_CONTROL_2                  0x71
#define FREQUENCY_DEVIATION                        0x72
#define FREQUENCY_OFFSET_1                         0x73
#define FREQUENCY_OFFSET_2                         0x74
#define FREQUENCY_BAND_SELECT                      0x75
#define NOMINAL_CARRIER_FREQUENCY_1                0x76
#define NOMINAL_CARRIER_FREQUENCY_0                0x77
#define FREQUENCY_HOPPING_CHANNEL_SELECT           0x79
#define FREQUENCY_HOPPING_STEP_SIZE                0x7A
#define TX_FIFO_CONTROL_1                          0x7C
#define TX_FIFO_CONTROL_2                          0x7D
#define RX_FIFO_CONTROL                            0x7E
#define FIFO_ACCESS                                0x7F

void RF4432_IOInit ( void );
void RF4432_Init ( void );
void RF4432_SetRxMode ( void );
void RF4432_TxPacket ( void );
unsigned char RF4432_RxPacket ( void );
void RF4432_ReadBurestReg ( unsigned char  addr, unsigned char  *p, unsigned char  count );
#endif