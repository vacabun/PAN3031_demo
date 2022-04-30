/*******************************************************************************
 * @note Copyright (C) 2020 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3031.h
 * @brief
 *
 * @history - V3.0, 2021-07-12
*******************************************************************************/
#ifndef __PAN_3031_H
#define __PAN_3031_H

#include "stdio.h"
#include "pan3031_port.h"
 		

/* result */
#define OK                              0
#define FAIL                            1
/* 3031B mode define*/
#define PAN3031_MODE_DEEP_SLEEP         0
#define PAN3031_MODE_SLEEP              1
#define PAN3031_MODE_STB1               2
#define PAN3031_MODE_STB2               3
#define PAN3031_MODE_STB3               4
#define PAN3031_MODE_TX                 5
#define PAN3031_MODE_RX                 6
/* 3031B Tx mode */
#define PAN3031_TX_SINGLE               0
#define PAN3031_TX_CONTINOUS            1
/* 3031B Rx mode */
#define PAN3031_RX_SINGLE               0
#define PAN3031_RX_SINGLE_TIMEOUT       1
#define PAN3031_RX_CONTINOUS            2

/* System control register */
#define REG_SYS_CTL                     0x00
#define REG_FIFO_ACC_ADDR               0x01
/* 3V Logical area register */
#define REG_OP_MODE                     0X02

#define MODEM_MODE_NORMAL               0x01
#define MODEM_MODE_MULTI_SECTOR         0x02

#define freq_336000000                  (336000000)
#define freq_470000000                  (470000000)
#define freq_510000000                  (510000000)
#define freq_800000000                  (800000000)
#define freq_920000000                  (920000000)

#define CODE_RATE_45                     0x01
#define CODE_RATE_46                     0x02
#define CODE_RATE_47                     0x03
#define CODE_RATE_48                     0x04

//PAN3031 only support SF7-SF9
#define SF_7                            7
#define SF_8                            8
#define SF_9                            9

    
//PAN3031 only support BW125K\BW250K\500K
#define BW_125K                         7
#define BW_250K                         8
#define BW_500K                         9
    
#define CRC_OFF                         0
#define CRC_ON                          1
    
#define PLHD_IRQ_OFF                    0
#define PLHD_IRQ_ON                     1

#define PLHD_OFF                        0
#define PLHD_ON                         1

#define PLHD_LEN8                       0
#define PLHD_LEN16                      1

#define AGC_ON                          1
#define AGC_OFF                         0
	
#define LO_400M                         0
#define LO_800M                         1

#define DCDC_OFF                    	0
#define DCDC_ON                     	1

#define LDR_OFF                    		0
#define LDR_ON                     		1

#define REG_PAYLOAD_LEN                 0x0C

/*IRQ BIT MASK*/
#define REG_IRQ_RX_PLHD_DONE            0x10
#define REG_IRQ_RX_DONE                 0x8
#define REG_IRQ_CRC_ERR                 0x4
#define REG_IRQ_RX_TIMEOUT              0x2
#define REG_IRQ_TX_DONE                 0x1

enum REF_CLK_SEL {REF_CLK_32M,REF_CLK_16M};		
enum PAGE_SEL {PAGE0_SEL,PAGE1_SEL,PAGE2_SEL, PAGE3_SEL};

uint32_t PAN3031_rst(void);
uint32_t PAN3031_agc_enable(uint32_t state);
uint32_t PAN3031_agc_config(void);
uint32_t PAN3031_init(void);
uint32_t PAN3031_deepsleep_wakeup(void);
uint32_t PAN3031_deepsleep(void);
uint32_t PAN3031_sleep_wakeup(void);
uint32_t PAN3031_sleep(void);

uint32_t PAN3031_set_freq(uint32_t freq);
uint32_t PAN3031_read_freq(void);
uint32_t PAN3031_calculate_tx_time(void);
uint32_t PAN3031_set_bw(uint32_t bw_val);
uint8_t PAN3031_get_bw(void);
uint32_t PAN3031_set_sf(uint32_t sf_val);
uint8_t PAN3031_get_sf(void);
uint32_t PAN3031_set_crc(uint32_t crc_val);
uint8_t PAN3031_get_crc(void);
uint8_t PAN3031_get_code_rate(void);
uint32_t PAN3031_set_code_rate(uint8_t code_rate);

uint32_t PAN3031_set_mode(uint8_t mode);
uint8_t PAN3031_get_mode(void);
uint32_t PAN3031_set_tx_mode(uint8_t mode);
uint32_t PAN3031_set_rx_mode(uint8_t mode);
uint32_t PAN3031_set_timeout(uint32_t timeout);
float PAN3031_get_snr(void);
float PAN3031_get_rssi(void);
uint32_t PAN3031_set_tx_power(uint8_t tx_power);
uint32_t PAN3031_get_tx_power(void);
uint32_t PAN3031_set_preamble(uint16_t reg);
uint32_t PAN3031_set_gpio_input(uint8_t gpio_pin);
uint32_t PAN3031_set_gpio_output(uint8_t gpio_pin);
uint32_t PAN3031_set_gpio_state(uint8_t gpio_pin, uint8_t state);

uint32_t PAN3031_cad_en(void);
uint32_t PAN3031_set_syncword(uint32_t sync);
uint8_t PAN3031_get_syncword(void);
uint32_t PAN3031_send_packet(uint8_t *buff, uint32_t len);
uint8_t PAN3031_recv_packet(uint8_t *buff);
uint32_t PAN3031_set_early_irq(uint32_t earlyirq_val);
uint8_t PAN3031_get_early_irq(void);
uint32_t PAN3031_set_plhd(uint8_t addr,uint8_t len);
uint8_t PAN3031_get_plhd(void);
uint32_t PAN3031_set_plhd_mask(uint32_t plhd_val);
uint8_t PAN3031_get_plhd_mask(void);
uint8_t PAN3031_recv_plhd8(uint8_t *buff);
uint8_t PAN3031_recv_plhd16(uint8_t *buff);
uint32_t PAN3031_plhd_receive(uint8_t *buf,uint8_t len);
uint32_t PAN3031_set_dcdc_mode(uint32_t dcdc_val);
uint32_t PAN3031_set_ldr(uint32_t mode);
void PAN3031_irq_handler(void);
uint32_t PAN3031_set_carrier_wave_test_mode(void);
#endif
