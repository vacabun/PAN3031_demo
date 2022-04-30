/*******************************************************************************
 * @note Copyright (C) 2020 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.h
 * @brief
 *
 * @history - V3.0, 2021-07-12
*******************************************************************************/
#ifndef __RADIO_H_
#define __RADIO_H_
#include "stdint.h"
#include "pan3031.h" 

#define DEFAULT_FREQ           (433000000)
#define DEFAULT_SF             SF_9
#define DEFAULT_BW             BW_125K
#define DEFAULT_CR             CODE_RATE_48

#define RADIO_FLAG_IDLE         0
#define RADIO_FLAG_TXDONE       1
#define RADIO_FLAG_RXDONE       2
#define RADIO_FLAG_RXTIMEOUT    3
#define RADIO_FLAG_RXERR        4
#define RADIO_FLAG_PLHDRXDONE   5

struct RxDoneMsg
{
	uint8_t *Payload;
	uint8_t *PlhdPayload;
	uint16_t PlhdSize;
	uint16_t Size;
	double Rssi;
	double Snr;
};

typedef enum{
	RF_PARA_TYPE_FREQ,
	RF_PARA_TYPE_CR,
	RF_PARA_TYPE_BW,
	RF_PARA_TYPE_SF,
	RF_PARA_TYPE_TXPOWER,
	RF_PARA_TYPE_CRC,
}rf_para_type_t;

uint32_t rf_get_recv_flag(void);
void rf_set_recv_flag(int status);
uint32_t rf_get_transmit_flag(void);
void rf_set_transmit_flag(int status);
uint32_t rf_init(void);
uint32_t rf_deepsleep_wakeup(void);
uint32_t rf_deepsleep(void);
uint32_t rf_sleep_wakeup(void);
uint32_t rf_sleep(void);

uint32_t rf_get_tx_time(void);
uint32_t rf_set_mode(uint8_t mode);
uint8_t rf_get_mode(void);
uint32_t rf_set_tx_mode(uint8_t mode);
uint32_t rf_set_rx_mode(uint8_t mode);
uint32_t rf_set_rx_single_timeout(uint32_t timeout);
float rf_get_snr(void);
float rf_get_rssi(void);
uint32_t rf_set_preamble(uint16_t pream);
uint32_t rf_set_cad(void);
uint32_t rf_set_syncword(uint8_t sync);
uint8_t rf_get_syncword(void);
void rf_irq_handler(void);
void rf_set_plhd_rx_on(uint8_t addr,uint8_t len);
void rf_set_plhd_rx_off(void);
uint32_t rf_receive(uint8_t *buf);
uint32_t rf_plhd_receive(uint8_t *buf,uint8_t len);

void rf_rx_plhddone_event( uint8_t *payload, uint16_t size );
void rf_rx_done_event( uint8_t *payload, uint16_t size, double rssi, double snr );
void rf_rx_err_event(void);
void rf_rx_timeout_event(void);
void rf_tx_done_event(void);
uint32_t rf_enter_continous_rx(void);
uint32_t rf_enter_single_timeout_rx(uint32_t timeout);
uint32_t rf_enter_single_rx(void);
uint32_t rf_single_tx_data(uint8_t *buf, uint8_t size, uint32_t *tx_time);
uint32_t rf_enter_continous_tx(void);
uint32_t rf_continous_tx_send_data(uint8_t *buf, uint8_t size);

uint32_t rf_set_agc(uint32_t state);
uint32_t rf_set_para(rf_para_type_t para_type, uint32_t para_val);
uint32_t rf_get_para(rf_para_type_t para_type, uint32_t *para_val);
void rf_set_default_para(void);

uint32_t rf_set_dcdc_mode(uint32_t dcdc_val);
uint32_t rf_set_ldr(uint32_t mode);
#endif

